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
#include "Salvage.hpp"

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

            // A name the world has no place for is kept whole rather than lost, so the entity is written
            // back as it came in and the component lands the moment something does name it.
            if (!Second.IsValid() || (!Pair.IsEmpty() && !First.IsValid()))
            {
                const Entity Keeper(World, _::Identify<Salvage>(World));

                if (const Ptr<Salvage> Kept = static_cast<Ptr<Salvage>>(Actor.Ensure(Keeper)))
                {
                    Kept->Keep(Pair, Name, Bundle);
                }
                return;
            }

            // Apply payload if present; otherwise attach component without data.
            if (Reader Data(Bundle); Data.GetAvailable() > 0)
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

            for (Reader Scope(Data); Scope.GetAvailable() > 0;)
            {
                ReadComponent(World, Scope, Actor);
            }
        }

        /// \brief Writes a component from the specified actor to a binary data stream.
        ///
        /// \param Archive   The binary data writer to write the component data to.
        /// \param Actor     The actor from which to save the component.
        /// \param Component The component entity to save, which may be a single component or a relation pair.
        /// \return `true` if the component has a serializer and was written, `false` otherwise.
        template<typename Owner>
        ZY_INLINE static Bool WriteComponent(Ref<Writer> Archive, Owner Actor, Entity Component)
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
                return true;
            }
            return false;
        }

        /// \brief Writes multiple components from the specified actor to a binary data stream.
        ///
        /// \param Archive The binary data writer to write the component data to.
        /// \param Actor   The actor from which to save the components.
        /// \return `true` if at least one component was written, `false` otherwise.
        template<typename Owner>
        ZY_INLINE static Bool WriteComponentsOf(Ref<Writer> Archive, Owner Actor)
        {
            Bool Written = false;

            Archive.WriteBlock<UInt32>([&](Ref<Writer> Output)
            {
                Actor.Each([&](Entity Component)
                {
                    Written |= WriteComponent<Owner>(Output, Actor, Component);
                });

                if (const ConstPtr<Salvage> Kept = Actor.template TryGet<const Salvage>())
                {
                    for (ConstRef<Salvage::Record> Record : Kept->GetRecords())
                    {
                        Output.WriteText(Record.Relation);
                        Output.WriteText(Record.Name);
                        Output.WriteBlock<UInt32>(ConstSpan<Byte>(Record.Data.GetData(), Record.Data.GetSize()));

                        Written = true;
                    }
                }
            });
            return Written;
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