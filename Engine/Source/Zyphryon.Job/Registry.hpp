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

namespace ZyJob
{
    /// \brief Represents the table of in-flight jobs, addressed by generation-tagged handles.
    class Registry final
    {
    public:

        /// \brief Holds the bookkeeping for a single in-flight job.
        struct Entry final
        {
            /// The work to execute, cleared as soon as it returns so captures do not outlive the job.
            Task   Work;

            /// The claims outstanding on the slot, one for the pending execution plus one if a handle was handed out.
            UInt8  References = 0;

            /// `true` once the job has finished executing, `false` while it is queued or running.
            Bool   Complete   = false;

            /// The lane the job runs on.
            Lane   Target     = Lane::Main;
        };

    public:

        /// \brief Takes a free slot and installs the given work in it.
        ///
        /// \param Work   The job to execute, left untouched when the table is full.
        /// \param Target The lane the job runs on.
        /// \return The handle of the slot, claimed for its execution and for the handle, or none if the table is full.
        Handle Acquire(AnyRef<Task> Work, Lane Target);

        /// \brief Resolves a handle to the slot it names.
        ///
        /// \param Job The handle to resolve.
        /// \return The slot the handle names, or `0` if the handle is invalid or stale.
        Slot Resolve(Handle Job) const;

        /// \brief Drops one claim on a slot and recycles it once none remain.
        ///
        /// \param Value The slot to retire.
        void Retire(Slot Value);

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