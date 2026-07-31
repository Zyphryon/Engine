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

#include "Converter.hpp"
#include "Texel.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Texture
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename From, typename To, Gamma Transfer>
    static void Transform(
        ConstPtr<Byte> Source, UInt32 SourceChannels,
        Ptr<Byte>      Target, UInt32 TargetChannels, UInt32 Texels)
    {
        const UInt32 Shared = Min(SourceChannels, kMaxChannels);

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

    using Translator = void (*)(ConstPtr<Byte>, UInt32, Ptr<Byte>, UInt32, UInt32);

    template<Component Source, Component Target>
    static constexpr Translator kTranslators[] =
    {
        Transform<StorageOf<Source>, StorageOf<Target>, Gamma::None>,
        Transform<StorageOf<Source>, StorageOf<Target>, Gamma::Linear>,
        Transform<StorageOf<Source>, StorageOf<Target>, Gamma::sRGB>,
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<Component Source>
    static Translator PickByTarget(Component Target, Gamma Transfer)
    {
        static constexpr ConstPtr<Translator> kRows[] =
        {
            kTranslators<Source, Component::UInt8>,
            kTranslators<Source, Component::SInt8>,
            kTranslators<Source, Component::UInt16>,
            kTranslators<Source, Component::SInt16>,
            kTranslators<Source, Component::Half>,
            kTranslators<Source, Component::Real32>,
        };
        return kRows[Enum::Cast(Target)][Enum::Cast(Transfer)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Translator Pick(ConstRef<Graphic::TextureMetadata> Source, ConstRef<Graphic::TextureMetadata> Target)
    {
        using Selector = Translator (*)(Component, Gamma);

        static constexpr Selector kSelectors[] =
        {
            PickByTarget<Component::UInt8>,
            PickByTarget<Component::SInt8>,
            PickByTarget<Component::UInt16>,
            PickByTarget<Component::SInt16>,
            PickByTarget<Component::Half>,
            PickByTarget<Component::Real32>,
        };
        return kSelectors[Enum::Cast(GetComponent(Source))](GetComponent(Target), GetGamma(Source.IsSRGB, Target.IsSRGB));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bitmap Converter::Convert(AnyRef<Bitmap> Source, Graphic::TextureFormat Format)
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

        const Translator Translate = Pick(Origin, Target);

        for (UInt8 Level = 0; Level < Levels; ++Level)
        {
            Translate(
                Source.GetPixels().GetData() + Graphic::GetLevelOffset(Source.GetFormat(), Width, Height, Level),
                Origin.Components,
                Output.GetData()             + Graphic::GetLevelOffset(Format, Width, Height, Level),
                Target.Components,
                Graphic::GetLevelExtent(Width, Level) * Graphic::GetLevelExtent(Height, Level));
        }
        return Bitmap(Format, Width, Height, Levels, Move(Output));
    }
}