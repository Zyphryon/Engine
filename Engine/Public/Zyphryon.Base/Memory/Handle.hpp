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

#include <Zyphryon.Base/Base.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Gameplay
{
    /// \brief Represents a strongly-typed handle for resource identification.
    template<typename Type>
    class Handle final
    {
    public:

        /// \brief Represents an invalid handle value.
        static constexpr Type kInvalid = static_cast<Type>(0);

    public:

        /// \brief Constructs an invalid handle.
        ZYPHRYON_INLINE constexpr Handle()
            : mID { kInvalid }
        {
        }

        /// \brief Constructs a handle with the specified ID.
        ///
        /// \param ID The identifier value for the handle.
        template<typename Other>
        ZYPHRYON_INLINE constexpr Handle(Other ID)
            : mID { static_cast<Type>(ID) }
        {
        }

        /// \brief Checks whether the handle is valid (not equal to `kInvalid`).
        ///
        /// \return `true` if the handle is valid, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsValid() const
        {
            return mID != kInvalid;
        }

        /// \brief Retrieves the underlying ID of the handle.
        ///
        /// \return The identifier value of the handle.
        ZYPHRYON_INLINE constexpr Type GetID() const
        {
            return mID;
        }

        /// \brief Resets the handle to an invalid state.
        ZYPHRYON_INLINE constexpr void Reset()
        {
            mID = kInvalid;
        }

        /// \brief Equality operator to compare two handles.
        ///
        /// \param Other The other handle to compare against.
        /// \return `true` if the handles are equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(Handle Other) const
        {
            return mID == Other.mID;
        }

        /// \brief Inequality operator to compare two handles.
        ///
        /// \param Other The other handle to compare against.
        /// \return `true` if the handles are not equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(Handle Other) const
        {
            return mID != Other.mID;
        }

        /// \brief Computes a hash value for the handle.
        ///
        /// \return The hash value derived from the handle's ID.
        ZYPHRYON_INLINE constexpr UInt64 Hash() const
        {
            return mID;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type mID;
    };
}