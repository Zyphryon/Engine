// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the number naming one entity, which stops naming it once the entity is destroyed.
    class Handle final
    {
    public:

        /// \brief Constructs a handle that names nothing.
        ZY_INLINE constexpr Handle()
            : mValue { 0 }
        {
        }

        /// \brief Constructs a handle from the number it is saved and sent as.
        ///
        /// \param Value The number, slot in the low half and generation in the high one.
        ZY_INLINE constexpr explicit Handle(UInt64 Value)
            : mValue { Value }
        {
        }

        /// \brief Constructs a handle naming one generation of one slot.
        ///
        /// \param Index      The slot the entity sits in, never zero.
        /// \param Generation The generation of that slot.
        ZY_INLINE constexpr Handle(UInt32 Index, UInt32 Generation)
            : mValue { (static_cast<UInt64>(Generation) << 32) | Index }
        {
        }

        /// \brief Checks whether the handle names anything at all.
        ///
        /// \return `true` if it names an entity, alive or not, `false` if it names nothing.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mValue != 0;
        }

        /// \brief Gets the slot the entity sits in.
        ///
        /// \return The slot.
        ZY_INLINE constexpr UInt32 GetIndex() const
        {
            return static_cast<UInt32>(mValue);
        }

        /// \brief Gets the generation of the slot this handle names.
        ///
        /// \return The generation.
        ZY_INLINE constexpr UInt32 GetGeneration() const
        {
            return static_cast<UInt32>(mValue >> 32);
        }

        /// \brief Gets the handle as one number, the form it is saved and sent in.
        ///
        /// \return The handle's value.
        ZY_INLINE constexpr UInt64 GetValue() const
        {
            return mValue;
        }

        /// \brief Checks whether two handles name the same generation of the same slot.
        ///
        /// \param Other The handle to compare against.
        /// \return `true` if both name the same entity, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(ConstRef<Handle> Other) const = default;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64 mValue;
    };
}