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

#include "FNTLoader.hpp"
#include "Zyphryon.Graphic/Loader/TEXLoader.hpp"
#include "Zyphryon.Render/Font/Font.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool FNTLoader::Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Font> Asset = Retainer<Font>::Cast(Scope.GetResource());

        Reader Input(Data);

        if (Input.Read<UInt32>() != ('Z' | ('F' << 8) | ('N' << 16) | ('T' << 24)))
        {
            LOG_W("'{0}' is not a ZFNT file (bad magic)", Asset->GetKey());
            return false;
        }

        if (Input.Read<UInt16>() != 1)
        {
            LOG_W("'{0}' has an unsupported ZFNT version", Asset->GetKey());
            return false;
        }

        Archive Serializer(Input);

        Font::Metrics Metrics;
        Serializer.Serialize(Metrics);

        Font::Glyphs Glyphs;
        Serializer.Serialize(Glyphs);

        // Optional (KERN) and variable-count (ATLS) sections follow as a tag-length-value chunk stream.
        Font::Kerning Kerning;
        Font::Atlases Atlases;

        while (Input.GetAvailable() >= 2 * sizeof(UInt32))
        {
            const UInt32 Tag    = Input.Read<UInt32>();
            const UInt32 Length = Input.Read<UInt32>();

            if (Length > Input.GetAvailable())
            {
                LOG_W("'{0}' has a truncated ZFNT chunk", Asset->GetKey());
                return false;
            }

            Reader  Body = Input.Split(Length);
            Archive Chunk(Body);

            switch (Tag)
            {
            case kKerning:
                Chunk.Serialize(Kerning);
                break;
            case kAtlas:
            {
                // The font alone owns the page's image and material.
                const Retainer<ZyGraphic::Image> Page = Retainer<ZyGraphic::Image>::Create("Atlas");
                Page->SetPolicy(ZyContent::Resource::Policy::Exclusive);

                if (!ZyGraphic::TEXLoader::Parse(Body, * Page))
                {
                    return false;
                }

                ConstRetainer<ZyGraphic::Material> Material = Atlases.Append(Retainer<ZyGraphic::Material>::Create("Material"));
                Material->SetPolicy(ZyContent::Resource::Policy::Exclusive);
                Material->SetImage("Albedo"_Hash, Page);
                Material->SetParameter(
                    "Range"_Hash,
                    Vector2(Metrics.Distance / Page->GetWidth(), Metrics.Distance / Page->GetHeight()));
                break;
            }
            default:
                break;
            }
        }

        if (Atlases.IsEmpty())
        {
            LOG_W("'{0}' is missing its atlas chunk", Asset->GetKey());
            return false;
        }

        Asset->Setup(Move(Metrics), Move(Glyphs), Move(Kerning), Move(Atlases));
        return true;
    }
}