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

#include "TEXImporter.hpp"
#include "Baker.Texture/Exporter.hpp"
#include <Zyphryon.Graphic/Metadata.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Texture
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Surface TEXImporter::Import(ConstSpan<Byte> Source, ConstRef<Profile> Profile) const
    {
        if (Source.IsEmpty())
        {
            LOG_E("Texture: source image is empty");

            return Surface();
        }

        Reader Input(Source.GetData(), static_cast<UInt32>(Source.GetSize()));

        if (Input.Read<UInt32>() != Exporter::kMagic)
        {
            LOG_E("Texture: source is not a ZTEX file (bad magic)");

            return Surface();
        }

        const UInt16 Version = Input.Read<UInt16>();

        if (Version != Exporter::kVersion)
        {
            LOG_E("Texture: ZTEX version {0} is not one this importer reads", Version);

            return Surface();
        }

        const Graphic::TextureLayout Layout  = Input.Read<Graphic::TextureLayout>();
        const Graphic::TextureFormat Format  = Input.Read<Graphic::TextureFormat>();
        const UInt16                 Width   = Input.Read<UInt16>();
        const UInt16                 Height  = Input.Read<UInt16>();
        const UInt16                 Layers  = Input.Read<UInt16>();
        const UInt8                  Levels  = Input.Read<UInt8>();
        const UInt32                 Size    = Input.Read<UInt32>();
        const ConstSpan<Byte>        Payload = Input.ReadBlock<UInt32, Byte>();

        if (Width == 0 || Height == 0 || Layers == 0 || Levels == 0 || Size == 0 || Payload.IsEmpty())
        {
            LOG_E("Texture: ZTEX header describes no surface to read back");

            return Surface();
        }

        // Cropping, filtering and transcoding each address one texel at a time, which a block-compressed or
        // bit-packed payload would only expose through a decoder this baker does not carry.
        const Graphic::TextureMetadata Description = Graphic::GetTextureMetadata(Format);

        if (Description.IsCompressed() || Description.IsPacked || !Description.IsSampler())
        {
            LOG_E("Texture: '{0}' stores no plain interleaved texels, so it cannot be read back", Enum::GetName(Format));

            return Surface();
        }

        // The payload is gathered slice-major, each slice holding its whole chain, so the two counts in the
        // header fix exactly how long it must be.
        const UInt32 Stride = Graphic::GetLevelOffset(Format, Width, Height, Levels);

        if (static_cast<UInt64>(Stride) * Layers != Size)
        {
            LOG_E("Texture: ZTEX payload is {0} bytes, but {1}x{2} over {3} slice(s) needs {4}",
                Size, Width, Height, Layers, static_cast<UInt64>(Stride) * Layers);

            return Surface();
        }

        // `Size` is the uncompressed byte count, so a shorter payload is compressed. LZ4 covers every slice in
        // one block, which has to be decoded whole before any slice can be cut out of it.
        Blob Scratch;

        if (Size != Payload.GetSize())
        {
            Scratch = Blob::Allocate<Byte>(Size);

            if (LZ4Decode(Payload, Scratch.GetData<Byte>(), Size) != Size)
            {
                LOG_E("Texture: ZTEX payload failed to decompress ({0} != {1})", Payload.GetSize(), Size);

                return Surface();
            }
        }

        const ConstPtr<Byte> Pixels = (Scratch == nullptr ? Payload.GetData() : Scratch.GetData<Byte>());

        // Every stage after an importer takes a single level, so each slice is read back at its base level
        // alone and the chain above it is filtered again when the bake asks for one.
        const UInt32 Length = Graphic::GetLevelSize(Format, Width, Height, 0);

        Surface Result;
        Result.Layout = Layout;
        Result.Slices.Reserve(Layers);

        for (UInt32 Slice = 0; Slice < Layers; ++Slice)
        {
            Blob Data = Blob::Allocate<Byte>(Length);
            Data.Copy<Byte>(Pixels + static_cast<UInt>(Slice) * Stride, Length);

            Result.Slices.Append(Bitmap(Format, Width, Height, 1, Move(Data)));
        }
        return Result;
    }
}