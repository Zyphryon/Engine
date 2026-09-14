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

#include <Zyphryon.Graphic/Service.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Interns the block a run of instances shares, so each instance carries a number instead of a copy.
    ///
    /// \tparam Type     The block one run shares, in the layout its technique reads.
    /// \tparam Capacity The most runs one page holds, which the technique's own array has to be declared at.
    template<typename Type, UInt32 Capacity>
    class Bank final
    {
    public:

        /// \brief Names one interned run, by the page it landed on and where on that page it sits.
        struct Slot final
        {
            /// Where the run sits on its page, which is what an instance carries.
            UInt16 Index;

            /// The page the run was interned into, which is what keeps its batch apart from another page's.
            UInt16 Page;

            /// \brief Constructs a slot naming no run at all.
            ZY_INLINE constexpr Slot()
                : Index { 0 },
                  Page  { 0 }
            {
            }

            /// \brief Constructs a slot naming where one run landed.
            ///
            /// \param Index Where the run sits on its page.
            /// \param Page  The page the run was interned into.
            ZY_INLINE constexpr Slot(UInt16 Index, UInt16 Page)
                : Index { Index },
                  Page  { Page }
            {
            }
        };

    public:

        /// \brief Constructs a bank with nothing interned into it.
        ZY_INLINE Bank()
            : mActives { 0 }
        {
        }

        /// \brief Interns one run, opening a fresh page when the last one has no room left.
        ///
        /// \param Block The block every instance of the run reads.
        /// \return Where the run landed, which its instances carry in place of the block itself.
        ZY_INLINE Slot Intern(ConstRef<Type> Block)
        {
            if (mActives == 0 || mPages[mActives - 1].Blocks.GetSize() >= Capacity)
            {
                if (mActives == mPages.GetSize())
                {
                    mPages.Append();
                }
                ++mActives;
            }

            Ref<Page> Current = mPages[mActives - 1];

            const Slot Result(static_cast<UInt16>(Current.Blocks.GetSize()), static_cast<UInt16>(mActives - 1));

            Current.Blocks.Append(Block);

            return Result;
        }

        /// \brief Uploads every page interned since the last reset, so no batch reads a stale one.
        ///
        /// \param Service The service the transient uniform streams are allocated from.
        ZY_INLINE void Prepare(Ref<ZyGraphic::Service> Service)
        {
            for (UInt32 Index = 0; Index < mActives; ++Index)
            {
                Ref<Page> Current = mPages[Index];

                ZyGraphic::Transient<Type> Slice = Service.AllocateInFlightUniforms<Type>(Capacity);
                Slice.template Copy<Type>(Current.Blocks);

                Current.Stream = Slice.GetStream();
            }
        }

        /// \brief Gets the stream one page's runs were uploaded into.
        ///
        /// \param Page The page, as the slot of any run on it names it.
        /// \return The uniform stream the batch reading that page binds.
        ZY_INLINE ZyGraphic::Stream GetStream(UInt16 Page) const
        {
            return mPages[Page].Stream;
        }

        /// \brief Drops every run interned so far, keeping the pages they were interned into.
        ZY_INLINE void Reset()
        {
            for (Ref<Page> Current : mPages)
            {
                Current.Blocks.Clear();
            }
            mActives = 0;
        }

    private:

        /// \brief Holds one page of runs and the stream they were uploaded into.
        struct Page final
        {
            /// The stream the page was uploaded into.
            ZyGraphic::Stream Stream;

            /// The runs interned onto the page, which their instances index by slot.
            Sequence<Type>    Blocks;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Page> mPages;
        UInt32         mActives;
    };
}