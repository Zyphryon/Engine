// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Service.hpp"
#include "Codec.hpp"
#include "Zyphryon.Job/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host }
    {
        mWorld.set_threads(Job::Service::GetConcurrency());

        // Ensures that handles within this range are exclusively for entities created during runtime,
        // preventing conflicts with internal engine objects like components or archetypes.
        mWorld.range_set(mWorld.range_new(kMinRangeEntities, kMaxRangeEntities));

        // Register engine’s built-in components and systems.
        RegisterDefaultComponentsAndSystems();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Real64 Delta)
    {
        ZY_PROFILE_SCOPE("Scene::Tick");

        // Scale the frame delta by the time multiplier to allow for time dilation effects.
        mClock.Tick(Delta);

        // Update the world time component.
        mWorld.set<Clock>(mClock);

        // Advance the ECS world simulation by the frame delta.
        mWorld.progress(static_cast<Real32>(mClock.GetDelta()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::LoadWorld(Ref<Reader> Archive)
    {
        Codec::ReadComponentsOf(mWorld, Archive, World(mWorld));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SaveWorld(Ref<Writer> Archive)
    {
        Codec::WriteComponentsOf(Archive, World(mWorld));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::LoadHierarchy(Ref<Reader> Archive, Entity Actor)
    {
        // Reads the entity's data.
        Actor.Load(Archive);

        // Reads the entity's hierarchy.
        const ConstSpan<Byte> Data = Archive.ReadBlock<UInt32, Byte>();

        for (Reader Hierarchy(Data.GetData(), Data.GetSize()); Hierarchy.GetAvailable() > 0;)
        {
            const Entity Children = LoadHierarchy(Hierarchy);
            Children.Attach(Actor, Scene::Hierarchy::Open);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SaveHierarchy(Ref<Writer> Archive, Entity Actor)
    {
        if (Actor.Has<Transient>())
        {
            return;
        }

        // Writes the entity's data.
        Actor.Save(Archive);

        // The archetype this entity was instantiated from, if any.
        const Entity Archetype = Actor.GetArchetype();

        // Writes the entity's hierarchy.
        Archive.WriteBlock<UInt32>([this, Actor, Archetype](Ref<Writer> Output)
        {
            Actor.Children([&](Entity Children)
            {
                const Entity Source = Children.GetArchetype();

                if (Archetype.IsValid() && Source.IsValid() && Source.GetParent() == Archetype)
                {
                    return;
                }
                SaveHierarchy(Output, Children);
            });
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::LoadArchetypes(Ref<Reader> Archive)
    {
        const UInt32 Size = Archive.Read<UInt32>();

        struct Defer
        {
            Entity Source;
            UInt64 Parent;
        };
        Sequence<Defer> Pending(Size);

        for (UInt32 Element = 1, Limit = Size; Element <= Limit; ++Element)
        {
            const UInt64 ID     = Archive.Read<UInt64>();
            const UInt64 Parent = Archive.Read<UInt64>();

            mArchetypes.Acquire(static_cast<UInt32>(ID) - kMinRangeArchetypes);

            Entity Archetype = Allocate<true>(ID);
            Archetype.Add(EcsPrefab);
            Archetype.Load(Archive);

            if (Parent)
            {
                Pending.Append(Defer(Archetype, Parent));
            }
        }

        for (ConstRef<Defer> Entry : Pending)
        {
            Entry.Source.Attach(GetEntity(Entry.Parent), Hierarchy::Fixed);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SaveArchetypes(Ref<Writer> Archive)
    {
        Archive.Write<UInt32>(mArchetypes.GetCount());

        for (UInt32 Element = 1, Limit = mArchetypes.GetTop(); Element <= Limit; ++Element)
        {
            // Resolve the live handle so `GetID` carries the current generation, then persist the full id.
            if (const Entity Archetype(mWorld.get_alive(kMinRangeArchetypes + Element)); Archetype.IsValid())
            {
                Archive.Write<UInt64>(Archetype.GetID());

                const Entity Parent = Archetype.GetParent();
                Archive.Write<UInt64>(Parent.IsValid() ? Parent.GetID() : 0);

                Archetype.Save(Archive);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RegisterDefaultComponentsAndSystems()
    {
#if defined(FLECS_REST_SERVICE)

        // Import built-in ECS statistics module.
        mWorld.import<flecs::stats>();

        // Enable REST service for remote ECS inspection.
        mWorld.emplace<flecs::Rest>();

#endif

        // Frees the archetype handle associated with the prefab to keep archetype tracking consistent.
        CreateObserver("_Archetypes::OnRemove", flecs::OnRemove, [this](Entity Actor)
        {
            mArchetypes.Free(Actor.GetID() - kMinRangeArchetypes);
        }, DSL::In(flecs::Prefab));

        // Register Factory component (serialization).
        GetComponent<Factory>("Factory").Grant(Trait::Final);

        // Register Time component as singleton (tracks global time state).
        GetComponent<Clock>("Clock").Grant(Trait::Final, Trait::Singleton);

        // Register Transient component (marks entities as non serializable).
        GetComponent<Transient>("Transient").Grant(Trait::Associative);

        // Register Deprecated tag (marks archetypes for later purge; never propagates to spawned instances).
        GetComponent<Deprecated>("Deprecated").GetHandle().add(flecs::OnInstantiate, flecs::DontInherit);

        // Periodically reclaims memory by removing empty internal storage tables.
        CreateSystem<DSL::Interval<15>>("_Compact", EcsPostFrame, Execution::Immediate,
            [this]
            {
                Compact();
            });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Compact()
    {
        ecs_delete_empty_tables_desc_t Description { };
        Description.clear_generation    = 1;
        Description.delete_generation   = 1;
        Description.time_budget_seconds = 0.004f;

        ecs_delete_empty_tables(mWorld, AddressOf(Description));
    }
}