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

#include "MDLLoader.hpp"
#include "MTLLoader.hpp"
#include "Zyphryon.Graphic/Model.hpp"
#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MDLLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Model> Asset = Retainer<Model>::Cast(Scope.GetResource());

        JsonValue        Document = JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()));
        const JsonObject Root(Document);

        if (!Root.IsValid())
        {
            LOG_W("'{}' is not a valid model manifest", Scope.GetResource()->GetKey());
            return false;
        }

        // Resolve the referenced binary mesh; the content system tracks it as a dependency of this model.
        if (const Text Path = Root.GetString("Mesh"); !Path.IsEmpty())
        {
            Asset->SetMesh(Service.Load<Mesh>(Path, AddressOf(Scope)));
        }
        else
        {
            LOG_W("'{}' model manifest has no mesh", Scope.GetResource()->GetKey());
            return false;
        }

        // Resolve the material table, in the slot order primitives reference by index.
        if (const JsonArray Materials = Root.GetArray("Materials"); !Materials.IsNullOrEmpty())
        {
            for (UInt Slot = 0, Count = Materials.GetSize(); Slot < Count; ++Slot)
            {
                Retainer<Material> Object;

                if (const JsonObject Definition = Materials.GetObject(Slot); Definition.IsValid())
                {
                    Object = Retainer<Graphic::Material>::Create(Asset->GetKey());
                    Object->SetPolicy(Content::Resource::Policy::Exclusive);

                    MTLLoader::Parse(Service, Scope, Definition, * Object);
                }
                else
                {
                    Object = Service.Load<Material>(Materials.GetString(Slot), AddressOf(Scope));
                }
                Asset->AddMaterial(Object);
            }
        }
        return true;
    }
}
