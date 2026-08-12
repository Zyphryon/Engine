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
#include "Importer/TEXImporter.hpp"
#include "Process/Slicer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Texture
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

    Baker::Baker(Ref<Job::Service> Scheduler)
        : mScheduler { Scheduler }
    {
        Register(Retainer<STBImporter>::Create());
        Register(Retainer<TEXImporter>::Create());
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

        // The two divisions describe different layouts, so an atlas cannot be cut along both at once.
        if (Profile.Cube.IsValid() && Profile.Slice.IsValid())
        {
            LOG_E("Texture: a source is cut either into cube faces or into array slices, not both");

            return Blob();
        }

        Surface Decoded = Codec->Import(Source, Profile);

        if (!Decoded.IsValid())
        {
            return Blob();
        }

        // A source that carries its own slices is already divided, so a grid would name a second, other cut.
        if (Decoded.Slices.GetSize() > 1)
        {
            if (Profile.Cube.IsValid() || Profile.Slice.IsValid())
            {
                LOG_E("Texture: the source already holds {0} slices, so it cannot also be cut along a grid",
                    Decoded.Slices.GetSize());

                return Blob();
            }
            return Exporter::Export(mScheduler, Move(Decoded.Slices), Decoded.Layout, Profile);
        }

        ConstRef<Bitmap> Atlas = Decoded.Slices.GetFront();

        if (Profile.Cube.IsValid())
        {
            Sequence<Bitmap> Faces = Slicer::Slice(Atlas, Slicer::Describe(Profile.Cube.Width, Profile.Cube.Height));

            if (Faces.IsEmpty())
            {
                return Blob();
            }
            return Exporter::Export(mScheduler, Move(Faces), Graphic::TextureLayout::TextureCube, Profile);
        }

        if (Profile.Slice.IsValid())
        {
            const Slicer::Layout Layout = Slicer::Divide(Atlas, Profile.Slice.Width, Profile.Slice.Height);

            Sequence<Bitmap> Slices = Slicer::Slice(Atlas, Layout);

            if (Slices.IsEmpty())
            {
                return Blob();
            }
            return Exporter::Export(mScheduler, Move(Slices), Graphic::TextureLayout::Texture2DArray, Profile);
        }

        // A lone slice keeps the layout it was decoded as, so a one-layer array does not come back as a plain 2D.
        return Exporter::Export(mScheduler, Move(Decoded.Slices), Decoded.Layout, Profile);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Baker::Assemble(ConstSpan<Manifest::Entry> Entries, ConstRef<Profile> Profile) const
    {
        if (Entries.IsEmpty())
        {
            LOG_E("Texture: the manifest names no frames");

            return Blob();
        }

        // A manifest already names where every frame sits, so a grid would describe a second, other cut.
        if (Profile.Cube.IsValid() || Profile.Slice.IsValid())
        {
            LOG_E("Texture: a manifest gathers its own frames, so it cannot also be cut along a grid");

            return Blob();
        }

        // Every slice of an array shares one extent, so the first frame sets what the rest must match.
        const UInt16 Width  = Entries.GetFront().Width;
        const UInt16 Height = Entries.GetFront().Height;

        Sequence<Bitmap> Slices(Entries.GetSize());

        // Sheets are decoded once and reused, because a manifest usually cuts many frames out of each.
        Table<Str, Surface> Sheets;

        for (ConstRef<Manifest::Entry> Entry : Entries)
        {
            if (Entry.Width != Width || Entry.Height != Height)
            {
                LOG_E("Texture: frame '{0}' is {1}x{2}, but the array holds {3}x{4}",
                    Entry.Source, Entry.Width, Entry.Height, Width, Height);

                return Blob();
            }

            if (!Sheets.Contains(Entry.Source))
            {
                const ConstPtr<Importer> Codec = Find(StrAfterLast(Entry.Source, '.'));

                if (Codec == nullptr)
                {
                    LOG_E("Texture: '{0}' is not a source format this baker understands", Entry.Source);

                    return Blob();
                }

                Blob Input;

                if (Filesystem::Read(Entry.Source, Input) != Filesystem::Result::Success || Input == nullptr)
                {
                    LOG_E("Texture: failed to read '{0}'", Entry.Source);

                    return Blob();
                }

                Surface Decoded = Codec->Import(Input, Profile);

                if (Decoded.Slices.GetSize() > 1)
                {
                    LOG_W("Texture: '{0}' holds {1} slices, of which the manifest cuts only the first",
                        Entry.Source, Decoded.Slices.GetSize());
                }
                Sheets.Assign(Entry.Source, Move(Decoded));
            }

            ConstRef<Surface> Sheet = (* Sheets.Find(Entry.Source));

            if (!Sheet.IsValid())
            {
                return Blob();
            }

            Bitmap Frame = Slicer::Crop(Sheet.Slices.GetFront(), Entry.X, Entry.Y, Entry.Width, Entry.Height);

            if (Frame.GetPixels().IsEmpty())
            {
                return Blob();
            }

            Slices.Append(Move(Frame));
        }
        return Exporter::Export(mScheduler, Move(Slices), Graphic::TextureLayout::Texture2DArray, Profile);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Baker::Bake(Text Source, Text Destination, ConstRef<Profile> Profile) const
    {
        // A baked texture is a source now, so a bake can be handed the very path it is about to write over.
        if (StrEqualCase(Source, Destination))
        {
            LOG_E("Texture: '{0}' is both the source and the destination", Source);

            return false;
        }

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