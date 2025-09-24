// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Component.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief A pin provides an efficient way to reference and manipulate a specific component instance on an entity.
    template<typename Type>
    class Pin
    {
    public:

        /// \brief Underlying handle type used to represent the pin internally.
        using Handle = flecs::ref<Type>;

    public:

        /// \brief Constructs an empty pin.
        ZYPHRYON_INLINE Pin() = default;

        /// \brief Constructs a pin from an existing handle.
        ///
        /// \param Handle The handle of this pin.
        ZYPHRYON_INLINE explicit Pin(AnyRef<Handle> Handle)
            : mHandle { Move(Handle) }
        {
        }

        /// \brief Retrieves the entity associated with this pin.
        ///
        /// \return The entity associated with this pin.
        ZYPHRYON_INLINE Entity GetEntity() const
        {
            return Entity(mHandle.entity());
        }

        /// \brief Retrieves the component associated with this pin.
        ///
        /// \return The component associated with this pin.
        ZYPHRYON_INLINE Component<Type> GetComponent() const
        {
            return Component<Type>(mHandle.entity());
        }

        /// \brief Marks the component as modified on the associated entity.
        ZYPHRYON_INLINE void Notify() const
        {
            mHandle.entity().modified(mHandle.component());
        }

        /// \brief Accesses the underlying component pointer.
        ///
        /// \return Pointer to the component data.
        ZYPHRYON_INLINE Ptr<Type> operator->()
        {
            return mHandle.get();
        }

        /// \brief Accesses the underlying component pointer
        ///
        /// \return A constant pointer to the component data.
        ZYPHRYON_INLINE ConstPtr<Type> operator->() const
        {
            return mHandle.get();
        }

        /// \brief Dereferences the pin to access the component.
        ///
        /// \return Reference to the component data.
        ZYPHRYON_INLINE Base::Ref<Type> operator*()
        {
            return (* mHandle.get());
        }

        /// \brief Dereferences the pin to access the component.
        ///
        /// \return A constant reference to the component data.
        ZYPHRYON_INLINE ConstRef<Type> operator*() const
        {
            return (* mHandle.get());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle mHandle;
    };
}