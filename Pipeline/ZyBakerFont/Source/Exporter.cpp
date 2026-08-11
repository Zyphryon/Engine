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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Font
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Exporter::Export(
        Ref<Render::Font::Metrics> Metrics,
        Ref<Render::Font::Glyphs>  Glyphs,
        Ref<Render::Font::Kerning> Kerning,
        ConstSpan<Blob>            Pages,
        UInt16                     Side)
    {
        if (Glyphs.IsEmpty() || Pages.IsEmpty())
        {
            LOG_E("Font: there is nothing to write");

            return Blob();
        }

        UInt Capacity = 1024;

        for (ConstRef<Blob> Page : Pages)
        {
            Capacity += Page.GetSize();
        }

        Writer Output(Capacity);
        Output.Write<UInt32>(kMagic);
        Output.Write<UInt16>(kVersion);

        Archive Serializer(Output);
        Serializer.Serialize(Metrics);
        Serializer.Serialize(Glyphs);

        if (!Kerning.IsEmpty())
        {
            Output.Write<UInt32>(kKerning);
            Output.WriteBlock<UInt32>([&Kerning](Ref<Writer> Body)
            {
                Archive Chunk(Body);
                Chunk.Serialize(Kerning);
            });
        }

        // One chunk per page, written in page order so the loader can index them by the page a glyph carries.
        for (ConstRef<Blob> Page : Pages)
        {
            const ConstSpan Texels(Page.GetData(), Page.GetSize());

            Output.Write<UInt32>(kAtlas);
            Output.WriteBlock<UInt32>([&Texels, Side](Ref<Writer> Body)
            {
                Body.Write<UInt32>(kTexture);
                Body.Write<UInt16>(1);
                Body.Write<Graphic::TextureLayout>(Graphic::TextureLayout::Texture2D);
                Body.Write<Graphic::TextureFormat>(Graphic::TextureFormat::RGBA8UIntNorm);
                Body.Write<UInt16>(Side);
                Body.Write<UInt16>(Side);
                Body.Write<UInt16>(1);
                Body.Write<UInt8>(1);
                Body.Write<UInt32>(Texels.GetSize());

                // The loader reads a payload the same size as the raw count as uncompressed, so only a payload
                // that actually shrank is worth keeping.
                Blob         Scratch = Blob::Allocate<Byte>(LZ4Bound(Texels.GetSize()));
                const UInt32 Size    = LZ4Encode(Texels, Scratch.GetData<Byte>(), LZ4Bound(Texels.GetSize()), kCompression);

                if (Size > 0 && Size < Texels.GetSize())
                {
                    Body.WriteBlock<UInt32, Byte>(ConstSpan(Scratch.GetData(), Size));
                    return;
                }
                Body.WriteBlock<UInt32, Byte>(Texels);
            });
        }

        return Output.Detach();
    }
}