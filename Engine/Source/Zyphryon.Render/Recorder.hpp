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

#include "Collector.hpp"
#include "Encoder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Gathers the draws of one pass for a \ref Collector to order, and writes each batch it hands back.
    ///
    /// \tparam Layout The per-instance data one draw is laid out from, in the layout its technique reads.
    template<typename Layout>
    class Recorder final
    {
    public:

        /// \brief Holds what every draw of one batch shares, which is what the batch was gathered by.
        struct Batch final
        {
            /// The technique the batch is drawn with.
            ConstPtr<ZyGraphic::Technique> Technique;

            /// The material the batch binds.
            ConstPtr<ZyGraphic::Material>  Material;

            /// The features the draw turns on beyond the ones its material implies.
            ZyGraphic::Technique::Key      Variant;

            /// What else keeps the draw out of a batch it does not belong in.
            UInt16                         Group;

            /// \brief Constructs a batch bound to no technique and no material.
            ZY_INLINE constexpr Batch()
                : Technique { nullptr },
                  Material  { nullptr },
                  Variant   { 0 },
                  Group     { 0 }
            {
            }

            /// \brief Constructs a batch with everything its draws share.
            ///
            /// \param Technique The technique the batch is drawn with.
            /// \param Material  The material the batch binds.
            /// \param Variant   The features the draw turns on beyond the ones its material implies.
            /// \param Group     What else keeps the draw out of a batch it does not belong in.
            ZY_INLINE constexpr Batch(ConstPtr<ZyGraphic::Technique> Technique,
                ConstPtr<ZyGraphic::Material> Material, ZyGraphic::Technique::Key Variant, UInt16 Group)
                : Technique { Technique },
                  Material  { Material },
                  Variant   { Variant },
                  Group     { Group }
            {
            }
        };

    public:

        /// \brief Constructs a recorder allocating from the specified service.
        ///
        /// \param Service   The service the transient instance streams are allocated from.
        /// \param Collector The collector the draws are ordered and batched by.
        /// \param Kind      The tag every draw is stamped with, so a drain routes its batches back here.
        /// \param Vertices  How many vertices one instance is drawn from, four being the quad most are.
        ZY_INLINE Recorder(
            ConstRetainer<ZyGraphic::Service> Service, Ref<Collector> Collector, UInt32 Kind, UInt32 Vertices = 4)
            : mService   { Service },
              mCollector { Collector },
              mKind      { Kind },
              mVertices  { Vertices }
        {
        }

        /// \brief Sets the technique subsequent draws are recorded under.
        ///
        /// \param Technique The technique to set for subsequent draws.
        ZY_INLINE void SetTechnique(ConstRetainer<ZyGraphic::Technique> Technique)
        {
            mTechnique = Technique;
        }

        /// \brief Gets the service the transient streams are allocated from.
        ///
        /// \return The service, for whoever holds the recorder to allocate alongside it.
        ZY_INLINE ConstRetainer<ZyGraphic::Service> GetService() const
        {
            return mService;
        }

        /// \brief Checks whether anything has been recorded since the last reset.
        ///
        /// \return `true` while nothing has been recorded.
        ZY_INLINE Bool IsEmpty() const
        {
            return mBatches.IsEmpty();
        }

        /// \brief Drops everything recorded so far, and the technique it was recorded under.
        ZY_INLINE void Reset()
        {
            mBatches.Clear();
            mLayouts.Clear();
            mTechnique = nullptr;
        }

        /// \brief Records one draw and files it with the collector, handing back the instance to fill.
        ///
        /// \param Order    Where the draw falls in the queue the collector orders.
        /// \param Material The material the draw binds, which its batch is gathered by.
        /// \param Group    What else keeps the draw out of a batch it does not belong in.
        /// \param Variant  The features the draw turns on beyond the ones its material implies.
        /// \return The instance the draw is laid out from.
        ZY_INLINE Ref<Layout> Open(
            Real32 Order, ConstRef<ZyGraphic::Material> Material, UInt16 Group, ZyGraphic::Technique::Key Variant = 0)
        {
            ZY_ASSERT(mTechnique, "A technique must be set before recording a draw");

            // Filed before either half exists, so the slot it is filed under is the one they land in.
            mCollector.Push(
                Collector::Object(mKind, mBatches.GetSize()),
                Order, 
                Group, 
                mTechnique->GetHandle(), 
                Material.GetHandle());

            mBatches.Append(Batch(AddressOf(* mTechnique), AddressOf(Material), Variant, Group));

            return mLayouts.Append();
        }

        /// \brief Gets what every draw of one batch shares.
        ///
        /// \param Commands The batch the collector handed back.
        /// \return The record the batch's first draw was recorded with.
        ZY_INLINE ConstRef<Batch> GetLeader(ConstSpan<Collector::Command> Commands) const
        {
            return mBatches[Commands.GetFront().Entry.Slot];
        }

        /// \brief Writes one batch as instanced draws through the encoder, one for each run of a single variant.
        ///
        /// \param Encoder  The encoder that builds the resulting draw commands.
        /// \param Commands The batch the collector handed back.
        /// \param Uniform  The per-instance uniform stream the batch reads, where it reads one at all.
        ZY_INLINE void Write(Ref<Encoder> Encoder, ConstSpan<Collector::Command> Commands,
            ConstRef<ZyGraphic::Stream> Uniform = ZyGraphic::Stream())
        {
            for (UInt Start = 0, Count = Commands.GetSize(); Start < Count;)
            {
                ConstRef<Batch> First = mBatches[Commands[Start].Entry.Slot];

                UInt End = Start + 1;

                while (End < Count && mBatches[Commands[End].Entry.Slot].Variant == First.Variant)
                {
                    ++End;
                }

                const ConstSpan<Collector::Command> Run = Commands.Slice(Start, End - Start);

                const ZyGraphic::Invocation Invocation {
                    .Count     = mVertices,
                    .Instances = static_cast<UInt32>(Run.GetSize())
                };
                Encoder.Draw(* First.Technique, First.Material, Gather(Run), Uniform, Invocation, First.Variant);

                Start = End;
            }
        }

    private:

        /// \brief Gathers one batch's instances into a stream, in the order the collector sorted them into.
        ///
        /// \param Commands The batch the collector handed back.
        /// \return The instance-rate stream the batch draws from.
        ZY_INLINE ZyGraphic::Stream Gather(ConstSpan<Collector::Command> Commands) const
        {
            ZyGraphic::Transient<Layout> Block = mService->AllocateInFlightVertices<Layout>(Commands.GetSize());

            for (UInt32 Element = 0, Limit = Commands.GetSize(); Element < Limit; ++Element)
            {
                Block[Element] = mLayouts[Commands[Element].Entry.Slot];
            }
            return Block.GetStream();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Service>   mService;
        Ref<Collector>                 mCollector;
        UInt32                         mKind;
        UInt32                         mVertices;
        Retainer<ZyGraphic::Technique> mTechnique;
        Sequence<Batch>                mBatches;
        Sequence<Layout>               mLayouts;
    };
}