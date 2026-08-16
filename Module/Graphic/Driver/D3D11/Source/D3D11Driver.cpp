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

    Bool D3D11Driver::Initialize(Ptr<void> Output, ConstRef<Configuration> Config)
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

            constexpr UInt Flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

            constexpr D3D_FEATURE_LEVEL Direct3DFeatureLevels[] = {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
            };

            // Tries hardware, which selects the highest level the adapter reaches.
            HRESULT Result = D3D11CreateDevicePtr(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                Flags,
                Direct3DFeatureLevels,
                _countof(Direct3DFeatureLevels),
                D3D11_SDK_VERSION,
                Device.GetAddressOf(),
                nullptr,
                DeviceImmediate.GetAddressOf());

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
                Successful = D3D11Check(Device.As<ID3D11Device1>(AddressOf(mDevice)))
                          && D3D11Check(DeviceImmediate.As<ID3D11DeviceContext1>(AddressOf(mDeviceImmediate)))
                          && D3D11Check(CreateDXGIFactoryPtr(IID_PPV_ARGS(& mDeviceFactory)));

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

    void D3D11Driver::Reset(UInt16 Width, UInt16 Height, Bool Tearless)
    {
        mDeviceProperties.Tearless = Tearless;

        DXGI_SWAP_CHAIN_DESC Description{};
        D3D11Check(mSwapchain->GetDesc(AddressOf(Description)));

        if (Description.BufferDesc.Width != Width || Description.BufferDesc.Height != Height)
        {
            if (mDeviceImmediate)
            {
                mDeviceImmediate->ClearState();
            }

            // Deletes the current display pass to release associated render targets.
            DeletePass(kDisplay);

            // Resizes the swap chain buffers with the new resolution and format.
            const UINT Flags = mDeviceProperties.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
            D3D11Check(mSwapchain->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, Flags));

            // Recreates swap chain resources, including color and depth-stencil attachments.
            Configuration Config;
            Config.Width = Width;
            Config.Height = Height;
            Config.Tearless = mDeviceProperties.Tearless;
            Config.ColorFormat = mDeviceProperties.ColorFormat;
            Config.DepthFormat = mDeviceProperties.DepthFormat;
            CreateSwapchainResources(mPasses[0], Config);
        }
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
        const UInt Size   = (Usage == Usage::Uniform) ? Align(Capacity, mDescription.Capabilities.UniformBlockAlignment) : Capacity;
        const UInt Access = (Storage == Storage::Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
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
        Destruct(mBuffers[ID]);
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
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::UnmapBuffer(Object ID)
    {
        mDeviceImmediate->Unmap(mBuffers[ID].Get(), 0);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth)
    {
        Ref<D3D11Pass> Target = mPasses[ID];

        // Configure and create render target views (RTVs) for each color attachment.
        for (const ColorAttachment Color : Colors)
        {
            ConstRef<D3D11Texture> Source = mTextures[Color.Target];

            Ref<D3D11ColorAttachment> Attachment = Target.Colors.Append();
            Attachment.Target      = Source.Object;
            Attachment.TargetSlice = D3D11CalcSubresource(Color.TargetLevel, Color.TargetLayer, Source.Levels);

            if (Color.Resolve)
            {
                ConstRef<D3D11Texture> Destination = mTextures[Color.Resolve];

                Attachment.Resolve       = Destination.Object;
                Attachment.ResolveSlice  = D3D11CalcSubresource(Color.ResolveLevel, Color.ResolveLayer, Destination.Levels);
                Attachment.ResolveFormat = D3D11TranslateSRV(Destination.Format);
            }
            Attachment.LoadAction    = Color.LoadAction;
            Attachment.StoreAction   = Color.StoreAction;

            const Bool                     IsSliced  = (Source.Layers > 1);
            const D3D11_RTV_DIMENSION      Dimension = Source.Samples > 1
                ? (IsSliced ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DMS)
                : (IsSliced ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY   : D3D11_RTV_DIMENSION_TEXTURE2D);

            const CD3D11_RENDER_TARGET_VIEW_DESC Description(
                Dimension, D3D11TranslateSRV(Source.Format), Color.TargetLevel, Color.TargetLayer, 1);

            D3D11Check(mDevice->CreateRenderTargetView(
                Attachment.Target.Get(), AddressOf(Description), Attachment.TargetResource.GetAddressOf()));
        }

        // Create and assign its depth-stencil view (DSV) if an auxiliary attachment is provided.
        if (Depth.Target > 0)
        {
            Target.DepthStencil.DepthLoadAction    = Depth.DepthLoadAction;
            Target.DepthStencil.DepthStoreAction   = Depth.DepthStoreAction;
            Target.DepthStencil.StencilLoadAction  = Depth.StencilLoadAction;
            Target.DepthStencil.StencilStoreAction = Depth.StencilStoreAction;

            ConstRef<D3D11Texture> Source = mTextures[Depth.Target];

            const Bool                Sliced    = (Source.Layers > 1);
            const D3D11_DSV_DIMENSION Dimension = Source.Samples > 1
                ? (Sliced ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DMS)
                : (Sliced ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY   : D3D11_DSV_DIMENSION_TEXTURE2D);

            const CD3D11_DEPTH_STENCIL_VIEW_DESC Description(
                Dimension, D3D11TranslateDSV(Source.Format), Depth.TargetLevel, Depth.TargetLayer, 1);

            D3D11Check(mDevice->CreateDepthStencilView(
                Source.Object.Get(), AddressOf(Description), Target.DepthStencil.Target.GetAddressOf()));
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

    void D3D11Driver::CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States)
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

            for (ConstRef<Attribute> Attribute : Signature.Attributes)
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

            if (Count > 0)
            {
                D3D11Check(mDevice->CreateInputLayout(Description, Count, VS->GetBufferPointer(), VS->GetBufferSize(), Pipeline.IL.GetAddressOf()));
            }
        }

        Pipeline.PT = D3D11Convert(States.Topology);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeletePipeline(Object ID)
    {
        Destruct(mPipelines[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateSampler(Object ID, Sampler Descriptor)
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

        D3D11Check(mDevice->CreateSamplerState(AddressOf(SamplerDescriptor), mSamplers[ID].GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeleteSampler(Object ID)
    {
        Destruct(mSamplers[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, Multisample Samples, ConstSpan<Byte> Data)
    {
        const Bool   IsCube = (Layout == TextureLayout::TextureCube);
        const UInt16 Slices = IsCube ? 6 : Max<UInt16>(1, Layers);

        CD3D11_TEXTURE2D_DESC Description(D3D11Convert(Format), Width, Height, Slices, Levels);
        Description.Usage      = D3D11Convert(Storage);
        Description.BindFlags  = HasBit(Usage, Usage::Sample) ? D3D11_BIND_SHADER_RESOURCE : 0;
        Description.MiscFlags  = IsCube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
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
        Texture.Levels  = Levels;
        Texture.Layers  = Slices;

        // Fill the data, slice-major so the entry order matches what D3D11CalcSubresource indexes.
        Sequence<D3D11_SUBRESOURCE_DATA> Content;
        Ptr<D3D11_SUBRESOURCE_DATA>      Memory = nullptr;

        if (ConstPtr<Byte> Bytes = Data.GetData(); Bytes)
        {
            Content.Reserve(Slices * Levels);

            for (UInt16 Slice = 0; Slice < Slices; ++Slice)
            {
                for (UInt8 Level = 0; Level < Levels; ++Level)
                {
                    const UInt32 Pitch = GetLevelPitch(Format, Width, Level);
                    const UInt32 Rows  = GetLevelRows(Format, Height, Level);

                    Ref<D3D11_SUBRESOURCE_DATA> Entry = Content.Append();
                    Entry.pSysMem          = Bytes;
                    Entry.SysMemPitch      = Pitch;
                    Entry.SysMemSlicePitch = Pitch * Rows;

                    Bytes += Pitch * Rows;
                }
            }
            Memory = Content.GetData();
        }
        D3D11Check(mDevice->CreateTexture2D(AddressOf(Description), Memory, Texture.Object.GetAddressOf()));

        if (HasBit(Usage, Usage::Sample))
        {
            D3D11_SRV_DIMENSION Dimension;

            if (IsCube)
            {
                Dimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            }
            else if (Layout == TextureLayout::Texture2DArray)
            {
                Dimension = (Samples != Multisample::X1)
                    ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY
                    : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            }
            else
            {
                Dimension = (Samples != Multisample::X1)
                    ? D3D11_SRV_DIMENSION_TEXTURE2DMS
                    : D3D11_SRV_DIMENSION_TEXTURE2D;
            }

            const CD3D11_SHADER_RESOURCE_VIEW_DESC View(Dimension, D3D11TranslateSRV(Format), 0, Levels, 0, Slices);
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

    void D3D11Driver::UpdateTexture(Object ID, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
    {
        constexpr D3D11_COPY_FLAGS Flags = D3D11_COPY_NO_OVERWRITE;

        ConstRef<D3D11Texture> Texture     = mTextures[ID];
        const UINT             Subresource = D3D11CalcSubresource(Level, Layer, Texture.Levels);

        const CD3D11_BOX Offset(X, Y, 0, X + Width, Y + Height, 1);
        mDeviceImmediate->UpdateSubresource1(Texture.Object.Get(), Subresource, AddressOf(Offset), Data.GetData(), Pitch, 0, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcLayer, UInt16 SrcX, UInt16  SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstLayer, UInt16 DstX, UInt16 DstY, UInt16 Width, UInt16 Height)
    {
        constexpr D3D11_COPY_FLAGS Flags = D3D11_COPY_NO_OVERWRITE;

        ConstRef<D3D11Texture> Target = mTextures[DstTexture];
        ConstRef<D3D11Texture> Source = mTextures[SrcTexture];

        const UINT DstSubresource = D3D11CalcSubresource(DstLevel, DstLayer, Target.Levels);
        const UINT SrcSubresource = D3D11CalcSubresource(SrcLevel, SrcLayer, Source.Levels);

        const CD3D11_BOX Offset(SrcX, SrcY, 0, SrcX + Width, SrcY + Height, 1);
        mDeviceImmediate->CopySubresourceRegion1(
            Target.Object.Get(), DstSubresource, DstX, DstY, 0, Source.Object.Get(), SrcSubresource, AddressOf(Offset), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        Ref<D3D11Pass> Target = mPasses[Pass];

        Sequence<Ptr<ID3D11RenderTargetView>, kMaxAttachments> ColorAttachments;
        for (ConstRef<D3D11ColorAttachment> Attachment : Target.Colors)
        {
            ColorAttachments.Append(Attachment.TargetResource.Get());
        }

        // Bind the render targets for the rendering pass.
        const Ptr<ID3D11DepthStencilView> DepthAttachment
            = (Target.DepthStencil.Target ? Target.DepthStencil.Target.Get() : nullptr);
        mDeviceImmediate->OMSetRenderTargets(ColorAttachments.GetSize(), ColorAttachments.GetData(), DepthAttachment);

        // Clear color attachments as specified.
        for (UInt32 Index = 0; Index < ColorAttachments.GetSize(); ++Index)
        {
            ConstRef<D3D11ColorAttachment> Attachment = Target.Colors[Index];

            if (Attachment.LoadAction == Action::Clear)
            {
                mDeviceImmediate->ClearRenderTargetView(
                    ColorAttachments[Index], reinterpret_cast<ConstPtr<FLOAT>>(AddressOf(Colors[Index])));
            }
        }

        // Clear the depth/stencil attachment as specified (only when the pass actually has one).
        if (DepthAttachment)
        {
            UINT Mode = 0;
            if (Target.DepthStencil.DepthLoadAction == Action::Clear)
            {
                Mode |= D3D11_CLEAR_DEPTH;
            }
            if (Target.DepthStencil.StencilLoadAction == Action::Clear)
            {
                Mode |= D3D11_CLEAR_STENCIL;
            }

            if (Mode != 0)
            {
                mDeviceImmediate->ClearDepthStencilView(DepthAttachment, Mode, Depth, Stencil);
            }
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
        // Resolve multisample color attachments into their corresponding single-sample targets.
        for (ConstRef<D3D11ColorAttachment> Attachment : mPasses[Pass].Colors)
        {
            if (Attachment.StoreAction == Action::Store)
            {
                if (Attachment.Resolve)
                {
                    mDeviceImmediate->ResolveSubresource(
                        Attachment.Resolve.Get(),
                        Attachment.ResolveSlice,
                        Attachment.Target.Get(),
                        Attachment.TargetSlice,
                        Attachment.ResolveFormat);
                }
            }
            else
            {
                mDeviceImmediate->DiscardView(Attachment.TargetResource.Get());
            }
        }

        ConstRef<D3D11DepthStencilAttachment> DepthAttachment = mPasses[Pass].DepthStencil;
        if (DepthAttachment.Target
            && (DepthAttachment.StencilStoreAction == Action::Discard || DepthAttachment.DepthStoreAction == Action::Discard))
        {
            mDeviceImmediate->DiscardView(DepthAttachment.Target.Get());
        }

        // Present the swap chain if this is the primary rendering pass.
        if (Pass == kDisplay)
        {
            const UInt Interval = mDeviceProperties.Tearless ? 1 : 0;
            const UInt Flag     = Interval == 0 && mDeviceProperties.Tearing  ? DXGI_PRESENT_ALLOW_TEARING : 0;
            D3D11Check(mSwapchain->Present(Interval, Flag));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::LoadAdapters(DXGI_FORMAT Format)
    {
        ComPtr<IDXGIAdapter1> DXGIAdapter;
        for (UInt Index = 0; SUCCEEDED(mDeviceFactory->EnumAdapters1(Index, DXGIAdapter.ReleaseAndGetAddressOf())); ++Index)
        {
            // Skip software (WARP) adapters so the enumeration reflects only physical devices.
            DXGI_ADAPTER_DESC1 DXGIDescription;
            if (FAILED(DXGIAdapter->GetDesc1(AddressOf(DXGIDescription)))
                || (DXGIDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
            {
                continue;
            }

            Ref<Adapter> AdapterInfo = mDescription.Adapters.Append();
            AdapterInfo.Description   = Str::ConvertFromUTF16(StrConvert(DXGIDescription.Description));
            AdapterInfo.Memory        = DXGIDescription.DedicatedVideoMemory >> 20;

            // Enumerate the supported display modes exposed by the adapter's primary output.
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
                        Ref<Resolution> Resolution = AdapterInfo.Resolutions.Append();
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

        // Determine common supported capabilities.
        mDescription.Capabilities.SupportsBaseVertex    = true;
        mDescription.Capabilities.SupportsFormatS3TC    = true;
        mDescription.Capabilities.UniformBlockAlignment = 256;

        // Determine the supported shader model based on the Direct3D feature level.
        switch (mDevice->GetFeatureLevel())
        {
        case D3D_FEATURE_LEVEL_12_1:
        case D3D_FEATURE_LEVEL_12_0:
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            mDescription.Tier                               = Tier::Level3;
            mDescription.Capabilities.SupportsFormatRGTC    = true;
            mDescription.Capabilities.SupportsFormatBPTC    = true;
            mDescription.Capabilities.MaxTextureDimension   = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            mDescription.Capabilities.MaxTextureLayers      = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            mDescription.Capabilities.MaxTextureMipmaps     = D3D11_REQ_MIP_LEVELS;
            mDescription.Capabilities.MaxTextureSlots       = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxRenderTargets      = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
            mDescription.Capabilities.MaxVertexAttributes   = D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
            mDescription.Capabilities.MaxVertexStreams      = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxAnisotropy         = D3D11_REQ_MAXANISOTROPY;
            mDescription.Capabilities.UniformBlockCapacity  = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            break;
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            mDescription.Tier                               = Tier::Level2;
            mDescription.Capabilities.SupportsFormatRGTC    = true;
            mDescription.Capabilities.MaxTextureDimension   = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            mDescription.Capabilities.MaxTextureLayers      = D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            mDescription.Capabilities.MaxTextureMipmaps     = D3D10_REQ_MIP_LEVELS;
            mDescription.Capabilities.MaxTextureSlots       = D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxRenderTargets      = D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT;
            mDescription.Capabilities.MaxVertexAttributes   = D3D10_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
            mDescription.Capabilities.MaxVertexStreams      = D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
            mDescription.Capabilities.MaxAnisotropy         = D3D10_REQ_MAXANISOTROPY;
            mDescription.Capabilities.UniformBlockCapacity  = D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            break;
        default:
            break;
        }

        // Check if we support tearing mode
        ComPtr<IDXGIFactory5> DXGIFactory5;
        if (SUCCEEDED(mDeviceFactory.As<IDXGIFactory5>(AddressOf(DXGIFactory5))))
        {
            BOOL AllowAdaptive = FALSE;
            D3D11Check(DXGIFactory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING, AddressOf(AllowAdaptive), sizeof(AllowAdaptive)));
            mDeviceProperties.Tearing = AllowAdaptive;
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

    void D3D11Driver::CreateSwapchain(Ref<D3D11Pass> Pass, HWND Window, ConstRef<Configuration> Config)
    {
        const UINT        Flags  = (mDeviceProperties.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
        const DXGI_FORMAT Format = (Config.ColorFormat == TextureFormat::RGBA8UIntNorm_sRGB)
            ? DXGI_FORMAT_R8G8B8A8_UNORM
            : D3D11Convert(Config.ColorFormat);

        DXGI_SWAP_CHAIN_DESC Description { };
        Description.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        Description.BufferDesc.Format = Format;
        Description.BufferDesc.Width  = Config.Width;
        Description.BufferDesc.Height = Config.Height;
        Description.SampleDesc        = { .Count = 1, .Quality = 0 };
        Description.OutputWindow      = Window;
        Description.Windowed          = true;
        Description.Flags             = Flags;

        ComPtr<IDXGIFactory4> DXGIFactory4;
        if (SUCCEEDED(mDeviceFactory.As<IDXGIFactory4>(AddressOf(DXGIFactory4))))
        {
            Description.BufferCount = 3;
            Description.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }
        else
        {
            Description.BufferCount = 1;
            Description.SwapEffect  = DXGI_SWAP_EFFECT_DISCARD;
        }

        D3D11Check(mDeviceFactory->CreateSwapChain(mDevice.Get(), AddressOf(Description), mSwapchain.GetAddressOf()));

        if (ComPtr<IDXGIFactory> DXGIParent; SUCCEEDED(mSwapchain->GetParent(IID_PPV_ARGS(DXGIParent.GetAddressOf()))))
        {
            D3D11Check(DXGIParent->MakeWindowAssociation(Window, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER));
        }

        mDeviceProperties.Tearless    = Config.Tearless;
        mDeviceProperties.ColorFormat = Config.ColorFormat;
        mDeviceProperties.DepthFormat = Config.DepthFormat;
        CreateSwapchainResources(Pass, Config);

        // Matches the two-slot buffer ring; the CPU can never outrun it.
        if (ComPtr<IDXGIDevice1> DXGIDevice1; SUCCEEDED(mDevice.As(AddressOf(DXGIDevice1))))
        {
            D3D11Check(DXGIDevice1->SetMaximumFrameLatency(2));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateSwapchainResources(Ref<D3D11Pass> Pass, ConstRef<Configuration> Config) const
    {
        // Acquire the swapchain back-buffer and create a render target view for it.
        ComPtr<ID3D11Resource> ColorBuffer;
        D3D11Check(mSwapchain->GetBuffer(
            0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(ColorBuffer.GetAddressOf())));

        Ref<D3D11ColorAttachment> Attachment = Pass.Colors.Append();
        Attachment.Target        = ColorBuffer;
        Attachment.TargetSlice   = 0;
        Attachment.ResolveFormat = D3D11Convert(Config.ColorFormat);
        Attachment.ResolveSlice  = 0;
        Attachment.LoadAction    = Action::Clear;
        Attachment.StoreAction   = Action::Store;

        const CD3D11_RENDER_TARGET_VIEW_DESC View(D3D11_RTV_DIMENSION_TEXTURE2D, Attachment.ResolveFormat);
        D3D11Check(mDevice->CreateRenderTargetView(
            Attachment.Target.Get(), AddressOf(View), Attachment.TargetResource.GetAddressOf()));

        // Create a depth-stencil buffer matching the swapchain dimensions and sample count.
        if (Config.DepthFormat != TextureFormat::Unspecified)
        {
            D3D11_TEXTURE2D_DESC DepthStencilTextureMetadata { };
            DepthStencilTextureMetadata.Width      = Config.Width;
            DepthStencilTextureMetadata.Height     = Config.Height;
            DepthStencilTextureMetadata.MipLevels  = 1;
            DepthStencilTextureMetadata.ArraySize  = 1;
            DepthStencilTextureMetadata.Format     = D3D11Convert(Config.DepthFormat);
            DepthStencilTextureMetadata.SampleDesc = { .Count = 1, .Quality = 0 };
            DepthStencilTextureMetadata.Usage      = D3D11_USAGE_DEFAULT;
            DepthStencilTextureMetadata.BindFlags  = D3D11_BIND_DEPTH_STENCIL;

            Pass.DepthStencil.DepthLoadAction    = Action::Clear;
            Pass.DepthStencil.DepthStoreAction   = Action::Discard;
            Pass.DepthStencil.StencilLoadAction  = Action::Clear;
            Pass.DepthStencil.StencilStoreAction = Action::Discard;

            ComPtr<ID3D11Texture2D> Depth;
            D3D11Check(mDevice->CreateTexture2D(AddressOf(DepthStencilTextureMetadata), nullptr, Depth.GetAddressOf()));

            CD3D11_DEPTH_STENCIL_VIEW_DESC DepthViewDescription(D3D11_DSV_DIMENSION_TEXTURE2D, D3D11TranslateDSV(Config.DepthFormat));
            D3D11Check(mDevice->CreateDepthStencilView(Depth.Get(), AddressOf(DepthViewDescription), Pass.DepthStencil.Target.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyVertexResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        const UInt32 OldSize = Oldest.Vertices.GetSize();
        const UInt32 NewSize = Newest.Vertices.GetSize();
        const UInt32 Limit   = ::Max(OldSize, NewSize);

        // The first pass only compares, so a draw that rebinds nothing touches no pool and writes no array.
        UInt32 Min = Limit;
        UInt32 Max = 0;

        for (UInt32 Element = 0; Element < Limit; ++Element)
        {
            const Stream Old = Element < OldSize ? Oldest.Vertices[Element] : Stream { };
            const Stream New = Element < NewSize ? Newest.Vertices[Element] : Stream { };

            if (Old.Buffer != New.Buffer || Old.Offset != New.Offset || Old.Stride != New.Stride)
            {
                Min = ::Min(Element, Min);
                Max = ::Max(Element + 1, Max);
            }
        }

        if (Min >= Max)
        {
            return;
        }

        // The second pass gathers the dirty span alone, which is also the span handed to the device.
        Ptr<ID3D11Buffer> Array[Command::kMaxVertices];
        UINT              ArrayOffset[Command::kMaxVertices];
        UINT              ArrayStride[Command::kMaxVertices];

        for (UInt32 Element = Min; Element < Max; ++Element)
        {
            const Stream New = Element < NewSize ? Newest.Vertices[Element] : Stream { };

            Array[Element]       = mBuffers[New.Buffer].Get();
            ArrayOffset[Element] = New.Offset;
            ArrayStride[Element] = New.Stride;
        }

        mDeviceImmediate->IASetVertexBuffers(Min, Max - Min, Array + Min, ArrayStride + Min, ArrayOffset + Min);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplySamplerResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        const UInt32 OldSize = Oldest.Samplers.GetSize();
        const UInt32 NewSize = Newest.Samplers.GetSize();
        const UInt32 Limit   = ::Max(OldSize, NewSize);

        UInt32 Min = Limit;
        UInt32 Max = 0;

        for (UInt32 Element = 0; Element < Limit; ++Element)
        {
            const Object Old = Element < OldSize ? Oldest.Samplers[Element] : Object { };
            const Object New = Element < NewSize ? Newest.Samplers[Element] : Object { };

            if (Old != New)
            {
                Min = ::Min(Element, Min);
                Max = ::Max(Element + 1, Max);
            }
        }

        if (Min >= Max)
        {
            return;
        }

        Ptr<ID3D11SamplerState> Array[Command::kMaxSamplers];

        for (UInt32 Element = Min; Element < Max; ++Element)
        {
            const Object New = Element < NewSize ? Newest.Samplers[Element] : Object { };

            Array[Element] = mSamplers[New].Get();
        }

        const UInt32 Count = Max - Min;
        mDeviceImmediate->VSSetSamplers(Min, Count, Array + Min);
        mDeviceImmediate->PSSetSamplers(Min, Count, Array + Min);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyTextureResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        const UInt32 OldSize = Oldest.Textures.GetSize();
        const UInt32 NewSize = Newest.Textures.GetSize();
        const UInt32 Limit   = ::Max(OldSize, NewSize);

        UInt32 Min = Limit;
        UInt32 Max = 0;

        for (UInt32 Element = 0; Element < Limit; ++Element)
        {
            const Object Old = Element < OldSize ? Oldest.Textures[Element] : Object { };
            const Object New = Element < NewSize ? Newest.Textures[Element] : Object { };

            if (Old != New)
            {
                Min = ::Min(Element, Min);
                Max = ::Max(Element + 1, Max);
            }
        }

        if (Min >= Max)
        {
            return;
        }

        Ptr<ID3D11ShaderResourceView> Array[Command::kMaxTextures];

        for (UInt32 Element = Min; Element < Max; ++Element)
        {
            const Object New = Element < NewSize ? Newest.Textures[Element] : Object { };

            Array[Element] = mTextures[New].Resource.Get();
        }

        const UInt32 Count = Max - Min;
        mDeviceImmediate->VSSetShaderResources(Min, Count, Array + Min);
        mDeviceImmediate->PSSetShaderResources(Min, Count, Array + Min);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyUniformResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        UInt32 Min = Command::kMaxUniforms;
        UInt32 Max = 0;

        for (UInt32 Element = 0; Element < Command::kMaxUniforms; ++Element)
        {
            ConstRef<Stream> Old = Oldest.Uniforms[Element];
            ConstRef<Stream> New = Newest.Uniforms[Element];

            if (Old.Buffer != New.Buffer || Old.Offset != New.Offset || Old.Stride != New.Stride)
            {
                Min = ::Min(Element, Min);
                Max = ::Max(Element + 1, Max);
            }
        }

        if (Min >= Max)
        {
            return;
        }

        Ptr<ID3D11Buffer> Array[Command::kMaxUniforms];
        UINT              ArrayOffset[Command::kMaxUniforms];
        UINT              ArrayLength[Command::kMaxUniforms];

        for (UInt32 Element = Min; Element < Max; ++Element)
        {
            ConstRef<Stream> New = Newest.Uniforms[Element];

            Array[Element]       = mBuffers[New.Buffer].Get();
            ArrayOffset[Element] = New.Offset / sizeof(Vector4);
            ArrayLength[Element] = New.Stride / sizeof(Vector4);
        }

        const UInt32 Count = Max - Min;
        mDeviceImmediate->VSSetConstantBuffers1(Min, Count, Array + Min, ArrayOffset + Min, ArrayLength + Min);
        mDeviceImmediate->PSSetConstantBuffers1(Min, Count, Array + Min, ArrayOffset + Min, ArrayLength + Min);
    }
}