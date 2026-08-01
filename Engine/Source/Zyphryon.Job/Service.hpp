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
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Job
{
    /// \brief A lane-based job scheduler with dependency tracking and helping waits.
    class Service final : public Engine::Subsystem
    {
    public:

        /// \brief Constructs the job service and starts the worker threads of every background lane.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Drains every job queued on \ref Lane::Main.
        ///
        /// \param Time The elapsed time in seconds since the last tick.
        void OnTick(Real64 Time) override;

        /// \brief Shuts down the service and stops all worker threads, discarding any job that has not started.
        void OnTeardown() override;

        /// \brief Submits a job that nothing will wait on.
        ///
        /// \param Target The lane to execute the job on.
        /// \param Work   The job to execute.
        void Dispatch(Lane Target, AnyRef<Task> Work);

        /// \brief Submits a job and returns a handle that tracks its completion.
        ///
        /// \param Target     The lane to execute the job on.
        /// \param Work       The job to execute.
        /// \param Dependency An optional job that must complete before this one is queued.
        /// \return A handle that must be passed to \ref Wait, \ref Block or \ref Discard exactly once.
        Handle Submit(Lane Target, AnyRef<Task> Work, Handle Dependency = 0);

        /// \brief Blocks until the given job completes, executing other compute jobs meanwhile, then releases it.
        ///
        /// \param Job The job to wait on.
        void Wait(Handle Job);

        /// \brief Blocks until the given job completes without executing anything else, then releases it.
        ///
        /// \param Job The job to wait on.
        void Block(Handle Job);

        /// \brief Releases a handle without waiting for the job to complete.
        ///
        /// \param Job The job to release.
        void Discard(Handle Job);

        /// \brief Checks whether the given job has finished executing.
        ///
        /// \param Job The job to query.
        /// \return `true` if the job completed or the handle is stale, otherwise `false`.
        Bool IsComplete(Handle Job) const;

        /// \brief Gets how many worker threads back the given lane.
        ///
        /// \param Target The lane to query.
        /// \return The number of worker threads, or `0` for \ref Lane::Main and on platforms without threads.
        ZY_INLINE UInt32 GetConcurrency(Lane Target) const
        {
            return mExecutors[Enum::Cast(Target)].GetConcurrency();
        }

    private:

        /// \brief Runs a job to completion, then publishes it and queues everything it was gating.
        ///
        /// \param Value The slot of the job to execute.
        void Execute(Slot Value);

        /// \brief Gets the executor backing a lane.
        ///
        /// \param Target The lane to address.
        /// \return The executor that lane queues on.
        ZY_INLINE Ref<Executor> GetExecutor(Lane Target)
        {
            return mExecutors[Enum::Cast(Target)];
        }

        /// \brief Maps a lane onto the one that actually runs it, collapsing to \ref Lane::Main without threads.
        ///
        /// \param Target The requested lane.
        /// \return The lane the job is queued on.
        ZY_INLINE static Lane Reconcile(Lane Target)
        {
#if defined(ZY_HAS_THREADS)
            return Target;
#else
            return Lane::Main;
#endif
        }

        /// \brief Entry point for the worker threads of a background lane.
        ///
        /// \param Target The lane the worker draws work from.
        /// \param Token  The stop token used to signal thread termination.
        void OnWorkerThread(Lane Target, ConstRef<std::stop_token> Token);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        mutable Mutex              mMutex;
        Gate                       mGate;
        Registry                   mRegistry;
        Array<Executor, kMaxLanes> mExecutors;
    };
}