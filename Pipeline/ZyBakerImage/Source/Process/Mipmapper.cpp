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

#include "Mipmapper.hpp"
#include "Texel.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Image
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type, UInt32 Channels, Bool sRGB>
    static void Process(
        ConstPtr<Byte> Source, UInt32 SourceWidth, UInt32 SourceHeight,
        Ptr<Byte>      Target, UInt32 TargetWidth, UInt32 TargetHeight)
    {
        for (UInt32 Y = 0; Y < TargetHeight; ++Y)
        {
            const UInt32 Y0 = Y * 2;
            const UInt32 Y1 = (Y0 + 1 < SourceHeight) ? (Y0 + 1) : Y0;

            for (UInt32 X = 0; X < TargetWidth; ++X)
            {
                const UInt32 X0 = X * 2;
                const UInt32 X1 = (X0 + 1 < SourceWidth) ? (X0 + 1) : X0;

                const Color P00 = Load<Type, Channels, sRGB>(Source, Y0 * SourceWidth + X0);
                const Color P01 = Load<Type, Channels, sRGB>(Source, Y0 * SourceWidth + X1);
                const Color P10 = Load<Type, Channels, sRGB>(Source, Y1 * SourceWidth + X0);
                const Color P11 = Load<Type, Channels, sRGB>(Source, Y1 * SourceWidth + X1);

                Store<Type, Channels, sRGB>(Target, Y * TargetWidth + X, (P00 + P01 + P10 + P11) * 0.25f);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static auto Pick(ConstRef<Graphic::TextureMetadata> Format)
    {
        const UInt32 Channel = Clamp<UInt32>(Format.Components, 1, kMaxComponents) - 1;
        const UInt32 Slot = Channel * 2 + (Format.IsSRGB ? 1 : 0);

        const auto Select = []<typename Type>(UInt32 Index)
        {
            using Return = void (*)(ConstPtr<Byte>, UInt32, UInt32, Ptr<Byte>, UInt32, UInt32);

            static constexpr Return kTable[] =
            {
                Process<Type, 1, false>, Process<Type, 1, true>,
                Process<Type, 2, false>, Process<Type, 2, true>,
                Process<Type, 3, false>, Process<Type, 3, true>,
                Process<Type, 4, false>, Process<Type, 4, true>,
            };
            return kTable[Index];
        };

        switch (GetComponent(Format))
        {
        case Component::SInt8:
            return Select.operator()<SInt8>(Slot);
        case Component::UInt16:
            return Select.operator()<UInt16>(Slot);
        case Component::SInt16:
            return Select.operator()<SInt16>(Slot);
        case Component::Half:
            return Select.operator()<Half>(Slot);
        case Component::Real32:
            return Select.operator()<Real32>(Slot);
        default:
            return Select.operator()<UInt8>(Slot);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bitmap Mipmapper::Generate(AnyRef<Bitmap> Source, UInt8 Levels)
    {
        const Graphic::TextureFormat   Format   = Source.GetFormat();
        const Graphic::TextureMetadata Metadata = Graphic::GetTextureMetadata(Format);

        if (Metadata.IsCompressed() || Metadata.IsPacked || Metadata.Components == 0)
        {
            LOG_E("Texture: '{0}' must have its texels unpacked before it can be filtered", Enum::GetName(Format));

            return Bitmap();
        }

        const UInt16 Width  = Source.GetWidth();
        const UInt16 Height = Source.GetHeight();

        ZY_ASSERT(Source.GetLevels() == 1, "A chain can only be built from a single level");
        ZY_ASSERT(Levels <= Graphic::GetLevelCount(Width, Height), "More levels than the extent can halve into");

        // Nothing to filter, so the surface passes straight through rather than being copied.
        if (Levels <= 1)
        {
            return Move(Source);
        }

        Blob Output = Blob::Allocate<Byte>(Graphic::GetLevelOffset(Format, Width, Height, Levels));

        const Ptr<Byte> Target = Output.GetData<Byte>();

        // The base level carries over untouched; each later level is filtered from the one immediately above.
        Copy(Target, Graphic::GetLevelSize(Format, Width, Height, 0), Source.GetPixels().GetData());

        const auto Process = Pick(Metadata);

        for (UInt8 Level = 1; Level < Levels; ++Level)
        {
            Process(
                Target + Graphic::GetLevelOffset(Format, Width, Height, Level - 1),
                Graphic::GetLevelExtent(Width,  Level - 1),
                Graphic::GetLevelExtent(Height, Level - 1),
                Target + Graphic::GetLevelOffset(Format, Width, Height, Level),
                Graphic::GetLevelExtent(Width,  Level),
                Graphic::GetLevelExtent(Height, Level));
        }
        return Bitmap(Format, Width, Height, Levels, Move(Output));
    }
}