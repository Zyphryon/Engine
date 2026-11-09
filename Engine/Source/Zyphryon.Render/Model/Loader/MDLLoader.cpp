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
#include "Zyphryon.Render/Model/Model3D.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MDLLoader::Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Model3D> Asset = Retainer<Model3D>::Cast(Scope.GetResource());

        JsonValue Document;
        JsonError Error;

        if (!JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()), Document, Error))
        {
            LOG_W("'{0}' is not valid JSON: {1}", Asset->GetKey(), Error);
            return false;
        }

        if (!Document.IsObject())
        {
            LOG_W("'{0}' is not a valid model manifest", Asset->GetKey());
            return false;
        }

        const JsonObject Root(Document);

        // Resolve the referenced binary mesh, which the content system tracks as a dependency of this model.
        if (const Text Path = Root.GetString("Mesh"); !Path.IsEmpty())
        {
            Asset->SetMesh(Service.Load<ZyGraphic::Mesh>(Path, AddressOf(Scope)));
        }
        else
        {
            LOG_W("'{0}' model manifest has no mesh", Asset->GetKey());
            return false;
        }

        // Resolve the bone hierarchy when the model deforms, since a model without one is static geometry.
        if (const Text Path = Root.GetString("Skeleton"); !Path.IsEmpty())
        {
            Asset->SetSkeleton(Service.Load<Skeleton3D>(Path, AddressOf(Scope)));
        }
        else
        {
            Asset->SetSkeleton(Retainer<Skeleton3D>());
        }

        // Resolve the material table, in the slot order primitives reference by index.
        const JsonArray Materials = Root.GetArray("Materials");

        Sequence<Retainer<ZyGraphic::Material>> Table;
        Table.Reserve(Materials.GetSize());

        for (UInt Slot = 0; Slot < Materials.GetSize(); ++Slot)
        {
            Retainer<ZyGraphic::Material> Object;

            if (const JsonObject Definition = Materials.GetObject(Slot); Definition.IsValid())
            {
                Object = Retainer<ZyGraphic::Material>::Create(ZyContent::Uri(Asset->GetKey()));
                Object->SetPolicy(ZyContent::Resource::Policy::Exclusive);

                ZyGraphic::MTLLoader::Parse(Service, Scope, Definition, * Object);
            }
            else
            {
                Object = Service.Load<ZyGraphic::Material>(Materials.GetString(Slot), AddressOf(Scope));
            }
            Table.Append(Move(Object));
        }
        Asset->SetMaterials(Move(Table));

        const JsonArray List = Root.GetArray("Clips");

        Sequence<Model3D::Clip> Clips;
        Clips.Reserve(List.GetSize());

        for (UInt Index = 0; Index < List.GetSize(); ++Index)
        {
            const JsonObject Definition = List.GetObject(Index);
            const Text       Name       = Definition.GetString("Name");

            Ref<Model3D::Clip> Clip = Clips.Append();
            Clip.Name   = Hash(Name);
            Clip.Label  = Name;
            Clip.Motion = Service.Load<Animation3D>(Definition.GetString("Path"), AddressOf(Scope));
            Clip.Mode   = Definition.GetEnum("Repeat", Repeat::Loop);
            Clip.Rate   = Max(Definition.GetNumber("Rate", 1.0f), 0.0f);
        }
        Asset->SetClips(Move(Clips));
        return true;
    }
}