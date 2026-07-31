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

#include <glad/gl.h>
#include <glad/glx.h>

#ifdef Bool
#undef Bool
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void ResolveDepthBits(TextureFormat Format, Ref<SInt32> Depth, Ref<SInt32> Stencil)
    {
        switch (Format)
        {
        case TextureFormat::D16UIntNorm:
            Depth   = 16;
            Stencil = 0;
            break;
        case TextureFormat::D32Float:
            Depth   = 32;
            Stencil = 0;
            break;
        case TextureFormat::D24S8UIntNorm:
            Depth   = 24;
            Stencil = 8;
            break;
        case TextureFormat::D32S8UIntNorm:
            Depth   = 32;
            Stencil = 8;
            break;
        default:
            Depth   = 0;
            Stencil = 0;
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static SInt32 GetConfigAttribute(Ptr<::Display> Connection, GLXFBConfig Config, SInt32 Attribute)
    {
        SInt32 Value = 0;
        ::glXGetFBConfigAttrib(Connection, Config, Attribute, AddressOf(Value));
        return Value;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool SelectConfig(Ptr<::Display> Connection, SInt32 Screen, VisualID Visual, SInt32 Depth, SInt32 Stencil, Ref<GLXFBConfig> Result)
    {
        SInt32 Count = 0;

        const Ptr<GLXFBConfig> Configs = ::glXGetFBConfigs(Connection, Screen, AddressOf(Count));

        if (!Configs)
        {
            return false;
        }

        Bool   Found = false;
        SInt32 Best  = 0;

        for (SInt32 Index = 0; Index < Count; ++Index)
        {
            const GLXFBConfig Config = Configs[Index];

            if (static_cast<VisualID>(GetConfigAttribute(Connection, Config, GLX_VISUAL_ID)) != Visual)
            {
                continue;
            }

            if (!GetConfigAttribute(Connection, Config, GLX_DOUBLEBUFFER)                     ||
                !(GetConfigAttribute(Connection, Config, GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT) ||
                !(GetConfigAttribute(Connection, Config, GLX_RENDER_TYPE)   & GLX_RGBA_BIT))
            {
                continue;
            }

            const SInt32 ConfigDepth   = GetConfigAttribute(Connection, Config, GLX_DEPTH_SIZE);
            const SInt32 ConfigStencil = GetConfigAttribute(Connection, Config, GLX_STENCIL_SIZE);

            if (ConfigDepth < Depth || ConfigStencil < Stencil)
            {
                continue;
            }

            // Prefer the least over-provisioned buffer that still satisfies the request.
            if (const SInt32 Excess = (ConfigDepth - Depth) + (ConfigStencil - Stencil); !Found || Excess < Best)
            {
                Result = Config;
                Best   = Excess;
                Found  = true;
            }
        }

        ::XFree(Configs);
        return Found;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLES3Context::GLES3Context()
        : mDeviceOutput  { 0 },
          mDeviceContext { nullptr },
          mRenderContext { nullptr }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLES3Context::~GLES3Context()
    {
        if (const Ptr<::Display> Connection = static_cast<Ptr<::Display>>(mDeviceContext))
        {
            ::glXMakeCurrent(Connection, 0, nullptr);

            if (mRenderContext)
            {
                ::glXDestroyContext(Connection, static_cast<GLXContext>(mRenderContext));
            }
            ::XCloseDisplay(Connection);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool GLES3Context::Initialize(Ptr<void> Output, ConstRef<Configuration> Config)
    {
        const Ptr<::Display> Connection = ::XOpenDisplay(nullptr);

        if (!Connection)
        {
            LOG_E("GLES3Context: Failed to open a connection to the X server (is DISPLAY set?)");
            return false;
        }

        mDeviceContext = Connection;
        mDeviceOutput  = reinterpret_cast<UInt>(Output);

        const SInt32 Screen = ::XDefaultScreen(Connection);

        if (gladLoaderLoadGLX(Connection, Screen) == 0)
        {
            LOG_E("GLES3Context: Failed to resolve GLX entry points (is a GPU driver installed?)");
            return false;
        }

        if (!GLAD_GLX_ARB_create_context)
        {
            LOG_E("GLES3Context: Driver lacks GLX_ARB_create_context");
            return false;
        }

        XWindowAttributes Attributes { };

        if (!::XGetWindowAttributes(Connection, static_cast<::Window>(mDeviceOutput), AddressOf(Attributes)))
        {
            LOG_E("GLES3Context: Failed to query the window's visual");
            return false;
        }

        SInt32 DepthBits;
        SInt32 StencilBits;
        ResolveDepthBits(Config.DepthFormat, DepthBits, StencilBits);

        GLXFBConfig Selection;

        if (!SelectConfig(
            Connection, Screen, ::XVisualIDFromVisual(Attributes.visual), DepthBits, StencilBits, Selection))
        {
            LOG_E("GLES3Context: No framebuffer configuration matches with {0}-bit depth and {1}-bit stencil", DepthBits, StencilBits);
            return false;
        }

        // Request an OpenGL 3.3 core profile context (the desktop analogue of OpenGL ES 3.0).
        constexpr SInt32 ContextAttributes[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#if !defined(NDEBUG)
            GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
            0
        };

        const GLXContext Context = glXCreateContextAttribsARB(Connection, Selection, nullptr, True, ContextAttributes);

        if (!Context)
        {
            LOG_E("GLES3Context: Failed to create an OpenGL 3.3 core context");
            return false;
        }
        mRenderContext = Context;

        if (!::glXMakeCurrent(Connection, static_cast<GLXDrawable>(mDeviceOutput), Context))
        {
            LOG_E("GLES3Context: Failed to make the OpenGL context current");

            ::glXDestroyContext(Connection, Context);
            mRenderContext = nullptr;
            return false;
        }

        if (gladLoaderLoadGL() == 0)
        {
            LOG_E("GLES3Context: Failed to load OpenGL 3.3 entry points");
            return false;
        }

        // Gamma-encode writes to the default framebuffer when the visual can carry an sRGB back-buffer.
        if (Config.ColorFormat == TextureFormat::RGBA8UIntNorm_sRGB)
        {
            if (GetConfigAttribute(Connection, Selection, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB))
            {
                glEnable(GL_FRAMEBUFFER_SRGB);
            }
            else
            {
                LOG_W("GLES3Context: The window's visual cannot present an sRGB back-buffer");
            }
        }

        SetTearless(Config.Tearless);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Context::SetTearless(Bool Tearless)
    {
        if (GLAD_GLX_EXT_swap_control)
        {
            const Ptr<::Display> Connection = static_cast<Ptr<::Display>>(mDeviceContext);
            const GLXDrawable    Drawable   = static_cast<GLXDrawable>(mDeviceOutput);
            glXSwapIntervalEXT(Connection, Drawable, Tearless ? 1 : 0);
        }
        else if (GLAD_GLX_MESA_swap_control)
        {
            glXSwapIntervalMESA(Tearless ? 1 : 0);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Context::Present()
    {
        ::glXSwapBuffers(static_cast<Ptr<::Display>>(mDeviceContext), static_cast<GLXDrawable>(mDeviceOutput));
    }
}
