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

    static void ReadImage(
        Ref<ZyContent::Service> Service,
        Ref<ZyContent::Scope>   Scope,
        Ref<Material>           Asset,
        ConstRef<Str>           Key,
        Ref<JsonValue>          Node)
    {
        if (Node.IsObject())
        {
            const JsonObject JsonImage(Node);
            const Text       Path(JsonImage.GetString("Path"));

            Asset.SetImage(Hash(Key), Service.Load<Image>(Path, AddressOf(Scope)));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void ReadSampler(Ref<Material> Asset, ConstRef<Str> Key, Ref<JsonValue> Node)
    {
        if (Node.IsObject())
        {
            Sampler Defaults;
            Defaults.Filter = TextureFilter::Point;

            Asset.SetSampler(Hash(Key), ParseSampler(JsonObject(Node), Defaults));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void ReadParameter(Ref<Material> Asset, ConstRef<Str> Key, Ref<JsonValue> Node)
    {
        if (Node.IsObject())
        {
            Asset.SetParameter(Hash(Key), ParseParameter(JsonObject(Node)));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MTLLoader::Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Material> Asset = Retainer<Material>::Cast(Scope.GetResource());

        JsonValue Document;
        JsonError Error;

        if (!JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()), Document, Error))
        {
            LOG_W("'{0}' is not valid JSON: {1}", Asset->GetKey(), Error);
            return false;
        }

        if (!Document.IsObject())
        {
            LOG_W("'{0}' is not a valid material document", Asset->GetKey());
            return false;
        }

        const JsonObject JsonRoot(Document);

        Parse(Service, Scope, JsonRoot, * Asset);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void MTLLoader::Parse(
        Ref<ZyContent::Service> Service,
        Ref<ZyContent::Scope>   Scope,
        ConstRef<JsonObject>    Root,
        Ref<Material>           Asset)
    {
        if (const JsonObject JsonImages = Root.GetObject("Images"); JsonImages.IsValid())
        {
            JsonImages.ForEach([&](ConstRef<Str> Key, Ref<JsonValue> Node)
            {
                ReadImage(Service, Scope, Asset, Key, Node);
            });
        }

        if (const JsonObject JsonSamplers = Root.GetObject("Samplers"); JsonSamplers.IsValid())
        {
            JsonSamplers.ForEach([&](ConstRef<Str> Key, Ref<JsonValue> Node)
            {
                ReadSampler(Asset, Key, Node);
            });
        }

        if (const JsonObject JsonParameters = Root.GetObject("Parameters"); JsonParameters.IsValid())
        {
            JsonParameters.ForEach([&](ConstRef<Str> Key, Ref<JsonValue> Node)
            {
                ReadParameter(Asset, Key, Node);
            });
        }
    }
}