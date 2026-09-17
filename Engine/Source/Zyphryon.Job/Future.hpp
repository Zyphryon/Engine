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

#include "Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyJob
{
    /// \brief Represents the value a job will produce, held in place where its owner keeps it until the job is done.
    template<typename Type>
    class Future final
    {
        friend class Service;

    public:

        /// \brief Constructs a future no job has been launched into.
        ZY_INLINE Future()
            : mService { nullptr },
              mJob     { },
              mValue   { }
        {
        }

        /// \brief Destroys the future, waiting for its job first, since the job writes into the future itself.
        ZY_INLINE ~Future()
        {
            Wait();
        }

        /// \brief Deleted copy constructor, since a job writes into one place alone.
        Future(ConstRef<Future>) = delete;

        /// \brief Deleted move constructor, since a job writes into one place alone.
        Future(AnyRef<Future>) = delete;

        /// \brief Checks whether a job is at work on the value.
        ///
        /// \return `true` while a job launched into the future has not been waited for, otherwise `false`.
        ZY_INLINE Bool IsPending() const
        {
            return static_cast<Bool>(mJob);
        }

        /// \brief Checks whether the value may be taken without waiting.
        ///
        /// \return `true` when the job is done or there is none, otherwise `false`.
        ZY_INLINE Bool IsReady() const
        {
            return !mJob || mService->IsComplete(mJob);
        }

        /// \brief Waits for the job, executing other jobs meanwhile, and releases it.
        ZY_INLINE void Wait()
        {
            if (mJob)
            {
                mService->Wait(mJob);

                mJob = Handle();
            }
        }

        /// \brief Gets the value, waiting for the job first when it is not done.
        ///
        /// \return A reference to the value, which the future keeps.
        ZY_INLINE Ref<Type> Get()
        {
            Wait();

            return mValue;
        }

        /// \brief Takes the value out, waiting for the job first when it is not done.
        ///
        /// \return The value, moved out of the future.
        ZY_INLINE Type Take()
        {
            return Move(Get());
        }

        /// \brief Deleted copy assignment, since a job writes into one place alone.
        Ref<Future> operator=(ConstRef<Future>) = delete;

        /// \brief Deleted move assignment, since a job writes into one place alone.
        Ref<Future> operator=(AnyRef<Future>) = delete;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Service> mService;
        Handle       mJob;
        Type         mValue;
    };
}