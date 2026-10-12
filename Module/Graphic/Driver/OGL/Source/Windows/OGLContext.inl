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

#include <windows.h>

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

    static Bool Inspect(ConstRef<Loader> Loader, HDC Device, Ref<WGLBootstrap> Output)
    {
        PIXELFORMATDESCRIPTOR Descriptor { };
        Descriptor.nSize      = sizeof(Descriptor);
        Descriptor.nVersion   = 1;
        Descriptor.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        Descriptor.iPixelType = PFD_TYPE_RGBA;
        Descriptor.cColorBits = 32;

        const SInt32 Format = ::ChoosePixelFormat(Device, AddressOf(Descriptor));

        if (Format == 0 || !::SetPixelFormat(Device, Format, AddressOf(Descriptor)))
        {
            return false;
        }

        const auto CreateContext = Resolve<WGLCreateContext>(Loader, "wglCreateContext");
        const auto DeleteContext = Resolve<WGLDeleteContext>(Loader, "wglDeleteContext");
        const auto MakeCurrent   = Resolve<WGLMakeCurrent>(Loader, "wglMakeCurrent");

        const HGLRC Handle = CreateContext ? CreateContext(Device) : nullptr;

        if (Handle == nullptr)
        {
            return false;
        }

        const Bool Current = MakeCurrent(Device, Handle);

        if (Current)
        {
            if (const auto GetExtensions = Resolve<WGLGetExtensionsStringARB>(Loader, "wglGetExtensionsStringARB"))
            {
                Output.Extensions = StrConvert(GetExtensions(Device));
            }

            Output.ChoosePixelFormat    = Resolve<WGLChoosePixelFormatARB>(Loader, "wglChoosePixelFormatARB");
            Output.CreateContextAttribs = Resolve<WGLCreateContextAttribsARB>(Loader, "wglCreateContextAttribsARB");
            Output.SwapInterval         = Resolve<WGLSwapIntervalEXT>(Loader, "wglSwapIntervalEXT");

            MakeCurrent(nullptr, nullptr);
        }

        DeleteContext(Handle);
        return Current;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool Bootstrap(ConstRef<Loader> Loader, Ref<WGLBootstrap> Output)
    {
        // A window's pixel format is set once for its lifetime, so the throwaway context needs a window of its own.
        WNDCLASSEXW Class { };
        Class.cbSize        = sizeof(Class);
        Class.style         = CS_OWNDC;
        Class.lpfnWndProc   = ::DefWindowProcW;
        Class.hInstance     = ::GetModuleHandleW(nullptr);
        Class.lpszClassName = L"ZyphryonOpenGLBootstrap";

        if (!::RegisterClassExW(AddressOf(Class)))
        {
            return false;
        }

        const HWND Window = ::CreateWindowExW(
            0,
            Class.lpszClassName,
            L"",
            0,
            0,
            0,
            1,
            1,
            nullptr,
            nullptr,
            Class.hInstance,
            nullptr);
        const HDC  Device = Window ? ::GetDC(Window) : nullptr;

        const Bool Result = Device && Inspect(Loader, Device, Output);

        if (Device)
        {
            ::ReleaseDC(Window, Device);
        }

        if (Window)
        {
            ::DestroyWindow(Window);
        }

        ::UnregisterClassW(Class.lpszClassName, Class.hInstance);
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void ResolveColorBits(TextureFormat Format, Ref<SInt32> Color, Ref<SInt32> Alpha)
    {
        switch (Format)
        {
        case TextureFormat::RGB10A2UInt:
        case TextureFormat::RGB10A2UIntNorm:
            Color = 30;
            Alpha = 2;
            break;
        case TextureFormat::RGBA16Float:
            Color = 48;
            Alpha = 16;
            break;
        default:
            Color = 24;
            Alpha = 8;
            break;
        }
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

    static void Request(Ref<Sequence<SInt32, 32>> Attributes, SInt32 Key, SInt32 Value)
    {
        Attributes.Append(Key);
        Attributes.Append(Value);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool Choose(ConstRef<WGLBootstrap> Interface, HDC Surface, ConstRef<Configuration> Config, Ref<SInt32> Format)
    {
        SInt32 Color;
        SInt32 Alpha;
        SInt32 Depth;
        SInt32 Stencil;
        ResolveColorBits(Config.ColorFormat, Color, Alpha);
        ResolveDepthBits(Config.DepthFormat, Depth, Stencil);

        const Bool IsFloat = Config.ColorFormat == TextureFormat::RGBA16Float
            ? WGL_TYPE_RGBA_FLOAT_ARB
            : WGL_TYPE_RGBA_ARB;

        // Select an accelerated, double-buffered pixel format through the ARB path, as key and value pairs.
        Sequence<SInt32, 32> Attributes;
        Request(Attributes, WGL_DRAW_TO_WINDOW_ARB, GL_TRUE);
        Request(Attributes, WGL_SUPPORT_OPENGL_ARB, GL_TRUE);
        Request(Attributes, WGL_DOUBLE_BUFFER_ARB,  GL_TRUE);
        Request(Attributes, WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB);

        if (Config.ColorFormat == TextureFormat::RGBA16Float)
        {
            Request(Attributes, WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ARB);
        }
        else
        {
            Request(Attributes, WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);
        }

        Request(Attributes, WGL_COLOR_BITS_ARB,     Color);
        Request(Attributes, WGL_ALPHA_BITS_ARB,     Alpha);
        Request(Attributes, WGL_DEPTH_BITS_ARB,     Depth);
        Request(Attributes, WGL_STENCIL_BITS_ARB,   Stencil);

        if (Config.ColorFormat == TextureFormat::RGBA8UIntNorm_sRGB)
        {
            Request(Attributes, WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE);
        }
        Request(Attributes, 0, 0);

        UINT Count;
        return Interface.ChoosePixelFormat(Surface, Attributes.GetData(), nullptr, 1, AddressOf(Format), AddressOf(Count)) && Count > 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ChooseFormat(ConstRef<WGLBootstrap> Interface, HDC Surface, Ref<Configuration> Config)
    {
        // A floating-point back-buffer needs a pixel type of its own; without it the request falls back to 8 bits.
        if (Config.ColorFormat == TextureFormat::RGBA16Float && !HasToken(Interface.Extensions, "WGL_ARB_pixel_format_float"))
        {
            LOG_W("Graphics: The driver lacks WGL_ARB_pixel_format_float, so the back-buffer stays at 8 bits");

            Config.ColorFormat = TextureFormat::RGBA8UIntNorm;
        }

        if (Config.ColorFormat == TextureFormat::RGBA8UIntNorm_sRGB
            && !HasToken(Interface.Extensions, "WGL_ARB_framebuffer_sRGB")
            && !HasToken(Interface.Extensions, "WGL_EXT_framebuffer_sRGB"))
        {
            LOG_W("Graphics: The driver cannot present an sRGB back-buffer");

            Config.ColorFormat = TextureFormat::RGBA8UIntNorm;
        }

        SInt32 Format = 0;
        Bool   Found  = Choose(Interface, Surface, Config, Format);

        // A driver may list the extension yet offer no window format of that kind, so 8 bits are tried before giving up.
        if (!Found && Config.ColorFormat != TextureFormat::RGBA8UIntNorm)
        {
            LOG_W("Graphics: No pixel format presents the requested back-buffer, so it falls back to 8 bits");

            Config.ColorFormat = TextureFormat::RGBA8UIntNorm;
            Found              = Choose(Interface, Surface, Config, Format);
        }

        if (!Found)
        {
            LOG_E("Graphics: Failed to choose a compatible pixel format");
            return false;
        }

        PIXELFORMATDESCRIPTOR Descriptor { };
        ::DescribePixelFormat(Surface, Format, sizeof(Descriptor), AddressOf(Descriptor));

        // A window takes one pixel format for life, so a driver tried after a failed one reuses the format it set.
        if (::GetPixelFormat(Surface) != Format && !::SetPixelFormat(Surface, Format, AddressOf(Descriptor)))
        {
            LOG_E("Graphics: Failed to set the pixel format");
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static HGLRC CreateHandle(ConstRef<WGLBootstrap> Interface, HDC Surface)
    {
        // The newest version the driver grants wins; a refused one only costs the attempt.
        for (const UInt8 Version : Context::kLadder)
        {
            const SInt32 Attributes[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, Version / 10,
                WGL_CONTEXT_MINOR_VERSION_ARB, Version % 10,
                WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if !defined(NDEBUG)
                WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
                0
            };

            if (const HGLRC Handle = Interface.CreateContextAttribs(Surface, nullptr, Attributes))
            {
                return Handle;
            }
        }
        return nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Context::Context()
        : mLoader   { nullptr },
          mConfig   { },
          mWindow   { nullptr },
          mSurface  { nullptr },
          mHandle   { nullptr },
          mInterval { nullptr }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Context::~Context()
    {
        if (mHandle)
        {
            Resolve<WGLMakeCurrent>(* mLoader, "wglMakeCurrent")(nullptr, nullptr);
            Resolve<WGLDeleteContext>(* mLoader, "wglDeleteContext")(mHandle);
        }

        if (mSurface)
        {
            ::ReleaseDC(mWindow, mSurface);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Context::Present()
    {
        ZY_PROFILE_SCOPE("Driver::Present");

        ::SwapBuffers(mSurface);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Context::SetTearless(Bool Tearless)
    {
        if (mInterval)
        {
            reinterpret_cast<WGLSwapIntervalEXT>(mInterval)(Tearless ? 1 : 0);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Context::Acquire()
    {
        return mHandle && Resolve<WGLMakeCurrent>(* mLoader, "wglMakeCurrent")(mSurface, mHandle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Context::Create(Ptr<void> Output, Ref<Device>)
    {
        ZY_ASSERT(Output, "The driver needs a window to present into");

        WGLBootstrap Interface;

        if (!Bootstrap(* mLoader, Interface))
        {
            LOG_E("Graphics: Failed to create a bootstrap OpenGL context (is a GPU driver installed?)");
            return false;
        }

        if (!HasToken(Interface.Extensions, "WGL_ARB_pixel_format") || !Interface.ChoosePixelFormat)
        {
            LOG_E("Graphics: The driver lacks WGL_ARB_pixel_format");
            return false;
        }

        if (!HasToken(Interface.Extensions, "WGL_ARB_create_context") || !Interface.CreateContextAttribs)
        {
            LOG_E("Graphics: The driver lacks WGL_ARB_create_context");
            return false;
        }

        if (!HasToken(Interface.Extensions, "WGL_ARB_create_context_profile"))
        {
            LOG_E("Graphics: The driver lacks WGL_ARB_create_context_profile");
            return false;
        }

        mWindow  = static_cast<HWND>(Output);
        mSurface = ::GetDC(mWindow);

        if (mSurface == nullptr)
        {
            LOG_E("Graphics: Failed to get device context");
            return false;
        }

        if (!ChooseFormat(Interface, mSurface, mConfig))
        {
            return false;
        }

        mHandle = CreateHandle(Interface, mSurface);

        if (mHandle == nullptr)
        {
            LOG_E("Graphics: Failed to create an OpenGL core context of any version from 4.6 down to 3.3");
            return false;
        }

        if (!Resolve<WGLMakeCurrent>(* mLoader, "wglMakeCurrent")(mSurface, mHandle))
        {
            LOG_E("Graphics: Failed to make the OpenGL context current");

            Resolve<WGLDeleteContext>(* mLoader, "wglDeleteContext")(mHandle);
            mHandle = nullptr;
            return false;
        }

        if (HasToken(Interface.Extensions, "WGL_EXT_swap_control"))
        {
            mInterval = reinterpret_cast<Ptr<void>>(Interface.SwapInterval);
        }

        SetTearless(mConfig.Tearless);
        return true;
    }
}