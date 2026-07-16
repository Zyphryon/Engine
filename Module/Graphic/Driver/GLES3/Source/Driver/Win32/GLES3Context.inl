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
#include <glad/wgl.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool BootstrapExtensions()
    {
        WNDCLASSEXW Class { };
        Class.cbSize        = sizeof(Class);
        Class.style         = CS_OWNDC;
        Class.lpfnWndProc   = ::DefWindowProcW;
        Class.hInstance     = ::GetModuleHandleW(nullptr);
        Class.lpszClassName = L"ZyphryonGLES3Bootstrap";
        ::RegisterClassExW(AddressOf(Class));

        const HWND Window = ::CreateWindowExW(
            0, Class.lpszClassName, L"", 0, 0, 0, 1, 1, nullptr, nullptr, Class.hInstance, nullptr);
        const HDC  Device = ::GetDC(Window);

        PIXELFORMATDESCRIPTOR Descriptor { };
        Descriptor.nSize      = sizeof(Descriptor);
        Descriptor.nVersion   = 1;
        Descriptor.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        Descriptor.iPixelType = PFD_TYPE_RGBA;
        Descriptor.cColorBits = 32;

        const SInt32 Format = ::ChoosePixelFormat(Device, AddressOf(Descriptor));
        ::SetPixelFormat(Device, Format, AddressOf(Descriptor));

        const HGLRC Context = ::wglCreateContext(Device);
        ::wglMakeCurrent(Device, Context);

        const Bool Result = (gladLoaderLoadWGL(Device) != 0);

        ::wglMakeCurrent(nullptr, nullptr);
        ::wglDeleteContext(Context);
        ::ReleaseDC(Window, Device);
        ::DestroyWindow(Window);
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

    GLES3Context::GLES3Context()
        : mDeviceOutput  { nullptr },
          mDeviceContext { nullptr },
          mRenderContext { nullptr }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLES3Context::~GLES3Context()
    {
        if (mRenderContext)
        {
            ::wglMakeCurrent(nullptr, nullptr);
            ::wglDeleteContext(mRenderContext);
        }

        if (mDeviceContext)
        {
            ::ReleaseDC(mDeviceOutput, mDeviceContext);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool GLES3Context::Initialize(Ptr<void> Output, ConstRef<Config> Config)
    {
        if (!BootstrapExtensions())
        {
            LOG_E("GLES3Context: Failed to resolve WGL extensions (is a GPU driver installed?)");
            return false;
        }

        if (!GLAD_WGL_ARB_create_context || !GLAD_WGL_ARB_pixel_format)
        {
            LOG_E("GLES3Context: Driver lacks WGL_ARB_create_context / WGL_ARB_pixel_format");
            return false;
        }

        mDeviceOutput  = static_cast<HWND>(Output);
        mDeviceContext = ::GetDC(mDeviceOutput);

        if (mDeviceContext == nullptr)
        {
            LOG_E("GLES3Context: Failed to get device context");
            return false;
        }

        // Derive the pixel format request from the requested colour and depth formats.
        SInt32 ColorBits;
        SInt32 AlphaBits;
        SInt32 DepthBits;
        SInt32 StencilBits;
        ResolveColorBits(Config.ColorFormat, ColorBits, AlphaBits);
        ResolveDepthBits(Config.DepthFormat, DepthBits, StencilBits);

        // A floating-point back-buffer (HDR) needs the float pixel type, which is its own extension.
        const Bool WantsFloat = (Config.ColorFormat == TextureFormat::RGBA16Float) && GLAD_WGL_ARB_pixel_format_float;
        const Bool WantsSRGB  = (Config.ColorFormat == TextureFormat::RGBA8UIntNorm_sRGB) && GLAD_WGL_ARB_framebuffer_sRGB;

        // Select an accelerated, double-buffered pixel format through the ARB path.
        struct KeyPair { SInt32 Key; SInt32 Value; };

        Sequence<KeyPair, 16> Attributes;
        Attributes.Append(WGL_DRAW_TO_WINDOW_ARB, GL_TRUE);
        Attributes.Append(WGL_SUPPORT_OPENGL_ARB, GL_TRUE);
        Attributes.Append(WGL_DOUBLE_BUFFER_ARB,  GL_TRUE);
        Attributes.Append(WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB);
        Attributes.Append(WGL_PIXEL_TYPE_ARB,     WantsFloat ? WGL_TYPE_RGBA_FLOAT_ARB : WGL_TYPE_RGBA_ARB);
        Attributes.Append(WGL_COLOR_BITS_ARB,     ColorBits);
        Attributes.Append(WGL_ALPHA_BITS_ARB,     AlphaBits);
        Attributes.Append(WGL_DEPTH_BITS_ARB,     DepthBits);
        Attributes.Append(WGL_STENCIL_BITS_ARB,   StencilBits);
        if (WantsSRGB)
        {
            Attributes.Append(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE);
        }
        Attributes.Append(0, 0);

        const ConstPtr<SInt32> KeyPairs = AddressOf(Attributes.GetData()->Key);

        SInt32 PixelFormat;
        UINT   PixelCount;
        if (!wglChoosePixelFormatARB(mDeviceContext, KeyPairs, nullptr, 1, AddressOf(PixelFormat), AddressOf(PixelCount)) || PixelCount == 0)
        {
            LOG_E("GLES3Context: Failed to choose a compatible pixel format");
            return false;
        }

        PIXELFORMATDESCRIPTOR Descriptor { };
        ::DescribePixelFormat(mDeviceContext, PixelFormat, sizeof(Descriptor), AddressOf(Descriptor));

        if (!::SetPixelFormat(mDeviceContext, PixelFormat, AddressOf(Descriptor)))
        {
            LOG_E("GLES3Context: Failed to set the pixel format");
            return false;
        }

        // Request an OpenGL 3.3 core profile context (the desktop analogue of OpenGL ES 3.0).
        constexpr SInt32 ContextAttributes[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if !defined(NDEBUG)
            WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
            0
        };

        mRenderContext = wglCreateContextAttribsARB(mDeviceContext, nullptr, ContextAttributes);

        if (mRenderContext == nullptr)
        {
            LOG_E("GLES3Context: Failed to create an OpenGL 3.3 core context");
            return false;
        }

        if (!::wglMakeCurrent(mDeviceContext, mRenderContext))
        {
            LOG_E("GLES3Context: Failed to make the OpenGL context current");

            ::wglDeleteContext(mRenderContext);
            mRenderContext = nullptr;
            return false;
        }

        if (gladLoaderLoadGL() == 0)
        {
            LOG_E("GLES3Context: Failed to load OpenGL 3.3 entry points");
            return false;
        }

        // Gamma-encode writes to the default framebuffer when an sRGB back-buffer was requested.
        if (WantsSRGB)
        {
            glEnable(GL_FRAMEBUFFER_SRGB);
        }

        wglSwapIntervalEXT(0);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Context::Present()
    {
        ::SwapBuffers(mDeviceContext);
    }
}