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

#include "Mutex.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Represents a condition that blocks one or more threads until it is signaled.
    class ZY_API Gate final
    {
    public:

        /// \brief Constructs a gate no thread waits on.
        Gate();

        /// \brief Lets go of the gate, which no thread may wait on.
        ~Gate();

        /// \brief Gates are not copied, since every thread has to wait on the same one.
        Gate(ConstRef<Gate> Other) = delete;

        /// \brief Blocks until the gate is signaled, giving the mutex back while blocked.
        ///
        /// \param Barrier The lock, which holds its mutex and holds it again on return.
        void Wait(Ref<Lock> Barrier);

        /// \brief Blocks until a condition holds, giving the mutex back while blocked.
        ///
        /// \param Barrier   The lock, which holds its mutex and holds it again on return.
        /// \param Condition The predicate that ends the wait, checked with the mutex held.
        template<typename Predicate>
        ZY_INLINE void Wait(Ref<Lock> Barrier, AnyRef<Predicate> Condition)
        {
            // A waiter can wake with nothing changed, so the condition is checked again every time.
            while (!Condition())
            {
                Wait(Barrier);
            }
        }

        /// \brief Wakes one thread waiting on the gate.
        void Signal();

        /// \brief Wakes every thread waiting on the gate.
        void Broadcast();

        /// \brief Gates are not copied, since every thread has to wait on the same one.
        Ref<Gate> operator=(ConstRef<Gate> Other) = delete;

    private:

        /// \brief The condition the system knows the gate by.
#if   defined(ZY_PLATFORM_WINDOWS)
        using Handle = Ptr<void>;
#else
        using Handle = pthread_cond_t;
#endif

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle mHandle;
    };
}