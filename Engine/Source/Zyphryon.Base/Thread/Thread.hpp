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

#include "Atomic.hpp"
#include "Zyphryon.Base/Functional/Delegate.hpp"
#include "Zyphryon.Base/Memory/Unique.hpp"

#if defined(ZY_PLATFORM_POSIX)
#include <pthread.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Identifies a thread, equal only to the identifier of the same thread.
#if   defined(ZY_PLATFORM_WINDOWS)
    using ThreadID = UInt32;
#else
    using ThreadID = pthread_t;
#endif

    /// \brief Represents a thread of execution, asked to stop and joined when it goes away.
    class ZY_API Thread final
    {
    public:

        /// \brief Represents what a running thread reads to learn it was asked to stop.
        class Token final
        {
            friend class Thread;

        public:

            /// \brief Constructs a token not yet asked to stop.
            ZY_INLINE Token()
                : mStopping { false }
            {
            }

            /// \brief Checks whether the thread was asked to stop.
            ///
            /// \return `true` once \ref Thread::Stop was called, `false` before.
            ZY_INLINE Bool Stopped() const
            {
                return mStopping.load(std::memory_order_acquire);
            }

        private:

            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

            Atomic<Bool> mStopping;
        };

        /// \brief The work a thread runs, handed the token that tells it when to stop.
        using Work   = Delegate<void(ConstRef<Token>)>;

    public:

        /// \brief Constructs a thread running nothing.
        Thread();

        /// \brief Constructs a thread and starts it on some work.
        ///
        /// \param Body The work to run, handed the token that tells it when to stop.
        explicit Thread(AnyRef<Work> Body);

        /// \brief Takes over the thread another one runs, leaving it running nothing.
        ///
        /// \param Other The thread to take from.
        Thread(AnyRef<Thread> Other);

        /// \brief Asks the thread to stop and waits for it to finish.
        ~Thread();

        /// \brief Threads are not copied, since each one is joined once.
        Thread(ConstRef<Thread> Other) = delete;

        /// \brief Asks the thread to stop, which its work learns from its token.
        ZY_INLINE void Stop()
        {
            ZY_ASSERT(Joinable(), "The thread runs nothing");

            mState->Signal.mStopping.store(true, std::memory_order_release);
        }

        /// \brief Waits for the thread to finish, then lets go of it.
        void Join();

        /// \brief Checks whether the thread was started and not joined yet.
        ///
        /// \return `true` if the thread runs work or finished it unjoined, `false` otherwise.
        ZY_INLINE Bool Joinable() const
        {
            return mState != nullptr;
        }

        /// \brief Threads are not copied, since each one is joined once.
        Ref<Thread> operator=(ConstRef<Thread> Other) = delete;

        /// \brief Takes over the thread another one runs, stopping and joining whatever this one ran.
        ///
        /// \param Other The thread to take from.
        /// \return This thread, now running what the other did.
        Ref<Thread> operator=(AnyRef<Thread> Other);

    public:

        /// \brief Gets the identifier of the calling thread.
        ///
        /// \return The identifier of the thread that makes the call.
        static ThreadID GetID();

        /// \brief Gets how many hardware threads the machine runs at once, every thread of every core counted.
        ///
        /// \return The number of hardware threads, never zero.
        static UInt32 GetProcessors();

        /// \brief Gets how many cores the machine has, counting a core once however many threads it runs.
        ///
        /// \return The number of cores, never zero.
        static UInt32 GetCores();

    private:

        /// \brief The handle the system knows a thread by.
#if   defined(ZY_PLATFORM_WINDOWS)
        using Handle = Ptr<void>;
#else
        using Handle = pthread_t;
#endif

        /// \brief Holds what a running thread shares with its owner, apart from the owner so the owner can move.
        struct State final
        {
            /// The token the work reads.
            Token Signal;

            /// The work the thread runs.
            Work  Body;

            /// \brief Constructs the state of a thread about to start.
            ///
            /// \param Callback The work the thread runs.
            ZY_INLINE explicit State(AnyRef<Work> Callback)
                : Body   { Move(Callback) }
            {
            }

            /// \brief Runs the work on the calling thread, handing it the token.
            ZY_INLINE void Run()
            {
                Body(Signal);
            }
        };

        /// \brief Starts the system thread on the state, which is ready.
        void Start();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle        mHandle;
        Unique<State> mState;
    };
}