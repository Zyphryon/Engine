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

#include "Zyphryon.Base/Concept.hpp"
#include "Zyphryon.Base/Utility.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Binary reader for in-memory data.
    class Reader final
    {
    public:

        /// \brief Creates an empty reader.
        ZYPHRYON_INLINE Reader() noexcept
            : mData   { nullptr },
              mSize   { 0 },
              mOffset { 0 }
        {
        }

        /// \brief Creates a reader from a pointer and element count.
        ///
        /// \param Pointer Address of the first element to read from.
        /// \param Length  Number of elements available at Pointer.
        template<typename Type>
        ZYPHRYON_INLINE Reader(ConstPtr<Type> Pointer, UInt32 Length)
            : mData   { reinterpret_cast<ConstPtr<Byte>>(Pointer) },
              mSize   { Length * static_cast<UInt32>(sizeof(Type)) },
              mOffset { 0 }
        {
            LOG_ASSERT(Pointer || Length == 0, "Null pointer with non-zero length");
        }

        /// \brief Constructs a reader from a span.
        ///
        /// \param Block A span representing the block of memory.
        ZYPHRYON_INLINE explicit Reader(ConstSpan<Byte> Block)
            : Reader(Block.data(), Block.size_bytes())
        {
        }

        /// \brief Retrieves a span covering the entire data block.
        ///
        /// \return A span that covers all the data held by the reader.
        ZYPHRYON_INLINE ConstSpan<Byte> GetData() const
        {
            return ConstSpan(mData, mSize);
        }

        /// \brief Gets the total capacity of the data.
        ///
        /// \return The total number of bytes in the data block.
        ZYPHRYON_INLINE UInt32 GetCapacity() const
        {
            return mSize;
        }

        /// \brief Gets the number of bytes still available for reading.
        ///
        /// \return The number of bytes available (capacity minus current offset).
        ZYPHRYON_INLINE UInt32 GetAvailable() const
        {
            return GetCapacity() - mOffset;
        }

        /// \brief Retrieves the current read offset.
        ///
        /// \return The current offset from the beginning of the data.
        ZYPHRYON_INLINE UInt32 GetOffset() const
        {
            return mOffset;
        }

        /// \brief Advances the read offset by a specified number of bytes.
        ///
        /// \param Length The number of bytes to skip.
        ZYPHRYON_INLINE void Skip(UInt32 Length)
        {
            LOG_ASSERT(mOffset + Length <= mSize, "Attempted to access beyond available data");

            mOffset += Length;
        }

        /// \brief Splits a portion of the data from the current offset.
        ///
        /// \param Length The number of bytes to split off.
        /// \return A new reader starting at the original offset.
        ZYPHRYON_INLINE Reader Split(UInt32 Length)
        {
            const UInt32 Offset   = mOffset;
            Skip(Length);
            return Reader(ConstSpan(mData + Offset, Length));
        }

        /// \brief Peeks at data without advancing the offset.
        ///
        /// \param Length The number of bytes to interpret.
        /// \return The value read from current offset, or default if insufficient data.
        template<typename Type, UInt32 Alignment = 1>
        ZYPHRYON_INLINE Type Peek(UInt32 Length = sizeof(Type))
        {
            UInt32 Offset;

            if constexpr (Alignment > 1)
            {
                Offset = Align(mOffset, Alignment);
            }
            else
            {
                Offset = mOffset;
            }

            if (Offset + Length > mSize)
            {
                LOG_WARNING("Attempted to read beyond available data");
                return Type { };
            }

            if constexpr (IsPointer<Type>)
            {
                return reinterpret_cast<Type>(mData + Offset);
            }
            else
            {
                static_assert(IsTrivial<Type>, "Read<Type> requires trivially copyable types");

                Type Value;
                std::memcpy(std::addressof(Value), mData + Offset, Length);
                return Value;
            }
        }

        /// \brief Reads data and advances the offset.
        ///
        /// \param Length The number of bytes to read.
        /// \return The value read from the current offset.
        template<typename Type, UInt32 Alignment = 1>
        ZYPHRYON_INLINE Type Read(UInt32 Length = sizeof(Type))
        {
            const Type Result = Peek<Type, Alignment>(Length);

            if constexpr (Alignment > 1)
            {
                mOffset = Align(mOffset, Alignment) + Length;
            }
            else
            {
                mOffset += Length;
            }
            return Result;
        }

        /// \brief Reads a Boolean value from the data.
        ///
        /// \return A Bool value read from the current offset.
        ZYPHRYON_INLINE Bool ReadBool()
        {
            return Read<Bool>();
        }

        /// \brief Reads an enumerated type value.
        ///
        /// \return The enumerated value.
        template<typename Type>
        ZYPHRYON_INLINE Type ReadEnum()
        {
            return static_cast<Type>(ReadUInt<std::underlying_type_t<Type>>());
        }

        /// \brief Reads a variable-length encoded signed integer.
        ///
        /// \return The decoded signed integer value.
        template<typename Type>
        ZYPHRYON_INLINE Type ReadInt()
        {
            using Unsigned = std::make_unsigned_t<Type>;

            const Unsigned Value = ReadUInt<Unsigned>();
            return static_cast<Type>((Value >> 1) ^ -(Value & 1));
        }

        /// \brief Reads a variable-length encoded unsigned integer.
        ///
        /// \return The decoded unsigned integer value.
        template<typename Type>
        ZYPHRYON_INLINE Type ReadUInt()
        {
            Type Result   = 0u;

            for (UInt32 Position = 0, Limit = (sizeof(Type) * 8u + 6u) / 7u; Position < Limit; ++Position)
            {
                const UInt8 Byte = ReadUInt8();
                Result          |= static_cast<Type>(Byte & 0x7Fu) << (7u * Position);

                if ((Byte & 0x80u) == 0)
                {
                    break;
                }
            }
            return Result;
        }

        /// \brief Reads an 8-bit signed integer.
        ///
        /// \return An 8-bit signed integer value.
        ZYPHRYON_INLINE SInt8 ReadInt8()
        {
            return Read<SInt8>();
        }

        /// \brief Reads an 8-bit unsigned integer.
        ///
        /// \return An 8-bit unsigned integer value.
        ZYPHRYON_INLINE UInt8 ReadUInt8()
        {
            return Read<UInt8>();
        }

        /// \brief Reads a 16-bit signed integer.
        ///
        /// \return A 16-bit signed integer value.
        ZYPHRYON_INLINE SInt16 ReadInt16()
        {
            return Read<SInt16>();
        }

        /// \brief Reads a 16-bit unsigned integer.
        ///
        /// \return A 16-bit unsigned integer value.
        ZYPHRYON_INLINE UInt16 ReadUInt16()
        {
            return Read<UInt16>();
        }

        /// \brief Reads a 32-bit signed integer.
        ///
        /// \return A 32-bit signed integer value.
        ZYPHRYON_INLINE SInt32 ReadInt32()
        {
            return Read<SInt32>();
        }

        /// \brief Reads a 32-bit unsigned integer.
        ///
        /// \return A 32-bit unsigned integer value.
        ZYPHRYON_INLINE UInt32 ReadUInt32()
        {
            return Read<UInt32>();
        }

        /// \brief Reads a 64-bit signed integer.
        ///
        /// \return A 64-bit signed integer value.
        ZYPHRYON_INLINE SInt64 ReadInt64()
        {
            return Read<SInt64>();
        }

        /// \brief Reads a 64-bit unsigned integer.
        ///
        /// \return A 64-bit unsigned integer value.
        ZYPHRYON_INLINE UInt64 ReadUInt64()
        {
            return Read<UInt64>();
        }

        /// \brief Reads a 32-bit floating-point number.
        ///
        /// \return A 32-bit floating-point number.
        ZYPHRYON_INLINE Real32 ReadReal32()
        {
            return Read<Real32>();
        }

        /// \brief Reads a 64-bit floating-point number.
        ///
        /// \return A 64-bit floating-point number.
        ZYPHRYON_INLINE Real64 ReadReal64()
        {
            return Read<Real64>();
        }

        /// \brief Reads a string preceded by its length.
        ///
        /// \return A view over the read string.
        ZYPHRYON_INLINE ConstStr8 ReadText()
        {
            const UInt32 Size = ReadUInt<UInt32>();
            const auto Data = Read<ConstPtr<Char>>(Size);
            return ConstStr8(Data, Data ? Size : 0);
        }

        /// \brief Reads a plain old data (POD) object from the buffer with proper alignment.
        ///
        /// \tparam Type The POD type to read (must be trivially copyable).
        /// \return A view over the POD object.
        template<typename Type>
        ZYPHRYON_INLINE ConstPtr<Type> ReadStruct()
        {
            return Read<ConstPtr<Type>, alignof(Type)>(sizeof(Type));
        }

        /// \brief Reads a contiguous block of trivially copyable elements from the buffer.
        ///
        /// \return A view of the contiguous block.
        template<typename Size, typename Type>
        ZYPHRYON_INLINE ConstSpan<Type> ReadBlock()
        {
            const Size Length = Read<Size>();

            if (Length > GetAvailable() / sizeof(Type))
            {
                LOG_WARNING("Trying to read block larger than available data");
                return ConstSpan<Type>();
            }

            const auto Data = Read<ConstPtr<Type>>(Length * sizeof(Type));
            return ConstSpan<Type>(Data, Data ? Length : 0);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstPtr<Byte> mData;
        UInt32         mSize;
        UInt32         mOffset;
    };
}