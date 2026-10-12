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

#include "Zyphryon.Graphic/Metadata.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool HasAlpha(TextureFormat Format)
    {
        switch (Format)
        {
        case TextureFormat::RGB32SInt:
        case TextureFormat::RGB32UInt:
        case TextureFormat::RGB32Float:
            return false;
        default:
            return true;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Context::Context()
        : mLoader { nullptr },
          mConfig { },
          mHandle { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Context::~Context()
    {
        if (mHandle)
        {
            emscripten_webgl_make_context_current(0);
            emscripten_webgl_destroy_context(mHandle);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Context::Create(Ptr<void>, Ref<Device> Device)
    {
        EmscriptenWebGLContextAttributes Attributes;
        emscripten_webgl_init_context_attributes(AddressOf(Attributes));

        Attributes.majorVersion                 = 2;
        Attributes.minorVersion                 = 0;
        Attributes.alpha                        = HasAlpha(mConfig.ColorFormat);
        Attributes.depth                        = (mConfig.DepthFormat != TextureFormat::Unspecified);
        Attributes.stencil                      = GetTextureMetadata(mConfig.DepthFormat).IsStencil;
        Attributes.antialias                    = true;
        Attributes.premultipliedAlpha           = false;
        Attributes.preserveDrawingBuffer        = false;
        Attributes.powerPreference              = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
        Attributes.failIfMajorPerformanceCaveat = false;

        mHandle = emscripten_webgl_create_context("!ZyWindowHTML5", AddressOf(Attributes));

        if (mHandle <= 0)
        {
            LOG_E("Graphics: Failed to create a WebGL 2 (OpenGL ES 3.0) context");
            return false;
        }

        if (emscripten_webgl_make_context_current(mHandle) != EMSCRIPTEN_RESULT_SUCCESS)
        {
            LOG_E("Graphics: Failed to make the WebGL context current");

            emscripten_webgl_destroy_context(mHandle);
            mHandle = 0;
            return false;
        }

        // Emscripten leaves this one off its own list, so it is enabled by hand and its answer is its presence.
        if (emscripten_webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(mHandle))
        {
            Device.Grant("WEBGL_draw_instanced_base_vertex_base_instance");
        }

        if (mConfig.ColorFormat == TextureFormat::RGBA8UIntNorm_sRGB)
        {
            LOG_W("Graphics: WebGL cannot present an sRGB back-buffer");

            mConfig.ColorFormat = TextureFormat::RGBA8UIntNorm;
        }

        SetTearless(mConfig.Tearless);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Context::SetTearless(Bool Tearless)
    {
        if (!Tearless)
        {
            LOG_W("Graphics: Tearing was requested, but the browser always presents on the vertical blank");
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Context::Acquire()
    {
        return mHandle > 0 && emscripten_webgl_make_context_current(mHandle) == EMSCRIPTEN_RESULT_SUCCESS;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Context::Present()
    {
        ZY_PROFILE_SCOPE("Driver::Present");

#if defined(ZY_HAS_THREADS)
        emscripten_webgl_commit_frame();
#endif
    }
}