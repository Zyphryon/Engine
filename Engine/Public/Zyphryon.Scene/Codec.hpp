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

#include "Entity.hpp"
#include "Factory.hpp"

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
        ZYPHRYON_INLINE static void ReadComponent(ConstRef<flecs::world> World, Ref<Reader> Archive, Owner Actor)
        {
            // Read first element of the pair (tag/relationship); empty means single component.
            const Str8 Pair(Archive.ReadText()); // TODO: Remove heap allocation (Flecs Limitation)
            const Entity First = Pair.empty() ? Entity() : World.lookup(Pair.c_str());

            // Read component name and resolve the component entity.
            const Str8 Name(Archive.ReadText()); // TODO: Remove heap allocation (Flecs Limitation)
            const Entity Second = World.lookup(Name.c_str());

            // Read serialized component payload.
            const ConstSpan<Byte> Bundle = Archive.ReadBlock<UInt32, Byte>();

            // Apply payload if present; otherwise attach component without data.
            if (Reader Data(Bundle); Data.GetAvailable() > 0)
            {
                if (const ConstPtr<Factory> Serializer = Second.TryGet<const Factory>())
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
                    LOG_WARNING("Serializer: Trying to load an unregistered component '{}'", Second.GetName());
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
        ZYPHRYON_INLINE static void ReadComponentsOf(ConstRef<flecs::world> World, Ref<Reader> Archive, Owner Actor)
        {
            for (Reader Scope(Archive.ReadBlock<UInt16, Byte>()); Scope.GetAvailable() > 0;)
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
        ZYPHRYON_INLINE static void WriteComponent(Ref<Writer> Archive, Owner Actor, Entity Component)
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

            const ConstPtr<Factory> Serializer = Second.IsValid() ? Second.TryGet<const Factory>() : nullptr;

            if (Serializer && (!First.IsValid() || First.Has<Factory>()))
            {
                // Write the name of the relation tag if valid, otherwise an empty string.
                Archive.WriteText(First.IsValid() ? First.GetName() : "");

                // Write the name of the relation target or component.
                Archive.WriteText(Second.GetName());

                // Write the serialized component bundle to the output stream.
                Archive.WriteBlock<UInt32>([&](Ref<Writer> Output)
                {
                    Serializer->Write(Output, Actor.TryGet(Component));
                });
            }
        }

        /// \brief Writes multiple components from the specified actor to a binary data stream.
        ///
        /// \param Archive The binary data writer to write the component data to.
        /// \param Actor   The actor from which to save the components.
        template<typename Owner>
        ZYPHRYON_INLINE static void WriteComponentsOf(Ref<Writer> Archive, Owner Actor)
        {
            Archive.WriteBlock<UInt16>([Actor](Ref<Writer> Output)
            {
                Actor.Each([&](Entity Component)
                {
                    WriteComponent<Owner>(Output, Actor, Component);
                });
            });
        }
    };
}

