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

#include "Zyphryon.Base/Functional/Delegate.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A move-only byte buffer with a caller-supplied deleter.
    class Blob final
    {
    public:

        /// \brief The type of the callable used to release the buffer when the blob is destroyed or cleared.
        using Deleter = Delegate<void(Ptr<Byte>), DelegateInlineSize::Smallest>;

    public:

        /// \brief Constructs an empty blob holding no memory.
        ZY_INLINE Blob()
            : mData { nullptr },
              mSize { 0 }
        {
        }

        /// \brief Constructs a blob that takes ownership of an existing buffer.
        ///
        /// \param Data    The pointer to the first byte of the buffer.
        /// \param Size    The size of the buffer in bytes.
        /// \param Deleter The callable invoked with \p Data when the buffer is released.
        ZY_INLINE Blob(Ptr<Byte> Data, UInt32 Size, AnyRef<Deleter> Deleter)
            : mData    { Data },
              mSize    { Size },
              mDeleter { Move(Deleter) }
        {
        }

        /// \brief Copy-constructing a blob is not allowed.
        ZY_INLINE Blob(ConstRef<Blob>) = delete;

        /// \brief Constructs a blob by transferring ownership from \p Other.
        ///
        /// \param Other The source blob, left empty after the transfer.
        ZY_INLINE Blob(AnyRef<Blob> Other)
            : mData    { Exchange(Other.mData, nullptr) },
              mSize    { Exchange(Other.mSize, 0u) },
              mDeleter { Move(Other.mDeleter) }
        {
        }

        /// \brief Releases the buffer by invoking the deleter, if any memory is held.
        ZY_INLINE ~Blob()
        {
            Clear();
        }

        /// \brief Gets a pointer to the raw byte buffer.
        ///
        /// \return The pointer to the first byte, or \c nullptr if the blob is empty.
        ZY_INLINE Ptr<Byte> GetData()
        {
            return mData;
        }

        /// \brief Gets a read-only pointer to the raw byte buffer.
        ///
        /// \return The pointer to the first byte, or \c nullptr if the blob is empty.
        ZY_INLINE ConstPtr<Byte> GetData() const
        {
            return mData;
        }

        /// \brief Gets the buffer reinterpreted as a pointer to \p Type.
        ///
        /// \return The pointer to the first element, or \c nullptr if the blob is empty.
        template<typename Type>
        ZY_INLINE Ptr<Type> GetData()
        {
            return reinterpret_cast<Ptr<Type>>(mData);
        }

        /// \brief Gets the buffer reinterpreted as a read-only pointer to \p Type.
        ///
        /// \return The pointer to the first element, or \c nullptr if the blob is empty.
        template<typename Type>
        ZY_INLINE ConstPtr<Type> GetData() const
        {
            return reinterpret_cast<ConstPtr<Type>>(mData);
        }

        /// \brief Gets the size of the buffer in bytes.
        ///
        /// \return The number of bytes held by this blob, or zero if empty.
        ZY_INLINE UInt32 GetSize() const
        {
            return mSize;
        }

        /// \brief Releases the buffer by invoking the deleter and resets the blob to empty.
        ZY_INLINE void Clear()
        {
            if (mData)
            {
                mDeleter(mData);

                mData = nullptr;
                mSize = 0;
            }
        }

        /// \brief Copies \p Size bytes from \p Source into the buffer at the given byte offset.
        ///
        /// \param Source The pointer to the data to copy from.
        /// \param Size   The number of bytes to copy.
        /// \param Offset The byte offset within this blob at which writing begins.
        template<typename Type>
        ZY_INLINE void Copy(ConstPtr<Type> Source, UInt32 Size, UInt Offset = 0)
        {
            ZY_ASSERT(mData != nullptr, "Blob is empty");
            ZY_ASSERT(Offset + Size <= mSize, "Copy range exceeds Blob size");

            ::Copy(mData + Offset, Size, Source);
        }

        /// \brief Copy-assigning a blob is not allowed.
        ZY_INLINE Ref<Blob> operator=(ConstRef<Blob>) = delete;

        /// \brief Move-assigns a blob by transferring ownership from \p Other.
        ///
        /// \param Other The source blob, left empty after the transfer.
        /// \return A reference to this blob.
        ZY_INLINE Ref<Blob> operator=(AnyRef<Blob> Other)
        {
            if (this != AddressOf(Other))
            {
                Clear();

                mData    = Exchange(Other.mData, nullptr);
                mSize    = Exchange(Other.mSize, 0);
                mDeleter = Move(Other.mDeleter);
            }
            return (* this);
        }

        /// \brief Checks if the blob is empty (i.e., holds no memory).
        ///
        /// \return `true` if the blob is empty, `false` otherwise.
        ZY_INLINE Bool operator==(nullptr_t) const
        {
            return mData == nullptr;
        }

        /// \brief Checks if the blob is not empty (i.e., holds memory).
        ///
        /// \return `true` if the blob is not empty, `false` otherwise.
        ZY_INLINE Bool operator!=(nullptr_t) const
        {
            return mData != nullptr;
        }

        /// \brief Checks if the blob is not empty (i.e., holds memory).
        ///
        /// \return `true` if the blob is not empty, `false` otherwise.
        ZY_INLINE explicit operator Bool() const
        {
            return mData != nullptr;
        }

    public:

        /// \brief Creates a blob that allocates a new buffer for the specified number of elements of \p Type.
        ///
        /// \param Count The number of elements to allocate in the buffer.
        /// \return A blob that owns a new buffer of the specified size, with a deleter that releases the memory.
        template<typename Type>
        ZY_INLINE static Blob Allocate(UInt Count)
        {
            const UInt Size = sizeof(Type) * Count;

            return Blob(new Byte[Size], Size, [](Ptr<Byte> Data)
            {
                ::delete[] reinterpret_cast<Ptr<Type>>(Data);
            });
        }

        /// \brief Creates a blob that borrows an existing buffer without taking ownership.
        ///
        /// \param Data  The pointer to the first element of the buffer.
        /// \param Count The number of elements in the buffer.
        /// \return A blob that references the existing buffer without owning it.
        template<typename Type>
        ZY_INLINE static Blob Borrow(ConstPtr<Type> Data, UInt Count)
        {
            return Blob(const_cast<Ptr<Byte>>(Data), Count * sizeof(Type), [](Ptr<Byte>)
            {
                // Non-owning — intentionally does not free the wrapped memory.
            });
        }

        /// \brief Creates a blob that borrows an existing buffer without taking ownership.
        ///
        /// \param Span The constant span referencing the buffer to borrow.
        /// \return A blob that references the existing buffer without owning it.
        template<typename Type>
        ZY_INLINE static Blob Borrow(ConstSpan<Type> Span)
        {
            return Borrow(Span.GetData(), Span.GetSize());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Byte> mData;
        UInt32    mSize;
        Deleter   mDeleter;
    };
}