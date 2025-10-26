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

#include "Primitive.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Bit
{
    /// \brief Extracts specific bits from a field based on an offset and mask.
    ///
    /// \param Field  The field from which to extract bits.
    /// \param Offset The bit offset to start extraction.
    /// \param Mask   The bitmask indicating which bits to extract.
    /// \return The extracted bits after applying the offset and mask.
    template<typename Type>
    constexpr auto GetBit(Type Field, auto Offset, auto Mask)
    {
        return (Field >> Offset) & Mask;
    }

    /// \brief Sets specific bits in a field at a given offset using a mask and data.
    ///
    /// \param Field  The original field value to modify.
    /// \param Offset The bit offset where the data should be set.
    /// \param Mask   The bitmask indicating which bits to set.
    /// \param Data   The data to set in the specified bits.
    /// \return A new value with the specified bits set.
    template<typename Type>
    constexpr auto SetBit(Type Field, auto Offset, auto Mask, auto Data)
    {
        return (Field & ~(Mask << Offset)) | ((Data & Mask) << Offset);
    }

    /// \brief Checks whether all bits in the given mask are set in the field.
    /// 
    /// \param Field The field to check.
    /// \param Mask  The bitmask indicating which bits to compare.
    /// \return `true` if all bits in `Mask` are set in `Field`, `false` otherwise.
    template<typename Type, typename Value>
    constexpr Bool HasBit(Type Field, Value Mask)
    {
        return (Field & Mask) == Mask;
    }

    /// \brief Sets one or more bits in the field.
    /// 
    /// \param Field The original field value to modify.
    /// \param Mask  The bitmask indicating which bits to set.
    /// \return A new value with the specified bits set.
    template<typename Type, typename Value>
    constexpr Type SetBit(Type Field, Value Mask)
    {
        return Field | Mask;
    }

    /// \brief Clears one or more bits in the field.
    /// 
    /// \param Field The original field value to modify.
    /// \param Mask  The bitmask indicating which bits to clear.
    /// \return A new value with the specified bits cleared.
    template<typename Type, typename Value>
    constexpr Type ClearBit(Type Field, Value Mask)
    {
        return Field & ~Mask;
    }

    /// \brief Conditionally sets or clears bits in a field based on a boolean flag.
    ///
    /// \param Field  The original field value to modify.
    /// \param Mask   The bitmask indicating which bits to set or clear.
    /// \param Enable If `true`, the bits specified by the mask are set, otherwise are cleared.
    /// \return A new value with the specified bits either set or cleared based on the condition.
    template<typename Type, typename Value>
    constexpr Type SetOrClearBit(Type Field, Value Mask, Bool Enable)
    {
        return (Enable ? SetBit(Field, Mask) : ClearBit(Field, Mask));
    }

    /// \brief Retrieves a pointer with specific lower bits cleared based on architecture.
    ///
    /// \note This is useful for pointer tagging schemes where lower bits are used for metadata.
    ///
    /// \param Pointer The original pointer.
    /// \return The pointer with lower bits cleared.
    template<typename Type>
    constexpr auto GetPointer(Ptr<Type> Pointer)
    {
        if constexpr (ZYPHRYON_CPU_BIT == 64)
        {
            // Clear the lower 4 bits of the pointer for 64-bit architectures.
            return reinterpret_cast<Ptr<Type>>(ClearBit(reinterpret_cast<UInt64>(Pointer), 0xFull));
        }
        else
        {
            // Clear the lower 2 bits of the pointer for 32-bit architectures.
            return reinterpret_cast<Ptr<Type>>(ClearBit(reinterpret_cast<UInt32>(Pointer), 0x3u));
        }
    }

    /// \brief Extracts specific bits from a pointer based on offset and mask.
    ///
    /// \note This is useful for pointer tagging schemes where lower bits are used for metadata.
    ///
    /// \param Pointer The pointer from which to extract bits.
    /// \param Offset  The bit offset to start extraction.
    /// \param Mask    The bitmask indicating which bits to extract.
    /// \return The extracted bits after applying the offset and mask.
    template<typename Type>
    constexpr auto GetBit(Ptr<Type> Pointer, auto Offset, auto Mask)
    {
        if constexpr (ZYPHRYON_CPU_BIT ==64)
        {
            // Extract specific bits from a pointer based on offset and mask for 64-bit architectures.
            return GetBit(reinterpret_cast<UInt64>(Pointer), Offset, Mask);
        }
        else
        {
            // Extract specific bits from a pointer based on offset and mask for 32-bit architectures.
            return GetBit(reinterpret_cast<UInt32>(Pointer), Offset, Mask);
        }
    }

    /// \brief Sets specific bits in a pointer at a given offset using a mask and data.
    ///
    /// \note This is useful for pointer tagging schemes where lower bits are used for metadata.
    ///
    /// \param Pointer The original pointer.
    /// \param Offset  The bit offset where the data should be set.
    /// \param Mask    The bitmask indicating which bits to set.
    /// \param Data    The data to set in the specified bits.
    /// \return A new pointer with the specified bits set.
    template<typename Type>
    constexpr auto SetBit(Ptr<Type> Pointer, auto Offset, auto Mask, auto Data)
    {
        if constexpr (ZYPHRYON_CPU_BIT == 64)
        {
            // Set specific bits in a pointer at a given offset using a mask and data for 64-bit architectures.
            return reinterpret_cast<Ptr<Type>>(SetBit(reinterpret_cast<UInt64>(Pointer), Offset, Mask, Data));
        }
        else
        {
            // Set specific bits in a pointer at a given offset using a mask and data for 32-bit architectures.
            return reinterpret_cast<Ptr<Type>>(SetBit(reinterpret_cast<UInt32>(Pointer), Offset, Mask, Data));
        }
    }

    /// \brief Packs two 16-bit values into a 32-bit integer with endian-aware layout.
    /// 
    /// \param Low  The lower 16 bits.
    /// \param High The upper 16 bits.
    /// \return A 32-bit packed integer.
    constexpr UInt32 PackInt16x2(UInt16 Low, UInt16 High)
    {
        if constexpr (ZYPHRYON_LITTLE_ENDIAN)
        {
            return (static_cast<UInt32>(High) << 0x10) | Low;
        }
        else
        {
            return (static_cast<UInt32>(Low) << 0x10) | High;
        }
    }

    /// \brief Extracts the lower 16 bits from a packed 32-bit integer.
    /// 
    /// \param Value A packed 32-bit integer.
    /// \return The lower 16 bits.
    constexpr UInt16 UnpackLowInt16(UInt32 Value)
    {
        if constexpr (ZYPHRYON_LITTLE_ENDIAN)
        {
            return static_cast<UInt16>(Value & 0xFFFF);
        }
        else
        {
            return static_cast<UInt16>(Value >> 0x10);
        }
    }

    /// \brief Extracts the upper 16 bits from a packed 32-bit integer.
    /// 
    /// \param Value A packed 32-bit integer.
    /// \return The upper 16 bits.
    constexpr UInt16 UnpackHighInt16(UInt32 Value)
    {
        if constexpr (ZYPHRYON_LITTLE_ENDIAN)
        {
            return static_cast<UInt16>(Value >> 0x10);
        }
        else
        {
            return static_cast<UInt16>(Value & 0xFFFF);
        }
    }

    /// \brief Packs two 32-bit integers into a 64-bit integer with endian-aware layout.
    /// 
    /// \param Low  The lower 32 bits.
    /// \param High The upper 32 bits.
    /// \return A 64-bit packed integer.
    constexpr UInt64 PackInt32x2(UInt32 Low, UInt32 High)
    {
        if constexpr (ZYPHRYON_LITTLE_ENDIAN)
        {
            return (static_cast<UInt64>(High) << 0x20) | Low;
        }
        else
        {
            return (static_cast<UInt64>(Low) << 0x20) | High;
        }
    }

    /// \brief Extracts the lower 32 bits from a packed 64-bit integer.
    /// 
    /// \param Value A packed 64-bit integer.
    /// \return The lower 32 bits.
    constexpr UInt32 UnpackLowInt32(UInt64 Value)
    {
        if constexpr (ZYPHRYON_LITTLE_ENDIAN)
        {
            return static_cast<UInt32>(Value & 0xFFFFFFFF);
        }
        else
        {
            return static_cast<UInt32>(Value >> 0x20);
        }
    }

    /// \brief Extracts the upper 32 bits from a packed 64-bit integer.
    /// 
    /// \param Value A packed 64-bit integer.
    /// \return The upper 32 bits.
    constexpr UInt32 UnpackHighInt32(UInt64 Value)
    {
        if constexpr (ZYPHRYON_LITTLE_ENDIAN)
        {
            return static_cast<UInt32>(Value >> 0x20);
        }
        else
        {
            return static_cast<UInt32>(Value & 0xFFFFFFFF);
        }
    }
}