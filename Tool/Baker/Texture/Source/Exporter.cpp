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
        const Graphic::TextureFormat Format = (Profile.Format == Graphic::TextureFormat::Unspecified) ? Source.GetFormat() : Profile.Format;

        if (!IsSupported(Format))
        {
            LOG_E("Texture: '{0}' is not a format this exporter can write", Enum::GetName(Format));

            return Blob();
        }

        // Filtering runs at the surface's own depth, so the conversion to the target format happens once, last.
        const UInt8  Levels = Profile.Mipmaps ? Graphic::GetLevelCount(Source.GetWidth(), Source.GetHeight()) : 1;
        const Bitmap Result = Converter::Convert(Mipmapper::Generate(Move(Source), Levels), Format);

        // TODO: Resizer

        if (Result.GetPixels().IsEmpty())
        {
            return Blob();
        }

        const ConstSpan<Byte> Bytes  = Result.GetPixels();
        const UInt32          Length = Bytes.GetSize();

        Writer Output(Length + 32);
        Output.Write<UInt32>(kMagic);
        Output.Write<UInt16>(kVersion);
        Output.Write<Graphic::TextureLayout>(Graphic::TextureLayout::Texture2D);
        Output.Write<Graphic::TextureFormat>(Result.GetFormat());
        Output.Write<UInt16>(Result.GetWidth());
        Output.Write<UInt16>(Result.GetHeight());
        Output.Write<UInt8>(Result.GetLevels());
        Output.Write<UInt32>(Length);

        // The loader reads a payload the same size as the raw count as uncompressed, so only a payload that
        // actually shrank is worth keeping.
        if (Profile.Compress)
        {
            Blob         Scratch = Blob::Allocate<Byte>(LZ4Bound(Length));
            const UInt32 Size    = LZ4Encode(Result.GetPixels(), Scratch.GetData<Byte>(), LZ4Bound(Length));

            if (Size > 0 && Size < Length)
            {
                Output.WriteBlock<UInt32, Byte>(ConstSpan(Scratch.GetData(), Size));
                return Output.Detach();
            }
        }

        Output.WriteBlock<UInt32, Byte>(Result.GetPixels());
        return Output.Detach();
    }
}
