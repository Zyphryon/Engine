// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Graphic/Technique.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the queue rendering commands are collected in for efficient submission to the graphics service.
    class ZY_API Collector final
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
            ZY_INLINE constexpr Object()
                : Type { 0 },
                  Slot { 0 }
            {
            }

            /// \brief Constructs an entry with the specified type and slot.
            ///
            /// \param Type The type of object associated with this entry.
            /// \param Slot The slot of object associated with this entry.
            ZY_INLINE constexpr Object(UInt8 Type, UInt32 Slot)
                : Type { Type },
                  Slot { Slot }
            {
            }
        };

        /// \brief Represents a single rendering command with associated state for sorting and batching.
        struct Command final
        {
            /// The key used for sorting this command within the rendering queue, whose state bits also batch it.
            UInt64 Order;

            /// The index of the command's associated resource slot, which may be used for binding during rendering.
            Object Entry;

            /// \brief Constructs an empty command with default order and entry values.
            ZY_INLINE constexpr Command()
                : Order { 0 }
            {
            }

            /// \brief Constructs a command with the specified order and entry values.
            ///
            /// \param Order The key used for sorting this command within the rendering queue.
            /// \param Entry The index of the command's associated object.
            ZY_INLINE constexpr Command(UInt64 Order, Object Entry)
                : Order { Order },
                  Entry { Entry }
            {
            }
        };

        /// \brief Constructs a collector with no commands recorded.
        ZY_INLINE Collector()
            : mPhase { Priority::Opaque }
        {
        }

        /// \brief Opens a recording phase, which every command pushed until the next drain is keyed by.
        ///
        /// \param Phase The rendering priority the phase records under.
        ZY_INLINE void Begin(Priority Phase)
        {
            ZY_ASSERT(mQueue.IsEmpty(), "The open phase must be drained before another one begins");

            mPhase = Phase;
        }

        /// \brief Pushes a rendering command into the collector for later sorting and submission.
        ///
        /// \param Entry    The index of the command's associated resource slot, which may be used for binding during rendering.
        /// \param Depth    The depth value for sorting the command, typically in the range [0, 1].
        /// \param Group    The value that splits batches beyond pipeline and material, such as the mesh a draw reads.
        /// \param Pipeline The graphics pipeline to use for rendering, which must not be null.
        /// \param Material The material to use for rendering, which may be null for default material.
        ZY_INLINE void Push(Object Entry, Real32 Depth, UInt16 Group, UInt16 Pipeline, UInt16 Material)
        {
            mQueue.Append(GenerateOrderKey(mPhase, Pipeline, Material, Group, Depth), Entry);
        }

        /// \brief Drains the open phase, invoking a callback for each batch of commands it collected.
        ///
        /// \param Callback The function to call for each batch of commands, which receives a span of commands.
        template<typename Function>
        ZY_INLINE void Poll(AnyRef<Function> Callback)
        {
            if (mQueue.IsEmpty())
            {
                return;
            }

            const UInt32 Size = mQueue.GetSize();
            const UInt64 Mask = kGroupMask[ZyEnum::Cast(mPhase)];

            // The sort overwrites every element it lands on, so the scratch only ever needs raw capacity.
            if (mScratch.GetSize() < Size * sizeof(Command))
            {
                mScratch = Blob::Allocate<Command>(Size);
            }

            // Sort the queue by the full sort key to ensure correct rendering order.
            const ConstPtr<Command> Sorted = RadixSort64(mQueue.GetData(), mScratch.GetData<Command>(), Size);

            UInt32            Start = 0;
            ConstPtr<Command> Base  = Sorted;

            // Iterate through the queue and batch commands with identical pipeline, material, and group.
            for (UInt32 End = 1; End < Size; ++End)
            {
                if (ConstRef<Command> Current = Sorted[End]; (Current.Order & Mask) != (Base->Order & Mask))
                {
                    // Invoke the callback for the current batch of commands.
                    Callback(Base->Entry.Type, ConstSpan(Base, End - Start));

                    Start = End;
                    Base  = Sorted + End;
                }
            }

            // Invoke the callback for the final batch, which a non-empty queue always leaves open.
            Callback(Base->Entry.Type, ConstSpan(Base, Size - Start));

            // Clear the command queue after processing it.
            mQueue.Clear();
        }

    private:

        /// \brief Defines a type alias for a queue of rendering commands.
        using Queue = Sequence<Command>;

        /// The bits of an order key that identify a batch, indexed by queue.
        static constexpr UInt64 kGroupMask[]
        {
            0xFFFFFFFFFF000000ull,  ///< Opaque:      [Pipeline|Material|Group] sit in the high 40 bits.
            0x000000FFFFFFFFFFull,  ///< Transparent: [Pipeline|Material|Group] sit in the low 40 bits.
        };

        /// \brief Sorts rendering commands using an in-place least-significant-digit radix sort.
        ///
        /// \param Input  The input buffer containing the commands to sort.
        /// \param Output The output buffer with space for at least \p Count commands.
        /// \param Count  The number of commands to sort.
        /// \return A pointer to the buffer containing the sorted commands.
        static Ptr<Command> RadixSort64(Ptr<Command> Input, Ptr<Command> Output, UInt32 Count);

        /// \brief Generates a sort key for a draw command based on rendering state and priority.
        ///
        /// \param Priority The rendering priority of the draw command, which determines the sorting strategy.
        /// \param Pipeline The graphics pipeline used for the draw call.
        /// \param Material The material used for the draw call.
        /// \param Group    The value that splits batches beyond pipeline and material.
        /// \param Depth    The depth value for sorting the draw call.
        /// \return A 64-bit key that encodes rendering state for optimal draw call ordering.
        ZY_INLINE static constexpr UInt64 GenerateOrderKey(
            Priority Priority, UInt16 Pipeline, UInt16 Material, UInt16 Group, Real32 Depth)
        {
            return (Priority == Priority::Opaque)
                ? GenerateOpaqueOrderKey(Pipeline, Material, Group, Depth)
                : GenerateAlphaOrderKey(Pipeline, Material, Group, Depth);
        }

        /// \brief Generates a sort key for opaque draw commands.
        ///
        /// \param Pipeline The graphics pipeline used for the draw call.
        /// \param Material The material used for the draw call.
        /// \param Group    The value that splits batches beyond pipeline and material.
        /// \param Depth    The depth value for sorting the draw call.
        /// \return A 64-bit key that can be used to sort opaque draw calls for optimal rendering order.
        ZY_INLINE static constexpr UInt64 GenerateOpaqueOrderKey(
            UInt16 Pipeline, UInt16 Material, UInt16 Group, Real32 Depth)
        {
            return (static_cast<UInt64>(Pipeline & 0x3FFu)  << 54) |    // [10:Pipeline]
                   (static_cast<UInt64>(Material & 0x3FFFu) << 40) |    // [14:Material]
                   (static_cast<UInt64>(Group    & 0xFFFFu) << 24) |    // [16:Group]
                   (static_cast<UInt64>(DepthToBits(Depth)));           // [24:Depth]
        }

        /// \brief Generates a sort key for transparent draw commands.
        ///
        /// \param Pipeline The graphics pipeline used for the draw call.
        /// \param Material The material used for the draw call.
        /// \param Group    The value that splits batches beyond pipeline and material.
        /// \param Depth    The depth value for sorting the draw call.
        /// \return A 64-bit key that can be used to sort transparent draw calls for correct back-to-front rendering order.
        ZY_INLINE static constexpr UInt64 GenerateAlphaOrderKey(
            UInt16 Pipeline, UInt16 Material, UInt16 Group, Real32 Depth)
        {
            return (static_cast<UInt64>(DepthToBits(1.0f - Depth))  << 40) |   // [24:Depth]
                   (static_cast<UInt64>(Pipeline & 0x3FFu)          << 30) |   // [10:Pipeline]
                   (static_cast<UInt64>(Material & 0x3FFFu)         << 16) |   // [14:Material]
                   (static_cast<UInt64>(Group    & 0xFFFFu));                  // [16:Group]
        }

        /// \brief Converts a floating-point depth value to the 24-bit depth field of an order key.
        ///
        /// \param Depth The depth value to convert, which may be any finite floating-point value.
        /// \return A sortable integer where the natural integer order matches the original float order.
        ZY_INLINE static constexpr UInt32 DepthToBits(Real32 Depth)
        {
            const UInt32 Raw  = CastBit<UInt32>(Depth);
            const UInt32 Mask = -static_cast<SInt32>(Raw >> 31) | 0x80000000u;
            return (Raw ^ Mask) >> 8;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Priority mPhase;
        Queue    mQueue;
        Blob     mScratch;
    };
}