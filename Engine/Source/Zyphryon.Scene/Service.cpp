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
        : Subsystem { Host },
          mWorld    { ecs_init() }
    {
        Context::Attach(mWorld);

        // Flecs runs stage 0 on the calling thread, so the count is one greater than the lane's worker count.
        ecs_set_threads(mWorld, Host.GetService<Job::Service>()->GetConcurrency(Job::Lane::Compute) + 1);

        // Ensures that handles within this range are exclusively for entities created during runtime,
        // preventing conflicts with internal engine objects like components or archetypes.
        ecs_entity_range_set(mWorld, ecs_entity_range_new(mWorld, kMinRangeEntities, kMaxRangeEntities));

        // Register engine’s built-in components and systems.
        RegisterDefaultComponentsAndSystems();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::~Service()
    {
        ecs_fini(mWorld);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Real64 Delta)
    {
        ZY_PROFILE_SCOPE("Scene::Tick");

        // Scale the frame delta by the time multiplier to allow for time dilation effects.
        mClock.Tick(Delta);

        // Update the world time component.
        GetWorld().Set(Clock(mClock));

        // Advance the ECS world simulation by the frame delta.
        ecs_progress(mWorld, static_cast<Real32>(mClock.GetDelta()));
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
        Defer([&]
        {
            Actor.Load(Archive);
        });

        // Reads the entity's hierarchy, over the parts its archetype already gave it where a record names one.
        const Entity          Archetype = Actor.GetArchetype();
        const ConstSpan<Byte> Data      = Archive.ReadBlock<UInt32, Byte>();

        for (Reader Hierarchy(Data.GetData(), Data.GetSize()); Hierarchy.GetAvailable() > 0;)
        {
            // A record naming a part of this entity's own archetype is laid over the part the archetype has
            // already given it, so what was changed on a part survives; anything else is a child of its own.
            Entity Part;

            if (Archetype.IsValid())
            {
                Reader Peek(Data.GetData() + Hierarchy.GetOffset(), Hierarchy.GetAvailable());
                Peek.ReadText();
                Peek.ReadText();

                if (const Entity Source(mWorld, Peek.Read<UInt64>()); Source.IsAlive() && Source.GetParent() == Archetype)
                {
                    Actor.Children([&](Entity Child)
                    {
                        if (Child.GetArchetype() == Source)
                        {
                            Part = Child;
                        }
                    });
                }
            }

            if (Part.IsValid())
            {
                LoadHierarchy(Hierarchy, Part);
            }
            else
            {
                const Entity Children = LoadHierarchy(Hierarchy);
                Children.Attach(Actor, Scene::Hierarchy::Open);
            }
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

        const auto SaveChildren = [this, Actor](Ref<Writer> Output)
        {
            Actor.Children([&](Entity Children)
            {
                SaveHierarchy(Output, Children);
            });
        };

        const Entity Source = Actor.GetArchetype();
        const Entity Parent = Actor.GetParent();

        if (Source.IsValid())
        {
            const Bool Part = Parent.IsValid() && Parent.GetArchetype().IsValid() && Source.GetParent() == Parent.GetArchetype();

            Writer Record;

            Bool Changed = Actor.Save(Record);

            Record.WriteBlock<UInt32>([&](Ref<Writer> Output)
            {
                const UInt32 Before = Output.GetSize();
                SaveChildren(Output);
                Changed |= (Output.GetSize() != Before);
            });

            if (Changed || !Part)
            {
                Archive.Write<Byte>(Record.GetData(), Record.GetSize());
            }
            return;
        }

        // Writes the entity's data.
        Actor.Save(Archive);

        // Writes the entity's hierarchy.
        Archive.WriteBlock<UInt32>(SaveChildren);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::LoadArchetypes(Ref<Reader> Archive)
    {
        const UInt32 Size = Archive.Read<UInt32>();

        struct Adoption
        {
            Entity Source;
            UInt64 Parent;
        };
        Sequence<Adoption> Pending(Size);

        for (UInt32 Element = 1, Limit = Size; Element <= Limit; ++Element)
        {
            const UInt64 ID     = Archive.Read<UInt64>();
            const UInt64 Parent = Archive.Read<UInt64>();

            mArchetypes.Acquire(static_cast<UInt32>(ID) - kMinRangeArchetypes);

            Entity Archetype = Allocate<true>(ID);

            Defer([&]
            {
                Archetype.Add(EcsPrefab);
                Archetype.Load(Archive);
            });

            if (Parent)
            {
                Pending.Append(Adoption(Archetype, Parent));
            }
        }

        for (ConstRef<Adoption> Entry : Pending)
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
            if (const Entity Archetype(mWorld, ecs_get_alive(mWorld, kMinRangeArchetypes + Element)); Archetype.IsValid())
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
        ECS_IMPORT(mWorld, FlecsStats);

        // Enable REST service for remote ECS inspection.
        const EcsRest Rest { };
        ecs_set_id(mWorld, ecs_id(EcsRest), ecs_id(EcsRest), sizeof(EcsRest), AddressOf(Rest));

#endif

        // Frees the archetype handle associated with the prefab to keep archetype tracking consistent.
        CreateObserver("_Archetypes::OnRemove", EcsOnRemove, [this](Entity Actor)
        {
            mArchetypes.Free(Actor.GetID() - kMinRangeArchetypes);
        }, DSL::In(EcsPrefab));

        // Register the built-in components in one pass, so a trait may name any of the others.
        Register(
            // Clock tracks global time state, of which the world holds a single instance.
            DSL::Declare<Clock>("Clock", DSL::Final, DSL::Singleton),

            // Deprecated marks archetypes for a later purge, and never propagates to spawned instances.
            DSL::Declare<Deprecated>("Deprecated", DSL::Local),

            // Orphaned marks an entity whose archetype no longer resolves.
            DSL::Declare<Orphaned>("Orphaned", DSL::Local),

            // Transient marks entities that are left out of serialization.
            DSL::Declare<Transient>("Transient", DSL::Associative));

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
