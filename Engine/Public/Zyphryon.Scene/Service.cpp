// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Service.hpp"
#include <SDL3/SDL_cpuinfo.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : AbstractService { Host }
    {
        mWorld.set_threads(SDL_GetNumLogicalCPUCores());

        // Ensures that handles within this range are exclusively for entities created during runtime,
        // preventing conflicts with internal engine objects like components or archetypes.
        mWorld.set_entity_range(kMinRangeEntities, kMaxRangeEntities);

        // Register engine’s built-in components and systems.
        RegisterDefaultComponentsAndSystems();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(ConstRef<Time> Time)
    {
        ZYPHRYON_PROFILE;

        /// Update the world time component.
        mWorld.set<Base::Time>(Time);

        /// Advance the ECS world simulation by the frame delta.
        mWorld.progress(Time.GetDelta());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::LoadArchetypes(Ref<Reader> Reader)
    {
        mArchetypes.OnSerialize(Archive(Reader));

        for (UInt32 Element = 1, Limit = mArchetypes.GetSize(); Element <= Limit; ++Element)
        {
            /// Allocate prefab entity with the serialized identifier.
            Entity Archetype = Allocate<true>(kMinRangeArchetypes + Reader.ReadInt<UInt32>());

            /// Read and set archetype name.
            if (ConstStr8 Name = Reader.ReadText(); !Name.empty())
            {
                Archetype.SetName(Name);
            }

            /// Read and set archetype display name.
            if (ConstStr8 Name = Reader.ReadText(); !Name.empty())
            {
                Archetype.SetDisplayName(Name);
            }

            /// Read and set base archetype reference if present.
            if (const UInt32 Base = Reader.ReadInt<UInt32>(); Base)
            {
                Archetype.SetArchetype(GetEntity(kMinRangeArchetypes + Base));
            }

            /// Load all serialized components for this archetype.
            LoadComponents(Reader, Archetype);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SaveArchetypes(Ref<Writer> Writer)
    {
        mArchetypes.OnSerialize(Archive(Writer));

        for (UInt32 Element = 1, Limit = mArchetypes.GetHead(); Element <= Limit; ++Element)
        {
            if (Entity Archetype = GetEntity(kMinRangeArchetypes + Element); Archetype.IsValid())
            {
                /// Write archetype identifier.
                Writer.WriteInt(Archetype.GetID() - kMinRangeArchetypes);

                /// Write archetype name.
                Writer.WriteText(Archetype.GetName());

                /// Write archetype display name.
                Writer.WriteText(Archetype.GetDisplayName());

                /// Write base archetype reference or `0` if none.
                const Entity Base = Archetype.GetArchetype();
                Writer.WriteInt(Base.IsValid() ? Base.GetID() - kMinRangeArchetypes : 0);

                /// Write all components of the archetype.
                SaveComponents(Writer, Archetype);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Service::LoadEntity(Ref<Reader> Reader)
    {
        Entity Actor = Allocate<false>();

        /// Read and assign entity name if present.
        if (const ConstStr8 Name = Reader.ReadText(); !Name.empty())
        {
            Actor.SetName(Name);
        }

        /// Read and assign entity display name if present.
        if (const ConstStr8 Name = Reader.ReadText(); !Name.empty())
        {
            Actor.SetDisplayName(Name);
        }

        /// Read and assign archetype reference if present.
        if (const UInt64 Base = Reader.ReadInt<UInt64>(); Base)
        {
            Actor.SetArchetype(GetEntity(kMinRangeArchetypes + Base));
        }

        /// Load all serialized components for the entity.
        LoadComponents(Reader, Actor);
        return Actor;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Service::LoadEntityHierarchy(Ref<Reader> Reader)
    {
        /// Read and construct the root entity.
        const Entity Actor = LoadEntity(Reader);

        /// Recursively read and attach all child entities.
        while (Reader.Peek<UInt32>() != -1)
        {
            Entity Children = LoadEntityHierarchy(Reader);
            Children.SetParent(Actor);
        }

        /// Skip delimiter marking the end of this hierarchy.
        Reader.Skip(sizeof(UInt32));
        return Actor;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SaveEntity(Ref<Writer> Writer, Entity Actor)
    {
        if (Actor.Has<EcsTransient>())
        {
            return;
        }

        /// Write entity name and display name.
        Writer.WriteText(Actor.GetName());
        Writer.WriteText(Actor.GetDisplayName());

        /// Write archetype reference or `0` if none.
        const Entity Archetype = Actor.GetArchetype();
        Writer.WriteInt<UInt64>(Archetype.IsValid() ? Archetype.GetID() - kMinRangeArchetypes : 0);

        /// Write all components of the entity.
        SaveComponents(Writer, Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SaveEntityHierarchy(Ref<Writer> Writer, Entity Actor)
    {
        if (Actor.Has<EcsTransient>())
        {
            return;
        }

        /// Write the entity and its components.
        SaveEntity(Writer, Actor);

        /// Recursively write all child entities.
        Actor.Children([&](Entity Children)
        {
            SaveEntityHierarchy(Writer, Children);
        });

        /// Write delimiter marking the end of the hierarchy.
        Writer.WriteUInt32(-1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::LoadComponents(Ref<Reader> Reader, Entity Actor)
    {
        while (Reader.Peek<UInt32>() != -1)
        {
            /// Read first element of the pair (tag/relationship); empty means single component.
            const Str8   Pair(Reader.ReadText()); // TODO: Remove heap allocation (Flecs Limitation)
            const Entity First = Pair.empty() ? Entity() : mWorld.component(Pair.c_str());

            /// Read component name and resolve the component entity.
            const Str8   Name(Reader.ReadText()); // TODO: Remove heap allocation (Flecs Limitation)
            const Entity Second = mWorld.component(Name.c_str());

            /// Read serialized component payload.
            const ConstSpan<Byte> Bundle = Reader.ReadBlock<Byte>();

            /// Apply payload if present; otherwise attach component without data.
            if (Base::Reader Data(Bundle); Data.GetAvailable() > 0)
            {
                if (const ConstPtr<Factory> Serializer = Second.Get<const Factory>())
                {
                    if (First.IsValid())
                    {
                        if (const Ptr<void> Memory = Actor.Ensure(First, Second))
                        {
                            Serializer->Read(Data, Memory);
                        }
                        Actor.Notify(First, Second);
                    }
                    else
                    {
                        if (const Ptr<void> Memory = Actor.Ensure(Second))
                        {
                            Serializer->Read(Data, Memory);
                        }
                        Actor.Notify(Second);
                    }
                }
                else
                {
                    LOG_WARNING("World: Trying to load an invalid component '{}'", Second.GetName());
                }
            }
            else
            {
                if (First.IsValid())
                {
                    Actor.Add(First, Second);
                }
                else
                {
                    Actor.Add(Second);
                }
            }
        }

        /// Skip delimiter marking the end of the component list.
        Reader.Skip(sizeof(UInt32));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SaveComponents(Ref<Writer> Writer, Entity Actor)
    {
        /// Iterate over each component attached to the entity.
        const auto OnIterateEntityComponents = [&](Entity Component)
        {
            Entity First;
            Entity Second;

            if (Component.IsPair())
            {
                First  = Component.GetFirst();
                Second = Component.GetSecond();
            }
            else
            {
                Second = Component;
            }

            const ConstPtr<Factory> Serializer = Second.IsValid() ? Second.Get<const Factory>() : nullptr;

            if (Serializer && (!First.IsValid() || First.Has<Factory>()))
            {
                /// Write the name of the relation tag if valid, otherwise an empty string.
                Writer.WriteText(First.IsValid() ? First.GetName() : "");

                /// Write the name of the relation target or component.
                Writer.WriteText(Second.GetName());

                /// Serialize the component data into a temporary bundle.
                // TODO: Prevent heap Allocation.
                Base::Writer Bundle;
                Serializer->Write(Bundle, Actor.Get(Component));

                /// Write the serialized component bundle to the output stream.
                Writer.WriteBlock(Bundle.GetData());
            }
        };
        Actor.Iterate(OnIterateEntityComponents);

        /// Write delimiter marking the end of the component list.
        Writer.WriteUInt32(-1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RegisterDefaultComponentsAndSystems()
    {
#ifdef   FLECS_REST_SERVICE
        /// Import built-in ECS statistics module.
        mWorld.import<flecs::stats>();

        /// Enable REST service for remote ECS inspection.
        mWorld.emplace<flecs::Rest>();
#endif // FLECS_REST_SERVICE

        // Frees the archetype handle associated with the prefab to keep archetype tracking consistent.
        CreateObserver("_Archetypes::OnRemove", flecs::OnRemove, [this](Entity Actor)
        {
            mArchetypes.Free(Actor.GetID() - kMinRangeArchetypes);
        }, DSL::In(flecs::Prefab));

        // Create a query that matches all archetypes (prefabs).
        mArchetypesQueryAll = CreateQuery("_Archetypes::Query", Cache::Default, DSL::In(EcsPrefab));

        // Register Factory component (serialization).
        GetComponent<Factory>("Factory").AddTrait(Trait::Final);

        // Register Time component as singleton (tracks global time state).
        GetComponent<Time>("Time").AddTrait(Trait::Final, Trait::Singleton);

        // Register Transient component (marks entities as non serializable).
        GetComponent<EcsTransient>("Transient").AddTrait(Trait::Associative);
    }
}