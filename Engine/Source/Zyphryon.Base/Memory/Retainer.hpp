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
    /// \brief Intrusive reference-counted smart pointer managing a `Retainable` object.
    template<class Type>
    class Retainer final
    {
        template<typename> friend class Retainer;

    public:

        /// \brief Constructs an empty retainer managing no object.
        ZY_INLINE Retainer()
            : mInstance { nullptr }
        {
        }

        /// \brief Constructs an empty retainer explicitly from a null pointer.
        ZY_INLINE Retainer(nullptr_t)
            : mInstance { nullptr }
        {
        }

        /// \brief Constructs a retainer taking ownership of the given heap-allocated instance.
        ///
        /// \param Instance The pointer to the instance to manage. Must be heap-allocated.
        template<typename Derived>
        ZY_INLINE explicit Retainer(Ptr<Derived> Instance)
            requires IsDerived<Type, Derived>
            : mInstance { static_cast<Ptr<Type>>(Instance) }
        {
            Retain();
        }

        /// \brief Copy constructor that shares ownership with another retainer of the same type.
        ///
        /// \param Other The retainer to copy from.
        ZY_INLINE Retainer(ConstRef<Retainer> Other)
            : mInstance { Other.mInstance }
        {
            Retain();
        }

        /// \brief Converting copy constructor that shares ownership from a retainer of a compatible derived type.
        ///
        /// \param Other The retainer of the derived type to copy from.
        template<typename Derived>
        ZY_INLINE Retainer(ConstRef<Retainer<Derived>> Other)
            requires IsDerived<Type, Derived>
            : mInstance { static_cast<Ptr<Type>>(Other.mInstance) }
        {
            Retain();
        }

        /// \brief Move constructor that transfers ownership from another retainer of the same type.
        ///
        /// \param Other The retainer to move from. Left empty after the transfer.
        ZY_INLINE Retainer(AnyRef<Retainer> Other)
            : mInstance { Exchange(Other.mInstance, nullptr) }
        {
        }

        /// \brief Converting move constructor that transfers ownership from a retainer of a compatible derived type.
        ///
        /// \param Other The retainer of the derived type to move from. Left empty after the transfer.
        template<typename Derived>
        ZY_INLINE Retainer(AnyRef<Retainer<Derived>> Other)
            requires IsDerived<Type, Derived>
            : mInstance { static_cast<Ptr<Type>>(Other.mInstance) }
        {
            Other.mInstance = nullptr;
        }

        /// \brief Destructor that releases the managed instance.
        ZY_INLINE ~Retainer()
        {
            Release();
        }

        /// \brief Copy assignment operator that shares ownership from another retainer of the same type.
        ///
        /// \param Other The retainer to copy from.
        /// \return A reference to this retainer.
        ZY_INLINE Ref<Retainer> operator=(ConstRef<Retainer> Other)
        {
            if (this != AddressOf(Other))
            {
                Release();
                mInstance = Other.mInstance;
                Retain();
            }
            return (* this);
        }

        /// \brief Move assignment operator that transfers ownership from another retainer of the same type.
        ///
        /// \param Other The retainer to move from. Left empty after the transfer.
        /// \return A reference to this retainer.
        ZY_INLINE Ref<Retainer> operator=(AnyRef<Retainer> Other)
        {
            if (this != AddressOf(Other))
            {
                Release();
                mInstance = Exchange(Other.mInstance, nullptr);
            }
            return (* this);
        }

        /// \brief Dereferences the retainer to access the managed instance.
        ///
        /// \return A reference to the managed instance.
        ZY_INLINE Ref<Type> operator*() const
        {
            return * mInstance;
        }

        /// \brief Accesses members of the managed instance through the retainer.
        ///
        /// \return A pointer to the managed instance.
        ZY_INLINE Ptr<Type> operator->() const
        {
            return mInstance;
        }

        /// \brief Compares two retainers for equality by checking if they manage the same instance.
        ///
        /// \param Other The retainer to compare with.
        /// \return `true` if both retainers manage the same instance, `false` otherwise.
        ZY_INLINE Bool operator==(ConstRef<Retainer> Other) const
        {
            return mInstance == Other.mInstance;
        }

        /// \brief Compares two retainers for inequality by checking if they manage different instances.
        ///
        /// \param Other The retainer to compare with.
        /// \return `true` if the retainers manage different instances, `false` otherwise.
        ZY_INLINE Bool operator!=(ConstRef<Retainer> Other) const
        {
            return mInstance != Other.mInstance;
        }

        /// \brief Checks if the retainer is empty.
        ///
        /// \return `true` if the retainer manages no instance, `false` otherwise.
        ZY_INLINE Bool operator==(nullptr_t) const
        {
            return mInstance == nullptr;
        }

        /// \brief Checks if the retainer is not empty.
        ///
        /// \return `true` if the retainer manages an instance, `false` otherwise.
        ZY_INLINE Bool operator!=(nullptr_t) const
        {
            return mInstance != nullptr;
        }

        /// \brief Converts the retainer to a boolean indicating whether it manages a valid instance.
        ///
        /// \return `true` if the retainer manages a valid instance, `false` otherwise.
        ZY_INLINE explicit operator Bool() const
        {
            return mInstance != nullptr;
        }

        /// \brief Implicitly converts the retainer to a raw pointer.
        ///
        /// \return A pointer to the managed instance, or `nullptr` if empty.
        ZY_INLINE explicit operator Ptr<Type>() const
        {
            return mInstance;
        }

        /// \brief Implicitly converts the retainer to a raw reference.
        ///
        /// \return A reference to the managed instance.
        ZY_INLINE explicit operator Ref<Type>() const
        {
            return * mInstance;
        }

    private:

        /// \brief Increments the reference count of the managed instance if it is not null.
        ZY_INLINE void Retain()
        {
            if (mInstance)
            {
                mInstance->Retain();
            }
        }

        /// \brief Decrements the reference count of the managed instance and resets the retainer to empty.
        ZY_INLINE void Release()
        {
            if (mInstance)
            {
                mInstance->Release();
                mInstance = nullptr;
            }
        }

    public:

        /// \brief Creates a new retainer managing a newly heap-allocated instance constructed with the given arguments.
        ///
        /// \param Parameters The arguments to forward to the constructor of the managed type.
        /// \return A retainer managing the newly created instance.
        template<typename... Arguments>
        ZY_INLINE static Retainer Create(AnyRef<Arguments>... Parameters)
        {
            return Retainer(new Type(Forward<Arguments>(Parameters)...));
        }

        /// \brief Downcasts a retainer of a base type to a retainer of a derived type.
        ///
        /// \param Other The retainer of the base type to downcast.
        /// \return A retainer managing the same instance, cast to the derived type.
        template<typename Base>
        ZY_INLINE static Retainer Cast(ConstRef<Retainer<Base>> Other)
            requires IsDerived<Base, Type>
        {
            return Retainer(static_cast<Ptr<Type>>(Other.mInstance));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Type> mInstance;
    };

    /// \brief Non-owning constant reference to a `Retainer` instance.
    ///
    /// Used as a parameter type only. Does not increment the reference count and must not outlive
    /// the source retainer.
    template<class Type>
    using ConstRetainer = ConstRef<Retainer<Type>>;
}