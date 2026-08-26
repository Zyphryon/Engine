// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Context.hpp"
#include "Entity.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Provides static methods for serializing and deserializing components to and from binary data streams.
    class Codec final
    {
    public:

        /// \brief Reads a component from a binary data stream and applies it to the specified actor.
        ///
        /// \param World   The world context used to resolve component entities and relationships.
        /// \param Archive The binary data reader to read the component data from.
        /// \param Actor   The actor to which the loaded component will be applied.
        template<typename Owner>
        ZY_INLINE static void ReadComponent(Ptr<ecs_world_t> World, Ref<Reader> Archive, Owner Actor)
        {
            // Read first element of the pair (tag/relationship); empty means single component.
            const Str64  Pair    = Archive.ReadText();
            const Entity First   = !Pair.IsEmpty() ? Resolve(World, Pair) : Entity();

            // Read component name and resolve the component entity.
            const Str64  Name   = Archive.ReadText();
            const Entity Second = Resolve(World, Name);

            // Read serialized component payload.
            const ConstSpan<Byte> Bundle = Archive.ReadBlock<UInt32, Byte>();

            // A relation that named a target it can no longer resolve is skipped the same way, because
            // attaching the component alone would grant the entity something it never had.
            if (!Second.IsValid() || (!Pair.IsEmpty() && !First.IsValid()))
            {
                return;
            }

            // Apply payload if present; otherwise attach component without data.
            if (Reader Data(Bundle.GetData(), Bundle.GetSize()); Data.GetAvailable() > 0)
            {
                if (const ConstPtr<Factory> Serializer = Context::Get(World).GetFactory(Second.GetID()))
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

        /// \brief Reads multiple components from a binary data stream.
        ///
        /// \param World   The world context used to resolve component entities and relationships.
        /// \param Archive The binary data reader to read the component data from.
        /// \param Actor   The actor to which the loaded components will be applied.
        template<typename Owner>
        ZY_INLINE static void ReadComponentsOf(Ptr<ecs_world_t> World, Ref<Reader> Archive, Owner Actor)
        {
            const ConstSpan<Byte> Data = Archive.ReadBlock<UInt32, Byte>();

            for (Reader Scope(Data.GetData(), Data.GetSize()); Scope.GetAvailable() > 0;)
            {
                ReadComponent(World, Scope, Actor);
            }
        }

        /// \brief Writes a component from the specified actor to a binary data stream.
        ///
        /// \param Archive   The binary data writer to write the component data to.
        /// \param Actor     The actor from which to save the component.
        /// \param Component The component entity to save, which may be a single component or a relation pair.
        template<typename Owner>
        ZY_INLINE static void WriteComponent(Ref<Writer> Archive, Owner Actor, Entity Component)
        {
            Entity First;
            Entity Second;

            if (Component.IsPair())
            {
                First  = Component.GetRelation();
                Second = Component.GetComponent();
            }
            else
            {
                Second = Component;
            }

            ConstRef<Context> Scope = Context::Get(Component.GetWorld());

            const ConstPtr<Factory> Serializer = Second.IsValid() ? Scope.GetFactory(Second.GetID()) : nullptr;

            if (Serializer && (!First.IsValid() || Scope.GetFactory(First.GetID())))
            {
                // Write the name of the relation tag if valid, otherwise an empty string.
                Archive.WriteText(First.IsValid() ? First.GetName() : "");

                // Write the name of the relation target or component.
                Archive.WriteText(Second.GetName());

                // Write the serialized component bundle to the output stream.
                Archive.WriteBlock<UInt32>([&](Ref<Writer> Output)
                {
					if (!Component.IsTag())
                    {
                        Serializer->Write(Output, Actor.TryGet(Component));
                    }
                });
            }
        }

        /// \brief Writes multiple components from the specified actor to a binary data stream.
        ///
        /// \param Archive The binary data writer to write the component data to.
        /// \param Actor   The actor from which to save the components.
        template<typename Owner>
        ZY_INLINE static void WriteComponentsOf(Ref<Writer> Archive, Owner Actor)
        {
            Archive.WriteBlock<UInt32>([Actor](Ref<Writer> Output)
            {
                Actor.Each([&](Entity Component)
                {
                    WriteComponent<Owner>(Output, Actor, Component);
                });
            });
        }

    private:

        /// \brief Resolves a component entity by the name it was written under.
        ///
        /// \param World The world context used to resolve the name.
        /// \param Name  The name to look up.
        /// \return The entity registered under that name, or an invalid entity if there is none.
        ZY_INLINE static Entity Resolve(Ptr<ecs_world_t> World, Text Name)
        {
            return Entity(World, ecs_lookup_path_w_sep(World, 0, Name.GetData(), "::", "::", true));
        }
    };
}