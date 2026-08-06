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

        // Resolve the bone hierarchy, when the model deforms. Absent means static geometry.
        if (const Text Path = Root.GetString("Skeleton"); !Path.IsEmpty())
        {
            Asset->SetSkeleton(Service.Load<Skeleton>(Path, AddressOf(Scope)));
        }

        // Resolve the material table, in the slot order primitives reference by index.
        if (const JsonArray Materials = Root.GetArray("Materials"); !Materials.IsNullOrEmpty())
        {
            const UInt Count = Materials.GetSize();

            Sequence<Retainer<Graphic::Material>> Table;
            Table.Reserve(Count);

            for (UInt Slot = 0; Slot < Count; ++Slot)
            {
                Retainer<Graphic::Material> Object;

                if (const JsonObject Definition = Materials.GetObject(Slot); Definition.IsValid())
                {
                    Object = Retainer<Graphic::Material>::Create(Content::Uri(Asset->GetKey()));
                    Object->SetPolicy(Content::Resource::Policy::Exclusive);

                    Graphic::MTLLoader::Parse(Service, Scope, Definition, * Object);
                }
                else
                {
                    Object = Service.Load<Graphic::Material>(Materials.GetString(Slot), AddressOf(Scope));
                }
                Table.Append(Move(Object));
            }
            Asset->SetMaterials(Move(Table));
        }
        return true;
    }
}