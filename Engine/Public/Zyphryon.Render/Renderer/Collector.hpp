// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Collects rendering commands for efficient submission to the graphics service.
    class Collector final
    {
    public:

        /// \brief Enumerates the rendering queues for collected commands.
        enum class Priority : UInt8
        {
            Opaque,         ///< Opaque objects that can be rendered in any order.
            Transparent,    ///< Transparent objects that must be rendered back-to-front.
        };

        /// \brief Represents a reference to a rendering object.
        struct Object final
        {
            /// The type of object associated with this entry.
            UInt32 Type : 8u;

            /// The slot of object associated with this entry.
            UInt32 Slot : 24u;

            /// \brief Constructs an empty entry with default type and slot values.
            ZYPHRYON_INLINE Object()
                : Type { 0 },
                  Slot { 0 }
            {
            }

            /// \brief Constructs an entry with the specified type and slot.
            ///
            /// \param Type The type of object associated with this entry.
            /// \param Slot The slot of object associated with this entry.
            ZYPHRYON_INLINE Object(UInt8 Type, UInt32 Slot)
                : Type { Type },
                  Slot { Slot }
            {
            }
        };

        /// \brief Represents a single rendering command with associated state for sorting and batching.
        struct Command final
        {
            /// The key used for sorting this command within the rendering queue.
            UInt64 Order;

            /// The key used for batching this command with others that share the same pipeline, material, and mesh.
            UInt64 Group;

            /// The index of the command's associated resource slot, which may be used for binding during rendering.
            Object Entry;

            /// \brief Constructs an empty command with default order, group, and entry values.
            ZYPHRYON_INLINE Command()
                : Order { 0 },
                  Group { 0 }
            {
            }

            /// \brief Constructs a command with the specified order, group, and entry values.
            ///
            /// \param Order The key used for sorting this command within the rendering queue.
            /// \param Group The key used for batching this command with others that share the same pipeline, material, and mesh.
            /// \param Entry The index of the command's associated object.
            ZYPHRYON_INLINE Command(UInt64 Order, UInt64 Group, Object Entry)
                : Order { Order },
                  Group { Group },
                  Entry { Entry }
            {
            }
        };

        /// \brief Pushes a rendering command into the collector for later sorting and submission.
        ///
        /// \param Entry    The index of the command's associated resource slot, which may be used for binding during rendering.
        /// \param Priority The rendering priority of the command, which determines the sorting strategy.
        /// \param Depth    The depth value for sorting the command, typically in the range [0, 1].
        /// \param Mesh     The identifier of the mesh to be rendered, used for sorting and batching.
        /// \param Pipeline The graphics pipeline to use for rendering, which must not be null.
        /// \param Material The material to use for rendering, which may be null for default material.
        ZYPHRYON_INLINE void Push(Object Entry, Priority Priority, Real32 Depth, UInt16 Mesh, UInt16 Pipeline, UInt16 Material)
        {
            LOG_ASSERT(Pipeline, "Pipeline cannot be null.");

            // Push the command reference into the corresponding queue for later sorting and submission.
            const UInt64 OrderKey = GenerateOrderKey(Priority, Pipeline, Material, Mesh, Depth);
            const UInt64 GroupKey = GenerateGroupKey(Pipeline, Material, Mesh);

            mQueues[Enum::Cast(Priority)].emplace_back(OrderKey, GroupKey, Entry);
        }

        /// \brief Polls the collected rendering commands and invokes a callback for each batch of commands.
        ///
        /// \param Callback The function to call for each batch of commands, which receives a span of commands.
        template<typename Function>
        ZYPHRYON_INLINE void Poll(AnyRef<Function> Callback)
        {
            for (Ref<Queue> Queue : mQueues)
            {
                // If the queue is empty, skip to the next one.
                if (Queue.empty())
                {
                    continue;
                }

                // Sort the queue by the full sort key to ensure correct rendering order.
                std::ranges::sort(Queue, [&](ConstRef<Command> First, ConstRef<Command> Second)
                {
                    return First.Order < Second.Order;
                });

                UInt32            Start = 0;
                ConstPtr<Command> Base  = &Queue.front();

                // Iterate through the queue and batch commands with identical pipeline, material, and mesh.
                for (UInt32 End = 1; End < Queue.size(); ++End)
                {
                    if (ConstRef<Command> Current = Queue[End]; Current.Group != Base->Group)
                    {
                        // Invoke the callback for the current batch of commands.
                        Callback(Base->Entry.Type, ConstSpan(Base, End - Start));

                        Start = End;
                        Base  = &Queue[End];
                    }
                }

                // Invoke the callback for the final batch of commands if any remain.
                if (Start < Queue.size())
                {
                    Callback(Base->Entry.Type, ConstSpan(Base, Queue.size() - Start));
                }

                // Clear the command queue after processing it.
                Queue.clear();
            }
        }

    private:

        /// \brief Defines a type alias for a queue of rendering commands.
        using Queue = Vector<Command>;

        /// \brief Generates a sort key for a draw command based.
        ///
        /// \param Priority The rendering priority of the draw command, which determines the sorting strategy.
        /// \param Pipeline The graphics pipeline used for the draw call.
        /// \param Material The material used for the draw call.
        /// \param Mesh     The mesh used for the draw call.
        /// \param Depth    The depth value for sorting the draw call.
        /// \return A 64-bit key that encodes rendering state for optimal draw call ordering.
        ZYPHRYON_INLINE static UInt64 GenerateOrderKey(Priority Priority, UInt16 Pipeline, UInt16 Material, UInt16 Mesh, Real32 Depth)
        {
            switch (Priority)
            {
                case Priority::Opaque:
                    return GenerateOpaqueOrderKey(Pipeline, Material, Mesh, Depth);
                case Priority::Transparent:
                    return GenerateAlphaOrderKey(Pipeline, Material, Mesh, Depth);
                default:
                    return 0;
            }
        }

        /// \brief Generates a sort key for opaque draw commands.
        ///
        /// \param Pipeline The graphics pipeline used for the draw call.
        /// \param Material The material used for the draw call.
        /// \param Mesh     The mesh used for the draw call.
        /// \param Depth    The depth value for sorting the draw call.
        /// \return A 64-bit key that can be used to sort opaque draw calls for optimal rendering order.
        ZYPHRYON_INLINE static UInt64 GenerateOpaqueOrderKey(UInt16 Pipeline, UInt16 Material, UInt16 Mesh, Real32 Depth)
        {
            return (static_cast<UInt64>(Pipeline & 0x3FFu)  << 54) |    // [10:Pipeline]
                   (static_cast<UInt64>(Material & 0x3FFFu) << 40) |    // [14:Material]
                   (static_cast<UInt64>(Mesh     & 0xFFFFu) << 24) |    // [16:Mesh]
                   (static_cast<UInt64>(DepthToBits(Depth)));           // [24:Depth]
        }

        /// \brief Generates a sort key for transparent draw commands.
        ///
        /// \param Pipeline The graphics pipeline used for the draw call.
        /// \param Material The material used for the draw call.
        /// \param Mesh     The mesh used for the draw call.
        /// \param Depth    The depth value for sorting the draw call.
        /// \return A 64-bit key that can be used to sort transparent draw calls for correct back-to-front rendering order.
        ZYPHRYON_INLINE static UInt64 GenerateAlphaOrderKey(UInt16 Pipeline, UInt16 Material, UInt16 Mesh, Real32 Depth)
        {
            return (static_cast<UInt64>(DepthToBits(1.0f - Depth))  << 40) |   // [24:Depth]
                   (static_cast<UInt64>(Pipeline & 0x3FFu)          << 30) |   // [10:Pipeline]
                   (static_cast<UInt64>(Material & 0x3FFFu)         << 16) |   // [14:Material]
                   (static_cast<UInt64>(Mesh     & 0xFFFFu));                  // [16:Mesh]
        }

        /// \brief Generates a batch key encoding only the pipeline, material, and mesh.
        ///
        /// \param Pipeline The graphics pipeline identifier.
        /// \param Material The material identifier.
        /// \param Mesh     The mesh identifier.
        /// \return A 64-bit key that uniquely identifies a pipeline/material/mesh combination.
        ZYPHRYON_INLINE static UInt64 GenerateGroupKey(UInt16 Pipeline, UInt16 Material, UInt16 Mesh)
        {
            return (static_cast<UInt64>(Pipeline & 0x3FFu)  << 54) |    // [10:Pipeline]
                   (static_cast<UInt64>(Material & 0x3FFFu) << 40) |    // [14:Material]
                   (static_cast<UInt64>(Mesh     & 0xFFFFu) << 24);     // [16:Mesh]
        }

        /// \brief Converts a floating-point depth value to a sortable integer representation.
        ///
        /// \param  Depth The depth value to convert, which may be any finite floating-point value.
        /// \return A sortable integer where the natural integer order matches the original float order.
        template<UInt32 Bits = 24>
        ZYPHRYON_INLINE static UInt32 DepthToBits(Real32 Depth)
        {
            const UInt32 Raw   = std::bit_cast<UInt32>(Depth);
            const UInt32 Mask  = -static_cast<SInt32>(Raw >> 31) | 0x80000000u;
            return (Raw ^ Mask) >> (32 - Bits);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Queue, Enum::Count<Priority>()> mQueues;
    };
}