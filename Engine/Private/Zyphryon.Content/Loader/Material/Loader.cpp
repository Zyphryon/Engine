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

#include "Loader.hpp"
#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MaterialLoader::Load(Ref<Service> Service, Ref<Scope> Scope, AnyRef<Blob> Data)
    {
        TOMLParser        Parser(Data.GetText());
        const TOMLSection Textures = Parser.GetSection("Textures");
        const TOMLSection Samplers = Parser.GetSection("Samplers");

        const Tracker<Graphic::Material> Asset = Tracker<Graphic::Material>::Cast(Scope.GetResource());

        // Parse 'textures' section
        for (const Graphic::TextureSemantic Semantic : Enum::Values<Graphic::TextureSemantic>())
        {
            if (const TOMLSection Texture = Textures.GetSection(Enum::Name(Semantic), false); !Texture.IsNull())
            {
                // Parse texture
                Asset->SetTexture(Semantic, Service.Load<Graphic::Texture>(Texture.GetString("Path"), &Scope));

                // Parse sampler
                if (const TOMLSection Sampler = Samplers.GetSection(Texture.GetString("Sampler")); !Sampler.IsNull())
                {
                    Graphic::Sampler Description;
                    Description.AddressModeU = Sampler.GetEnum("AddressModeU", Graphic::TextureAddress::Clamp);
                    Description.AddressModeV = Sampler.GetEnum("AddressModeV", Graphic::TextureAddress::Clamp);
                    Description.AddressModeW = Sampler.GetEnum("AddressModeW", Graphic::TextureAddress::Clamp);
                    Description.Filter       = Sampler.GetEnum("Filter", Graphic::TextureFilter::Point);
                    Description.Comparison   = Sampler.GetEnum("Comparison", Graphic::TestCondition::Always);
                    Description.Border       = Sampler.GetEnum("Border", Graphic::TextureBorder::OpaqueBlack);

                    Asset->SetSampler(Semantic, Description);
                }
            }
        }

        // TODO: Parameter(s)
        return true;
    }
}