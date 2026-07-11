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

#include "../Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Job
{
    /// \brief A simple task scheduler service that runs tasks on a separate thread.
    class Service final : public Engine::Subsystem
    {
    public:

        /// \brief Signature for a unit of work submitted to the scheduler.
        using Task = Delegate<void()>;

    public:

        /// \brief Constructs the scene service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Called once per frame to process pending tasks.
        ///
        /// \param Time The elapsed time in seconds since the last tick.
        void OnTick(Real64 Time) override;

        /// \brief Shuts down the service and stops all worker threads.
        void OnTeardown() override;

        /// \brief Submits a task for execution on the main thread.
        ///
        /// \param Work The task to execute.
        void SubmitOnMain(AnyRef<Task> Work);

        /// \brief Submits a task for execution on a background worker thread.
        ///
        /// \param Work The task to execute.
        void SubmitOnBackground(AnyRef<Task> Work);

    private:

        /// \brief Entry point for worker threads that process pending background tasks.
        ///
        /// \param Token The stop token used to signal thread termination.
        void OnWorkerThread(ConstRef<std::stop_token> Token);

    public:

        /// \brief Gets the number of concurrent threads supported by the hardware.
        ///
        /// \return The number of concurrent threads supported by the hardware.
        ZY_INLINE static UInt32 GetConcurrency()
        {
            return std::thread::hardware_concurrency();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Mutex            mMainMutex;
        Sequence<Task>   mMainQueue;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Thread> mTaskThreads;
        Mutex            mTaskMutex;
        Sequence<Task>   mTaskQueue;
        Gate             mTaskGate;
    };
}