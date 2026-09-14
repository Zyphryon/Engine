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

#include "Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    /// \brief Represents a store of same-sized pages spread across texture arrays, lent out one page at a time.
    ///
    /// \tparam Capacity The number of pages the album can lend in total.
    template<UInt Capacity>
    class Album final
    {
    public:

        /// \brief Constructs an album that has lent nothing and made no bank yet.
        ///
        /// \param Service The graphic service the banks are made on.
        /// \param Format  The format every page is stored in.
        /// \param Width   The width of one page, in texels.
        /// \param Height  The height of one page, in texels.
        /// \param Count   The number of pages one bank holds, which is the depth of its texture array.
        ZY_INLINE Album(ConstRetainer<Service> Service, TextureFormat Format, UInt16 Width, UInt16 Height, UInt16 Count)
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
            for (const Object Bank : mBanks)
            {
                mService->DeleteTexture(Bank);
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
                mBanks.Append(mService->CreateTexture(
                    TextureLayout::Texture2DArray,
                    mFormat,
                    Storage::Stream,
                    Usage::Sample,
                    mWidth,
                    mHeight,
                    mCount,
                    1,
                    Multisample::X1,
                    Blob()));
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

        /// \brief Writes a whole page into its slice of its bank.
        ///
        /// \param Page  The page to write.
        /// \param Data  The texels, which the service takes ownership of.
        /// \param Pitch The number of bytes one row of texels spans.
        ZY_INLINE void Write(UInt16 Page, AnyRef<Blob> Data, UInt32 Pitch)
        {
            ZY_ASSERT(Page != 0, "Cannot write an invalid page");

            const Object Texture = mBanks[GetBank(Page)];
            mService->UpdateTexture(Texture, 0, GetSlice(Page), 0, 0, mWidth, mHeight, Pitch, Move(Data));
        }

        /// \brief Gets how many banks the pages are spread across.
        ///
        /// \return The number of banks.
        ZY_INLINE UInt32 GetBanks() const
        {
            return mBanks.GetSize();
        }

        /// \brief Gets the texture array of a bank.
        ///
        /// \param Bank The bank.
        /// \return The array.
        ZY_INLINE Object GetTexture(UInt32 Bank) const
        {
            return mBanks[Bank];
        }

        /// \brief Gets the bank a page lives in.
        ///
        /// \param Page The page.
        /// \return The bank.
        ZY_INLINE UInt16 GetBank(UInt16 Page) const
        {
            return (Page - 1) / mCount;
        }

        /// \brief Gets the slice a page takes of its bank's array.
        ///
        /// \param Page The page.
        /// \return The slice.
        ZY_INLINE UInt16 GetSlice(UInt16 Page) const
        {
            return (Page - 1) % mCount;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Service>      mService;
        TextureFormat          mFormat;
        UInt16                 mWidth;
        UInt16                 mHeight;
        UInt16                 mCount;
        Freelist<Capacity, 0>  mPages;
        Sequence<Object>       mBanks;
    };
}