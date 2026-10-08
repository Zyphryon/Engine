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

#include "Zyphryon.Graphic/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents a store of same-sized pages spread across texture arrays, lent out one page at a time.
    ///
    /// \note A page may span several layers, each its own texture array in its own format, all lent as one number.
    ///
    /// \tparam Capacity The number of pages the album can lend in total.
    /// \tparam Layers   The number of texture arrays every page spans.
    template<UInt Capacity, UInt Layers = 1>
    class Album final
    {
    public:

        /// \brief The texture arrays one bank is made of, one per layer.
        using Bank   = Array<ZyGraphic::Object, Layers>;

        /// \brief The Format the layers of every page are stored in, in layer order.
        using Format = Array<ZyGraphic::TextureFormat, Layers>;

    public:

        /// \brief Constructs an album that has lent nothing and made no bank yet.
        ///
        /// \param Service The graphic service the banks are made on.
        /// \param Format  The format each layer of every page is stored in.
        /// \param Width   The width of one page, in texels.
        /// \param Height  The height of one page, in texels.
        /// \param Count   The number of pages one bank holds, which is the depth of its texture arrays.
        ZY_INLINE Album(
            ConstRetainer<ZyGraphic::Service> Service,
            ConstRef<Format>                  Format,
            UInt16                            Width,
            UInt16                            Height,
            UInt16                            Count)
            : mService { Service },
              mFormat  { Format },
              mWidth   { Width },
              mHeight  { Height },
              mCount   { Count }
        {
            ZY_ASSERT(Count > 0, "A bank must hold at least one page");
        }

        /// \brief Destroys the album and every bank it made.
        ZY_INLINE ~Album()
        {
            for (ConstRef<Bank> Arrays : mBanks)
            {
                for (const ZyGraphic::Object Texture : Arrays)
                {
                    mService->DeleteTexture(Texture);
                }
            }
        }

        /// \brief Lends a page, making a new bank when the page lands in one that does not exist yet.
        ///
        /// \return The page, or zero when the album is full.
        ZY_INLINE UInt16 Acquire()
        {
            if (mPages.IsFull())
            {
                return 0;
            }

            const UInt16 Page = mPages.Allocate();

            if (GetBank(Page) >= mBanks.GetSize())
            {
                Ref<Bank> Arrays = mBanks.Append();

                for (UInt Layer = 0; Layer < Layers; ++Layer)
                {
                    Arrays[Layer] = mService->CreateTexture(
                        ZyGraphic::TextureLayout::Texture2DArray,
                        mFormat[Layer],
                        ZyGraphic::Storage::Stream,
                        ZyGraphic::Usage::Sample,
                        mWidth,
                        mHeight,
                        mCount,
                        1,
                        Blob());
                }
            }
            return Page;
        }

        /// \brief Takes a page back, so a later \ref Acquire may lend it again.
        ///
        /// \param Page The page to take back.
        ZY_INLINE void Release(UInt16 Page)
        {
            ZY_ASSERT(Page != 0, "Cannot release an invalid page");

            mPages.Release(Page);
        }

        /// \brief Writes one layer of a whole page into its slice of its bank.
        ///
        /// \param Page  The page to write.
        /// \param Layer The layer of the page to write.
        /// \param Data  The texels, which the service takes ownership of.
        /// \param Pitch The number of bytes one row of texels spans.
        ZY_INLINE void Write(UInt16 Page, UInt32 Layer, AnyRef<Blob> Data, UInt32 Pitch)
        {
            ZY_ASSERT(Page != 0, "Cannot write an invalid page");
            ZY_ASSERT(Layer < Layers, "A page has no such layer");

            const ZyGraphic::Object Texture = mBanks[GetBank(Page)][Layer];
            mService->UpdateTexture(Texture, 0, GetSlice(Page), 0, 0, mWidth, mHeight, Pitch, Move(Data));
        }

        /// \brief Gets the texture array one layer of a bank is stored in.
        ///
        /// \param Bank  The bank to read, as \ref GetBank names it for a page lent.
        /// \param Layer The layer to read, in the order the Format were given.
        /// \return The array, a slice per page of the bank, in that layer's format.
        ZY_INLINE ZyGraphic::Object GetTexture(UInt32 Bank, UInt32 Layer = 0) const
        {
            return mBanks[Bank][Layer];
        }

        /// \brief Gets the bank a page lives in.
        ///
        /// \param Page The page \ref Acquire lent, never zero.
        /// \return The bank whose arrays hold the page.
        ZY_INLINE UInt16 GetBank(UInt16 Page) const
        {
            return (Page - 1) / mCount;
        }

        /// \brief Gets the slice a page takes of its bank's arrays.
        ///
        /// \param Page The page \ref Acquire lent, never zero.
        /// \return The layer of the bank's arrays the page is stored in, the same in every one of them.
        ZY_INLINE UInt16 GetSlice(UInt16 Page) const
        {
            return (Page - 1) % mCount;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Service> mService;
        Format                       mFormat;
        UInt16                       mWidth;
        UInt16                       mHeight;
        UInt16                       mCount;
        Freelist<Capacity, 0>        mPages;
        Sequence<Bank>               mBanks;
    };
}