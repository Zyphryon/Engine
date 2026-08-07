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

#include "SKLLoader.hpp"
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Render/3D/Skeleton.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool SKLLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        Reader Input(Data.GetData(), Data.GetSize());

        if (Input.Read<UInt32>() != ('Z' | ('S' << 8) | ('K' << 16) | ('L' << 24)))
        {
            LOG_W("'{0}' is not a ZSKL file (bad magic)", Scope.GetResource()->GetKey());
            return false;
        }

        const UInt16 Version = Input.Read<UInt16>();

        if (Version != 1)
        {
            LOG_W("'{0}' has an unsupported ZSKL version", Scope.GetResource()->GetKey());
            return false;
        }

        const SInt16 Count = Input.Read<SInt16>();

        // The table is stored exactly as it sits in memory, so the whole rig lands in one copy.
        const UInt Length = Count * sizeof(Skeleton::Bone);

        if (Input.GetAvailable() < Length)
        {
            LOG_W("'{0}' has a truncated ZSKL bone table", Scope.GetResource()->GetKey());
            return false;
        }

        Sequence<Skeleton::Bone> Bones;
        Bones.Advance(Count);

        if (Count > 0)
        {
            Blit(Bones.GetData(), Length, Input.Read<ConstPtr<Skeleton::Bone>>(static_cast<UInt32>(Length)));
        }

        const Retainer<Skeleton> Asset = Retainer<Skeleton>::Cast(Scope.GetResource());
        Asset->SetBones(Move(Bones));
        return true;
    }
}