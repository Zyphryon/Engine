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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TEXLoader::Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data)
    {
        Reader Input(Data);

        return Parse(Input, * Retainer<Image>::Cast(Scope.GetResource()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TEXLoader::Parse(Ref<Reader> Input, Ref<Image> Asset)
    {
        if (Input.Read<UInt32>() != ('Z' | ('T' << 8) | ('E' << 16) | ('X' << 24)))
        {
            LOG_W("'{0}' is not a ZTEX file (bad magic)", Asset.GetKey());
            return false;
        }

        if (Input.Read<UInt16>() != 1)
        {
            LOG_W("'{0}' has an unsupported ZTEX version", Asset.GetKey());
            return false;
        }

        const TextureLayout   Layout  = Input.Read<TextureLayout>();
        const TextureFormat   Format  = Input.Read<TextureFormat>();
        const UInt16          Width   = Input.Read<UInt16>();
        const UInt16          Height  = Input.Read<UInt16>();
        const UInt16          Layers  = Input.Read<UInt16>();
        const UInt8           Mipmaps = Input.Read<UInt8>();
        const UInt32          Size    = Input.Read<UInt32>();
        const ConstSpan<Byte> Payload = Input.ReadBlock<UInt32, Byte>();

        if (Size == 0 || Payload.IsEmpty())
        {
            LOG_W("'{0}' has empty texture data", Asset.GetKey());
            return false;
        }

        Blob Buffer = LZ4Expand(Payload, Size);

        if (!Buffer)
        {
            LOG_W("'{0}' failed to decompress ({1} != {2})", Asset.GetKey(), Payload.GetSize(), Size);
            return false;
        }

        Asset.Setup(Layout, Format, Width, Height, Layers, Mipmaps, Move(Buffer));

        // The outlines trail the payload as an optional block, so a file baked without them ends right here.
        if (Input.GetAvailable() > 0)
        {
            const UInt16 Count  = Input.Read<UInt16>();
            const UInt   Length = static_cast<UInt>(Count) * Image::Outline().GetSize() * 2 * sizeof(UInt16);

            if (Count != 0 && Count != Layers)
            {
                LOG_W("'{0}' has {1} outline(s) for {2} layer(s), ignoring them", Asset.GetKey(), Count, Layers);
            }
            else if (Length > Input.GetAvailable())
            {
                LOG_W("'{0}' has a truncated outline block, ignoring it", Asset.GetKey());
            }
            else
            {
                Sequence<Image::Outline> Outlines(Count);

                for (UInt Index = 0; Index < Count; ++Index)
                {
                    for (Ref<Vector2> Point : Outlines.Append())
                    {
                        const Real32 X = Input.Read<UInt16>();
                        const Real32 Y = Input.Read<UInt16>();
                        
                        Point = Vector2(X, Y) / kMaximum<UInt16>;
                    }
                }
                Asset.SetOutlines(Move(Outlines));
            }
        }
        return true;
    }
}