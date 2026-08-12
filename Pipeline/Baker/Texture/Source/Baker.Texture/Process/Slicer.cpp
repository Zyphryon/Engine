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

#include "Slicer.hpp"
#include <Zyphryon.Graphic/Metadata.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Texture
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    // Every cut addresses whole texels of one level, whichever shape it takes, so both share this gate.
    static Bool Accepts(ConstRef<Bitmap> Source)
    {
        const Graphic::TextureFormat   Format   = Source.GetFormat();
        const Graphic::TextureMetadata Metadata = Graphic::GetTextureMetadata(Format);

        // Cutting addresses whole texels, which a block-compressed or bit-packed surface does not have.
        if (Metadata.IsCompressed() || Metadata.IsPacked || Metadata.Components == 0)
        {
            LOG_E("Texture: '{0}' must have its texels unpacked before it can be cut", Enum::GetName(Format));

            return false;
        }

        if (Source.GetLevels() != 1)
        {
            LOG_E("Texture: a surface must be cut before its mips are filtered");

            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bitmap Process(ConstRef<Bitmap> Source, ConstRef<Slicer::Layout::Cell> Cell, UInt32 Width, UInt32 Height, UInt32 Stride)
    {
        const UInt32 Sweep = Source.GetWidth() * Stride;   // One row of the atlas.
        const UInt32 Line  = Width * Stride;               // One row of the cell.

        Blob            Output = Blob::Allocate<Byte>(static_cast<UInt64>(Line) * Height);
        const Ptr<Byte> Target = Output.GetData<Byte>();

        ConstPtr<Byte>  Origin = Source.GetPixels().GetData()
            + static_cast<UInt64>(Cell.Row) * Height * Sweep
            + static_cast<UInt64>(Cell.Column) * Line;

        for (UInt32 Row = 0; Row < Height; ++Row, Origin += Sweep)
        {
            if (Cell.Turned)
            {
                Ptr<Byte> Slot = Target + static_cast<UInt64>(Height - 1 - Row) * Line + (Width - 1) * Stride;

                for (UInt32 Column = 0; Column < Width; ++Column, Slot -= Stride)
                {
                    Copy(Slot, Stride, Origin + Column * Stride);
                }
            }
            else
            {
                Copy(Target + static_cast<UInt64>(Row) * Line, Line, Origin);
            }
        }
        return Bitmap(Source.GetFormat(), Width, Height, 1, Move(Output));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Slicer::Layout Slicer::Describe(UInt32 Columns, UInt32 Rows)
    {
        struct Arrangement final
        {
            UInt16       Columns;
            UInt16       Rows;
            Layout::Cell Faces[kFaces];
        };

        // Every grid recognized as a cube map, each holding its faces as +X, -X, +Y, -Y, +Z, -Z.
        static constexpr Arrangement kArrangements[] =
        {
            { 6, 1, { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 } } },           // 6x1
            { 1, 6, { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 } } },           // 1x6
            { 4, 3, { { 2, 1 }, { 0, 1 }, { 1, 0 }, { 1, 2 }, { 1, 1 }, { 3, 1 } } },           // 4x3
            { 3, 4, { { 2, 1 }, { 0, 1 }, { 1, 0 }, { 1, 2 }, { 1, 1 }, { 1, 3, true } } },     // 3x4
            { 3, 2, { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } } },           // 3x2
            { 2, 3, { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { 0, 2 }, { 1, 2 } } },           // 2x3
        };

        for (ConstRef<Arrangement> Candidate : kArrangements)
        {
            if (Candidate.Columns == Columns && Candidate.Rows == Rows)
            {
                return Layout(Candidate.Columns, Candidate.Rows, ConstSpan(Candidate.Faces, kFaces));
            }
        }

        LOG_E("Texture: {0}x{1} is not a grid any cube arrangement is packed in", Columns, Rows);
        return Layout();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Slicer::Layout Slicer::Divide(ConstRef<Bitmap> Source, UInt32 Width, UInt32 Height)
    {
        if (Width == 0 || Height == 0)
        {
            LOG_E("Texture: a cell cannot measure zero on either axis");

            return Layout();
        }

        if (Source.GetWidth() % Width != 0 || Source.GetHeight() % Height != 0)
        {
            LOG_E("Texture: {0}x{1} cells do not tile a {2}x{3} atlas exactly",
                Width, Height, Source.GetWidth(), Source.GetHeight());

            return Layout();
        }
        return Layout(
            static_cast<UInt16>(Source.GetWidth() / Width),
            static_cast<UInt16>(Source.GetHeight() / Height));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sequence<Bitmap> Slicer::Slice(ConstRef<Bitmap> Source, ConstRef<Layout> Layout)
    {
        if (!Accepts(Source))
        {
            return Sequence<Bitmap>();
        }

        if (!Layout.IsValid())
        {
            LOG_E("Texture: the atlas was given no division to cut along");

            return Sequence<Bitmap>();
        }

        // A layout may be built by hand, so the grid is checked here rather than trusted from its maker.
        if (Source.GetWidth() % Layout.Columns != 0 || Source.GetHeight() % Layout.Rows != 0)
        {
            LOG_E("Texture: a {0}x{1} grid does not divide a {2}x{3} atlas exactly",
                Layout.Columns, Layout.Rows, Source.GetWidth(), Source.GetHeight());

            return Sequence<Bitmap>();
        }

        const Graphic::TextureMetadata Metadata = Graphic::GetTextureMetadata(Source.GetFormat());

        const UInt32 Width  = Source.GetWidth()  / Layout.Columns;
        const UInt32 Height = Source.GetHeight() / Layout.Rows;
        const UInt32 Stride = Metadata.BitsPerPixel / 8;
        const UInt32 Count  = Layout.GetCount();

        // A cell outside the grid would read past the surface, which no assert catches in a release build.
        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            const Layout::Cell Cell = Layout.GetCell(Index);

            if (Cell.Column >= Layout.Columns || Cell.Row >= Layout.Rows)
            {
                LOG_E("Texture: cell {0} sits at {1},{2}, outside a {3}x{4} grid",
                    Index, Cell.Column, Cell.Row, Layout.Columns, Layout.Rows);

                return Sequence<Bitmap>();
            }
        }

        Sequence<Bitmap> Slices(Count);

        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            Slices.Append(Process(Source, Layout.GetCell(Index), Width, Height, Stride));
        }
        return Slices;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bitmap Slicer::Crop(ConstRef<Bitmap> Source, UInt32 X, UInt32 Y, UInt32 Width, UInt32 Height)
    {
        if (!Accepts(Source))
        {
            return Bitmap();
        }

        // A rectangle reaching past the surface would read memory the bitmap does not own.
        if (Width == 0 || Height == 0 || X + Width > Source.GetWidth() || Y + Height > Source.GetHeight())
        {
            LOG_E("Texture: a {0}x{1} rectangle at {2},{3} leaves a {4}x{5} surface",
                Width, Height, X, Y, Source.GetWidth(), Source.GetHeight());

            return Bitmap();
        }

        const Graphic::TextureMetadata Metadata = Graphic::GetTextureMetadata(Source.GetFormat());

        const UInt32 Stride = Metadata.BitsPerPixel / 8;
        const UInt32 Sweep  = Source.GetWidth() * Stride;
        const UInt32 Line   = Width * Stride;

        Blob            Output = Blob::Allocate<Byte>(static_cast<UInt64>(Line) * Height);
        const Ptr<Byte> Target = Output.GetData<Byte>();

        ConstPtr<Byte>  Origin = Source.GetPixels().GetData()
            + static_cast<UInt64>(Y) * Sweep
            + static_cast<UInt64>(X) * Stride;

        for (UInt32 Row = 0; Row < Height; ++Row, Origin += Sweep)
        {
            Copy(Target + static_cast<UInt64>(Row) * Line, Line, Origin);
        }
        return Bitmap(Source.GetFormat(), static_cast<UInt16>(Width), static_cast<UInt16>(Height), 1, Move(Output));
    }
}