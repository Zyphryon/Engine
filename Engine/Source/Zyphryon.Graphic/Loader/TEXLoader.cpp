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

#include "TEXLoader.hpp"
#include "Zyphryon.Graphic/Image.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TEXLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        Reader Input(Data.GetData(), Data.GetSize());

        if (Input.Read<UInt32>() != ('Z' | ('T' << 8) | ('E' << 16) | ('X' << 24)))
        {
            LOG_W("'{}' is not a ZTEX file (bad magic)", Scope.GetResource()->GetKey());
            return false;
        }

        const TextureLayout   Layout  = Input.Read<TextureLayout>();
        const TextureFormat   Format  = Input.Read<TextureFormat>();
        const UInt16          Width   = Input.Read<UInt16>();
        const UInt16          Height  = Input.Read<UInt16>();
        const UInt8           Mipmaps = Input.Read<UInt8>();
        const UInt32          Size    = Input.Read<UInt32>();
        const ConstSpan<Byte> Payload = Input.ReadBlock<UInt32, Byte>();

        if (Size == 0 || Payload.IsEmpty())
        {
            LOG_W("'{}' has empty texture data", Scope.GetResource()->GetKey());
            return false;
        }

        // `Size` is the uncompressed byte count, so a payload smaller than `Size` is decompressed.
        Blob Buffer = Blob::Allocate<Byte>(Size);

        if (Size != Payload.GetSize())
        {
            if (LZ4Decode(Payload, Buffer.GetData<Byte>(), Size) != Size)
            {
                LOG_W("'{}' failed to decompress ({} compressed -> {} expected)",
                    Scope.GetResource()->GetKey(), Payload.GetSize(), Size);
                return false;
            }
        }
        else
        {
            Buffer.Copy<Byte>(Payload.GetData(), Size);
        }

        const Retainer<Image> Asset = Retainer<Image>::Cast(Scope.GetResource());
        Asset->Setup(Layout, Format, Width, Height, Mipmaps, Move(Buffer));
        return true;
    }
}
