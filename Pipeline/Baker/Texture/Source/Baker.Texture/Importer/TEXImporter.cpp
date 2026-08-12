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

    Bitmap TEXImporter::Import(ConstSpan<Byte> Source, ConstRef<Profile> Profile) const
    {
        if (Source.IsEmpty())
        {
            LOG_E("Texture: source image is empty");

            return Bitmap();
        }

        Reader Input(Source.GetData(), static_cast<UInt32>(Source.GetSize()));

        if (Input.Read<UInt32>() != Exporter::kMagic)
        {
            LOG_E("Texture: source is not a ZTEX file (bad magic)");

            return Bitmap();
        }

        const UInt16 Version = Input.Read<UInt16>();

        if (Version != Exporter::kVersion)
        {
            LOG_E("Texture: ZTEX version {0} is not one this importer reads", Version);

            return Bitmap();
        }

        Input.Skip(sizeof(Graphic::TextureLayout));

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

            return Bitmap();
        }

        // Cropping, filtering and transcoding each address one texel at a time, which a block-compressed or
        // bit-packed payload would only expose through a decoder this baker does not carry.
        const Graphic::TextureMetadata Description = Graphic::GetTextureMetadata(Format);

        if (Description.IsCompressed() || Description.IsPacked || !Description.IsSampler())
        {
            LOG_E("Texture: '{0}' stores no plain interleaved texels, so it cannot be read back", Enum::GetName(Format));

            return Bitmap();
        }

        // The payload is gathered slice-major, each slice holding its whole chain, so the two counts in the
        // header fix exactly how long it must be.
        const UInt32 Stride = Graphic::GetLevelOffset(Format, Width, Height, Levels);

        if (static_cast<UInt64>(Stride) * Layers != Size)
        {
            LOG_E("Texture: ZTEX payload is {0} bytes, but {1}x{2} over {3} slice(s) needs {4}",
                Size, Width, Height, Layers, static_cast<UInt64>(Stride) * Layers);

            return Bitmap();
        }

        // An importer hands back one surface and every stage after it takes a single level, so the rest of a
        // sliced file is dropped rather than folded into the bake.
        if (Layers > 1)
        {
            LOG_W("Texture: the source holds {0} slices, of which only the first is read back", Layers);
        }

        const UInt32 Length = Graphic::GetLevelSize(Format, Width, Height, 0);

        Blob Data = Blob::Allocate<Byte>(Length);

        // `Size` is the uncompressed byte count, so a shorter payload is compressed. LZ4 covers every slice in
        // one block, which has to be decoded whole before the base level can be cut out of its front.
        if (Size != Payload.GetSize())
        {
            Blob Scratch = Blob::Allocate<Byte>(Size);

            if (LZ4Decode(Payload, Scratch.GetData<Byte>(), Size) != Size)
            {
                LOG_E("Texture: ZTEX payload failed to decompress ({0} != {1})", Payload.GetSize(), Size);

                return Bitmap();
            }
            Data.Copy<Byte>(Scratch.GetData<Byte>(), Length);
        }
        else
        {
            Data.Copy<Byte>(Payload.GetData(), Length);
        }
        return Bitmap(Format, Width, Height, 1, Move(Data));
    }
}