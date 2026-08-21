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

#include "Resampler.hpp"
#include "Texel.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Texture
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
            const UInt32 MinY = Y * SourceHeight / TargetHeight;
            const UInt32 MaxY = Max<UInt32>((Y + 1) * SourceHeight / TargetHeight, MinY + 1);

            for (UInt32 X = 0; X < TargetWidth; ++X)
            {
                const UInt32 MinX = X * SourceWidth / TargetWidth;
                const UInt32 MaxX = Max<UInt32>((X + 1) * SourceWidth / TargetWidth, MinX + 1);

                Color  Total(0.0f, 0.0f, 0.0f, 0.0f);
                UInt32 Count = 0;

                for (UInt32 Row = MinY; Row < MaxY; ++Row)
                {
                    for (UInt32 Column = MinX; Column < MaxX; ++Column)
                    {
                        Total += Load<Type, Channels, sRGB>(Source, Row * SourceWidth + Column);
                        ++Count;
                    }
                }

                Store<Type, Channels, sRGB>(Target, Y * TargetWidth + X, Total * (1.0f / static_cast<Real32>(Count)));
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

    Bitmap Resampler::Resize(ConstRef<Bitmap> Source, UInt16 Width, UInt16 Height)
    {
        const Graphic::TextureFormat   Format   = Source.GetFormat();
        const Graphic::TextureMetadata Metadata = Graphic::GetTextureMetadata(Format);

        if (Metadata.IsCompressed() || Metadata.IsPacked || Metadata.Components == 0)
        {
            LOG_E("Texture: '{0}' must have its texels unpacked before it can be filtered", Enum::GetName(Format));

            return Bitmap();
        }

        if (Width == 0 || Height == 0 || Source.GetWidth() == 0 || Source.GetHeight() == 0)
        {
            LOG_E("Texture: neither end of a filter can be empty");

            return Bitmap();
        }

        Blob Output = Blob::Allocate<Byte>(Graphic::GetLevelSize(Format, Width, Height, 0));

        if (Source.GetWidth() == Width && Source.GetHeight() == Height)
        {
            Copy(Output.GetData<Byte>(), Output.GetSize(), Source.GetPixels().GetData());
        }
        else
        {
            Pick(Metadata)(
                Source.GetPixels().GetData(), Source.GetWidth(), Source.GetHeight(),
                Output.GetData<Byte>(), Width, Height);
        }
        return Bitmap(Format, Width, Height, 1, Move(Output));
    }
}