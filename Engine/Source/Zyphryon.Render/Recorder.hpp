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
#include <Zyphryon.Graphic/Service.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Gathers the draws of one pass for a \ref Collector to order, and reads each batch back out.
    ///
    /// \tparam Layout  The per-instance data one draw is laid out from, in the layout its technique reads.
    /// \tparam Command The record a draw's batch is keyed by, which every draw in that batch shares.
    template<typename Layout, typename Command>
    class Recorder
    {
    public:

        /// \brief Holds the two halves of one recorded draw, for whoever recorded it to fill.
        struct Record final
        {
            /// The instance the draw is laid out from.
            Ref<Layout>  Instance;

            /// The record the draw's batch is keyed by.
            Ref<Command> Entry;
        };

    public:

        /// \brief Constructs a recorder allocating from the specified service.
        ///
        /// \param Service   The service the transient instance streams are allocated from.
        /// \param Collector The collector the draws are ordered and batched by.
        /// \param Kind      The tag every draw is stamped with, so a drain routes its batches back here.
        ZY_INLINE Recorder(ConstRetainer<ZyGraphic::Service> Service, Ref<Collector> Collector, UInt32 Kind)
            : mService   { Service },
              mCollector { Collector },
              mKind      { Kind }
        {
        }

        /// \brief Sets the technique subsequent draws are recorded under.
        ///
        /// \param Technique The technique to set for subsequent draws.
        ZY_INLINE void SetTechnique(ConstRetainer<ZyGraphic::Technique> Technique)
        {
            mTechnique = Technique;
        }

        /// \brief Gets the technique subsequent draws are recorded under.
        ///
        /// \return The technique, or nothing while none has been set.
        ZY_INLINE ConstRetainer<ZyGraphic::Technique> GetTechnique() const
        {
            return mTechnique;
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
            return mCommands.IsEmpty();
        }

        /// \brief Drops everything recorded so far, and the technique it was recorded under.
        ZY_INLINE void Reset()
        {
            mCommands.Clear();
            mLayouts.Clear();
            mTechnique = nullptr;
        }

        /// \brief Records one draw and files it with the collector, handing back the halves to fill.
        ///
        /// \param Order    Where the draw falls in the queue the collector orders.
        /// \param Key      What keeps the draw out of a batch it does not belong in.
        /// \param Material The material the batch is grouped by.
        /// \return The instance the draw is laid out from, and the record its batch is keyed by.
        ZY_INLINE Record Open(Real32 Order, UInt16 Key, ZyGraphic::Object Material)
        {
            ZY_ASSERT(mTechnique, "A technique must be set before recording a draw");

            // Filed before either half exists, so the slot it is filed under is the one they land in.
            const Collector::Object Entry(mKind, mCommands.GetSize());

            mCollector.Push(Entry, Order, Key, mTechnique->GetHandle(), Material);

            return Record(mLayouts.Append(), mCommands.Append());
        }

        /// \brief Gathers one batch's instances into a stream, in the order the collector sorted them into.
        ///
        /// \param Commands The batch the collector handed back.
        /// \return The instance-rate stream the batch draws from.
        ZY_INLINE ZyGraphic::Stream Gather(ConstSpan<Collector::Command> Commands) const
        {
            ZyGraphic::Transient<Layout> Block
                = mService->template AllocateInFlightVertices<Layout>(Commands.GetSize());

            for (UInt32 Element = 0, Limit = Commands.GetSize(); Element < Limit; ++Element)
            {
                Block[Element] = mLayouts[Commands[Element].Entry.Slot];
            }
            return Block.GetStream();
        }

        /// \brief Gets the record a batch is keyed by, which every draw in it shares.
        ///
        /// \param Commands The batch the collector handed back.
        /// \return The record the batch's first draw was recorded with.
        ZY_INLINE ConstRef<Command> GetLeader(ConstSpan<Collector::Command> Commands) const
        {
            return mCommands[Commands.GetFront().Entry.Slot];
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Service>   mService;
        Ref<Collector>                 mCollector;
        UInt32                         mKind;
        Retainer<ZyGraphic::Technique> mTechnique;
        Sequence<Command>              mCommands;
        Sequence<Layout>               mLayouts;
    };
}