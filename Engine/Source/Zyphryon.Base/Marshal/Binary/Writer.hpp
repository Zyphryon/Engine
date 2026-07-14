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
#include "Zyphryon.Base/Memory/Blob.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Encapsulates a binary writer for serialization.
    class Writer final
    {
    public:

        /// \brief Constructs an empty writer with no allocated storage.
        ZY_INLINE Writer()
            : mData     { nullptr },
              mSize     { 0 },
              mCapacity { 0 }
        {
        }

        /// \brief Constructs a writer with a pre-allocated capacity.
        ///
        /// \param Capacity The initial capacity in bytes to reserve.
        ZY_INLINE explicit Writer(UInt32 Capacity)
            : Writer { }
        {
            Ensure(Capacity);
        }

        /// \brief Destroys the writer and releases the underlying storage.
        ZY_INLINE ~Writer()
        {
            if (mData)
            {
                delete[] mData;

                mData     = nullptr;
                mSize     = 0;
                mCapacity = 0;
            }
        }

        /// \brief Transfers ownership of the written data to a new blob.
        ///
        /// \return A blob containing the written data.
        ZY_INLINE Blob Detach()
        {
            Blob Memory = Blob(mData, mSize, [](Ptr<Byte> Data)
            {
                ::operator delete[](Data);
            });

            mData     = nullptr;
            mSize     = 0;
            mCapacity = 0;

            return Memory;
        }

        /// \brief Gets a pointer to the contiguous buffer storage.
        ///
        /// \return A pointer to the first byte, or \c nullptr when empty.
        ZY_INLINE Ptr<Byte> GetData()
        {
            return mData;
        }

        /// \brief Gets a read-only pointer to the contiguous buffer storage.
        ///
        /// \return A pointer to the first byte, or \c nullptr when empty.
        ZY_INLINE ConstPtr<Byte> GetData() const
        {
            return mData;
        }

        /// \brief Gets the number of bytes currently written to the buffer.
        ///
        /// \return The byte count.
        ZY_INLINE UInt32 GetSize() const
        {
            return mSize;
        }

        /// \brief Gets the number of bytes the buffer can hold without reallocating.
        ///
        /// \return The current capacity.
        ZY_INLINE UInt32 GetCapacity() const
        {
            return mCapacity;
        }

        /// \brief Ensures that the buffer can hold at least \p Length additional bytes without reallocating.
        ///
        /// \param Length The number of additional bytes to guarantee space for.
        ZY_INLINE void Ensure(UInt32 Length)
        {
            const UInt Capacity = mCapacity + Length;

            if (Capacity > mCapacity)
            {
                const Ptr<Byte> Data = static_cast<Ptr<Byte>>(::operator new[](sizeof(Byte) * Capacity));
                Copy(Data, mSize, mData);

                if (mData)
                {
                    ::operator delete[](mData);
                }

                mData     = Data;
                mCapacity = Capacity;
            }
        }

        /// \brief Destroys all written bytes without releasing the underlying storage.
        ZY_INLINE void Clear()
        {
            mSize = 0;
        }

        /// \brief Writes raw bytes to the buffer.
        ///
        /// \param Value The pointer to the bytes to write.
        /// \param Size  The number of bytes to write.
        template<typename Type>
        ZY_INLINE void Write(ConstPtr<Type> Value, UInt32 Size)
        {
            Blit(mData + mSize, Size, Value);
            mSize += Size;
        }

        /// \brief Writes a single value of \p Type into the buffer.
        ///
        /// \param Value The value to write.
        template<typename Type>
        ZY_INLINE void Write(Type Value)
        {
            Write(AddressOf(Value), sizeof(Type));
        }

        /// \brief Writes a signed integer using variable-length zigzag encoding.
        ///
        /// \param Value The signed integer value to encode and write.
        template<typename Type = SInt>
        ZY_INLINE void WriteInt(Type Value)
        {
            using Unsigned = Unsigned<Type>;

            WriteUInt((static_cast<Unsigned>(Value) << 1) ^ (Value >> (sizeof(Type) * 8 - 1)));
        }

        /// \brief Writes an unsigned integer using variable-length encoding.
        ///
        /// \param Value The unsigned integer value to encode and write.
        template<typename Type = UInt>
        ZY_INLINE void WriteUInt(Type Value)
        {
            while (Value > 0x7F)
            {
                Write<UInt8>(static_cast<UInt8>(Value & 0x7Fu) | 0x80u);
                Value >>= 7u;
            }
            Write<UInt8>(static_cast<UInt8>(Value) & 0x7Fu);
        }

        /// \brief Writes a text string prefixed with its character count.
        ///
        /// \param Value The text to write.
        ZY_INLINE void WriteText(Text Value)
        {
            WriteUInt(Value.GetSize());

            if (!Value.IsEmpty())
            {
                Write<Char>(Value.GetData(), Value.GetSize() * sizeof(Char));
            }
        }

        /// \brief Writes a contiguous block of trivially copyable elements into the buffer.
        ///
        /// \param Value The view of elements to write.
        template<typename Header, typename Type>
        ZY_INLINE void WriteBlock(ConstSpan<Type> Value)
            requires IsTriviallyCopyable<Type>
        {
            Write<Header>(Value.GetSize());

            if (!Value.empty())
            {
                Write<ConstPtr<Type>>(Value.GetData(), Value.GetSizeInBytes());
            }
        }

        /// \brief Writes a data block with a length prefix, using a user-defined function to populate the contents.
        ///
        /// \param Trampoline The user-defined function that writes the block contents.
        template<typename Header, typename Function>
        ZY_INLINE void WriteBlock(AnyRef<Function> Trampoline)
        {
            // Reserve space for the segment length.
            const UInt32 Offset = mSize;
            Write<Header>(static_cast<Header>(0));

            // Invoke the user-defined function to write the segment data.
            Trampoline(* this);

            // Calculate and backfill the length of the segment.
            const UInt32 Length = mSize - (Offset + sizeof(Header));
            Blit(mData + Offset, sizeof(Length), AddressOf(Length));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Byte> mData;
        UInt      mSize;
        UInt      mCapacity;
    };
}