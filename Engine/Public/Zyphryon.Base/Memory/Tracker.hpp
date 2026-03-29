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

#include "Trackable.hpp"
#include "Zyphryon.Base/Concept.hpp"
#include "Zyphryon.Base/Utility.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A tracker that manages the lifetime of an instance through reference counting.
    template<class Type>
    class Tracker final
    {
        template<typename> friend class Tracker;

    public:

        /// \brief Default constructor that initializes the tracker with a null pointer.
        ZYPHRYON_INLINE constexpr Tracker()
            : mInstance { nullptr }
        {
        }

        /// \brief Constructs a tracker with a null pointer.
        ZYPHRYON_INLINE constexpr Tracker(std::nullptr_t)
            : mInstance { nullptr }
        {
        }

        /// \brief Constructs a tracker that takes ownership of the provided instance.
        ///
        /// \param Instance The instance to be managed by the tracker.
        template<typename Derived>
        ZYPHRYON_INLINE Tracker(Ptr<Derived> Instance)
            requires IsDerived<Type, Derived>
            : mInstance(static_cast<Ptr<Type>>(Instance))
        {
            Retain();
        }

        /// \brief Copy constructor that creates a new tracker sharing ownership of the same instance.
        ///
        /// \param Other The other tracker to copy from.
        ZYPHRYON_INLINE Tracker(ConstRef<Tracker> Other)
            : mInstance(Other.mInstance)
        {
            Retain();
        }

        /// \brief Copy constructor that creates a new tracker sharing ownership of the same instance.
        ///
        /// \param Other The other tracker to copy from.
        template<typename Derived>
        ZYPHRYON_INLINE Tracker(ConstRef<Tracker<Derived>> Other)
            requires IsDerived<Type, Derived>
            : mInstance(static_cast<Ptr<Type>>(Other.mInstance))
        {
            Retain();
        }

        /// \brief Move constructor that transfers ownership from another tracker.
        ///
        /// \param Other The other tracker to move from.
        template<typename Derived>
        ZYPHRYON_INLINE Tracker(AnyRef<Tracker<Derived>> Other) noexcept
            requires IsDerived<Type, Derived>
            : mInstance(static_cast<Ptr<Type>>(Other.mInstance))
        {
            Other.mInstance = nullptr;
        }

        /// \brief Destructor that releases the managed instance.
        ZYPHRYON_INLINE ~Tracker()
        {
            Release();
        }

        /// \brief Copy assignment operator that shares ownership of the same instance.
        ///
        /// \param Other The other tracker to copy from.
        /// \return A reference to this tracker.
        ZYPHRYON_INLINE Ref<Tracker> operator=(ConstRef<Tracker> Other)
        {
            if (this != &Other)
            {
                // Releases the current reference, if any.
                Release();

                // Assigns the pointer from another intrusive pointer (shared ownership).
                mInstance = Other.mInstance;

                // Increments the reference count on the newly assigned instance.
                Retain();
            }
            return (* this);
        }

        /// \brief Move assignment operator that transfers ownership from another tracker.
        ///
        /// \param Other The other tracker to move from.
        /// \return A reference to this tracker.
        ZYPHRYON_INLINE Ref<Tracker> operator=(AnyRef<Tracker> Other) noexcept
        {
            if (this != &Other)
            {
                // Releases the current reference, if any.
                Release();

                // Transfers the pointer from the other instance and nulls it in the source.
                mInstance = Exchange(Other.mInstance, nullptr);
            }
            return (* this);
        }

        /// \brief Dereference operator to access the managed instance.
        ///
        /// \return A reference to the managed instance.
        ZYPHRYON_INLINE constexpr Ref<Type> operator*() const
        {
            return * mInstance;
        }

        /// \brief Arrow operator to access members of the managed instance.
        ///
        /// \return A pointer to the managed instance.
        ZYPHRYON_INLINE constexpr Ptr<Type> operator->() const
        {
            return mInstance;
        }

        /// \brief Equality operator to compare two trackers.
        ///
        /// \param Other The other tracker to compare with.
        /// \return `true` if both trackers manage the same instance, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(ConstRef<Tracker> Other) const
        {
            return mInstance == Other.mInstance;
        }

        /// \brief Inequality operator to compare two trackers.
        ///
        /// \param Other The other tracker to compare with.
        /// \return `true` if both trackers manage different instances, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(ConstRef<Tracker> Other) const
        {
            return mInstance != Other.mInstance;
        }

        /// \brief Equality operator to compare the tracker with a null pointer.
        ///
        /// \return `true` if the tracker manages a null instance, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(std::nullptr_t) const
        {
            return mInstance == nullptr;
        }

        /// \brief Inequality operator to compare the tracker with a null pointer.
        ///
        /// \return `true` if the tracker manages a non-null instance, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(std::nullptr_t) const
        {
            return mInstance != nullptr;
        }

        /// \brief Boolean conversion operator to check if the tracker manages a valid instance.
        ///
        /// \return `true` if the tracker manages a non-null instance, `false` otherwise.
        ZYPHRYON_INLINE constexpr operator Bool() const
        {
            return mInstance != nullptr;
        }

        ZYPHRYON_INLINE constexpr operator Ptr<Type>()
        {
            return mInstance;
        }

        ZYPHRYON_INLINE constexpr operator ConstPtr<Type>() const
        {
            return mInstance;
        }

    private:

        /// \brief Increments the reference count of the managed instance.
        ZYPHRYON_INLINE void Retain()
        {
            if (mInstance)
            {
                mInstance->Retain();
            }
        }

        /// \brief Decrements the reference count of the managed instance and releases it if necessary.
        ZYPHRYON_INLINE void Release()
        {
            if (mInstance)
            {
                mInstance->Release();
                mInstance = nullptr;
            }
        }

    public:

        /// \brief Creates a new tracker managing a newly constructed instance of the specified type.
        ///
        /// \param Parameters The constructor arguments to forward.
        /// \return A tracker managing the newly created instance.
        template<typename... Arguments>
        ZYPHRYON_INLINE static Tracker Create(AnyRef<Arguments>... Parameters)
        {
            return Tracker(new Type(Forward<Arguments>(Parameters)...));
        }

        /// \brief Casts a tracker of a base type to a tracker of a derived type.
        ///
        /// \param Other The other tracker to cast.
        /// \return A tracker managing the same instance, cast to the derived type.
        template<typename Base>
        ZYPHRYON_INLINE static Tracker Cast(ConstRef<Tracker<Base>> Other)
            requires IsDerived<Base, Type>
        {
            return Tracker(reinterpret_cast<Ptr<Type>>(Other.mInstance));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Type> mInstance;
    };

    /// \brief A const tracker that manages an instance of the specified type.
    template<class Type>
    using ConstTracker = ConstRef<Tracker<Type>>;
}