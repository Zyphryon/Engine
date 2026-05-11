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

#include "Zyphryon.Base/Primitive.hpp"
#include "Zyphryon.Base/Delegate.hpp"
#include "Zyphryon.Base/Log.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Represents a binary data buffer with custom memory management.
    class Blob final
    {
    public:

        /// \brief A delegate type for custom deleter functions.
        using Deleter = Delegate<void(Ptr<void>), DelegateInlineSize::Smallest>;

    public:

        /// \brief Default constructor, creates an empty blob.
        ZYPHRYON_INLINE Blob() noexcept
            : mPointer { nullptr },
              mSize    { 0 }
        {
        }

        /// \brief Constructs a blob from a raw pointer, size, and custom deleter.
        ///
        /// \param Pointer The raw pointer to the binary data.
        /// \param Size    The size of the binary data in bytes.
        /// \param Deleter The custom deleter function to use when destroying the binary data.
        template<typename Type>
        ZYPHRYON_INLINE Blob(Type Pointer, UInt32 Size, AnyRef<Deleter> Deleter)
            : mPointer { reinterpret_cast<Ptr<Byte>>(Pointer) },
              mSize    { Size },
              mDeleter { Move(Deleter) }
        {
            LOG_ASSERT(!mDeleter.IsEmpty(), "Attempted to construct a Blob without a valid destructor");
        }

        /// \brief Move constructor, transfers ownership from another blob.
        ///
        /// \param Other The other blob to move from.
        ZYPHRYON_INLINE Blob(AnyRef<Blob> Other) noexcept
            : mPointer { Exchange(Other.mPointer, nullptr) },
              mSize    { Exchange(Other.mSize, 0) },
              mDeleter { Move(Other.mDeleter) }
        {
        }

        /// \brief Copy constructor is deleted to prevent copying.
        ZYPHRYON_INLINE Blob(ConstRef<Blob>) = delete;

        /// \brief Destructor, releases the binary data using the custom deleter.
        ZYPHRYON_INLINE ~Blob() noexcept
        {
            Reset();
        }

        /// \brief Resets the blob, releasing any held data.
        ZYPHRYON_INLINE void Reset()
        {
            if (mPointer)
            {
                mDeleter(mPointer);

                mPointer = nullptr;
            }
        }

        /// \brief Retrieves a pointer to the binary data at the specified offset.
        ///
        /// \param Offset The offset in bytes from the start of the binary data.
        /// \return A pointer to the binary data at the specified offset.
        template<typename Type = void>
        ZYPHRYON_INLINE Ptr<Type> GetData(UInt32 Offset = 0)
        {
            return reinterpret_cast<Ptr<Type>>(mPointer + Offset);
        }

        /// \brief Retrieves a constant pointer to the binary data at the specified offset.
        ///
        /// \param Offset The offset in bytes from the start of the binary data.
        /// \return A constant pointer to the binary data at the specified offset.
        template<typename Type = void>
        ZYPHRYON_INLINE ConstPtr<Type> GetData(UInt32 Offset = 0) const
        {
            return reinterpret_cast<ConstPtr<Type>>(mPointer + Offset);
        }

        /// \brief Retrieves a span representing the binary data as an array of the specified type.
        ///
        /// \param Offset The offset in elements from the start of the binary data.
        /// \return A span of the binary data as the specified type.
        template<typename Type>
        ZYPHRYON_INLINE Span<Type> GetSpan(UInt32 Offset = 0)
        {
            return Span<Type>(static_cast<Ptr<Type>>(mPointer) + Offset, mSize / sizeof(Type));
        }

        /// \brief Retrieves a constant span representing the binary data as an array of the specified type.
        ///
        /// \param Offset The offset in elements from the start of the binary data.
        /// \return A constant span of the binary data as the specified type.
        template<typename Type>
        ZYPHRYON_INLINE ConstSpan<Type> GetSpan(UInt32 Offset = 0) const
        {
            return ConstSpan<Type>(static_cast<ConstPtr<Type>>(mPointer) + Offset, mSize / sizeof(Type));
        }

        /// \brief Returns a read-only text representation of the payload.
        ///
        /// \return A textual view over the payload buffer.
        ZYPHRYON_INLINE ConstStr8 GetText() const
        {
            return ConstStr8(reinterpret_cast<Ptr<Char>>(mPointer), mSize);
        }

        /// \brief Retrieves the size of the binary data in bytes.
        ///
        /// \return The size of the binary data in bytes.
        ZYPHRYON_INLINE UInt32 GetSize() const
        {
            return mSize;
        }

        /// \brief Copies data from the specified source into the binary buffer at the given offset.
        ///
        /// \param Source  A pointer to the source data to copy from.
        /// \param Size    The number of bytes to copy.
        /// \param Offset  The offset in bytes within the binary buffer to copy to.
        template<typename Type>
        ZYPHRYON_INLINE void Copy(ConstPtr<Type> Source, UInt32 Size, UInt32 Offset = 0)
        {
            LOG_ASSERT(Offset + Size <= mSize, "Copy operation exceeds buffer bounds");

            std::memcpy(mPointer + Offset, Source, Size);
        }

        /// \brief Move assignment operator, transfers ownership from another blob.
        ///
        /// \param Other The other blob to move from.
        /// \return Reference to this blob.
        ZYPHRYON_INLINE Ref<Blob> operator=(AnyRef<Blob> Other) noexcept
        {
            if (this != &Other)
            {
                if (mPointer)
                {
                    mDeleter(mPointer);
                }

                mPointer = Exchange(Other.mPointer, nullptr);
                mSize    = Exchange(Other.mSize, 0);
                mDeleter = Move(Other.mDeleter);
            }
            return (* this);
        }

        /// \brief Copy assignment operator is deleted to prevent copying.
        ZYPHRYON_INLINE Ref<Blob> operator=(ConstRef<Blob>) = delete;

        /// \brief Compares this Blob to another for equality.
        ///
        /// \param Other The other Blob to compare against.
        /// \return `true` if both Binaries point to the same data and have the same size, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator==(ConstRef<Blob> Other) const
        {
            return mPointer == Other.mPointer && mSize == Other.mSize;
        }

        /// \brief Compares this blob to another for inequality.
        ///
        /// \param Other The other blob to compare against.
        /// \return `true` if the Binaries are not equal, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator!=(ConstRef<Blob> Other) const
        {
            return !(* this == Other);
        }

        /// \brief Compares this blob to nullptr for equality.
        ///
        /// \return `true` if the blob is empty (null), otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator==(std::nullptr_t) const
        {
            return mPointer == nullptr;
        }

        /// \brief Compares this blob to nullptr for inequality.
        ///
        /// \return `true` if the blob is not empty (not null), otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator!=(std::nullptr_t) const
        {
            return mPointer != nullptr;
        }

        /// \brief Checks whether the blob contains valid data.
        ///
        /// \return `true` if the blob is not empty, otherwise `false`.
        ZYPHRYON_INLINE constexpr operator Bool() const
        {
            return mPointer != nullptr;
        }

        /// \brief Implicitly converts the blob to a span of bytes.
        ///
        /// \return A constant span of bytes representing the binary data.
        ZYPHRYON_INLINE operator ConstSpan<Byte>() const
        {
            return GetSpan<Byte>();
        }

    public:

        /// \brief Creates a blob by allocating a buffer of the specified size.
        ///
        /// \param Size The size of the buffer to allocate in bytes.
        /// \return A blob managing the allocated buffer.
        ZYPHRYON_INLINE static Blob Bytes(UInt32 Size)
        {
            return Blob(new Byte[Size], Size, []<typename T = Byte>(Ptr<void> Address)
            {
                delete[] static_cast<Ptr<T>>(Address);
            });
        }

        /// \brief Creates a blob that wraps an existing pointer and size, without taking ownership.
        ///
        /// \param Pointer The pointer to the existing data to wrap.
        /// \param Size    The size of the data in bytes.
        /// \return A blob managing the provided pointer and size.
        template<typename Type>
        ZYPHRYON_INLINE static Blob Wrap(Ptr<Type> Pointer, UInt32 Size)
        {
            return Blob(Pointer, Size, []<typename T = Byte>(Ptr<void> Address)
            {
            });
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Byte> mPointer;
        UInt32    mSize;
        Deleter   mDeleter;
    };
}