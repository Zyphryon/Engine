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

#include "Manifest.hpp"
#include "Zyphryon.Scene/World.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::Protocol
{
    /// \brief Provides the encoding of a set of components both sides agree on.
    class Wire final
    {
    public:

        /// \brief Represents where one component landed in a scratch.
        struct Slice final
        {
            /// The offset of the payload in the scratch.
            UInt32 Offset;

            /// The size of the payload in bytes.
            UInt32 Size;
        };

        /// \brief The slices of every component in the manifest, indexed by their place in it.
        using Slices = Array<Slice, kMaxComponents>;

    public:

        /// \brief Serializes the components of a mask an owner holds into a scratch.
        ///
        /// \param Scratch The scratch to serialize into.
        /// \param Actor   The entity or world holding the components.
        /// \param Wanted  The components to serialize, one bit each.
        /// \param Output  Receives where each component landed.
        /// \return The bits of \p Wanted the owner actually holds, which is all a set may name.
        template<typename Owner>
        ZY_INLINE static Mask Encode(Ref<Writer> Scratch, Owner Actor, Mask Wanted, Ref<Slices> Output)
        {
            ConstRef<Manifest>     Table = Manifest::Get();
            const Ptr<ecs_world_t> World = WorldOf(Actor);

            Mask Held;

            Wanted.ForEach([&](UInt Place)
            {
                const UInt8               Index = static_cast<UInt8>(Place);
                ConstRef<Manifest::Entry> Entry = Table.GetEntry(Index);
                const Entity              Component(World, Entry.Component);

                if constexpr (requires { Actor.Owns(Component); })
                {
                    if (!Actor.Owns(Component))
                    {
                        return;
                    }
                }
                else
                {
                    if (!Actor.Has(Component))
                    {
                        return;
                    }
                }

                Ref<Slice> Slice = Output[Index];
                Slice.Offset = Scratch.GetSize();

                if (!Entry.Tag)
                {
                    Entry.Serializer.Write(Scratch, Actor.TryGet(Component));
                }

                Slice.Size = Scratch.GetSize() - Slice.Offset;
                Held.Set(Index);
            });
            return Held;
        }

        /// \brief Writes a set naming the components present, from the slices a scratch holds, and the ones gone.
        ///
        /// \param Output  The stream to write the set into.
        /// \param Scratch The scratch the components were serialized into.
        /// \param Slices  Where each component landed in the scratch.
        /// \param Wanted  The components to name as present, which must all have been encoded.
        /// \param Absent  The components to name as gone, which carry nothing.
        ZY_INLINE static void Compose(Ref<Writer> Output, ConstRef<Writer> Scratch, ConstRef<Slices> Slices, Mask Wanted, Mask Absent)
        {
            Output.Write<UInt8>(static_cast<UInt8>(Wanted.Count()));

            Wanted.ForEach([&](UInt Place)
            {
                const UInt8     Index = static_cast<UInt8>(Place);
                ConstRef<Slice> Slice = Slices[Index];

                Output.Write<UInt8>(Index);
                Output.WriteBlock<UInt16>(ConstSpan<Byte>(Scratch.GetData() + Slice.Offset, Slice.Size));
            });

            Output.Write<UInt8>(static_cast<UInt8>(Absent.Count()));

            Absent.ForEach([&](UInt Place)
            {
                Output.Write<UInt8>(static_cast<UInt8>(Place));
            });
        }

        /// \brief Reads a set and applies it to an owner, adding or updating what is present and removing what is gone.
        ///
        /// \param Input The stream to read the set from.
        /// \param Actor The entity or world receiving the components.
        /// \return `false` if the set named a component this build does not know, `true` otherwise.
        template<typename Owner>
        ZY_INLINE static Bool Decode(Ref<Reader> Input, Owner Actor)
        {
            ConstRef<Manifest>     Table = Manifest::Get();
            const Ptr<ecs_world_t> World = WorldOf(Actor);

            for (UInt8 Count = Input.Read<UInt8>(); Count > 0; --Count)
            {
                const UInt8           Index   = Input.Read<UInt8>();
                const ConstSpan<Byte> Payload = Input.ReadBlock<UInt16, Byte>();

                if (Index >= Table.GetCount())
                {
                    return false;
                }

                ConstRef<Manifest::Entry> Entry = Table.GetEntry(Index);
                const Entity              Component(World, Entry.Component);

                if (Entry.Tag || Payload.IsEmpty())
                {
                    Actor.Add(Component);
                }
                else
                {
                    Reader Data(Payload);

                    if (const Ptr<void> Memory = Actor.Ensure(Component))
                    {
                        Entry.Serializer.Read(Data, Memory);
                    }
                    Actor.Notify(Component);
                }
            }

            for (UInt8 Count = Input.Read<UInt8>(); Count > 0; --Count)
            {
                const UInt8 Index = Input.Read<UInt8>();

                if (Index >= Table.GetCount())
                {
                    return false;
                }
                Actor.Remove(Entity(World, Table.GetEntry(Index).Component));
            }
            return true;
        }

    private:

        /// \brief Gets the world an owner belongs to, whether it is an entity in one or the world itself.
        ///
        /// \param Actor The entity or world.
        /// \return The world.
        template<typename Owner>
        ZY_INLINE static Ptr<ecs_world_t> WorldOf(Owner Actor)
        {
            if constexpr (requires { Actor.GetWorld(); })
            {
                return Actor.GetWorld();
            }
            else
            {
                return Actor.GetHandle();
            }
        }
    };
}