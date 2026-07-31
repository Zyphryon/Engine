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

namespace Tool::Baker::Texture
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    namespace
    {
        /// Where one face sits, counted in whole faces from the top-left.
        struct Cell final
        {
            UInt8 Column;
            UInt8 Row;
            Bool  Turned;
        };

        /// One way of packing six faces, and where each lands in it.
        struct Arrangement final
        {
            UInt8 Columns;
            UInt8 Rows;
            Cell  Faces[Slicer::kFaces];
        };
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static ConstPtr<Arrangement> Recognize(UInt32 Width, UInt32 Height)
    {
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
            const Bool Divides = (Width % Candidate.Columns == 0) && (Height % Candidate.Rows == 0);

            if (Divides && Width / Candidate.Columns == Height / Candidate.Rows)
            {
                return AddressOf(Candidate);
            }
        }
        return nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bitmap Cut(ConstRef<Bitmap> Source, ConstRef<Cell> Cell, UInt32 Extent, UInt32 Stride)
    {
        const UInt32 Sweep = Source.GetWidth() * Stride;   // One row of the atlas.
        const UInt32 Line  = Extent * Stride;              // One row of the face.

        Blob            Output = Blob::Allocate<Byte>(static_cast<UInt64>(Line) * Extent);
        const Ptr<Byte> Target = Output.GetData<Byte>();

        ConstPtr<Byte>  Origin = Source.GetPixels().GetData()
            + static_cast<UInt64>(Cell.Row) * Extent * Sweep
            + static_cast<UInt64>(Cell.Column) * Line;

        for (UInt32 Row = 0; Row < Extent; ++Row, Origin += Sweep)
        {
            if (Cell.Turned)
            {
                // Half a revolution reverses both axes, so the row fills backwards from the far corner.
                Ptr<Byte> Slot = Target + static_cast<UInt64>(Extent - 1 - Row) * Line + (Extent - 1) * Stride;

                for (UInt32 Column = 0; Column < Extent; ++Column, Slot -= Stride)
                {
                    Copy(Slot, Stride, Origin + Column * Stride);
                }
            }
            else
            {
                Copy(Target + static_cast<UInt64>(Row) * Line, Line, Origin);
            }
        }
        return Bitmap(Source.GetFormat(), Extent, Extent, 1, Move(Output));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sequence<Bitmap> Slicer::Generate(ConstRef<Bitmap> Source)
    {
        const Graphic::TextureFormat   Format   = Source.GetFormat();
        const Graphic::TextureMetadata Metadata = Graphic::GetTextureMetadata(Format);

        // Cutting addresses whole texels, which a block-compressed or bit-packed surface does not have.
        if (Metadata.IsCompressed() || Metadata.IsPacked || Metadata.Components == 0)
        {
            LOG_E("Texture: '{0}' must have its texels unpacked before it can be cut", Enum::GetName(Format));

            return Sequence<Bitmap>();
        }

        if (Source.GetLevels() != 1)
        {
            LOG_E("Texture: an atlas must be cut before its mips are filtered");

            return Sequence<Bitmap>();
        }

        const UInt32                Width  = Source.GetWidth();
        const UInt32                Height = Source.GetHeight();
        const ConstPtr<Arrangement> Shape  = Recognize(Width, Height);

        if (Shape == nullptr)
        {
            LOG_E("Texture: {0}x{1} holds no arrangement of six square faces", Width, Height);

            return Sequence<Bitmap>();
        }

        const UInt32 Extent = Width / Shape->Columns;
        const UInt32 Stride = Metadata.BitsPerPixel / 8;

        Sequence<Bitmap> Faces;
        Faces.Reserve(kFaces);

        for (ConstRef<Cell> Cell : Shape->Faces)
        {
            Faces.Append(Cut(Source, Cell, Extent, Stride));
        }
        return Faces;
    }
}