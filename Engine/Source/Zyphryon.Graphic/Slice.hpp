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

#include "Backend/Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief A typed view over a contiguous region of a GPU buffer.
    template<typename Type>
    class Slice final
    {
    public:

        /// \brief Constructs an empty slice with no buffer association.
        ZY_INLINE Slice()
            : mAddress { nullptr }
        {
        }

        /// \brief Constructs a slice from a pointer and stream descriptor.
        ///
        /// \param Address    The pointer to the start of the buffer region.
        /// \param Descriptor The stream descriptor defining the buffer layout.
        ZY_INLINE Slice(Ptr<Type> Address, Stream Descriptor)
            : mAddress    { Address },
              mDescriptor { Descriptor }
        {

        }

        /// \brief Copies data from a span into the buffer at the given offset.
        ///
        /// \param Data   The source data to copy.
        /// \param Offset The element offset at which to begin writing.
        template<typename Other>
        ZY_INLINE void Copy(ConstSpan<Other> Data, UInt32 Offset = 0)
        {
            Blit(mAddress + Offset, Data.GetSizeInBytes(), Data.GetData());
        }

        /// \brief Gets the stream descriptor for this slice.
        ///
        /// \return The stream descriptor defining the buffer layout.
        ZY_INLINE Stream GetDescriptor() const
        {
            return mDescriptor;
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