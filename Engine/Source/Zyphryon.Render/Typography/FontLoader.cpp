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

#include "FontLoader.hpp"
#include "Font.hpp"
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Graphic/Loader/TEXLoader.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool FontLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        Reader Input(Data.GetData(), Data.GetSize());

        if (Input.Read<UInt32>() != ('Z' | ('F' << 8) | ('N' << 16) | ('T' << 24)))
        {
            LOG_W("'{}' is not a ZFNT file (bad magic)", Scope.GetResource()->GetKey());
            return false;
        }

        if (Input.Read<UInt16>() != 1)
        {
            LOG_W("'{}' has an unsupported ZFNT version", Scope.GetResource()->GetKey());
            return false;
        }

        Archive Serializer(Input);

        // Load mandatory font metrics.
        Font::Metrics Metrics;
        Serializer.Serialize(Metrics);

        // Load mandatory glyph table.
        Font::Glyphs Glyphs;
        Serializer.Serialize(Glyphs);

        // Optional (KERN) and variable-count (ATLS) sections follow as a tag-length-value chunk stream.
        Font::Kerning            Kerning;
        Retainer<Graphic::Image> Atlas;

        while (Input.GetAvailable() >= 2 * sizeof(UInt32))
        {
            const UInt32 Tag    = Input.Read<UInt32>();
            const UInt32 Length = Input.Read<UInt32>();

            if (Length > Input.GetAvailable())
            {
                LOG_W("'{}' has a truncated ZFNT chunk", Scope.GetResource()->GetKey());
                return false;
            }

            Reader  Body = Input.Split(Length);
            Archive Chunk(Body);

            switch (Tag)
            {
            case ('K' | ('E' << 8) | ('R' << 16) | ('N' << 24)):
                Chunk.Serialize(Kerning);
                break;
            case ('A' | ('T' << 8) | ('L' << 16) | ('S' << 24)):
            {
                Atlas = Retainer<Graphic::Image>::Create("Atlas");
                Atlas->SetPolicy(Content::Resource::Policy::Exclusive);

                if (!Graphic::TEXLoader::Parse(Scope, Body, * Atlas))
                {
                    return false;
                }
                break;
            }
            default:
                break;
            }
        }

        if (!Atlas)
        {
            LOG_W("'{}' is missing its atlas chunk", Scope.GetResource()->GetKey());
            return false;
        }

        // The atlas image and material are owned exclusively by the font.
        const Retainer<Graphic::Material> Material = Retainer<Graphic::Material>::Create("Material");
        Material->SetPolicy(Content::Resource::Policy::Exclusive);
        Material->SetImage(Graphic::TextureSlot::Albedo, Atlas);
        Material->SetParameter("Range"_Hash,
            Vector2(Metrics.Distance / Atlas->GetWidth(),
                    Metrics.Distance / Atlas->GetHeight()));

        const Retainer<Font> Asset = Retainer<Font>::Cast(Scope.GetResource());
        Asset->Setup(Move(Metrics), Move(Glyphs), Move(Kerning), Material);
        return true;
    }
}
