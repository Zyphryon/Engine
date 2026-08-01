// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Atlas.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Font
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void Descend(Ref<Sequence<Cell>> Cells)
    {
        // Shelf packing only wastes the difference between the tallest and shortest cell on a row, so feeding
        // it the cells tallest-first is what keeps that difference small.
        for (UInt Index = 1; Index < Cells.GetSize(); ++Index)
        {
            Cell Pivot = Move(Cells[Index]);
            UInt Slot  = Index;

            while (Slot > 0 && Cells[Slot - 1].Data.Height < Pivot.Data.Height)
            {
                Cells[Slot] = Move(Cells[Slot - 1]);
                --Slot;
            }
            Cells[Slot] = Move(Pivot);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt32 Place(Ref<Sequence<Cell>> Cells, UInt32 Side, UInt32 Padding, Bool Paginate)
    {
        UInt32 Page    = 0;
        UInt32 CursorX = Padding;
        UInt32 CursorY = Padding;
        UInt32 Shelf   = 0;

        for (Ref<Cell> Entry : Cells)
        {
            if (Entry.Data.Width == 0 || Entry.Data.Height == 0)
            {
                continue;
            }

            // A cell that no longer fits on this shelf opens the next one above it.
            if (CursorX + Entry.Data.Width + Padding > Side)
            {
                CursorX  = Padding;
                CursorY += Shelf + Padding;
                Shelf    = 0;
            }

            if (CursorY + Entry.Data.Height + Padding > Side)
            {
                // Without pagination the caller is still growing the sheet, so running out only means this
                // side was too small to be worth trying.
                if (!Paginate)
                {
                    return 0;
                }

                // A glyph too tall for an entire empty page can never be placed, however many pages open.
                if (CursorY == Padding)
                {
                    return 0;
                }

                ++Page;
                CursorX = Padding;
                CursorY = Padding;
                Shelf   = 0;
            }

            Entry.Page = Page;
            Entry.X    = CursorX;
            Entry.Y    = CursorY;
            CursorX   += Entry.Data.Width + Padding;
            Shelf      = Max(Shelf, Entry.Data.Height);
        }
        return Page + 1;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Atlas::Layout Atlas::Arrange(Ref<Sequence<Cell>> Cells, UInt32 Padding, UInt32 Limit)
    {
        Descend(Cells);

        // The total ink area is a lower bound on the side, and shelf packing overshoots it by a little, so the
        // search starts there and grows until everything lands.
        UInt   Area    = 0;
        UInt32 Widest  = 0;
        UInt32 Tallest = 0;

        for (ConstRef<Cell> Entry : Cells)
        {
            const UInt32 Width  = Entry.Data.Width  + Padding;
            const UInt32 Height = Entry.Data.Height + Padding;

            Area   += static_cast<UInt>(Width) * Height;
            Widest  = Max(Widest,  Width  + Padding);
            Tallest = Max(Tallest, Height + Padding);
        }

        if (Area == 0)
        {
            return Layout();
        }

        UInt32 Side = static_cast<UInt32>(Ceil(Sqrt(static_cast<Real32>(Area))));
        Side        = Max(Side, Max(Widest, Tallest));

        // One sheet is cheaper for the runtime than several, so the side grows for as long as the limit allows
        // and only a bake that genuinely overflows it spills onto further pages.
        while (Side <= Limit)
        {
            if (const UInt32 Pages = Place(Cells, Side, Padding, false); Pages > 0)
            {
                return Layout(Side, Pages);
            }
            Side += Max<UInt32>(1, Side / 32);
        }

        const UInt32 Pages = Place(Cells, Limit, Padding, true);

        return Pages > 0 ? Layout(Limit, Pages) : Layout();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Atlas::Compose(ConstRef<Sequence<Cell>> Cells, UInt32 Side, UInt32 Page)
    {
        Blob Canvas = Blob::Allocate<Byte>(static_cast<UInt>(Side) * Side * 4);
        Zero(Canvas.GetData<Byte>(), Canvas.GetSize());

        for (ConstRef<Cell> Entry : Cells)
        {
            if (Entry.Page != Page)
            {
                continue;
            }

            for (UInt32 Y = 0; Y < Entry.Data.Height; ++Y)
            {
                const UInt Source = static_cast<UInt>(Y) * Entry.Data.Width * 4;
                const UInt Target = (static_cast<UInt>(Entry.Y + Y) * Side + Entry.X) * 4;

                Blit(Canvas.GetData<Byte>() + Target,
                     static_cast<UInt>(Entry.Data.Width) * 4,
                     Entry.Data.Texels.GetData() + Source);
            }
        }
        return Canvas;
    }
}