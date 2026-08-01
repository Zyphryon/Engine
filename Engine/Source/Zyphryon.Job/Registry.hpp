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

#include "Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Job
{
    /// \brief The table of in-flight jobs, addressed by generation-tagged handles.
    class Registry final
    {
    public:

        /// \brief Bookkeeping for a single in-flight job.
        struct Entry final
        {
            /// The work to execute, cleared as soon as it returns so captures do not outlive the job.
            Task   Work;

            /// Links the job into either its lane's queue or the successor chain of the job it depends on.
            Slot   Next       = 0;

            /// Head of the chain of jobs that this one gates.
            Slot   Successor  = 0;

            /// Distinguishes this use of the slot from every previous one, so stale handles resolve as complete.
            UInt16 Generation = 0;

            /// Outstanding claims on the slot: one for the pending execution, plus one if a handle was handed out.
            UInt8  References = 0;

            /// The lane the job runs on.
            Lane   Target     = Lane::Compute;

            /// Whether the job has finished executing.
            Bool   Complete   = false;
        };

    public:

        /// \brief Checks whether every slot is already spoken for.
        ///
        /// \return `true` if no further job can be acquired, otherwise `false`.
        ZY_INLINE Bool IsFull() const
        {
            return mAllocator.IsFull();
        }

        /// \brief Takes a free slot and installs the given work in it.
        ///
        /// \param Work   The job to execute.
        /// \param Target The lane the job runs on.
        /// \return The slot the job occupies, carrying one claim for its execution and one for its handle.
        Slot Acquire(AnyRef<Task> Work, Lane Target);

        /// \brief Mints the handle that names the given slot.
        ///
        /// \param Value The slot to name.
        /// \return A handle that resolves back to \p Value until the slot is recycled.
        ZY_INLINE Handle Mint(Slot Value) const
        {
            return (static_cast<Handle>(mEntries[Index(Value)].Generation) << 16) | static_cast<Handle>(Value);
        }

        /// \brief Resolves a handle to the slot it names.
        ///
        /// \param Job The handle to resolve.
        /// \return The slot the handle names, or `0` if the handle is invalid or stale.
        Slot Resolve(Handle Job) const;

        /// \brief Drops one claim on a slot and recycles it once none remain.
        ///
        /// \param Value The slot to retire.
        void Retire(Slot Value);

        /// \brief Parks a job on the job it depends on, to be queued when that one completes.
        ///
        /// \param Value      The slot of the job to park.
        /// \param Dependency The job that must complete first.
        /// \return `true` if the job was parked, or `false` if the dependency is already done and it may queue now.
        Bool Park(Slot Value, Handle Dependency);

        /// \brief Detaches the chain of jobs the given slot was gating.
        ///
        /// \param Value The slot whose successors are being released.
        /// \return The head of the detached chain, walked through \ref Entry::Next.
        ZY_INLINE Slot Detach(Slot Value)
        {
            return Exchange(mEntries[Index(Value)].Successor, 0);
        }

        /// \brief Drops the captures of every live job and empties the table.
        void Clear();

        /// \brief Gets the entry backing a slot.
        ///
        /// \param Value The slot to read, which must be allocated.
        /// \return The entry stored in that slot.
        ZY_INLINE Ref<Entry> operator[](Slot Value)
        {
            return mEntries[Index(Value)];
        }

        /// \brief Gets the entry backing a slot.
        ///
        /// \param Value The slot to read, which must be allocated.
        /// \return The entry stored in that slot.
        ZY_INLINE ConstRef<Entry> operator[](Slot Value) const
        {
            return mEntries[Index(Value)];
        }

    private:

        /// \brief Turns a slot into the index of the entry backing it.
        ///
        /// \param Value The slot to convert, which must name a real entry.
        /// \return The zero-based index into the entries array.
        ZY_INLINE static UInt32 Index(Slot Value)
        {
            ZY_ASSERT(Value != 0 && Value <= kMaxJobs, "Addressed a job slot outside the table");

            return Value - 1;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Entry, kMaxJobs> mEntries;
        Freelist<kMaxJobs>     mAllocator;
    };
}