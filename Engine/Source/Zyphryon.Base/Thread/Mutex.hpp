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

#if defined(ZY_PLATFORM_POSIX)
#include <pthread.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Represents a mutex guarding shared resources, which one thread holds at a time.
    class ZY_API Mutex final
    {
        friend class Gate;

    public:

        /// \brief Constructs a mutex no thread holds.
        Mutex();

        /// \brief Lets go of the mutex, which no thread may hold.
        ~Mutex();

        /// \brief Mutexes are not copied, since every thread has to take the same one.
        Mutex(ConstRef<Mutex> Other) = delete;

        /// \brief Takes the mutex, blocking while another thread holds it.
        void Acquire();

        /// \brief Gives the mutex back, which the calling thread holds.
        void Release();

        /// \brief Mutexes are not copied, since every thread has to take the same one.
        Ref<Mutex> operator=(ConstRef<Mutex> Other) = delete;

    private:

        /// \brief The lock the system knows the mutex by.
#if   defined(ZY_PLATFORM_WINDOWS)
        using Handle = Ptr<void>;
#else
        using Handle = pthread_mutex_t;
#endif

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle mHandle;
    };

    /// \brief Represents a hold on a \ref Mutex for as long as the guard lives.
    class Guard final
    {
    public:

        /// \brief Constructs a guard taking a mutex.
        ///
        /// \param Target The mutex to hold.
        ZY_INLINE explicit Guard(Ref<Mutex> Target)
            : mTarget { Target }
        {
            mTarget.Acquire();
        }

        /// \brief Gives the mutex back.
        ZY_INLINE ~Guard()
        {
            mTarget.Release();
        }

        /// \brief Guards are not copied, since each one gives its mutex back once.
        Guard(ConstRef<Guard> Other) = delete;

        /// \brief Guards are not copied, since each one gives its mutex back once.
        Ref<Guard> operator=(ConstRef<Guard> Other) = delete;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Mutex> mTarget;
    };

    /// \brief Represents a hold on a \ref Mutex that can be given back and taken again, the one a \ref Gate waits on.
    class Lock final
    {
    public:

        /// \brief Constructs a lock taking a mutex.
        ///
        /// \param Target The mutex to hold.
        ZY_INLINE explicit Lock(Ref<Mutex> Target)
            : mTarget { Target },
              mHeld   { true }
        {
            mTarget.Acquire();
        }

        /// \brief Gives the mutex back, if the lock still holds it.
        ZY_INLINE ~Lock()
        {
            if (mHeld)
            {
                mTarget.Release();
            }
        }

        /// \brief Locks are not copied, since each one gives its mutex back once.
        Lock(ConstRef<Lock> Other) = delete;

        /// \brief Takes the mutex again, after the lock gave it back.
        ZY_INLINE void Acquire()
        {
            ZY_ASSERT(!mHeld, "The lock already holds its mutex");

            mTarget.Acquire();
            mHeld = true;
        }

        /// \brief Gives the mutex back, keeping the lock to take it again.
        ZY_INLINE void Release()
        {
            ZY_ASSERT(mHeld, "The lock does not hold its mutex");

            mTarget.Release();
            mHeld = false;
        }

        /// \brief Gets the mutex the lock takes.
        ///
        /// \return The mutex, held or not.
        ZY_INLINE Ref<Mutex> GetMutex() const
        {
            return mTarget;
        }

        /// \brief Locks are not copied, since each one gives its mutex back once.
        Ref<Lock> operator=(ConstRef<Lock> Other) = delete;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Mutex> mTarget;
        Bool       mHeld;
    };
}