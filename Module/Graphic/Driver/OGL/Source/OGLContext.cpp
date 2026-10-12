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

#include "OGLContext.hpp"
#include "OGLApi.hpp"

#if   defined(ZY_PLATFORM_WINDOWS)
#include "Windows/OGLContext.inl"
#elif defined(ZY_PLATFORM_LINUX)
#include "Linux/OGLContext.inl"
#elif defined(ZY_PLATFORM_WEB)
#include "Emscripten/OGLContext.inl"
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Str Summarize(ConstRef<Device> Device)
    {
        Str Result;

        for (const Feature Value : ZyEnum::GetValues<Feature>())
        {
            if (Device.Has(Value))
            {
                if (!Result.IsEmpty())
                {
                    Result.Append(' ');
                }
                Result.Append(ZyEnum::GetName(Value));
            }
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Context::Initialize(Ptr<void> Output, ConstRef<Configuration> Config, ConstRef<Loader> Loader, Ref<Device> Device)
    {
        mLoader = AddressOf(Loader);
        mConfig = Config;

        return Create(Output, Device);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Context::Resize(UInt16 Width, UInt16 Height)
    {
        mConfig.Width  = Width;
        mConfig.Height = Height;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Context::Prepare(ConstRef<Device> Device)
    {
        ConstRef<Description> Description = Device.GetDescription();

        LOG_I("Graphics: {0} {1}.{2} with GLSL {3} on '{4}' [{5}]",
            Description.Backend,
            Device.GetVersion() / 10,
            Device.GetVersion() % 10,
            Device.GetLanguage(),
            Text(Description.Adapters[0].Description),
            Summarize(Device));

        // Every texture is allocated whole and immutable, which desktop only gained in 4.2.
        if (!Device.Has(Feature::TextureStorage))
        {
            LOG_E("Graphics: The context lacks immutable texture storage (OpenGL 4.2 or GL_ARB_texture_storage)");
            return false;
        }

        // Depth runs over [0, 1] as in D3D, so one projection and one shader library serve every driver.
        if (!Device.Has(Feature::ClipControl))
        {
            LOG_E("Graphics: The context lacks ClipControl (OpenGL 4.5, GL_ARB_clip_control or GL_EXT_clip_control)");
            return false;
        }

        // Core profiles need a bound vertex array, so one stays bound for the context's lifetime and goes with it.
        GLuint Vertices = 0;
        glGenVertexArrays(1, AddressOf(Vertices));
        glBindVertexArray(Vertices);

        // Tightly-packed uploads, matching the engine's row layout.
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Front faces wind clockwise, as they do in D3D.
        glFrontFace(GL_CW);

        // ES and D3D11 behave as if these were always on; only desktop leaves them off by default.
        if (Device.GetProfile() == Profile::Desktop)
        {
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
            glEnable(GL_PROGRAM_POINT_SIZE);

            if (Device.Has(Feature::PrimitiveRestart))
            {
                glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
            }
        }
        return true;
    }
}