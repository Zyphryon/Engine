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

#include "Transcoder.hpp"
#include "Texel.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Image
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename From, typename To, Gamma Transfer>
    static void Process(
        ConstPtr<Byte> Source, UInt32 SourceChannels,
        Ptr<Byte>      Target, UInt32 TargetChannels, UInt32 Texels)
    {
        const UInt32 Shared = Min(SourceChannels, kMaxComponents);

        // Either direction of the transfer collapses into a table.
        constexpr Bool TabledDecode = (Transfer == Gamma::Linear) && IsAnyOf<From, UInt8>;
        constexpr Bool TabledEncode = (Transfer == Gamma::sRGB)   && IsAnyOf<To, UInt8>;

        for (UInt32 Index = 0; Index < Texels; ++Index)
        {
            const ConstPtr<Byte> Pixel = Source + static_cast<UInt64>(Index) * SourceChannels * sizeof(From);
            const Ptr<Byte>      Slot  = Target + static_cast<UInt64>(Index) * TargetChannels * sizeof(To);

            // A wider target fills the channels the source never carried, leaving alpha opaque.
            Array Components(0.0f, 0.0f, 0.0f, 1.0f);

            for (UInt32 Channel = 0; Channel < Shared; ++Channel)
            {
                // Alpha is never gamma-encoded, so it keeps the plain path.
                if constexpr (TabledDecode)
                {
                    Components[Channel] = (Channel < 3) ? kCurve.Linear[Pixel[Channel]] : Decode<From>(Pixel + Channel);
                }
                else
                {
                    Components[Channel] = Decode<From>(Pixel + Channel * sizeof(From));
                }
            }

            Color Value(Components[0], Components[1], Components[2], Components[3]);

            // Whichever end the tables covered needs nothing further here.
            if constexpr (Transfer == Gamma::Linear && !TabledDecode)
            {
                Value = Value.ToLinear();
            }
            else if constexpr (Transfer == Gamma::sRGB && !TabledEncode)
            {
                Value = Value.ToSRGB();
            }

            Array Result(Value.GetRed(), Value.GetGreen(), Value.GetBlue(), Value.GetAlpha());

            for (UInt32 Channel = 0; Channel < TargetChannels; ++Channel)
            {
                if constexpr (TabledEncode)
                {
                    if (Channel < 3)
                    {
                        Slot[Channel] = EncodeGamma(Result[Channel]);
                    }
                    else
                    {
                        Encode<To>(Slot + Channel, Result[Channel]);
                    }
                }
                else
                {
                    Encode<To>(Slot + Channel * sizeof(To), Result[Channel]);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static auto Pick(ConstRef<Graphic::TextureMetadata> Source, ConstRef<Graphic::TextureMetadata> Target)
    {
        // Target and transfer are contiguous enums, so they flatten into one index.
        const UInt32 Slot = Enum::Cast(GetComponent(Target)) * Enum::Count<Gamma>()
                          + Enum::Cast(GetGamma(Source.IsSRGB, Target.IsSRGB));

        // Every target and transfer for one source type, in the order the index above walks them.
        const auto Select = []<typename From>(UInt32 Index)
        {
            using Function = void (*)(ConstPtr<Byte>, UInt32, Ptr<Byte>, UInt32, UInt32);

            static constexpr Function kTable[] =
            {
                Process<From, UInt8,  Gamma::None>, Process<From, UInt8,  Gamma::Linear>, Process<From, UInt8,  Gamma::sRGB>,
                Process<From, SInt8,  Gamma::None>, Process<From, SInt8,  Gamma::Linear>, Process<From, SInt8,  Gamma::sRGB>,
                Process<From, UInt16, Gamma::None>, Process<From, UInt16, Gamma::Linear>, Process<From, UInt16, Gamma::sRGB>,
                Process<From, SInt16, Gamma::None>, Process<From, SInt16, Gamma::Linear>, Process<From, SInt16, Gamma::sRGB>,
                Process<From, Half,   Gamma::None>, Process<From, Half,   Gamma::Linear>, Process<From, Half,   Gamma::sRGB>,
                Process<From, Real32, Gamma::None>, Process<From, Real32, Gamma::Linear>, Process<From, Real32, Gamma::sRGB>,
            };
            return kTable[Index];
        };

        switch (GetComponent(Source))
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

    Bitmap Transcoder::Transcode(AnyRef<Bitmap> Source, Graphic::TextureFormat Format)
    {
        const Graphic::TextureMetadata Target = Graphic::GetTextureMetadata(Format);
        const Graphic::TextureMetadata Origin = Graphic::GetTextureMetadata(Source.GetFormat());

        if (Target.IsCompressed() || Target.IsPacked || Target.Components == 0)
        {
            LOG_E("Texture: '{0}' cannot be written as interleaved components", Enum::GetName(Format));

            return Bitmap();
        }

        if (Origin.IsCompressed() || Origin.IsPacked || Origin.Components == 0)
        {
            LOG_E("Texture: '{0}' must be unpacked before it can be converted", Enum::GetName(Source.GetFormat()));

            return Bitmap();
        }

        // Matching formats already agree on layout, so the surface passes straight through.
        if (Format == Source.GetFormat())
        {
            return Move(Source);
        }

        const UInt16 Width  = Source.GetWidth();
        const UInt16 Height = Source.GetHeight();
        const UInt8  Levels = Max<UInt8>(Source.GetLevels(), 1);

        Blob Output = Blob::Allocate<Byte>(Graphic::GetLevelOffset(Format, Width, Height, Levels));

        const auto Process = Pick(Origin, Target);

        for (UInt8 Level = 0; Level < Levels; ++Level)
        {
            Process(
                Source.GetPixels().GetData() + Graphic::GetLevelOffset(Source.GetFormat(), Width, Height, Level),
                Origin.Components,
                Output.GetData()             + Graphic::GetLevelOffset(Format, Width, Height, Level),
                Target.Components,
                Graphic::GetLevelExtent(Width, Level) * Graphic::GetLevelExtent(Height, Level));
        }
        return Bitmap(Format, Width, Height, Levels, Move(Output));
    }
}