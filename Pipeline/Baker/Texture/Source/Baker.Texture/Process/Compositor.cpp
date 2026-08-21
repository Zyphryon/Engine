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

#include "Compositor.hpp"
#include "Resampler.hpp"
#include "Texel.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Texture
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type, UInt32 Channels, Bool sRGB, Bool Write>
    static void Process(
        ConstPtr<Byte> Pixels, Ptr<Byte> Output, Ptr<Real32> Values, UInt32 Texels, UInt32 Channel)
    {
        for (UInt32 Index = 0; Index < Texels; ++Index)
        {
            const Color Texel = Load<Type, Channels, sRGB>(Pixels, Index);

            Array Components(Texel.GetRed(), Texel.GetGreen(), Texel.GetBlue(), Texel.GetAlpha());

            if constexpr (Write)
            {
                Components[Channel] = Values[Index];

                Store<Type, Channels, sRGB>(
                    Output, Index, Color(Components[0], Components[1], Components[2], Components[3]));
            }
            else
            {
                Values[Index] = Components[Channel];
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static auto Pick(ConstRef<Graphic::TextureMetadata> Format, Bool Write)
    {
        const UInt32 Channel = Clamp<UInt32>(Format.Components, 1, kMaxComponents) - 1;
        const UInt32 Slot = Channel * 4 + (Format.IsSRGB ? 2 : 0) + (Write ? 1 : 0);

        const auto Select = []<typename Type>(UInt32 Index)
        {
            using Return = void (*)(ConstPtr<Byte>, Ptr<Byte>, Ptr<Real32>, UInt32, UInt32);

            static constexpr Return kTable[] =
            {
                Process<Type, 1, false, false>, Process<Type, 1, false, true>,
                Process<Type, 1, true,  false>, Process<Type, 1, true,  true>,
                Process<Type, 2, false, false>, Process<Type, 2, false, true>,
                Process<Type, 2, true,  false>, Process<Type, 2, true,  true>,
                Process<Type, 3, false, false>, Process<Type, 3, false, true>,
                Process<Type, 3, true,  false>, Process<Type, 3, true,  true>,
                Process<Type, 4, false, false>, Process<Type, 4, false, true>,
                Process<Type, 4, true,  false>, Process<Type, 4, true,  true>,
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

    Bitmap Compositor::Insert(ConstRef<Bitmap> Target, ConstRef<Bitmap> Source, UInt8 TargetChannel, UInt8 SourceChannel)
    {
        const Graphic::TextureFormat   Format = Target.GetFormat();
        const Graphic::TextureMetadata Reader = Graphic::GetTextureMetadata(Source.GetFormat());
        const Graphic::TextureMetadata Writer = Graphic::GetTextureMetadata(Format);

        if (Reader.IsCompressed() || Reader.IsPacked || Writer.IsCompressed() || Writer.IsPacked)
        {
            LOG_E("Texture: both ends of a merge must have their texels unpacked");

            return Bitmap();
        }

        if (SourceChannel >= Reader.Components || TargetChannel >= Writer.Components)
        {
            LOG_E("Texture: '{0}' channel {1} cannot merge into '{2}' channel {3}",
                Enum::GetName(Source.GetFormat()), SourceChannel, Enum::GetName(Format), TargetChannel);

            return Bitmap();
        }

        // The source lends nothing but one channel, so it is filtered to the target's extent before it is read.
        const Bitmap Fitted = Resampler::Resize(Source, Target.GetWidth(), Target.GetHeight());

        if (Fitted.GetPixels().IsEmpty())
        {
            return Bitmap();
        }

        const UInt32 Texels = static_cast<UInt32>(Target.GetWidth()) * Target.GetHeight();

        Blob Values = Blob::Allocate<Real32>(Texels);
        Blob Output = Blob::Allocate<Byte>(
            Graphic::GetLevelSize(Format, Target.GetWidth(), Target.GetHeight(), 0));

        Copy(Output.GetData<Byte>(), Output.GetSize(), Target.GetPixels().GetData());

        Pick(Reader, false)(Fitted.GetPixels().GetData(), nullptr, Values.GetData<Real32>(), Texels, SourceChannel);
        Pick(Writer, true)(Output.GetData<Byte>(), Output.GetData<Byte>(), Values.GetData<Real32>(), Texels, TargetChannel);

        return Bitmap(Format, Target.GetWidth(), Target.GetHeight(), 1, Move(Output));
    }
}