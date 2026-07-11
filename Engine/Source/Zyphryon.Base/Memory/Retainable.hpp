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

#include "Zyphryon.Base/Primitive.hpp"
#include "Zyphryon.Base/Concurrency/Atomic.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Base class for reference-counted objects managed by `Retainer<Type>`.
    template<typename Type>
    class Retainable
    {
    public:

        /// \brief Constructs a retainable object with a reference count of zero.
        ZY_INLINE Retainable()
            : mReferences { 0 }
        {
        }

        /// \brief Copying a retainable object is not permitted as it would duplicate reference-count state.
        Retainable(ConstRef<Retainable>) = delete;

        /// \brief Copying a retainable object is not permitted as it would duplicate reference-count state.
        Ref<Retainable> operator=(ConstRef<Retainable>) = delete;

        /// \brief Increments the reference count.
        ZY_INLINE void Retain()
        {
            mReferences.fetch_add(1, std::memory_order_relaxed);
        }

        /// \brief Decrements the reference count and destroys the object if it reaches zero.
        ZY_INLINE void Release()
        {
            if (mReferences.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete static_cast<Ptr<Type>>(this);
            }
        }

        /// \brief Checks whether this object has exactly one active reference.
        ///
        /// \return `true` if the reference count is exactly one, `false` otherwise.
        ZY_INLINE Bool IsUnique() const
        {
            return mReferences.load(std::memory_order_relaxed) == 1;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Atomic<UInt32> mReferences;
    };
}