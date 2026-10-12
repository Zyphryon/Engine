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

#include "D3D11Device.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static HRESULT Create(
        PFN_D3D11_CREATE_DEVICE          Function,
        D3D_DRIVER_TYPE                  Type,
        Ref<ComPtr<ID3D11Device>>        Device,
        Ref<ComPtr<ID3D11DeviceContext>> Context)
    {
        // Every driver call runs on the GPU thread, so the runtime need not lock.
        constexpr UINT Flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

        constexpr D3D_FEATURE_LEVEL kLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        return Function(
            nullptr,
            Type,
            nullptr,
            Flags,
            kLevels,
            _countof(kLevels),
            D3D11_SDK_VERSION,
            Device.ReleaseAndGetAddressOf(),
            nullptr,
            Context.ReleaseAndGetAddressOf());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LoadAdapters(Ptr<IDXGIFactory1> Factory, DXGI_FORMAT Format, Ref<Description> Output)
    {
        ComPtr<IDXGIAdapter1> Adapter;
        for (UInt Index = 0; SUCCEEDED(Factory->EnumAdapters1(Index, Adapter.ReleaseAndGetAddressOf())); ++Index)
        {
            // Skip software (WARP) adapters so the enumeration reflects only physical devices.
            DXGI_ADAPTER_DESC1 Description;
            if (FAILED(Adapter->GetDesc1(AddressOf(Description))) || (Description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
            {
                continue;
            }

            Ref<ZyGraphic::Adapter> Entry = Output.Adapters.Append();
            Entry.Description = Str::ConvertFromUTF16(StrConvert(Description.Description));
            Entry.Memory      = Description.DedicatedVideoMemory >> 20;

            // Enumerate the supported display modes exposed by the adapter's primary output.
            if (ComPtr<IDXGIOutput> Monitor; SUCCEEDED(Adapter->EnumOutputs(0, Monitor.GetAddressOf())))
            {
                UINT Length = 0;

                if (SUCCEEDED(Monitor->GetDisplayModeList(Format, 0, AddressOf(Length), nullptr)))
                {
                    Sequence<DXGI_MODE_DESC> Modes(Length);
                    Modes.Advance(Length);

                    Check(Monitor->GetDisplayModeList(Format, 0, AddressOf(Length), Modes.GetData()));

                    for (ConstRef<DXGI_MODE_DESC> Mode : Modes)
                    {
                        Ref<Resolution> Resolution = Entry.Resolutions.Append();
                        Resolution.Width     = Mode.Width;
                        Resolution.Height    = Mode.Height;
                        Resolution.Frequency = 0;

                        if (Mode.RefreshRate.Denominator)
                        {
                            Resolution.Frequency = Mode.RefreshRate.Numerator / Mode.RefreshRate.Denominator;
                        }
                    }
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LoadCapabilities(D3D_FEATURE_LEVEL Level, Ref<Description> Output)
    {
        Output.Backend  = "D3D11";
        Output.Language = ShaderLanguage::HLSL;

        // Determine common supported capabilities.
        Output.Capabilities.SupportsBaseVertex    = true;
        Output.Capabilities.SupportsFormatS3TC    = true;
        Output.Capabilities.UniformBlockAlignment = 256;

        // Determine the supported shader model based on the Direct3D feature level.
        switch (Level)
        {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            Output.Tier                               = Tier::Level3;
            Output.Capabilities.SupportsFormatRGTC    = true;
            Output.Capabilities.SupportsFormatBPTC    = true;
            Output.Capabilities.MaxTextureDimension   = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            Output.Capabilities.MaxTextureLayers      = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            Output.Capabilities.MaxTextureMipmaps     = D3D11_REQ_MIP_LEVELS;
            Output.Capabilities.MaxTextureSlots       = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
            Output.Capabilities.MaxRenderTargets      = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
            Output.Capabilities.MaxVertexAttributes   = D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
            Output.Capabilities.MaxVertexStreams      = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
            Output.Capabilities.MaxAnisotropy         = D3D11_REQ_MAXANISOTROPY;
            Output.Capabilities.UniformBlockCapacity  = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            break;
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            Output.Tier                               = Tier::Level2;
            Output.Capabilities.SupportsFormatRGTC    = true;
            Output.Capabilities.MaxTextureDimension   = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            Output.Capabilities.MaxTextureLayers      = D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            Output.Capabilities.MaxTextureMipmaps     = D3D10_REQ_MIP_LEVELS;
            Output.Capabilities.MaxTextureSlots       = D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
            Output.Capabilities.MaxRenderTargets      = D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT;
            Output.Capabilities.MaxVertexAttributes   = D3D10_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
            Output.Capabilities.MaxVertexStreams      = D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
            Output.Capabilities.MaxAnisotropy         = D3D10_REQ_MAXANISOTROPY;
            Output.Capabilities.UniformBlockCapacity  = D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            break;
        default:
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool LoadFactory(ConstRef<ComPtr<ID3D11Device>> Device, Ref<ComPtr<IDXGIFactory1>> Output)
    {
        // The factory the device's adapter came from, as a swapchain made by any other factory is refused.
        ComPtr<IDXGIDevice>  Interface;
        ComPtr<IDXGIAdapter> Adapter;

        return Check(Device.As<IDXGIDevice>(AddressOf(Interface)))
            && Check(Interface->GetAdapter(Adapter.GetAddressOf()))
            && Check(Adapter->GetParent(IID_PPV_ARGS(Output.GetAddressOf())));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Device::Initialize(TextureFormat Format)
    {
        PFN_D3D11_CREATE_DEVICE CreateDevice = nullptr;

        // Left loaded for the process, since what it creates outlives this call and is released by other owners.
        if (const HMODULE Dll = LoadSystemLibrary(L"D3D11.DLL"); Dll != nullptr)
        {
            CreateDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(::GetProcAddress(Dll, "D3D11CreateDevice"));
        }

        if (!CreateDevice)
        {
            LOG_E("Graphics: Failed to load D3D11.DLL");
            return false;
        }

        ComPtr<ID3D11Device>        Device;
        ComPtr<ID3D11DeviceContext> Context;

        // Tries hardware, which selects the highest level the adapter reaches, then the software rasterizer.
        HRESULT Result = Create(CreateDevice, D3D_DRIVER_TYPE_HARDWARE, Device, Context);

        if (FAILED(Result))
        {
            Result = Create(CreateDevice, D3D_DRIVER_TYPE_WARP, Device, Context);

            if (SUCCEEDED(Result))
            {
                LOG_I("Graphics: Enabling software mode (WARP)");
            }
        }

        if (!Check(Result)
            || !Check(Device.As<ID3D11Device1>(AddressOf(mHandle)))
            || !Check(Context.As<ID3D11DeviceContext1>(AddressOf(mContext)))
            || !LoadFactory(Device, mFactory))
        {
            return false;
        }

        LoadAdapters(mFactory.Get(), Convert(Format), mDescription);
        LoadCapabilities(mHandle->GetFeatureLevel(), mDescription);
        return true;
    }
}