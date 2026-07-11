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

#include "Zyphryon.Base/Utility.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Exclusive-ownership smart pointer that manages a heap-allocated object.
    template<class Type>
    class Unique final
    {
        template<class> friend class Unique;

    public:

        /// \brief Constructs an empty unique managing no object.
        ZY_INLINE Unique()
            : mInstance { nullptr }
        {
        }

        /// \brief Constructs a unique taking exclusive ownership of the given heap-allocated instance.
        ///
        /// \param Instance The pointer to the instance to manage. Must be heap-allocated or `nullptr`.
        ZY_INLINE Unique(Ptr<Type> Instance)
            : mInstance { Instance }
        {
        }

        /// \brief Move constructor that transfers ownership from another unique of the same type.
        ///
        /// \param Other The unique to move from. Left empty after the transfer.
        ZY_INLINE Unique(AnyRef<Unique> Other)
            : mInstance { Exchange(Other.mInstance, nullptr) }
        {
        }

        /// \brief Converting move constructor that transfers ownership from a unique of a compatible derived type.
        ///
        /// \param Other The unique of the derived type to move from. Left empty after the transfer.
        template<typename Derived>
        ZY_INLINE Unique(AnyRef<Unique<Derived>> Other)
            requires IsDerived<Type, Derived>
            : mInstance { Exchange(Other.mInstance, nullptr) }
        {
        }

        /// \brief Destructor that deletes the managed instance.
        ZY_INLINE ~Unique()
        {
            Reset();
        }

        /// \brief Deletes the currently managed instance and optionally replaces it with a new instance.
        ///
        /// \param Pointer The new instance to manage, or `nullptr` to leave the unique empty. Defaults to `nullptr`.
        ZY_INLINE void Reset(Ptr<Type> Pointer = nullptr)
        {
            if (mInstance)
            {
               delete mInstance;
            }
            mInstance = Pointer;
        }

        /// \brief Grabs the raw pointer to the managed instance without relinquishing ownership.
        ///
        /// \return The raw pointer to the managed instance, or `nullptr` if the unique is empty.
        ZY_INLINE Ptr<Type> Grab()
        {
            return mInstance;
        }

        /// \brief Releases ownership of the managed instance without deleting it.
        ///
        /// \return A pointer to the previously managed instance, or `nullptr` if the unique was empty.
        ZY_INLINE Ptr<Type> Release()
        {
            return Exchange(mInstance, nullptr);
        }

        /// \brief Deletes the currently managed instance and leaves the unique empty.
        ///
        /// \return A reference to this unique.
        ZY_INLINE Ref<Unique> operator=(nullptr_t)
        {
            Reset();
            return (* this);
        }

        /// \brief Move assignment operator that transfers ownership from another unique of the same type.
        ///
        /// \param Other The unique to move from. Left empty after the transfer.
        /// \return A reference to this unique.
        ZY_INLINE Ref<Unique> operator=(AnyRef<Unique> Other)
        {
            if (this != AddressOf(Other))
            {
                Reset();

                mInstance = Exchange(Other.mInstance, nullptr);
            }
            return (* this);
        }

        /// \brief Accesses members of the managed instance through the unique.
        ///
        /// \return A pointer to the managed instance.
        ZY_INLINE Ptr<Type> operator->() const
        {
            return mInstance;
        }

        /// \brief Dereferences the unique to access the managed instance.
        ///
        /// \return A reference to the managed instance.
        ZY_INLINE Ref<Type> operator*() const
        {
            return (* mInstance);
        }

        /// \brief Checks if the unique is empty.
        ///
        /// \return `true` if the unique manages no instance, `false` otherwise.
        ZY_INLINE Bool operator==(nullptr_t) const
        {
            return mInstance == nullptr;
        }

        /// \brief Checks if the unique is not empty.
        ///
        /// \return `true` if the unique manages an instance, `false` otherwise.
        ZY_INLINE Bool operator!=(nullptr_t) const
        {
            return mInstance != nullptr;
        }

        /// \brief Converts the unique to a boolean indicating whether it manages a valid instance.
        ///
        /// \return `true` if the unique manages a valid instance, `false` otherwise.
        ZY_INLINE explicit operator Bool() const
        {
            return mInstance != nullptr;
        }

    public:

        /// \brief Creates a new unique managing a newly heap-allocated instance constructed with the given arguments.
        ///
        /// \param Parameters The arguments to forward to the constructor of the managed type.
        /// \return A unique managing the newly created instance.
        template<typename... Arguments>
        ZY_INLINE static Unique Create(AnyRef<Arguments>... Parameters)
        {
            return Unique(new Type(Forward<Arguments>(Parameters)...));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Type> mInstance;
    };
}