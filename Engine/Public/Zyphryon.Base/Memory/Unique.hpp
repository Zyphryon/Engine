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
    /// \brief A smart pointer that has unique ownership of a dynamically allocated instance.
    template<class Type, class Destructor = std::default_delete<Type>>
    class Unique final
    {
        template<typename, class> friend class Unique;

    public:

        /// \brief Constructs a new unique object with no managed instance.
        ZYPHRYON_INLINE constexpr Unique()
            : mInstance { nullptr }
        {
        }

        /// \brief Constructs a new unique object managing the specified instance.
        ///
        /// \param Instance The instance to manage.
        ZYPHRYON_INLINE explicit Unique(Ptr<Type> Instance)
            : mInstance { Instance }
        {
        }

        /// \brief Move constructs a new unique object, taking ownership from another.
        ///
        /// \param Other The other unique object to move from.
        ZYPHRYON_INLINE Unique(AnyRef<Unique> Other) noexcept
            : mInstance { Exchange(Other.mInstance, nullptr) }
        {
        }

        /// \brief Move constructs a new unique object from a derived unique object, taking ownership from it.
        ///
        /// \param Other The other unique object to move from.
        template<typename Derived>
        ZYPHRYON_INLINE Unique(AnyRef<Unique<Derived>> Other) noexcept
            requires IsDerived<Type, Derived>
            : mInstance { Exchange(Other.mInstance, nullptr) }
        {
        }

        /// \brief Copy construction is deleted to enforce unique ownership.
        Unique(ConstRef<Unique>)                = delete;

        /// \brief Copy assignment is deleted to enforce unique ownership.
        Ref<Unique> operator=(ConstRef<Unique>) = delete;

        /// \brief Destroys the unique object and releases the managed instance if any.
        ZYPHRYON_INLINE ~Unique() noexcept
        {
            Reset();
        }

        /// \brief Resets the unique object, releasing the managed instance if any.
        ///
        /// \param Pointer The new instance to manage, or `nullptr` to release ownership.
        ZYPHRYON_INLINE void Reset(Ptr<Type> Pointer = nullptr) noexcept
        {
            if (mInstance)
            {
                Destructor()(mInstance);
            }
            mInstance = Pointer;
        }

        /// \brief Releases ownership of the managed instance without destroying it.
        ///
        /// \return Pointer to the released instance.
        ZYPHRYON_INLINE Ptr<Type> Release() noexcept
        {
            return Exchange(mInstance, nullptr);
        }


        /// \brief Assigns nullptr to the unique object, releasing any managed instance.
        ///
        /// \return A reference to the updated unique object.
        ZYPHRYON_INLINE Ref<Unique> operator=(std::nullptr_t) noexcept
        {
            Reset();
            return (* this);
        }

        /// \brief Move assigns another unique object, taking ownership from it.
        ///
        /// \param Other The other unique object to move from.
        /// \return A reference to the updated unique object.
        ZYPHRYON_INLINE Ref<Unique> operator=(AnyRef<Unique> Other) noexcept
        {
            if (this != &Other)
            {
                Reset();

                mInstance = Exchange(Other.mInstance, nullptr);
            }
            return (* this);
        }

        /// \brief Gets the managed instance pointer.
        ///
        /// \return Pointer to the managed instance.
        ZYPHRYON_INLINE Ptr<Type> operator->() const noexcept
        {
            return mInstance;
        }

        /// \brief Gets a reference to the managed instance.
        ///
        /// \return A reference to the managed instance.
        ZYPHRYON_INLINE Ref<Type> operator*() const
        {
            return (* mInstance);
        }

        /// \brief Checks if the unique object is managing a null instance.
        ///
        /// \return `true` if managing a null instance, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(std::nullptr_t) const
        {
            return mInstance == nullptr;
        }

        /// \brief Checks if the unique object is managing a non-null instance.
        ///
        /// \return `true` if managing a non-null instance, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(std::nullptr_t) const
        {
            return mInstance != nullptr;
        }

        /// \brief Checks if the unique object is managing an instance.
        ///
        /// \return `true` if managing an instance, `false` otherwise.
        ZYPHRYON_INLINE constexpr operator Bool() const
        {
            return mInstance != nullptr;
        }

    public:

        /// \brief Creates a new unique object managing a newly constructed instance.
        ///
        /// \param Parameters The constructor arguments to forward.
        /// \return A new unique object managing the constructed instance.
        template<typename... Arguments>
        ZYPHRYON_INLINE static Unique Create(AnyRef<Arguments>... Parameters)
        {
            return Unique(new Type(Forward<Arguments>(Parameters)...));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Type> mInstance;
    };
}