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

#include "Factory.hpp"
#include <flecs.h>

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
            const flecs::entity First = Pair.empty() ? flecs::entity() : World.lookup(Pair.c_str());

            // Read component name and resolve the component entity.
            const Str8 Name(Archive.ReadText()); // TODO: Remove heap allocation (Flecs Limitation)
            const flecs::entity Second = World.lookup(Name.c_str());

            // Read serialized component payload.
            const ConstSpan<Byte> Bundle = Archive.ReadBlock<UInt16, Byte>();

            // Apply payload if present; otherwise attach component without data.
            if (Reader Data(Bundle); Data.GetAvailable() > 0)
            {
                if (const ConstPtr<Factory> Serializer = Second.try_get<const Factory>())
                {
                    if (First.is_valid())
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
                    LOG_WARNING("Serializer: Trying to load an unregistered component '{}'", Second.name().c_str());
                }
            }
            else
            {
                if (First.is_valid())
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
            Reader Scope(Archive.ReadBlock<UInt32, Byte>());

            while (Scope.GetAvailable() > 0)
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
        ZYPHRYON_INLINE static void WriteComponent(Ref<Writer> Archive, Owner Actor, flecs::entity Component)
        {
            flecs::entity First;
            flecs::entity Second;

            if (Component.is_pair())
            {
                First  = Component.first();
                Second = Component.second();
            }
            else
            {
                Second = Component;
            }

            const ConstPtr<Factory> Serializer = Second.is_valid() ? Second.try_get<const Factory>() : nullptr;

            if (Serializer && (!First.is_valid() || First.has<Factory>()))
            {
                // Write the name of the relation tag if valid, otherwise an empty string.
                const flecs::string_view FirstName(First.is_valid() ? First.name() : "");
                Archive.WriteText(First.is_valid() ? ConstStr8(FirstName.c_str(), FirstName.size()) : "");

                // Write the name of the relation target or component.
                const flecs::string_view SecondName(Second.name());
                Archive.WriteText(ConstStr8(SecondName.c_str(), SecondName.size()));

                // Write the serialized component bundle to the output stream.
                Archive.WriteBlock<UInt16>([&](Ref<Writer> Output)
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
                Actor.Each([&]<typename T0>(T0 Component)
                {
                    WriteComponent<Owner>(Output, Actor, flecs::entity(Component));
                });
            });
        }
    };
}

