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

#include "D3D11Swapchain.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool LoadTearing(Ptr<IDXGIFactory1> Factory)
    {
        BOOL Allowed = FALSE;

        // Tearing presentation needs DXGI 1.5.
        if (ComPtr<IDXGIFactory5> Latest; SUCCEEDED(Factory->QueryInterface(IID_PPV_ARGS(Latest.GetAddressOf()))))
        {
            Check(Latest->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, AddressOf(Allowed), sizeof(Allowed)));
        }
        return Allowed != FALSE;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UINT GetFlags(Bool Tearing)
    {
        // A resize must pass the flags the swapchain was created with.
        constexpr UINT Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        return Tearing ? Flags | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : Flags;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static DXGI_FORMAT GetBufferFormat(TextureFormat Format)
    {
        // The display scans out BGRA natively, and the flip model keeps an sRGB buffer in its linear twin.
        switch (Format)
        {
        case TextureFormat::RGBA8UIntNorm:
        case TextureFormat::RGBA8UIntNorm_sRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        default:
            return Convert(Format);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static DXGI_FORMAT GetViewFormat(TextureFormat Format)
    {
        switch (Format)
        {
        case TextureFormat::RGBA8UIntNorm:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::RGBA8UIntNorm_sRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        default:
            return Convert(Format);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Swapchain::Swapchain()
        : mWaitable { nullptr },
          mFormat   { DXGI_FORMAT_UNKNOWN },
          mTearing  { false }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Swapchain::~Swapchain()
    {
        if (mWaitable)
        {
            ::CloseHandle(mWaitable);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Swapchain::Create(ConstRef<Device> Device, HWND Window, ConstRef<Configuration> Config)
    {
        const Ptr<IDXGIFactory1> Factory = Device.GetFactory();

        mTearing = LoadTearing(Factory);

        DXGI_SWAP_CHAIN_DESC1 Description { };
        Description.Width       = Config.Width;
        Description.Height      = Config.Height;
        Description.Format      = GetBufferFormat(Config.ColorFormat);
        Description.SampleDesc  = { .Count = 1, .Quality = 0 };
        Description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        Description.BufferCount = 3;
        Description.Scaling     = DXGI_SCALING_NONE;
        Description.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        Description.AlphaMode   = DXGI_ALPHA_MODE_IGNORE;
        Description.Flags       = GetFlags(mTearing);

        ComPtr<IDXGIFactory2>   Creator;
        ComPtr<IDXGISwapChain1> Handle;

        if (!Check(Factory->QueryInterface(IID_PPV_ARGS(Creator.GetAddressOf())))
            || !Check(Creator->CreateSwapChainForHwnd(Device.GetHandle(), Window, AddressOf(Description), nullptr, nullptr, Handle.GetAddressOf()))
            || !Check(Handle.As<IDXGISwapChain2>(AddressOf(mHandle))))
        {
            LOG_E("Graphics: Failed to create a swapchain of '{0}'", ZyEnum::GetName(Config.ColorFormat));
            return false;
        }

        Check(Factory->MakeWindowAssociation(Window, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER));

        // The CPU never queues more frames than the ring it fills, and waits for a free one before it starts the next.
        Check(mHandle->SetMaximumFrameLatency(kMaxFrames));
        mWaitable = mHandle->GetFrameLatencyWaitableObject();

        mConfig = Config;
        mFormat = GetViewFormat(Config.ColorFormat);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Swapchain::Resize(UInt16 Width, UInt16 Height)
    {
        // A zero count and an unknown format keep the buffers' own.
        if (Check(mHandle->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, GetFlags(mTearing))))
        {
            mConfig.Width  = Width;
            mConfig.Height = Height;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Swapchain::Present()
    {
        ZY_PROFILE_SCOPE("Driver::Present");

        const UINT Interval = (mConfig.Tearless ? 1 : 0);
        const UINT Flags    = (Interval == 0 && mTearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
        Check(mHandle->Present(Interval, Flags));

        // Waiting here, between frames, keeps the stall for a free buffer out of the next frame's draws.
        ::WaitForSingleObjectEx(mWaitable, 1000, TRUE);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ComPtr<ID3D11Texture2D> Swapchain::GetBuffer() const
    {
        ComPtr<ID3D11Texture2D> Buffer;
        Check(mHandle->GetBuffer(0, IID_PPV_ARGS(Buffer.GetAddressOf())));
        return Buffer;
    }
}