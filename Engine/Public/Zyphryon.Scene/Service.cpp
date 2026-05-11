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
#include <SDL3/SDL_cpuinfo.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : AbstractService { Host },
          mMultiplier     { 1.0f }
    {
        mWorld.set_threads(SDL_GetNumLogicalCPUCores());

        // Ensures that handles within this range are exclusively for entities created during runtime,
        // preventing conflicts with internal engine objects like components or archetypes.
        mWorld.range_set(mWorld.range_new(kMinRangeEntities, kMaxRangeEntities));

        // Register engine’s built-in components and systems.
        RegisterDefaultComponentsAndSystems();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Time Time)
    {
        ZYPHRYON_PROFILE_SCOPE("Scene::Tick");

        // Scale the frame delta by the time multiplier to allow for time dilation effects.
        const Real64 Delta = Time.GetDelta() * mMultiplier;
        mTime = Base::Time(mTime.GetAbsolute() + Delta, Delta);

        // Update the world time component.
        mWorld.set<Base::Time>(mTime);

        // Advance the ECS world simulation by the frame delta.
        mWorld.progress(static_cast<Real32>(mTime.GetDelta()));

        // TODO: Optimize (ecs_delete_empty_tables)
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

    Entity Service::LoadHierarchy(Ref<Reader> Archive)
    {
        const Entity Actor = CreateEntity();
        Actor.Load(Archive);

        // Read entity's hierarchy.
        for (Reader Hierarchy(Archive.ReadBlock<UInt32, Byte>()); Hierarchy.GetAvailable() > 0;)
        {
            const Entity Children = LoadHierarchy(Hierarchy);
            Children.SetParent(Actor);
        }

        return Actor;
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

        // Writes the entity's hierarchy.
        Archive.WriteBlock<UInt16>([this, Actor](Ref<Writer> Output)
        {
            Actor.Children([&](Entity Children)
            {
                SaveHierarchy(Output, Children);
            });
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::LoadArchetypes(Ref<Reader> Archive)
    {
        const UInt32 Size = Archive.ReadUInt32();

        for (UInt32 Element = 1, Limit = Size; Element <= Limit; ++Element)
        {
            const UInt32 ID = Archive.ReadInt<UInt32>();
            mArchetypes.Acquire(ID);

            Entity Archetype = Allocate<true>(kMinRangeArchetypes + ID);
            Archetype.Add(EcsPrefab);

            Archetype.Load(Archive);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SaveArchetypes(Ref<Writer> Archive)
    {
        Archive.WriteUInt32(mArchetypes.GetSize());

        for (UInt32 Element = 1, Limit = mArchetypes.GetHead(); Element <= Limit; ++Element)
        {
            if (Entity Archetype = GetEntity(kMinRangeArchetypes + Element); Archetype.IsValid())
            {
                Archive.WriteInt(Archetype.GetID() - kMinRangeArchetypes);

                Archetype.Save(Archive);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RegisterDefaultComponentsAndSystems()
    {
#ifdef   FLECS_REST_SERVICE
        // Import built-in ECS statistics module.
        mWorld.import<flecs::stats>();

        // Enable REST service for remote ECS inspection.
        mWorld.emplace<flecs::Rest>();
#endif // FLECS_REST_SERVICE

        // Frees the archetype handle associated with the prefab to keep archetype tracking consistent.
        CreateObserver("_Archetypes::OnRemove", flecs::OnRemove, [this](Entity Actor)
        {
            mArchetypes.Free(Actor.GetID() - kMinRangeArchetypes);
        }, DSL::In(flecs::Prefab));

        // Register Factory component (serialization).
        GetComponent<Factory>("Factory").Grant(Trait::Final);

        // Register Time component as singleton (tracks global time state).
        GetComponent<Time>("Time").Grant(Trait::Final, Trait::Singleton);

        // Register Transient component (marks entities as non serializable).
        GetComponent<Transient>("Transient").Grant(Trait::Associative);
    }
}