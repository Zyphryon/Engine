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

#include "STBImporter.hpp"
#include <Zyphryon.Graphic/Metadata.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Image
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Graphic::TextureFormat Resolve(SInt32 Channels, Bool Real, Bool Wide, Bool sRGB)
    {
        switch (Channels)
        {
        case 1:
            return Real ? Graphic::TextureFormat::R32Float
                 : Wide ? Graphic::TextureFormat::R16UIntNorm
                        : Graphic::TextureFormat::R8UIntNorm;
        case 2:
            return Real ? Graphic::TextureFormat::RG32Float
                 : Wide ? Graphic::TextureFormat::RG16UIntNorm
                        : Graphic::TextureFormat::RG8UIntNorm;
        default:
            return Real ? Graphic::TextureFormat::RGBA32Float
                 : Wide ? Graphic::TextureFormat::RGBA16UIntNorm
                 : sRGB ? Graphic::TextureFormat::RGBA8UIntNorm_sRGB
                        : Graphic::TextureFormat::RGBA8UIntNorm;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bitmap STBImporter::Import(ConstSpan<Byte> Source, ConstRef<Profile> Profile) const
    {
        if (Source.IsEmpty())
        {
            LOG_E("Texture: source image is empty");

            return Bitmap();
        }

        const ConstPtr<stbi_uc> Encoded = Source.GetData();
        const SInt32            Length  = static_cast<SInt32>(Source.GetSize());

        SInt32 Width    = 0;
        SInt32 Height   = 0;
        SInt32 Channels = 0;

        if (!stbi_info_from_memory(Encoded, Length, AddressOf(Width), AddressOf(Height), AddressOf(Channels)))
        {
            LOG_E("Texture: failed to read source image header ({0})", StrConvert(stbi_failure_reason()));

            return Bitmap();
        }

        // Decode at the source's own precision. Routing a floating-point or 16-bit source through the 8-bit
        // entry point would tone-map or truncate it silently, so each depth gets its matching loader.
        const Bool Real = stbi_is_hdr_from_memory(Encoded, Length);
        const Bool Wide = !Real && stbi_is_16_bit_from_memory(Encoded, Length);

        // Floating-point pixels are linear by definition, and the engine has no wide sRGB format, so an sRGB
        // request only means anything at 8-bit.
        const Bool sRGB = !Profile.Linear && !Real && !Wide;

        if (!Profile.Linear && (Real || Wide))
        {
            LOG_W("Texture: sRGB was requested but the source is {0}, which the engine only stores as linear",
                Real ? "floating-point"_Text : "16-bit"_Text);
        }

        // Three-channel pixels have no 8-bit or 16-bit engine format, and sRGB exists only as a four-channel
        // variant, so both cases expand to RGBA. One and two channel sources keep their width.
        const Bool   Expand  = (Channels == 3) || (sRGB && Channels < 4);
        const SInt32 Request = Expand ? STBI_rgb_alpha : Channels;

        if (sRGB && Channels < 3)
        {
            LOG_W("Texture: expanding a {0}-channel sRGB source to RGBA, the only sRGB format the engine has", Channels);
        }

        Ptr<void> Pixels;

        if (Real)
        {
            Pixels = static_cast<Ptr<void>>(stbi_loadf_from_memory(Encoded, Length, AddressOf(Width), AddressOf(Height), AddressOf(Channels), Request));
        }
        else if (Wide)
        {
            Pixels = static_cast<Ptr<void>>(stbi_load_16_from_memory(Encoded, Length, AddressOf(Width), AddressOf(Height), AddressOf(Channels), Request));
        }
        else
        {
            Pixels = static_cast<Ptr<void>>(stbi_load_from_memory(Encoded, Length, AddressOf(Width), AddressOf(Height), AddressOf(Channels), Request));
        }

        if (Pixels == nullptr)
        {
            LOG_E("Texture: failed to decode source image ({0})", StrConvert(stbi_failure_reason()));

            return Bitmap();
        }

        if (Width > kMaximum<UInt16> || Height > kMaximum<UInt16>)
        {
            LOG_E("Texture: source image is {0}x{1}, which exceeds the {2} pixel limit", Width, Height, kMaximum<UInt16>);

            stbi_image_free(Pixels);
            return Bitmap();
        }

        const Graphic::TextureFormat Format = Resolve(Request, Real, Wide, sRGB);

        // Adopt the decoder's buffer rather than copying it; the blob releases it through the matching free.
        const UInt32 Size = Graphic::GetLevelSize(Format, static_cast<UInt16>(Width), static_cast<UInt16>(Height), 0);

        Blob Data(static_cast<Ptr<Byte>>(Pixels), Size, Blob::Deleter([](Ptr<Byte> Address)
        {
            stbi_image_free(Address);
        }));
        return Bitmap(Format, static_cast<UInt16>(Width), static_cast<UInt16>(Height), 1, Move(Data));
    }
}