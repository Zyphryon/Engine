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

#include "MTLLoader.hpp"
#include "Parser.hpp"
#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MTLLoader::Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Material> Asset = Retainer<Material>::Cast(Scope.GetResource());

        // Parse Json document
        JsonValue JsonDocument = JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()));
        const JsonObject JsonRoot(JsonDocument);

        if (!JsonRoot.IsValid())
        {
            LOG_W("'{0}' is not a valid material document", Scope.GetResource()->GetKey());
            return false;
        }

        Parse(Service, Scope, JsonRoot, * Asset);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void MTLLoader::Parse(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, ConstRef<JsonObject> Root, Ref<Material> Asset)
    {
        // Parse 'Images' section, keyed by the texture name the technique declares.
        if (const JsonObject JsonImages = Root.GetObject("Images"); JsonImages.IsValid())
        {
            JsonImages.ForEach([&](ConstRef<Str> Key, Ref<JsonValue> Node)
            {
                if (!Node.IsObject())
                {
                    return;
                }

                const JsonObject JsonImage(Node);
                const Text       Path(JsonImage.GetString("Path"));

                Asset.SetImage(Hash(Key), Service.Load<Image>(Path, AddressOf(Scope)));
            });
        }

        // Parse 'Samplers' section, keyed by the sampler name the technique declares.
        if (const JsonObject JsonSamplers = Root.GetObject("Samplers"); JsonSamplers.IsValid())
        {
            JsonSamplers.ForEach([&](ConstRef<Str> Key, Ref<JsonValue> Node)
            {
                if (!Node.IsObject())
                {
                    return;
                }

                Sampler Defaults;
                Defaults.Filter = TextureFilter::Point;

                Asset.SetSampler(Hash(Key), ParseSampler(JsonObject(Node), Defaults));
            });
        }

        // Parse 'Parameters' section, keyed by the uniform name the technique declares.
        if (const JsonObject JsonParameters = Root.GetObject("Parameters"); JsonParameters.IsValid())
        {
            JsonParameters.ForEach([&](ConstRef<Str> Key, Ref<JsonValue> Node)
            {
                if (!Node.IsObject())
                {
                    return;
                }

                Asset.SetParameter(Hash(Key), ParseParameter(JsonObject(Node)));
            });
        }
    }
}