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

#include "OGLSampler.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sampler::Sampler()
        : mHandle { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Sampler::Create(ConstRef<Device> Device, ConstRef<ZyGraphic::Sampler> Descriptor)
    {
        const Bool Border = Device.Has(Feature::BorderClamp);

        glGenSamplers(1, AddressOf(mHandle));
        glSamplerParameteri(mHandle, GL_TEXTURE_WRAP_S, ConvertAddress(Descriptor.AddressModeU, Border));
        glSamplerParameteri(mHandle, GL_TEXTURE_WRAP_T, ConvertAddress(Descriptor.AddressModeV, Border));
        glSamplerParameteri(mHandle, GL_TEXTURE_WRAP_R, ConvertAddress(Descriptor.AddressModeW, Border));
        glSamplerParameteri(mHandle, GL_TEXTURE_MIN_FILTER, ConvertMinFilter(Descriptor.Filter));
        glSamplerParameteri(mHandle, GL_TEXTURE_MAG_FILTER, ConvertMagFilter(Descriptor.Filter));

        if (Descriptor.Comparison != TestCondition::None)
        {
            glSamplerParameteri(mHandle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glSamplerParameteri(mHandle, GL_TEXTURE_COMPARE_FUNC, Convert(Descriptor.Comparison));
        }
        else
        {
            glSamplerParameteri(mHandle, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        }

        if (Border)
        {
            glSamplerParameterfv(mHandle, GL_TEXTURE_BORDER_COLOR, Convert(Descriptor.Border));
        }

        if (const UInt8 Limit = Device.GetDescription().Capabilities.MaxAnisotropy; Limit > 0)
        {
            glSamplerParameterf(mHandle, GL_TEXTURE_MAX_ANISOTROPY, Min(ConvertAnisotropy(Descriptor.Filter), Limit));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Sampler::Delete()
    {
        glDeleteSamplers(1, AddressOf(mHandle));

        * this = Sampler();
    }
}