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

#include "D3D11Driver.hpp"
#include "Zyphryon.Graphic/Metadata.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool D3D11Check(HRESULT Result)
    {
        if (FAILED(Result))
        {
            constexpr UINT Flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

            TCHAR Message[1024];
            ::FormatMessage(Flags, nullptr, Result, 0, Message, sizeof(Message), nullptr);

            LOG_E("Driver: {0}", Str::ConvertFromUTF16(StrConvert(Message)));
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static auto D3D11TranslateDSV(TextureFormat Value)
    {
        switch (Value)
        {
        case TextureFormat::D16UIntNorm:
            return DXGI_FORMAT_R16_UNORM;
        case TextureFormat::D32Float:
            return DXGI_FORMAT_R32_FLOAT;
        case TextureFormat::D24S8UIntNorm:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32S8UIntNorm:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static auto D3D11TranslateSRV(TextureFormat Value)
    {
        switch (Value)
        {
        case TextureFormat::D16UIntNorm:
            return DXGI_FORMAT_R16_UNORM;
        case TextureFormat::D32Float:
            return DXGI_FORMAT_R32_FLOAT;
        case TextureFormat::D24S8UIntNorm:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case TextureFormat::D32S8UIntNorm:
            return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        default:
            return D3D11Convert(Value);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool D3D11Driver::Initialize(Ptr<void> Output, ConstRef<Config> Config)
    {
        decltype(& D3D11CreateDevice)  D3D11CreateDevicePtr = nullptr;
        decltype(& CreateDXGIFactory1) CreateDXGIFactoryPtr = nullptr;

        if (const auto Dll = ::LoadLibraryExW(L"D3D11.DLL", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); Dll != nullptr)
        {
            D3D11CreateDevicePtr = reinterpret_cast<decltype(&D3D11CreateDevice)>(GetProcAddress(Dll, "D3D11CreateDevice"));
        }
        if (const auto Dll = ::LoadLibraryExW(L"DXGI.DLL", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); Dll != nullptr)
        {
            CreateDXGIFactoryPtr = reinterpret_cast<decltype(&CreateDXGIFactory1)>(GetProcAddress(Dll, "CreateDXGIFactory1"));
        }

        Bool Successful = D3D11CreateDevicePtr && CreateDXGIFactoryPtr;

        if (Successful)
        {
            ComPtr<ID3D11Device>        Device;
            ComPtr<ID3D11DeviceContext> DeviceImmediate;

            UInt Flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

            constexpr D3D_FEATURE_LEVEL Direct3DFeatureLevels[] = {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
                D3D_FEATURE_LEVEL_9_3,
                D3D_FEATURE_LEVEL_9_2,
                D3D_FEATURE_LEVEL_9_1,
            };

            // Tries Direct3D 11.1
            HRESULT Result = D3D11CreateDevicePtr(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                Flags,
                Direct3DFeatureLevels,
                _countof(Direct3DFeatureLevels) - 3, // 11.1 ... 10.0
                D3D11_SDK_VERSION,
                Device.GetAddressOf(),
                nullptr,
                DeviceImmediate.GetAddressOf());

            // Tries Direct3D 10.0 and below.
            if (FAILED(Result))
            {
                Result = D3D11CreateDevicePtr(
                    nullptr,
                    D3D_DRIVER_TYPE_HARDWARE,
                    nullptr,
                    Flags,
                    Direct3DFeatureLevels + 1,
                    _countof(Direct3DFeatureLevels) - 1, // 11.0 ... 9.1
                    D3D11_SDK_VERSION,
                    Device.GetAddressOf(),
                    nullptr,
                    DeviceImmediate.GetAddressOf());
            }

            // Tries Direct3D WARP mode.
            if (FAILED(Result))
            {
                Result = D3D11CreateDevicePtr(
                    nullptr,
                    D3D_DRIVER_TYPE_WARP,
                    nullptr,
                    Flags,
                    Direct3DFeatureLevels,
                    _countof(Direct3DFeatureLevels),
                    D3D11_SDK_VERSION,
                    Device.GetAddressOf(),
                    nullptr,
                    DeviceImmediate.GetAddressOf());

                if (SUCCEEDED(Result))
                {
                    LOG_I("Driver: Enabling software mode (WARP)");
                }
            }

            Successful = D3D11Check(Result);

            if (Successful)
            {
                Successful = D3D11Check(Device.As<ID3D11Device1>(AddressOf(mDevice)));
                Successful = Successful && D3D11Check(DeviceImmediate.As<ID3D11DeviceContext1>(AddressOf(mDeviceImmediate)));
                Successful = Successful && D3D11Check(CreateDXGIFactoryPtr(IID_PPV_ARGS(& mDeviceFactory)));

                if (Successful)
                {
                    LoadAdapters(D3D11Convert(Config.ColorFormat));
                    LoadCapabilities();

                    if (Output)
                    {
                        CreateSwapchain(mPasses[0], static_cast<HWND>(Output), Config);
                    }
                }
            }
        }

        return Successful;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Reset(UInt16 Width, UInt16 Height)
    {
        if (mDeviceImmediate)
        {
            mDeviceImmediate->ClearState();
        }

        /// Deletes the current display pass to release associated render targets.
        DeletePass(kDisplay);

        /// Resizes the swap chain buffers with the new resolution and format.
        D3D11Check(mSwapchain->ResizeBuffers(0, Width, Height,  DXGI_FORMAT_UNKNOWN, 0));

        /// Recreates swap chain resources, including color and depth-stencil attachments.
        Config Configuration;
        Configuration.Width       = Width;
        Configuration.Height      = Height;
        Configuration.ColorFormat = mDeviceProperties.ColorFormat;
        Configuration.DepthFormat = mDeviceProperties.DepthFormat;
        CreateSwapchainResources(mPasses[0], Configuration);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Probe(Ref<Description> Output) const
    {
        Output = mDescription;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data)
    {
        const UInt Size   = Usage   ==   Usage::Uniform ? Align(Capacity, 256)   : Capacity;
        const UInt Access = Storage == Storage::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
        const CD3D11_BUFFER_DESC Descriptor(Size, D3D11Convert(Usage), D3D11Convert(Storage),Access);

        D3D11_SUBRESOURCE_DATA Content {
            .pSysMem     = Data.GetData(),
            .SysMemPitch = static_cast<UINT>(Data.GetSizeInBytes())
        };

        const Ptr<D3D11_SUBRESOURCE_DATA> Memory = (Data.IsEmpty() ? nullptr : AddressOf(Content));
        D3D11Check(mDevice->CreateBuffer(AddressOf(Descriptor), Memory, mBuffers[ID].GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data)
    {
        constexpr D3D11_COPY_FLAGS Flags = D3D11_COPY_NO_OVERWRITE;

        const CD3D11_BOX Destination(Offset, 0, 0, Offset + Data.GetSizeInBytes(), 1, 1);
        mDeviceImmediate->UpdateSubresource1(mBuffers[ID].Get(), 0, AddressOf(Destination), Data.GetData(), 0, 0, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeleteBuffer(Object ID)
    {
        mBuffers[ID]->Release();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, UInt32 Size)
    {
        constexpr D3D11_COPY_FLAGS Flags = D3D11_COPY_NO_OVERWRITE;

        const CD3D11_BOX        Offset(SrcOffset, 0, 0, SrcOffset + Size, 1, 1);
        const Ptr<ID3D11Buffer> Target = mBuffers[DstBuffer].Get();
        const Ptr<ID3D11Buffer> Source = mBuffers[SrcBuffer].Get();
        mDeviceImmediate->CopySubresourceRegion1(Target, 0, DstOffset, 0, 0, Source, 0, AddressOf(Offset), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> D3D11Driver::MapBuffer(Object ID, UInt32 Offset, UInt32 Size)
    {
        constexpr D3D11_MAP      Mapping = D3D11_MAP_WRITE_NO_OVERWRITE;
        D3D11_MAPPED_SUBRESOURCE Memory;

        if (D3D11Check(mDeviceImmediate->Map(mBuffers[ID].Get(), 0, Mapping, 0, AddressOf(Memory))))
        {
            return static_cast<Ptr<Byte>>(Memory.pData) + Offset;
        }
        return nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::UnmapBuffer(Object ID)
    {
        mDeviceImmediate->Unmap(mBuffers[ID].Get(), 0);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth)
    {
        Ref<D3D11Pass> D3D11Pass = mPasses[ID];

        // Configure and create render target views (RTVs) for each color attachment.
        for (const ColorAttachment Color : Colors)
        {
            Ref<D3D11ColorAttachment> Attachment = D3D11Pass.Colors.Append();
            Attachment.Target      = mTextures[Color.Target].Object;
            Attachment.TargetLevel = Color.TargetLevel;

            if (Color.Resolve)
            {
                Attachment.Resolve       = mTextures[Color.Resolve].Object;
                Attachment.ResolveLevel  = Color.ResolveLevel;
                Attachment.ResolveFormat = D3D11TranslateSRV(mTextures[Color.Resolve].Format);
            }
            Attachment.LoadAction    = Color.LoadAction;
            Attachment.StoreAction   = Color.StoreAction;

            const CD3D11_RENDER_TARGET_VIEW_DESC Description(
                mTextures[Color.Target].Samples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D,
                D3D11TranslateSRV(mTextures[Color.Target].Format),
                Color.TargetLevel);

            D3D11Check(mDevice->CreateRenderTargetView(
                Attachment.Target.Get(), AddressOf(Description), Attachment.TargetResource.GetAddressOf()));
        }

        // Create and assign its depth-stencil view (DSV) if an auxiliary attachment is provided.
        if (Depth.Target > 0)
        {
            D3D11Pass.DepthStencil.DepthLoadAction    = Depth.DepthLoadAction;
            D3D11Pass.DepthStencil.DepthStoreAction   = Depth.DepthStoreAction;
            D3D11Pass.DepthStencil.StencilLoadAction  = Depth.StencilLoadAction;
            D3D11Pass.DepthStencil.StencilStoreAction = Depth.StencilStoreAction;

            const Ptr<ID3D11Texture2D> Attachment = mTextures[Depth.Target].Object.Get();

            const DXGI_FORMAT Format = D3D11TranslateDSV(mTextures[Depth.Target].Format);
            const CD3D11_DEPTH_STENCIL_VIEW_DESC Description(
                D3D11_DSV_DIMENSION_TEXTURE2D, Format, Depth.TargetLevel);
            D3D11Check(mDevice->CreateDepthStencilView(
                Attachment, AddressOf(Description), D3D11Pass.DepthStencil.Target.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeletePass(Object ID)
    {
        Destruct(mPasses[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<States> States)
    {
        Ref<D3D11Pipeline> Pipeline = mPipelines[ID];

        Sequence<D3D_SHADER_MACRO, 64> Macros;

        for (ConstRef<Macro> Macro : Program.Macros)
        {
            Macros.Append(Macro.Name.GetData(), Macro.Value.GetData());
        }
        Macros.Append("VERTEX_SHADER", nullptr);
        Macros.Append(nullptr, nullptr);

        const Ptr<ID3DBlob> VS = mCompiler.Compile(Program.Modules[0], mDescription.Tier, ShaderStage::Vertex, Macros.GetData());
        D3D11Check(mDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), nullptr, Pipeline.VS.GetAddressOf()));

        Macros[Macros.GetSize() - 2].Name = "FRAGMENT_SHADER";

        const Ptr<ID3DBlob> PS = mCompiler.Compile(Program.Modules[1], mDescription.Tier, ShaderStage::Fragment, Macros.GetData());
        D3D11Check(mDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), nullptr, Pipeline.PS.GetAddressOf()));

        {
            D3D11_BLEND_DESC Description = CD3D11_BLEND_DESC(CD3D11_DEFAULT());

            Description.RenderTarget[0].BlendEnable           = !(
                   States.BlendSrcColor       == BlendFactor::One
                && States.BlendSrcAlpha       == BlendFactor::One
                && States.BlendDstColor       == BlendFactor::Zero
                && States.BlendDstAlpha       == BlendFactor::Zero
                && States.BlendEquationColor  == BlendFunction::Add
                && States.BlendEquationAlpha  == BlendFunction::Add);
            Description.RenderTarget[0].SrcBlend              = D3D11Convert(States.BlendSrcColor);
            Description.RenderTarget[0].DestBlend             = D3D11Convert(States.BlendDstColor);
            Description.RenderTarget[0].BlendOp               = D3D11Convert(States.BlendEquationColor);
            Description.RenderTarget[0].SrcBlendAlpha         = D3D11Convert(States.BlendSrcAlpha);
            Description.RenderTarget[0].DestBlendAlpha        = D3D11Convert(States.BlendDstAlpha);
            Description.RenderTarget[0].BlendOpAlpha          = D3D11Convert(States.BlendEquationAlpha);
            Description.RenderTarget[0].RenderTargetWriteMask = static_cast<D3D11_COLOR_WRITE_ENABLE>(States.Channel);
            Description.AlphaToCoverageEnable                 = States.AlphaToCoverage;

            D3D11Check(mDevice->CreateBlendState(& Description, Pipeline.BS.GetAddressOf()));
        }

        {
            D3D11_DEPTH_STENCIL_DESC Description = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());

            Description.DepthEnable    = States.DepthTest != TestCondition::Always || States.DepthMask;
            Description.DepthFunc      = D3D11Convert(States.DepthTest);
            Description.DepthWriteMask = static_cast<D3D11_DEPTH_WRITE_MASK>(States.DepthMask);
            Description.StencilEnable  = States.StencilFrontTest      != TestCondition::Always
                                      || States.StencilFrontFail      != TestAction::Keep
                                      || States.StencilFrontDepthFail != TestAction::Keep
                                      || States.StencilFrontDepthPass != TestAction::Keep
                                      || States.StencilBackTest       != TestCondition::Always
                                      || States.StencilBackFail       != TestAction::Keep
                                      || States.StencilBackDepthFail  != TestAction::Keep
                                      || States.StencilBackDepthPass  != TestAction::Keep;
            Description.StencilReadMask              = States.StencilReadMask;
            Description.StencilWriteMask             = States.StencilWriteMask;
            Description.FrontFace.StencilFunc        = D3D11Convert(States.StencilFrontTest);
            Description.FrontFace.StencilFailOp      = D3D11Convert(States.StencilFrontFail);
            Description.FrontFace.StencilDepthFailOp = D3D11Convert(States.StencilFrontDepthFail);
            Description.FrontFace.StencilPassOp      = D3D11Convert(States.StencilFrontDepthPass);
            Description.BackFace.StencilFunc         = D3D11Convert(States.StencilBackTest);
            Description.BackFace.StencilFailOp       = D3D11Convert(States.StencilBackFail);
            Description.BackFace.StencilDepthFailOp  = D3D11Convert(States.StencilBackDepthFail);
            Description.BackFace.StencilPassOp       = D3D11Convert(States.StencilBackDepthPass);

            D3D11Check(mDevice->CreateDepthStencilState(& Description, Pipeline.DS.GetAddressOf()));
        }

        {
            D3D11_RASTERIZER_DESC Description = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());

            Description.FrontCounterClockwise = FALSE;
            Description.DepthBias             = States.DepthBias;
            Description.DepthBiasClamp        = States.DepthBiasClamp;
            Description.SlopeScaledDepthBias  = States.DepthBiasSlope;
            Description.DepthClipEnable       = States.DepthClip;
            Description.CullMode              = D3D11Convert(States.Cull);
            Description.ScissorEnable         = States.Scissor;
            Description.MultisampleEnable     = TRUE;
            Description.AntialiasedLineEnable = TRUE;
            Description.FillMode              = D3D11Convert(States.Fill);

            D3D11Check(mDevice->CreateRasterizerState(& Description, Pipeline.RS.GetAddressOf()));
        }

        {
            D3D11_INPUT_ELEMENT_DESC Description[kMaxAttributes];
            UInt                     Count = 0;

            for (ConstRef<Attribute> Attribute : States.InputAttributes)
            {
                Ref<D3D11_INPUT_ELEMENT_DESC> Element = Description[Count++];
                Element.SemanticName         = "SLOT";
                Element.SemanticIndex        = Attribute.Location;
                Element.Format               = D3D11Convert(Attribute.Format);
                Element.AlignedByteOffset    = Attribute.Offset;
                Element.InputSlot            = Attribute.Stream;
                Element.InputSlotClass       = Attribute.Divisor ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
                Element.InstanceDataStepRate = Attribute.Divisor;
            }

            D3D11Check(mDevice->CreateInputLayout(Description, Count, VS->GetBufferPointer(), VS->GetBufferSize(), Pipeline.IL.GetAddressOf()));
        }

        Pipeline.PT = D3D11Convert(States.InputPrimitive);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeletePipeline(Object ID)
    {
        Destruct(mPipelines[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, ConstSpan<Byte> Data)
    {
        CD3D11_TEXTURE2D_DESC Description(D3D11Convert(Format), Width, Height, 1, Mipmaps);
        Description.Usage      = D3D11Convert(Storage);
        Description.BindFlags  = HasBit(Usage, Usage::Sample) ? D3D11_BIND_SHADER_RESOURCE : 0;
        Description.MiscFlags  = Mipmaps < 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
        Description.SampleDesc = {
            .Count = Enum::Cast(Samples),
            .Quality = mDeviceProperties.Multisample[Enum::Cast(Format)][Enum::Cast(Samples)]
        };

        if (HasBit(Usage, Usage::Target))
        {
            switch (Format)
            {
            case TextureFormat::D16UIntNorm:
            case TextureFormat::D32Float:
            case TextureFormat::D24S8UIntNorm:
            case TextureFormat::D32S8UIntNorm:
                Description.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
                break;
            default:
                Description.BindFlags |= D3D11_BIND_RENDER_TARGET;
                break;
            }
        }

        Ref<D3D11Texture> Texture = mTextures[ID];
        Texture.Format  = Format;
        Texture.Samples = Enum::Cast(Samples);

        // Fill the data.
        D3D11_SUBRESOURCE_DATA      Content[kMaxMipmaps];
        Ptr<D3D11_SUBRESOURCE_DATA> Memory = nullptr;

        const TDescTextureFormat TDescFormat = GetMetadata(Format);

        if (ConstPtr<Byte> Bytes = Data.GetData(); Bytes)
        {
            const UInt32 BitsPerPixel = TDescFormat.BitsPerPixel;
            const UInt32 BlockSize    = TDescFormat.BlockSize;
            const UInt32 BlockPitch   = (Width + BlockSize - 1) / BlockSize * (BitsPerPixel * BlockSize * BlockSize / 8);

            for (UInt32 Level = 0; Level < Mipmaps; ++Level)
            {
                Content[Level].pSysMem          = Bytes;
                Content[Level].SysMemPitch      = BlockPitch;
                Content[Level].SysMemSlicePitch = BlockPitch * ((Height + BlockSize - 1) / BlockSize);

                Bytes += BlockPitch * ((Height + BlockSize - 1) / BlockSize);
                Width  = Max(1U, Width  >> 1);
                Height = Max(1U, Height >> 1);
            }
            Memory = Content;
        }
        D3D11Check(mDevice->CreateTexture2D(AddressOf(Description), Memory, Texture.Object.GetAddressOf()));

        if (HasBit(Usage, Usage::Sample))
        {
            const D3D11_SRV_DIMENSION Dimension = (Samples != Multisample::X1)
                ? D3D11_SRV_DIMENSION_TEXTURE2DMS
                : D3D11_SRV_DIMENSION_TEXTURE2D;

            const CD3D11_SHADER_RESOURCE_VIEW_DESC View(Dimension, D3D11TranslateSRV(Format), 0, Mipmaps);
            D3D11Check(mDevice->CreateShaderResourceView(Texture.Object.Get(), AddressOf(View), Texture.Resource.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeleteTexture(Object ID)
    {
        Destruct(mTextures[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
    {
        constexpr D3D11_COPY_FLAGS Flags = D3D11_COPY_NO_OVERWRITE;

        const CD3D11_BOX Offset(X, Y, 0, X + Width, Y + Height, 1);
        mDeviceImmediate->UpdateSubresource1(mTextures[ID].Object.Get(), Level, AddressOf(Offset), Data.GetData(), Pitch, 0, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16  SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, UInt16 Width, UInt16 Height)
    {
        constexpr D3D11_COPY_FLAGS Flags = D3D11_COPY_NO_OVERWRITE;

        const CD3D11_BOX           Offset(SrcX, SrcY, 0, SrcX + Width, SrcY + Height, 1);
        const Ptr<ID3D11Texture2D> Target = mTextures[DstTexture].Object.Get();
        const Ptr<ID3D11Texture2D> Source = mTextures[SrcTexture].Object.Get();
        mDeviceImmediate->CopySubresourceRegion1(Target, DstLevel, DstX, DstY, 0, Source, SrcLevel, AddressOf(Offset), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        Ref<D3D11Pass> D3D11Pass = mPasses[Pass];

        Sequence<Ptr<ID3D11RenderTargetView>, kMaxAttachments> ColorAttachments;
        for (ConstRef<D3D11ColorAttachment> Attachment : D3D11Pass.Colors)
        {
            ColorAttachments.Append(Attachment.TargetResource.Get());
        }

        // Bind the render targets for the rendering pass.
        const Ptr<ID3D11DepthStencilView> DepthAttachment
            = (D3D11Pass.DepthStencil.Target ? D3D11Pass.DepthStencil.Target.Get() : nullptr);
        mDeviceImmediate->OMSetRenderTargets(ColorAttachments.GetSize(), ColorAttachments.GetData(), DepthAttachment);

        // Clear color attachments as specified.
        for (UInt32 Index = 0; Index < ColorAttachments.GetSize(); ++Index)
        {
            ConstRef<D3D11ColorAttachment> Attachment = D3D11Pass.Colors[Index];

            if (Attachment.LoadAction == Action::Clear)
            {
                mDeviceImmediate->ClearRenderTargetView(
                    ColorAttachments[Index], reinterpret_cast<ConstPtr<FLOAT>>(AddressOf(Colors[Index])));
            }
        }

        // Clear depth/stencil attachment as specified.
        UINT Mode = 0;
        if (D3D11Pass.DepthStencil.DepthLoadAction == Action::Clear)
        {
            Mode |= D3D11_CLEAR_DEPTH;
        }
        if (D3D11Pass.DepthStencil.StencilLoadAction == Action::Clear)
        {
            Mode |= D3D11_CLEAR_STENCIL;
        }

        if (Mode != 0)
        {
            mDeviceImmediate->ClearDepthStencilView(D3D11Pass.DepthStencil.Target.Get(), Mode, Depth, Stencil);
        }

        // Set the viewport for rendering.
        const CD3D11_VIEWPORT Rect(
            Viewport.X,
            Viewport.Y,
            Viewport.Width,
            Viewport.Height,
            Viewport.MinDepth,
            Viewport.MaxDepth);
        mDeviceImmediate->RSSetViewports(1, AddressOf(Rect));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Submit(ConstSpan<Command> Commands)
    {
        static constexpr Command kEmptyCommand { };

        for (UInt32 Batch = 0; Batch < Commands.GetSize(); ++Batch)
        {
            ConstRef<Command> NewestSubmission = Commands[Batch];
            ConstRef<Command> OldestSubmission = Batch > 0 ? Commands[Batch - 1] : kEmptyCommand;

            // Apply vertices
            ApplyVertexResources(OldestSubmission, NewestSubmission);

            // Apply indices
            if (OldestSubmission.Indices.Buffer != NewestSubmission.Indices.Buffer ||
                OldestSubmission.Indices.Offset != NewestSubmission.Indices.Offset ||
                OldestSubmission.Indices.Stride != NewestSubmission.Indices.Stride)
            {
                ConstRef<D3D11Buffer> Buffer = mBuffers[NewestSubmission.Indices.Buffer];
                const DXGI_FORMAT     Format =
                    NewestSubmission.Indices.Stride == 1 ? DXGI_FORMAT_R8_UINT  :
                    NewestSubmission.Indices.Stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
                mDeviceImmediate->IASetIndexBuffer(Buffer.Get(), Format, NewestSubmission.Indices.Offset);
            }

            // Apply the scissor rect
            const Bool ScissorDirty = (OldestSubmission.Scissor.X      != NewestSubmission.Scissor.X)
                                   || (OldestSubmission.Scissor.Y      != NewestSubmission.Scissor.Y)
                                   || (OldestSubmission.Scissor.Width  != NewestSubmission.Scissor.Width)
                                   || (OldestSubmission.Scissor.Height != NewestSubmission.Scissor.Height);

            if (ScissorDirty)
            {
                const RECT Scissor = CD3D11_RECT(
                    NewestSubmission.Scissor.X,
                    NewestSubmission.Scissor.Y,
                    NewestSubmission.Scissor.X + NewestSubmission.Scissor.Width,
                    NewestSubmission.Scissor.Y + NewestSubmission.Scissor.Height);
                mDeviceImmediate->RSSetScissorRects(1, AddressOf(Scissor));
            }

            // Apply pipeline or stencil value
            if (OldestSubmission.Pipeline != NewestSubmission.Pipeline)
            {
                ConstRef<D3D11Pipeline> Old = mPipelines[OldestSubmission.Pipeline];
                ConstRef<D3D11Pipeline> New = mPipelines[NewestSubmission.Pipeline];

                if (Old.VS != New.VS)
                {
                    mDeviceImmediate->VSSetShader(New.VS.Get(), nullptr, 0);
                }
                if (Old.PS != New.PS)
                {
                    mDeviceImmediate->PSSetShader(New.PS.Get(), nullptr, 0);
                }
                if (Old.BS != New.BS)
                {
                    mDeviceImmediate->OMSetBlendState(New.BS.Get(), nullptr, D3D11_DEFAULT_SAMPLE_MASK);
                }
                if (Old.DS != New.DS || OldestSubmission.Stencil != NewestSubmission.Stencil)
                {
                    mDeviceImmediate->OMSetDepthStencilState(New.DS.Get(), NewestSubmission.Stencil);
                }
                if (Old.RS != New.RS)
                {
                    mDeviceImmediate->RSSetState(New.RS.Get());
                }
                if (Old.IL != New.IL)
                {
                    mDeviceImmediate->IASetInputLayout(New.IL.Get());
                }
                if (Old.PT != New.PT)
                {
                    mDeviceImmediate->IASetPrimitiveTopology(New.PT);
                }
            }
            else if (OldestSubmission.Stencil != NewestSubmission.Stencil)
            {
                mDeviceImmediate->OMSetDepthStencilState(
                    mPipelines[NewestSubmission.Pipeline].DS.Get(), NewestSubmission.Stencil);
            }

            // Apply stage(s) resources
            ApplySamplerResources(OldestSubmission, NewestSubmission);
            ApplyTextureResources(OldestSubmission, NewestSubmission);
            ApplyUniformResources(OldestSubmission, NewestSubmission);

            // Issue draw command
            const UInt32 Count     = NewestSubmission.Parameters.Count;
            const SInt32 Base      = NewestSubmission.Parameters.Base;
            const UInt32 Offset    = NewestSubmission.Parameters.Offset;
            const UInt32 Instances = NewestSubmission.Parameters.Instances;

            if (NewestSubmission.Indices.Buffer)
            {
                if (Instances > 1)
                {
                    mDeviceImmediate->DrawIndexedInstanced(Count, Instances, Offset, Base, 0);
                }
                else
                {
                    mDeviceImmediate->DrawIndexed(Count, Offset, Base);
                }
            }
            else
            {
                if (Instances > 1)
                {
                    mDeviceImmediate->DrawInstanced(Count, Instances, Offset, 0);
                }
                else
                {
                    mDeviceImmediate->Draw(Count, Offset);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Commit(Object Pass)
    {
        /// Resolve multisample color attachments into their corresponding single-sample targets.
        for (ConstRef<D3D11ColorAttachment> Attachment : mPasses[Pass].Colors)
        {
            if (Attachment.StoreAction == Action::Store)
            {
                if (Attachment.Resolve)
                {
                    mDeviceImmediate->ResolveSubresource(
                        Attachment.Resolve.Get(),
                        Attachment.ResolveLevel,
                        Attachment.Target.Get(),
                        Attachment.TargetLevel,
                        Attachment.ResolveFormat);
                }
            }
            else
            {
                mDeviceImmediate->DiscardView(Attachment.TargetResource.Get());
            }
        }

        Ref<D3D11DepthStencilAttachment> DepthAttachment = mPasses[Pass].DepthStencil;
        if (DepthAttachment.StencilStoreAction == Action::Discard || DepthAttachment.DepthStoreAction == Action::Discard)
        {
            mDeviceImmediate->DiscardView(DepthAttachment.Target.Get());
        }

        /// Present the swap chain if this is the primary rendering pass.
        if (Pass == kDisplay)
        {
            D3D11Check(mSwapchain->Present(0, 0));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::LoadAdapters(DXGI_FORMAT Format)
    {
        ComPtr<IDXGIAdapter1> DXGIAdapter;
        for (UInt Index = 0; SUCCEEDED(mDeviceFactory->EnumAdapters1(Index, DXGIAdapter.GetAddressOf())); ++Index)
        {
            DXGI_ADAPTER_DESC1 DXGIDescription;
            if (SUCCEEDED(DXGIAdapter->GetDesc1(AddressOf(DXGIDescription))))
            {
                if ((DXGIDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
                {
                    Ref<Adapter> AdapterInfo = mDescription.Endpoints.Append();

                    AdapterInfo.Description = Str::ConvertFromUTF16(StrConvert(DXGIDescription.Description));
                    AdapterInfo.Memory      = DXGIDescription.DedicatedVideoMemory >> 20;
                }
            }

            if (ComPtr<IDXGIOutput> DXGIOutput; SUCCEEDED(DXGIAdapter->EnumOutputs(0, DXGIOutput.GetAddressOf())))
            {
                UINT Length = 0;

                if (SUCCEEDED(DXGIOutput->GetDisplayModeList(Format, 0, AddressOf(Length), nullptr)))
                {
                    Sequence<DXGI_MODE_DESC> Descriptions(Length);
                    Descriptions.Advance(Length);

                    D3D11Check(DXGIOutput->GetDisplayModeList(Format, 0, AddressOf(Length), Descriptions.GetData()));

                    for (Ref<DXGI_MODE_DESC> Description : Descriptions)
                    {
                        Ref<Resolution> Resolution = mDescription.Endpoints[Index].Resolutions.Append();
                        Resolution.Width     = Description.Width;
                        Resolution.Height    = Description.Height;
                        Resolution.Frequency = Description.RefreshRate.Numerator / Description.RefreshRate.Denominator;
                    }
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::LoadCapabilities()
    {
        mDescription.Backend  = "D3D11";
        mDescription.Language = ShaderLanguage::HLSL;

        // Determine the supported shader model based on the Direct3D feature level.
        switch (mDevice->GetFeatureLevel())
        {
        case D3D_FEATURE_LEVEL_12_1:
        case D3D_FEATURE_LEVEL_12_0:
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            mDescription.Tier                               = Tier::Level3;
            mDescription.Capabilities.MaxTextureDimension   = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            mDescription.Capabilities.MaxTextureLayers      = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            mDescription.Capabilities.MaxTextureMipmaps     = D3D11_REQ_MIP_LEVELS;
            mDescription.Capabilities.MaxTextureSlots       = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxRenderTargets      = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
            mDescription.Capabilities.MaxVertexAttributes   = D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
            mDescription.Capabilities.MaxVertexStreams      = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxAnisotropy         = D3D11_REQ_MAXANISOTROPY;
            mDescription.Capabilities.UniformBlockAlignment = 256;
            mDescription.Capabilities.UniformBlockCapacity  = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            break;
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            mDescription.Tier                               = Tier::Level2;
            mDescription.Capabilities.MaxTextureDimension   = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            mDescription.Capabilities.MaxTextureLayers      = D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            mDescription.Capabilities.MaxTextureMipmaps     = D3D10_REQ_MIP_LEVELS;
            mDescription.Capabilities.MaxTextureSlots       = D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxRenderTargets      = D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT;
            mDescription.Capabilities.MaxVertexAttributes   = D3D10_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
            mDescription.Capabilities.MaxVertexStreams      = D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxAnisotropy         = D3D10_REQ_MAXANISOTROPY;
            mDescription.Capabilities.UniformBlockAlignment = 256;
            mDescription.Capabilities.UniformBlockCapacity  = D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            break;
        case D3D_FEATURE_LEVEL_9_3:
            mDescription.Tier                               = Tier::Level1;
            mDescription.Capabilities.MaxTextureDimension   = D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            mDescription.Capabilities.MaxTextureLayers      = 0;
            mDescription.Capabilities.MaxTextureMipmaps     = D3D10_REQ_MIP_LEVELS;
            mDescription.Capabilities.MaxTextureSlots       = 16;
            mDescription.Capabilities.MaxRenderTargets      = 4;
            mDescription.Capabilities.MaxVertexAttributes   = D3D10_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
            mDescription.Capabilities.MaxVertexStreams      = D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxAnisotropy         = D3D10_REQ_MAXANISOTROPY;
            mDescription.Capabilities.UniformBlockAlignment = 256;
            mDescription.Capabilities.UniformBlockCapacity  = D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            break;
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            mDescription.Tier                               = Tier::Level1;
            mDescription.Capabilities.MaxTextureDimension   = D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            mDescription.Capabilities.MaxTextureLayers      = 0;
            mDescription.Capabilities.MaxTextureMipmaps     = 12;
            mDescription.Capabilities.MaxTextureSlots       = 16;
            mDescription.Capabilities.MaxRenderTargets      = 1;
            mDescription.Capabilities.MaxVertexAttributes   = D3D10_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
            mDescription.Capabilities.MaxVertexStreams      = D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxAnisotropy         = 2;
            mDescription.Capabilities.UniformBlockAlignment = 256;
            mDescription.Capabilities.UniformBlockCapacity  = D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            break;
        default:
            break;
        }

        // Query supported multisample anti-aliasing (MSAA) levels for each texture format.
        for (const TextureFormat Format : Enum::GetValues<TextureFormat>())
        {
            if (Format == TextureFormat::Unspecified)
            {
                continue;
            }

            const DXGI_FORMAT DXGIFormat = D3D11Convert(Format);

            for (const Multisample Sample : Enum::GetValues<Multisample>())
            {
                const UINT Count   = static_cast<UInt32>(Sample);
                UINT       Quality = 0;

                if (SUCCEEDED(mDevice->CheckMultisampleQualityLevels(DXGIFormat, Count, AddressOf(Quality))) && Quality > 0)
                {
                    mDeviceProperties.Multisample[Enum::Cast(Format)][Enum::Cast(Sample)] = static_cast<UInt8>(Quality - 1);
                }
                else
                {
                    mDeviceProperties.Multisample[Enum::Cast(Format)][Enum::Cast(Sample)] = 0;
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateSwapchain(Ref<D3D11Pass> Pass, HWND Window, ConstRef<Config> Config)
    {
        const DXGI_FORMAT Format = (Config.ColorFormat == TextureFormat::RGBA8UIntNorm_sRGB)
            ? DXGI_FORMAT_R8G8B8A8_UNORM
            : D3D11Convert(Config.ColorFormat);

        DXGI_SWAP_CHAIN_DESC Description { };
        Description.BufferCount       = 1;
        Description.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        Description.Flags             = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        Description.BufferDesc.Format = Format;
        Description.BufferDesc.Width  = Config.Width;
        Description.BufferDesc.Height = Config.Height;
        Description.SampleDesc        = { .Count = 1, .Quality = 0 };
        Description.OutputWindow      = Window;
        Description.Windowed          = true;
        Description.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;

        D3D11Check(mDeviceFactory->CreateSwapChain(mDevice.Get(), AddressOf(Description), mSwapchain.GetAddressOf()));
        D3D11Check(mDeviceFactory->MakeWindowAssociation(Description.OutputWindow, DXGI_MWA_NO_WINDOW_CHANGES));

        CreateSwapchainResources(Pass, Config);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateSwapchainResources(Ref<D3D11Pass> Pass, ConstRef<Config> Config) const
    {
        // Acquire the swapchain back-buffer and create a render target view for it.
        ComPtr<ID3D11Resource> ColorBuffer;
        D3D11Check(mSwapchain->GetBuffer(
            0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(ColorBuffer.GetAddressOf())));

        Ref<D3D11ColorAttachment> Attachment = Pass.Colors.Append();
        Attachment.Target        = ColorBuffer;
        Attachment.TargetLevel   = 0;
        Attachment.ResolveLevel  = 0;
        Attachment.ResolveFormat = D3D11Convert(Config.ColorFormat);
        Attachment.LoadAction    = Action::Clear;
        Attachment.StoreAction   = Action::Store;

        D3D11Check(mDevice->CreateRenderTargetView(Attachment.Target.Get(), nullptr, Attachment.TargetResource.GetAddressOf()));

        // Create a depth-stencil buffer matching the swapchain dimensions and sample count.
        if (Config.DepthFormat != TextureFormat::Unspecified)
        {
            D3D11_TEXTURE2D_DESC DepthStencilTextureDescription { };
            DepthStencilTextureDescription.Width      = Config.Width;
            DepthStencilTextureDescription.Height     = Config.Height;
            DepthStencilTextureDescription.MipLevels  = 1;
            DepthStencilTextureDescription.ArraySize  = 1;
            DepthStencilTextureDescription.Format     = D3D11Convert(Config.DepthFormat);
            DepthStencilTextureDescription.SampleDesc = { .Count = 1, .Quality = 0 };
            DepthStencilTextureDescription.Usage      = D3D11_USAGE_DEFAULT;
            DepthStencilTextureDescription.BindFlags  = D3D11_BIND_DEPTH_STENCIL;

            Pass.DepthStencil.DepthLoadAction    = Action::Clear;
            Pass.DepthStencil.DepthStoreAction   = Action::Discard;
            Pass.DepthStencil.StencilLoadAction  = Action::Clear;
            Pass.DepthStencil.StencilStoreAction = Action::Discard;

            ComPtr<ID3D11Texture2D> Depth;
            D3D11Check(mDevice->CreateTexture2D(AddressOf(DepthStencilTextureDescription), nullptr, Depth.GetAddressOf()));

            CD3D11_DEPTH_STENCIL_VIEW_DESC DepthViewDescription(D3D11_DSV_DIMENSION_TEXTURE2D, D3D11TranslateDSV(Config.DepthFormat));
            D3D11Check(mDevice->CreateDepthStencilView(Depth.Get(), AddressOf(DepthViewDescription), Pass.DepthStencil.Target.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyVertexResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        Ptr<ID3D11Buffer> Array[Command::kMaxVertices];
        UINT ArrayOffset[Command::kMaxVertices];
        UINT ArrayStride[Command::kMaxVertices];
        UInt32 Min = Command::kMaxVertices;
        UInt32 Max = 0u;

        const UInt32 OldSize = Oldest.Vertices.GetSize();
        const UInt32 NewSize = Newest.Vertices.GetSize();

        for (UInt32 Element = 0; Element < NewSize; ++Element)
        {
            const Stream Old = Element < OldSize ? Oldest.Vertices[Element] : Stream {};
            const Stream New = Element < NewSize ? Newest.Vertices[Element] : Stream {};

            if (Old.Buffer != New.Buffer || Old.Offset != New.Offset || Old.Stride != New.Stride)
            {
                Min = ::Min(Element, Min);
                Max = ::Max(Element + 1, Max);
            }

            Array[Element]       = mBuffers[New.Buffer].Get();
            ArrayOffset[Element] = New.Offset;
            ArrayStride[Element] = New.Stride;
        }

        if (Min < Max)
        {
            const UInt32 Count = Max - Min;

            mDeviceImmediate->IASetVertexBuffers(Min, Count, Array + Min, ArrayStride + Min, ArrayOffset + Min);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplySamplerResources(ConstRef<Command> Oldest, ConstRef<Command> Newest)
    {
        Ptr<ID3D11SamplerState> Array[Command::kMaxTextures];
        UInt32 Min = Command::kMaxTextures;
        UInt32 Max = 0u;

        const UInt32 OldSize = Oldest.Samplers.GetSize();
        const UInt32 NewSize = Newest.Samplers.GetSize();

        for (UInt32 Element = 0; Element < NewSize; ++Element)
        {
            const Sampler Old = Element < OldSize ? Oldest.Samplers[Element] : Sampler {};
            const Sampler New = Element < NewSize ? Newest.Samplers[Element] : Sampler {};

            if (Hash(Old) != Hash(New))
            {
                Min = ::Min(Element, Min);
                Max = ::Max(Element + 1, Max);
            }
            Array[Element] = GetOrCreateSampler(New).Get();
        }

        if (Min < Max)
        {
            const UInt32 Count = Max - Min;
            mDeviceImmediate->VSSetSamplers(Min, Count, Array + Min);
            mDeviceImmediate->PSSetSamplers(Min, Count, Array + Min);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyTextureResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        Ptr<ID3D11ShaderResourceView> Array[Command::kMaxTextures];
        UInt32 Min = Command::kMaxTextures;
        UInt32 Max = 0u;

        const UInt32 OldSize = Oldest.Textures.GetSize();
        const UInt32 NewSize = Newest.Textures.GetSize();

        for (UInt32 Element = 0; Element < NewSize; ++Element)
        {
            const Object Old = Element < OldSize ? Oldest.Textures[Element] : Object {};
            const Object New = Element < NewSize ? Newest.Textures[Element] : Object {};

            if (Old != New)
            {
                Min = ::Min(Element, Min);
                Max = ::Max(Element + 1, Max);
            }
            Array[Element] = mTextures[New].Resource.Get();
        }

        if (Min < Max)
        {
            const UInt32 Count = Max - Min;
            mDeviceImmediate->VSSetShaderResources(Min, Count, Array + Min);
            mDeviceImmediate->PSSetShaderResources(Min, Count, Array + Min);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyUniformResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        Ptr<ID3D11Buffer> Array[Command::kMaxUniforms];
        UINT ArrayOffset[Command::kMaxUniforms];
        UINT ArrayLength[Command::kMaxUniforms];
        UInt32 Min = Command::kMaxUniforms;
        UInt32 Max = 0u;

        for (UInt32 Element = 0; Element < Command::kMaxUniforms; ++Element)
        {
            const Stream Old = Oldest.Uniforms[Element];
            const Stream New = Newest.Uniforms[Element];

            if (Old.Buffer != New.Buffer || Old.Offset != New.Offset || Old.Stride != New.Stride)
            {
                Min = ::Min(Element, Min);
                Max = ::Max(Element + 1, Max);
            }

            Array[Element]       = mBuffers[New.Buffer].Get();
            ArrayOffset[Element] = New.Offset / sizeof(Vector4);
            ArrayLength[Element] = New.Stride / sizeof(Vector4);
        }

        if (Min < Max)
        {
            const UInt32 Count = Max - Min;

            mDeviceImmediate->VSSetConstantBuffers1(Min, Count, Array + Min, ArrayOffset + Min, ArrayLength + Min);
            mDeviceImmediate->PSSetConstantBuffers1(Min, Count, Array + Min, ArrayOffset + Min, ArrayLength + Min);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    D3D11Driver::D3D11Sampler D3D11Driver::GetOrCreateSampler(Sampler Descriptor)
    {
        D3D11Sampler Sampler = mSamplers.FindOrInsert(Hash(Descriptor));

        if (Sampler == nullptr)
        {
            UINT   Anisotropy = 1;
            Real32 MinLOD     = -FLT_MAX;
            Real32 MaxLOD     = +FLT_MAX;

            switch (Descriptor.Filter)
            {
            case TextureFilter::Point:
            case TextureFilter::Linear:
                MinLOD = 0.0f;
                MaxLOD = 0.0f;
                break;
            case TextureFilter::Anisotropic2x:
                Anisotropy = 2;
                break;
            case TextureFilter::Anisotropic4x:
                Anisotropy = 4;
                break;
            case TextureFilter::Anisotropic8x:
                Anisotropy = 8;
                break;
            case TextureFilter::Anisotropic16x:
                Anisotropy = 16;
                break;
            default:
                break;
            }

            const CD3D11_SAMPLER_DESC SamplerDescriptor(
                D3D11Convert(Descriptor.Filter),
                D3D11Convert(Descriptor.AddressModeU),
                D3D11Convert(Descriptor.AddressModeV),
                D3D11Convert(Descriptor.AddressModeW),
                0,
                Anisotropy,
                D3D11Convert(Descriptor.Comparison),
                D3D11Convert(Descriptor.Border),
                MinLOD,
                MaxLOD);

            D3D11Check(mDevice->CreateSamplerState(AddressOf(SamplerDescriptor), Sampler.GetAddressOf()));
        }
        return Sampler;
    }
}
