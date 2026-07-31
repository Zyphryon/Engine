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

#include "Baker.hpp"
#include "Importer/STBImporter.hpp"
#include "Transform/Slicer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Texture
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Baker::Extension Normalize(Text Type)
    {
        const Text Trimmed = (!Type.IsEmpty() && Type[0] == '.') ? Type.Slice(1) : Type;

        Baker::Extension Result(Trimmed.GetSize());

        for (UInt Index = 0; Index < Trimmed.GetSize(); ++Index)
        {
            Result.Append(StrLowercase(Trimmed[Index]));
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Baker::Baker()
    {
        Register(Retainer<Codec::STBImporter>::Create());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Register(ConstRetainer<Importer> Codec)
    {
        ZY_ASSERT(Codec != nullptr, "Cannot register a null importer");

        for (const Text Type : Codec->GetTypes())
        {
            mRegistry.Assign(Normalize(Type), Codec);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Baker::Unregister(Text Type)
    {
        return mRegistry.Erase(Normalize(Type));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<Importer> Baker::Find(Text Type) const
    {
        const Extension Key = Normalize(Type);

        if (const ConstPtr<Retainer<Importer>> Found = mRegistry.Find(Key))
        {
            return static_cast<Ptr<Importer>>(* Found);
        }
        return nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Baker::Bake(ConstSpan<Byte> Source, Text Type, ConstRef<Profile> Profile) const
    {
        const ConstPtr<Importer> Codec = Find(Type);

        if (Codec == nullptr)
        {
            LOG_E("Texture: '{0}' is not a source format this baker understands", Type);

            return Blob();
        }

        Bitmap Surface = Codec->Import(Source, Profile);

        if (Surface.GetPixels().IsEmpty())
        {
            return Blob();
        }

        if (Profile.Cube)
        {
            Sequence<Bitmap> Faces = Slicer::Generate(Surface);

            if (Faces.IsEmpty())
            {
                return Blob();
            }
            return Exporter::Export(Move(Faces), Graphic::TextureLayout::TextureCube, Profile);
        }
        return Exporter::Export(Move(Surface), Profile);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Baker::Bake(Text Source, Text Destination, ConstRef<Profile> Profile) const
    {
        Blob Input;

        if (Filesystem::Read(Source, Input) != Filesystem::Result::Success || Input == nullptr)
        {
            LOG_E("Texture: failed to read '{0}'", Source);

            return false;
        }

        const Blob Output = Bake(Input, StrAfterLast(Source, '.'), Profile);

        if (Output == nullptr)
        {
            return false;
        }

        Filesystem::Ensure(Destination);

        if (Filesystem::Write(Destination, Output) != Filesystem::Result::Success)
        {
            LOG_E("Texture: failed to write '{0}'", Destination);

            return false;
        }

        LOG_I("Texture: '{0}' -> '{1}' ({2} bytes)", Source, Destination, Output.GetSize());
        return true;
    }
}