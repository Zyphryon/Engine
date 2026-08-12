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

#include "Zyphryon.Graphic/Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Texture
{
    /// \brief A decoded source bitmap, handed from an importer to the exporter.
    class Bitmap final
    {
    public:

        /// \brief The engine texture format describing how this bitmap's pixels are laid out.
        using Format = Graphic::TextureFormat;

    public:

        /// \brief Constructs an empty, invalid bitmap.
        ZY_INLINE Bitmap()
            : mFormat { Format::Unspecified },
              mWidth  { 0 },
              mHeight { 0 },
              mLevels { 0 }
        {
        }

        /// \brief Constructs an Bitmap from decoded pixel data.
        ///
        /// \param Format The format of the data.
        /// \param Width  The width of the base level, in pixels.
        /// \param Height The height of the base level, in pixels.
        /// \param Levels The number of mip levels \p Data holds, including the base level.
        /// \param Data   The tightly-packed pixels, with each level following the one above it.
        ZY_INLINE Bitmap(Format Format, UInt16 Width, UInt16 Height, UInt8 Levels, AnyRef<Blob> Data)
            : mFormat { Format },
              mWidth  { Width },
              mHeight { Height },
              mLevels { Levels },
              mData   { Move(Data) }
        {
        }

        /// \brief Gets the format the pixels are stored in.
        ///
        /// \return The texture format describing the bitmap's layout.
        ZY_INLINE Format GetFormat() const
        {
            return mFormat;
        }

        /// \brief Gets the width of the bitmap.
        ///
        /// \return The width, in pixels.
        ZY_INLINE UInt16 GetWidth() const
        {
            return mWidth;
        }

        /// \brief Gets the height of the bitmap.
        ///
        /// \return The height, in pixels.
        ZY_INLINE UInt16 GetHeight() const
        {
            return mHeight;
        }

        /// \brief Gets the number of mip levels the pixels hold, including the base level.
        ///
        /// \return The level count, or zero when the bitmap carries no pixels.
        ZY_INLINE UInt8 GetLevels() const
        {
            return mLevels;
        }

        /// \brief Gets the pixels as a read-only byte span.
        ///
        /// \return A span over every level of the bitmap.
        ZY_INLINE ConstSpan<Byte> GetPixels() const
        {
            return ConstSpan(mData.GetData(), mData.GetSize());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Format mFormat;
        UInt16 mWidth;
        UInt16 mHeight;
        UInt8  mLevels;
        Blob   mData;
    };
}