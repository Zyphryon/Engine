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
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Graphic/Loader/MTLLoader.hpp"
#include "Zyphryon.Render/Resource/Model.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
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
            LOG_W("'{0}' is not a valid model manifest", Scope.GetResource()->GetKey());
            return false;
        }

        // Resolve the referenced binary mesh; the content system tracks it as a dependency of this model.
        if (const Text Path = Root.GetString("Mesh"); !Path.IsEmpty())
        {
            Asset->SetMesh(Service.Load<Graphic::Mesh>(Path, AddressOf(Scope)));
        }
        else
        {
            LOG_W("'{0}' model manifest has no mesh", Scope.GetResource()->GetKey());
            return false;
        }

        // Bounds wide enough for every pose the model can strike, which only deforming geometry needs to state;
        // anything else is culled by the extent its mesh already reports.
        if (const JsonArray Bounds = Root.GetArray("Bounds"); Bounds.GetSize() >= 6)
        {
            Asset->SetBounds(Box(
                Bounds.GetNumber<Real32>(0), Bounds.GetNumber<Real32>(1), Bounds.GetNumber<Real32>(2),
                Bounds.GetNumber<Real32>(3), Bounds.GetNumber<Real32>(4), Bounds.GetNumber<Real32>(5)));
        }

        // Resolve the material table, in the slot order primitives reference by index.
        if (const JsonArray Materials = Root.GetArray("Materials"); !Materials.IsNullOrEmpty())
        {
            for (UInt Slot = 0, Count = Materials.GetSize(); Slot < Count; ++Slot)
            {
                Retainer<Graphic::Material> Object;

                if (const JsonObject Definition = Materials.GetObject(Slot); Definition.IsValid())
                {
                    Object = Retainer<Graphic::Material>::Create(Asset->GetKey());
                    Object->SetPolicy(Content::Resource::Policy::Exclusive);

                    Graphic::MTLLoader::Parse(Service, Scope, Definition, * Object);
                }
                else
                {
                    Object = Service.Load<Graphic::Material>(Materials.GetString(Slot), AddressOf(Scope));
                }
                Asset->AddMaterial(Object);
            }
        }
        return true;
    }
}
