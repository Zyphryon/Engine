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

#include "Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a transient CPU mapping backed by a GPU buffer allocation.
    template<typename Type>
    class Transient final
    {
    public:

        /// \brief Constructs an empty transient allocation.
        ZY_INLINE Transient()
            : mAddress { nullptr }
        {
        }

        /// \brief Constructs a transient allocation from a pointer and stream descriptor.
        ///
        /// \param Address    The pointer to the start of the buffer region.
        /// \param Descriptor The stream descriptor defining the buffer layout.
        ZY_INLINE Transient(Ptr<Type> Address, Stream Descriptor)
            : mAddress    { Address },
              mDescriptor { Descriptor }
        {

        }

        /// \brief Gets the stream descriptor associated with this transient allocation.
        ///
        /// \return The stream descriptor defining the buffer layout.
        ZY_INLINE Stream GetStream() const
        {
            return mDescriptor;
        }

        /// \brief Gets a pointer to the start of the transient buffer region.
        ///
        /// \return A pointer to the beginning of the mapped buffer region.
        ZY_INLINE Ptr<Type> GetData()
        {
            return mAddress;
        }

        /// \brief Gets a const pointer to the start of the transient buffer region.
        ///
        /// \return A const pointer to the beginning of the mapped buffer region.
        ZY_INLINE ConstPtr<Type> GetData() const
        {
            return mAddress;
        }

        /// \brief Copies data from a span into the transient buffer at the specified offset.
        ///
        /// \param Data   The source span containing the data to copy.
        /// \param Offset The byte offset within the transient buffer where copying begins.
        template<typename Other>
        ZY_INLINE void Copy(ConstSpan<Other> Data, UInt32 Offset = 0)
        {
            Blit(mAddress + Offset, Data.GetSizeInBytes(), Data.GetData());
        }

        /// \brief Gets a reference to the element at the given index.
        ///
        /// \param Index The zero-based index of the element to access.
        /// \return A reference to the element at \p Index.
        ZY_INLINE Ref<Type> operator[](UInt Index)
        {
            return mAddress[Index];
        }

        /// \brief Returns a const reference to the element at the given index.
        ///
        /// \param Index The zero-based index of the element to access.
        /// \return A const reference to the element at \p Index.
        ZY_INLINE ConstRef<Type> operator[](UInt Index) const
        {
            return mAddress[Index];
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Type> mAddress;
        Stream    mDescriptor;
    };
}