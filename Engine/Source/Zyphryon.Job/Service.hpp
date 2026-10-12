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

#include "Executor.hpp"
#include "Registry.hpp"
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyJob
{
    /// \brief Represents a lane-based job scheduler with helping waits.
    class ZY_API Service final : public ZyEngine::Subsystem
    {
    public:

        /// \brief Constructs the job service and starts the worker threads of every background lane.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Runs the jobs queued on \ref Lane::Main before this tick, then any that found the job table full.
        ///
        /// \param Delta The elapsed time in seconds since the last tick.
        void OnTick(Real64 Delta) override;

        /// \brief Shuts down the service and stops all worker threads, discarding any job that has not started.
        void OnTeardown() override;

        /// \brief Gets how many worker threads back the given lane.
        ///
        /// \param Target The lane to query.
        /// \return The number of worker threads, or `0` for \ref Lane::Main and on platforms without threads.
        ZY_INLINE UInt32 GetConcurrency(Lane Target) const
        {
            return mExecutors[ZyEnum::Cast(Target)].GetConcurrency();
        }

        /// \brief Submits a job that nothing will wait on.
        ///
        /// \param Target The lane to execute the job on.
        /// \param Work   The job to execute.
        void Dispatch(Lane Target, AnyRef<Task> Work);

        /// \brief Submits a job and returns a handle that tracks its completion.
        ///
        /// \param Target The lane to execute the job on.
        /// \param Work   The job to execute.
        /// \return A handle that must be passed to \ref Wait or \ref Discard exactly once.
        Handle Submit(Lane Target, AnyRef<Task> Work);

        /// \brief Blocks until the given job completes, executing other compute jobs meanwhile, then releases it.
        ///
        /// \param Job The job to wait on.
        void Wait(Handle Job);

        /// \brief Releases a handle without waiting for the job to complete.
        ///
        /// \param Job The job to release.
        void Discard(Handle Job);

        /// \brief Splits a range across a lane, returning once every chunk of it has been visited.
        ///
        /// \param Target    The lane to spread the range over, which cannot be \ref Lane::Main.
        /// \param Count     The number of indices to visit.
        /// \param Callback  The callback invoked with one half-open chunk of the range, on whichever worker took it.
        /// \param Partition The number of chunks to cut per worker; raise it when the per-index cost is very uneven.
        template<typename Callable>
        void Parallel(Lane Target, UInt32 Count, AnyRef<Callable> Callback, UInt32 Partition = 8)
        {
            ZY_ASSERT(Target != Lane::Main, "Cannot spread a range over the main lane");

            if (Count == 0)
            {
                return;
            }

            const UInt32 Concurrency = Max(1u, GetConcurrency(Target));
            const UInt32 Grain       = Max(1u, Count / (Concurrency * Max(1u, Partition)));
            const UInt32 Workers     = Min(Concurrency, (Count + Grain - 1) / Grain);

            Atomic<UInt32> Cursor { 0 };

            Sequence<Handle> Jobs(Workers);

            for (UInt32 Worker = 0; Worker < Workers; ++Worker)
            {
                Jobs.Append(Submit(Target, [&]
                {
                    Sweep(Cursor, Grain, Count, Callback);
                }));
            }

            for (const Handle Job : Jobs)
            {
                Wait(Job);
            }
        }

    private:

        /// \brief Maximum time, in seconds, a tick may spend on the main lane without threads before the rest waits.
        static constexpr Real64 kMaxTickBudget = 0.006;

        /// \brief Runs a job to completion, then publishes it to its waiters.
        ///
        /// \param Value The slot of the job to execute.
        void Execute(Slot Value);

        /// \brief Gets the executor backing a lane.
        ///
        /// \param Target The lane whose executor to get.
        /// \return The executor that lane queues on.
        ZY_INLINE Ref<Executor> GetExecutor(Lane Target)
        {
            return mExecutors[ZyEnum::Cast(Target)];
        }

        /// \brief Maps a lane onto the one that actually runs it, collapsing to \ref Lane::Main without threads.
        ///
        /// \param Target The lane the job was submitted to.
        /// \return The lane the job is queued on.
        ZY_INLINE static Lane Reconcile(Lane Target)
        {
#if defined(ZY_HAS_THREADS)
            return Target;
#else
            return Lane::Main;
#endif
        }

        /// \brief Visits chunks of a range through a shared cursor until none is left.
        ///
        /// \param Cursor   The cursor every worker claims its next chunk from.
        /// \param Grain    The number of indices in one chunk.
        /// \param Count    The number of indices in the range.
        /// \param Callback The callback invoked with each half-open chunk claimed.
        template<typename Function>
        ZY_INLINE static void Sweep(Ref<Atomic<UInt32>> Cursor, UInt32 Grain, UInt32 Count, Ref<Function> Callback)
        {
            for (UInt32 Start; (Start = Cursor.fetch_add(Grain, std::memory_order_relaxed)) < Count; )
            {
                Callback(Start, Min(Start + Grain, Count));
            }
        }

        /// \brief Runs a background lane's worker until it is asked to stop.
        ///
        /// \param Target The lane the worker draws work from.
        /// \param Token  The stop token used to signal thread termination.
        void OnWorkerThread(Lane Target, ConstRef<Thread::Token> Token);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        mutable Mutex              mMutex;
        Gate                       mGate;
        Registry                   mRegistry;
        Array<Executor, kMaxLanes> mExecutors;
        ThreadID                   mMain;
        Sequence<Task>             mOverflow;
    };
}