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

#include "Zyphryon.Base/Lexical/Text.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Encapsulates a binary reader for deserialization.
    class Reader final
    {
    public:

        /// \brief Default constructs an empty reader.
        ZY_INLINE Reader()
            : mData   { nullptr },
              mSize   { 0 },
              mOffset { 0 }
        {
        }

        /// \brief Constructs a reader over the given data buffer.
        ///
        /// \param Data The pointer to the start of the data buffer.
        /// \param Size The size of the data buffer in bytes.
        ZY_INLINE Reader(ConstPtr<Byte> Data, UInt32 Size)
            : mData   { Data },
              mSize   { Size },
              mOffset { 0 }
        {
            ZY_ASSERT(Data && Size > 0, "Invalid data or size.");
        }

        /// \brief Gets the underlying data pointer.
        ///
        /// \return The constant pointer to the data buffer.
        ZY_INLINE ConstPtr<Byte> GetData() const
        {
            return mData;
        }

        /// \brief Gets the total size of the data buffer.
        ///
        /// \return The size in bytes.
        ZY_INLINE UInt32 GetSize() const
        {
            return mSize;
        }

        /// \brief Gets the number of bytes remaining to read.
        ///
        /// \return The number of available bytes.
        ZY_INLINE UInt32 GetAvailable() const
        {
            return mSize - mOffset;
        }

        /// \brief Gets the current read offset.
        ///
        /// \return The offset in bytes from the start of the buffer.
        ZY_INLINE UInt32 GetOffset() const
        {
            return mOffset;
        }

        /// \brief Advances the read offset by the specified length.
        ///
        /// \param Length The number of bytes to skip.
        ZY_INLINE void Skip(UInt32 Length)
        {
            ZY_ASSERT(mOffset + Length <= mSize, "Attempted to access beyond available data");

            mOffset += Length;
        }

        /// \brief Creates a new reader representing a sub‑range of the current buffer.
        ///
        /// \param Length The length of the sub‑range in bytes.
        /// \return A reader positioned at the start of the sub‑range.
        ZY_INLINE Reader Split(UInt32 Length)
        {
            const UInt32 Offset = mOffset;

            Skip(Length);
            return Reader(mData + Offset, Length);
        }

        /// \brief Reads a value of the specified type without advancing the offset.
        ///
        /// \param Size The number of bytes to read.
        /// \return The value read, or a default‑constructed instance if out of bounds.
        template<typename Type>
        ZY_INLINE Type Peek(UInt32 Size = sizeof(Type))
        {
            if (mOffset + Size > mSize)
            {
                return Type { };
            }

            if constexpr (IsPointer<Type>)
            {
                return reinterpret_cast<Type>(mData + mOffset);
            }
            else
            {
                static_assert(IsTriviallyCopyable<Type>, "Read<Type> requires trivially copyable types");

                Type Value;
                Blit(AddressOf(Value), sizeof(Type), mData + mOffset);
                return Value;
            }
        }

        /// \brief Reads a value of the specified type and advances the offset.
        ///
        /// \param Size The number of bytes to read.
        /// \return The value read, or a default‑constructed instance if out of bounds.
        template<typename Type>
        ZY_INLINE Type Read(UInt32 Size = sizeof(Type))
        {
            const Type Result = Peek<Type>(Size);
            mOffset += Size;
            return Result;
        }

        /// \brief Reads an unsigned integer encoded with variable‑length encoding.
        ///
        /// \return The decoded unsigned integer value.
        template<typename Type = UInt>
        ZY_INLINE Type ReadUInt()
        {
            Type Result = 0u;

            for (UInt Position = 0, Limit = (sizeof(Type) * 8u + 6u) / 7u; Position < Limit; ++Position)
            {
                const UInt8 Byte = Read<UInt8>();
                Result          |= static_cast<Type>(Byte & 0x7Fu) << (7u * Position);

                if ((Byte & 0x80u) == 0)
                {
                    break;
                }
            }
            return Result;
        }

        /// \brief Reads a signed integer encoded with variable‑length encoding.
        ///
        /// \return The decoded signed integer value.
        template<typename Type = SInt>
        ZY_INLINE Type ReadInt()
        {
            using Unsigned = Unsigned<Type>;

            const Unsigned Value = ReadUInt<Unsigned>();
            return static_cast<Type>((Value >> 1) ^ -(Value & 1));
        }

        /// \brief Reads a UTF‑8 text string prefixed with its length.
        ///
        /// \return A Text object containing the read string.
        ZY_INLINE Text ReadText()
        {
            const UInt32         Size = ReadUInt<UInt32>();
            const ConstPtr<Char> Data = Read<ConstPtr<Char>>(Size * sizeof(Char));
            return Text(Data, Size);
        }

        /// \brief Reads a sequence of bytes into the provided buffer.
        ///
        /// \return A Span representing the read bytes, or an empty span if out of bounds.
        template<typename Header, typename Type>
        ZY_INLINE ConstSpan<Type> ReadBlock()
        {
            const Header         Size = Read<Header>();
            const ConstPtr<Type> Data = Read<ConstPtr<Type>>(Size * sizeof(Type));
            return ConstSpan(Data, Size);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstPtr<Byte> mData;
        UInt32         mSize;
        UInt32         mOffset;
    };
}