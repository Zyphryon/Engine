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

#include <X11/Xlib.h>

#ifdef Bool
#undef Bool
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    static Type Resolve(ConstRef<Loader> Loader, Text Name)
    {
        return reinterpret_cast<Type>(Loader.Find(Name));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool HasToken(Text List, Text Name)
    {
        Bool Found = false;

        StrSplit(List, ' ', [&](Text Token)
        {
            Found = (Token == Name);
            return !Found;
        });
        return Found;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Ref<Bool> GetRejection()
    {
        static Bool Rejected = false;
        return Rejected;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static SInt32 OnError(Ptr<::Display>, Ptr<XErrorEvent>)
    {
        GetRejection() = true;
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static GLXContext CreateContext(ConstRef<GLXInterface> Interface, Ptr<::Display> Connection, GLXFBConfig Config, UInt8 Version)
    {
        const SInt32 Attributes[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, Version / 10,
            GLX_CONTEXT_MINOR_VERSION_ARB, Version % 10,
            GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#if !defined(NDEBUG)
            GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
            0
        };

        // A refused version is reported as an X error, whose default handler would end the process.
        GetRejection() = false;

        const XErrorHandler Previous = ::XSetErrorHandler(OnError);
        const GLXContext    Handle   = Interface.CreateContextAttribs(Connection, Config, nullptr, True, Attributes);

        ::XSync(Connection, False);
        ::XSetErrorHandler(Previous);

        if (Handle && GetRejection())
        {
            Interface.DestroyContext(Connection, Handle);
            return nullptr;
        }
        return Handle;
    }

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

    static SInt32 GetConfigAttribute(ConstRef<GLXInterface> Interface, Ptr<::Display> Connection, GLXFBConfig Config, SInt32 Attribute)
    {
        SInt32 Value = 0;
        Interface.GetFBConfigAttrib(Connection, Config, Attribute, AddressOf(Value));
        return Value;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool SelectConfig(ConstRef<GLXInterface> Interface, Ptr<::Display> Connection, SInt32 Screen, VisualID Visual, SInt32 Depth, SInt32 Stencil, Ref<GLXFBConfig> Result)
    {
        SInt32 Count = 0;

        const Ptr<GLXFBConfig> Configs = Interface.GetFBConfigs(Connection, Screen, AddressOf(Count));

        if (!Configs)
        {
            return false;
        }

        Bool   Found = false;
        SInt32 Best  = 0;

        for (SInt32 Index = 0; Index < Count; ++Index)
        {
            const GLXFBConfig Config = Configs[Index];

            if (static_cast<VisualID>(GetConfigAttribute(Interface, Connection, Config, GLX_VISUAL_ID)) != Visual)
            {
                continue;
            }

            if (!GetConfigAttribute(Interface, Connection, Config, GLX_DOUBLEBUFFER)                     ||
                !(GetConfigAttribute(Interface, Connection, Config, GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT) ||
                !(GetConfigAttribute(Interface, Connection, Config, GLX_RENDER_TYPE)   & GLX_RGBA_BIT))
            {
                continue;
            }

            const SInt32 ConfigDepth   = GetConfigAttribute(Interface, Connection, Config, GLX_DEPTH_SIZE);
            const SInt32 ConfigStencil = GetConfigAttribute(Interface, Connection, Config, GLX_STENCIL_SIZE);

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

    Context::Context()
        : mLoader     { nullptr },
          mConfig     { },
          mWindow     { 0 },
          mConnection { nullptr },
          mHandle     { nullptr },
          mSwap       { nullptr },
          mInterval   { nullptr },
          mMesa       { false }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Context::~Context()
    {
        if (const Ptr<::Display> Connection = static_cast<Ptr<::Display>>(mConnection))
        {
            if (mHandle)
            {
                Resolve<GLXMakeCurrent>(* mLoader, "glXMakeCurrent")(Connection, 0, nullptr);
                Resolve<GLXDestroyContext>(* mLoader, "glXDestroyContext")(Connection, mHandle);
            }
            ::XCloseDisplay(Connection);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Context::SetTearless(Bool Tearless)
    {
        if (mInterval == nullptr)
        {
            return;
        }

        if (mMesa)
        {
            reinterpret_cast<GLXSwapIntervalMESA>(mInterval)(Tearless ? 1 : 0);
        }
        else
        {
            const Ptr<::Display> Connection = static_cast<Ptr<::Display>>(mConnection);
            const GLXDrawable    Drawable   = static_cast<GLXDrawable>(mWindow);
            reinterpret_cast<GLXSwapIntervalEXT>(mInterval)(Connection, Drawable, Tearless ? 1 : 0);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Context::Present()
    {
        ZY_PROFILE_SCOPE("Driver::Present");

        const Ptr<::Display> Connection = static_cast<Ptr<::Display>>(mConnection);
        reinterpret_cast<GLXSwapBuffers>(mSwap)(Connection, static_cast<GLXDrawable>(mWindow));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Context::Acquire()
    {
        if (!mHandle)
        {
            return false;
        }

        const Ptr<::Display> Connection = static_cast<Ptr<::Display>>(mConnection);
        const GLXDrawable    Drawable   = static_cast<GLXDrawable>(mWindow);

        // A context left current to the ended drawing thread is refused with an X error, which would end the process.
        GetRejection() = false;

        const XErrorHandler Previous = ::XSetErrorHandler(OnError);
        const Bool          Current  = Resolve<GLXMakeCurrent>(* mLoader, "glXMakeCurrent")(Connection, Drawable, mHandle);

        ::XSync(Connection, False);
        ::XSetErrorHandler(Previous);

        return Current && !GetRejection();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Context::Create(Ptr<void> Output, Ref<Device>)
    {
        ZY_ASSERT(Output, "The driver needs a window to present into");

        GLXInterface Interface;
        Interface.GetFBConfigs          = Resolve<GLXGetFBConfigs>(* mLoader, "glXGetFBConfigs");
        Interface.GetFBConfigAttrib     = Resolve<GLXGetFBConfigAttrib>(* mLoader, "glXGetFBConfigAttrib");
        Interface.MakeCurrent           = Resolve<GLXMakeCurrent>(* mLoader, "glXMakeCurrent");
        Interface.DestroyContext        = Resolve<GLXDestroyContext>(* mLoader, "glXDestroyContext");
        Interface.QueryExtensionsString = Resolve<GLXQueryExtensionsString>(* mLoader, "glXQueryExtensionsString");
        Interface.CreateContextAttribs  = Resolve<GLXCreateContextAttribsARB>(* mLoader, "glXCreateContextAttribsARB");
        mSwap                    = mLoader->Find("glXSwapBuffers");

        const Bool Complete = Interface.GetFBConfigs && Interface.GetFBConfigAttrib && Interface.MakeCurrent
                           && Interface.DestroyContext && Interface.QueryExtensionsString && mSwap;

        if (!Complete)
        {
            LOG_E("Graphics: Failed to resolve GLX 1.4 entry points (is a GPU driver installed?)");
            return false;
        }

        const Ptr<::Display> Connection = ::XOpenDisplay(nullptr);

        if (!Connection)
        {
            LOG_E("Graphics: Failed to open a connection to the X server (is DISPLAY set?)");
            return false;
        }

        mConnection = Connection;
        mWindow  = reinterpret_cast<UInt>(Output);

        const SInt32         Screen     = ::XDefaultScreen(Connection);
        const ConstPtr<Char> Listing    = Interface.QueryExtensionsString(Connection, Screen);
        const Text           Extensions = Listing ? StrConvert(Listing) : Text();

        // GLX answers for any name it is asked, so a function is only there when its extension is listed.
        if (!HasToken(Extensions, "GLX_ARB_create_context") || !Interface.CreateContextAttribs)
        {
            LOG_E("Graphics: The driver lacks GLX_ARB_create_context");
            return false;
        }

        if (!HasToken(Extensions, "GLX_ARB_create_context_profile"))
        {
            LOG_E("Graphics: The driver lacks GLX_ARB_create_context_profile");
            return false;
        }

        XWindowAttributes Attributes { };

        if (!::XGetWindowAttributes(Connection, static_cast<::Window>(mWindow), AddressOf(Attributes)))
        {
            LOG_E("Graphics: Failed to query the window's visual");
            return false;
        }

        SInt32 DepthBits;
        SInt32 StencilBits;
        ResolveDepthBits(mConfig.DepthFormat, DepthBits, StencilBits);

        GLXFBConfig Selection = nullptr;

        if (!SelectConfig(
            Interface, Connection, Screen, ::XVisualIDFromVisual(Attributes.visual), DepthBits, StencilBits, Selection))
        {
            LOG_E("Graphics: No framebuffer configuration matches with {0}-bit depth and {1}-bit stencil", DepthBits, StencilBits);
            return false;
        }

        // The newest version the driver grants wins; a refused one only costs the attempt.
        for (const UInt8 Version : Context::kLadder)
        {
            mHandle = CreateContext(Interface, Connection, Selection, Version);

            if (mHandle)
            {
                break;
            }
        }

        if (mHandle == nullptr)
        {
            LOG_E("Graphics: Failed to create an OpenGL core context of any version from 4.6 down to 3.3");
            return false;
        }

        if (!Interface.MakeCurrent(Connection, static_cast<GLXDrawable>(mWindow), mHandle))
        {
            LOG_E("Graphics: Failed to make the OpenGL context current");

            Interface.DestroyContext(Connection, mHandle);
            mHandle = nullptr;
            return false;
        }

        if (HasToken(Extensions, "GLX_EXT_swap_control"))
        {
            mInterval = mLoader->Find("glXSwapIntervalEXT");
            mMesa     = false;
        }
        else if (HasToken(Extensions, "GLX_MESA_swap_control"))
        {
            mInterval = mLoader->Find("glXSwapIntervalMESA");
            mMesa     = true;
        }

        // Writes to the window are gamma-encoded only when its visual can carry an sRGB back-buffer.
        if (mConfig.ColorFormat == TextureFormat::RGBA8UIntNorm_sRGB
            && GetConfigAttribute(Interface, Connection, Selection, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB) == 0)
        {
            LOG_W("Graphics: The window's visual cannot present an sRGB back-buffer");

            mConfig.ColorFormat = TextureFormat::RGBA8UIntNorm;
        }

        SetTearless(mConfig.Tearless);
        return true;
    }
}