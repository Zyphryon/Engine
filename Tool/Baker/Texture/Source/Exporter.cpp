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

#include "Exporter.hpp"
#include "Transform/Converter.hpp"
#include "Transform/Mipmapper.hpp"
#include <Zyphryon.Graphic/Metadata.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Texture
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Exporter::IsSupported(Graphic::TextureFormat Format)
    {
        const Graphic::TextureMetadata Description = Graphic::GetTextureMetadata(Format);

        // TODO: Block-compressed

        // Block-compressed, bit-packed, depth, and non sampleable formats each need an encoder this exporter
        // does not have, and would otherwise silently receive plain interleaved bytes.
        if (Description.IsCompressed() ||  Description.IsPacked || Description.IsDepth
         || Description.IsStencil      || !Description.IsSampler())
        {
            return false;
        }

        // The sampler produces normalized or floating-point values, so a raw integer format would have its
        // bits reinterpreted rather than scaled.
        if (!Description.IsFloat && !Description.IsNormalized)
        {
            return false;
        }

        const UInt32 Bits = Description.BitsPerComponent();
        return Description.IsFloat ? (Bits == 16 || Bits == 32) : (Bits == 8 || Bits == 16);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Exporter::Export(AnyRef<Bitmap> Source, ConstRef<Profile> Profile)
    {
        Sequence<Bitmap> Slices;
        Slices.Append(Move(Source));

        return Export(Move(Slices), Graphic::TextureLayout::Texture2D, Profile);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Exporter::Export(AnyRef<Sequence<Bitmap>> Slices, Graphic::TextureLayout Layout, ConstRef<Profile> Profile)
    {
        if (Slices.IsEmpty())
        {
            return Blob();
        }

        const UInt16 Width  = Slices[0].GetWidth();
        const UInt16 Height = Slices[0].GetHeight();

        const Graphic::TextureFormat Format =
            (Profile.Format == Graphic::TextureFormat::Unspecified) ? Slices[0].GetFormat() : Profile.Format;

        if (!IsSupported(Format))
        {
            LOG_E("Texture: '{0}' is not a format this exporter can write", Enum::GetName(Format));

            return Blob();
        }

        for (ConstRef<Bitmap> Slice : Slices)
        {
            if (Slice.GetWidth() != Width || Slice.GetHeight() != Height)
            {
                LOG_E("Texture: every slice must share one extent ({0}x{1} against {2}x{3})",
                    Slice.GetWidth(), Slice.GetHeight(), Width, Height);

                return Blob();
            }
        }

        // Filtering runs at each surface's own depth, so the conversion to the target format happens last.
        const UInt8 Levels = Profile.Mipmaps ? Graphic::GetLevelCount(Width, Height) : 1;

        Sequence<Bitmap> Baked;
        Baked.Reserve(Slices.GetSize());

        UInt32 Length = 0;

        for (Ref<Bitmap> Slice : Slices)
        {
            Ref<Bitmap> Result = Baked.Append(Converter::Convert(Mipmapper::Generate(Move(Slice), Levels), Format));

            // TODO: Resizer

            if (Result.GetPixels().IsEmpty())
            {
                return Blob();
            }
            Length += Result.GetPixels().GetSize();
        }

        // Gathered slice-major, which is the order the loader and both drivers read a sliced payload in.
        Blob      Payload = Blob::Allocate<Byte>(Length);
        Ptr<Byte> Cursor  = Payload.GetData<Byte>();

        for (ConstRef<Bitmap> Result : Baked)
        {
            const ConstSpan<Byte> Bytes = Result.GetPixels();

            Copy(Cursor, Bytes.GetSize(), Bytes.GetData());
            Cursor += Bytes.GetSize();
        }

        const ConstSpan<Byte> Bytes = ConstSpan(Payload.GetData<Byte>(), Length);

        Writer Output(Length + 32);
        Output.Write<UInt32>(kMagic);
        Output.Write<UInt16>(kVersion);
        Output.Write<Graphic::TextureLayout>(Layout);
        Output.Write<Graphic::TextureFormat>(Format);
        Output.Write<UInt16>(Width);
        Output.Write<UInt16>(Height);
        Output.Write<UInt16>(static_cast<UInt16>(Baked.GetSize()));
        Output.Write<UInt8>(Baked[0].GetLevels());
        Output.Write<UInt32>(Length);

        // The loader reads a payload the same size as the raw count as uncompressed, so only a payload that
        // actually shrank is worth keeping.
        if (Profile.Compress)
        {
            Blob         Scratch = Blob::Allocate<Byte>(LZ4Bound(Length));
            const UInt32 Size    = LZ4Encode(Bytes, Scratch.GetData<Byte>(), LZ4Bound(Length));

            if (Size > 0 && Size < Length)
            {
                Output.WriteBlock<UInt32, Byte>(ConstSpan(Scratch.GetData(), Size));
                return Output.Detach();
            }
        }

        Output.WriteBlock<UInt32, Byte>(Bytes);
        return Output.Detach();
    }
}
