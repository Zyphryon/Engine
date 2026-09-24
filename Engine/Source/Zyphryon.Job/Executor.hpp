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
    /// \brief One lane's runtime state: the jobs queued on it and the workers that drain them.
    class Executor final
    {
    public:

        /// \brief Starts the workers that drain this executor.
        ///
        /// \param Count The number of worker threads to run.
        /// \param Body  The thread body, copied once per worker and handed a stop token.
        template<typename Function>
        void Start(UInt32 Count, ConstRef<Function> Body)
        {
            mThreads.Reserve(Count);

            for (UInt32 Worker = 0; Worker < Count; ++Worker)
            {
                mThreads.Append(Body);
            }
        }

        /// \brief Asks every worker to stop and waits for them to leave.
        ///
        /// \note The caller must not hold the scheduler's mutex, since a worker needs it to observe the stop.
        void Stop();

        /// \brief Appends a slot to the tail of the queue.
        ///
        /// \param Value The slot to enqueue.
        ZY_INLINE void Push(Slot Value)
        {
            if (mQueue.IsFull())
            {
                mQueue.Compact();
            }
            mQueue.Append(Value);
        }

        /// \brief Removes the slot at the head of the queue.
        ///
        /// \return The dequeued slot, or `0` if the executor is empty.
        ZY_INLINE Slot Pop()
        {
            if (mQueue.IsEmpty())
            {
                return 0;
            }

            const Slot Value = mQueue.Peek();
            mQueue.Pop();

            // Rewinding once the queue runs dry keeps compaction for the rare queue that never empties.
            if (mQueue.IsEmpty())
            {
                mQueue.Clear();
            }
            return Value;
        }

        /// \brief Checks whether the executor holds no queued job.
        ///
        /// \return `true` if nothing is queued, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mQueue.IsEmpty();
        }

        /// \brief Gets how many jobs are queued.
        ///
        /// \return The number of slots waiting to be taken.
        ZY_INLINE UInt GetSize() const
        {
            return mQueue.GetSize();
        }

        /// \brief Forgets every queued job without running it.
        ZY_INLINE void Clear()
        {
            mQueue.Clear();
        }

        /// \brief Wakes one worker after a job was queued.
        ZY_INLINE void Signal()
        {
            mGate.notify_one();
        }

        /// \brief Blocks the calling worker until the executor has work or it is asked to stop.
        ///
        /// \param Barrier   The held lock, released while parked.
        /// \param Condition The predicate that ends the wait.
        template<typename Predicate>
        ZY_INLINE void Await(Ref<Lock> Barrier, AnyRef<Predicate> Condition)
        {
            mGate.wait(Barrier, Forward<Predicate>(Condition));
        }

        /// \brief Gets how many workers back this executor.
        ///
        /// \return The number of worker threads, which is `0` for an executor drained by the main thread.
        ZY_INLINE UInt32 GetConcurrency() const
        {
            return static_cast<UInt32>(mThreads.GetSize());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Spool<Slot, kMaxJobs> mQueue;
        Gate                  mGate;
        Sequence<Thread>      mThreads;
    };
}