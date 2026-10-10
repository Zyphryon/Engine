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

namespace ZyGraphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    static Bool Differ(ConstSpan<Type> Oldest, ConstSpan<Type> Newest, Ref<UInt32> Min, Ref<UInt32> Max)
    {
        const UInt32 Limit = static_cast<UInt32>(::Max(Oldest.GetSize(), Newest.GetSize()));

        Min = Limit;
        Max = 0;

        for (UInt32 Element = 0; Element < Limit; ++Element)
        {
            const Type Old = Element < Oldest.GetSize() ? Oldest[Element] : Type { };
            const Type New = Element < Newest.GetSize() ? Newest[Element] : Type { };

            if (Old != New)
            {
                Min = ::Min(Element, Min);
                Max = ::Max(Element + 1, Max);
            }
        }
        return Min < Max;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool D3D11Check(HRESULT Result)
    {
        if (FAILED(Result))
        {
            constexpr UINT Flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

            TCHAR Message[1024];
            ::FormatMessage(Flags, nullptr, Result, 0, Message, sizeof(Message), nullptr);

            LOG_E("Graphics: {0}", Str::ConvertFromUTF16(StrConvert(Message)));
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
            return DXGI_FORMAT_D16_UNORM;
        case TextureFormat::D32Float:
            return DXGI_FORMAT_D32_FLOAT;
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
            D3D11CreateDevicePtr = reinterpret_cast<decltype(& D3D11CreateDevice)>(::GetProcAddress(
                Dll,
                "D3D11CreateDevice"));
        }
        if (const auto Dll = ::LoadLibraryExW(L"DXGI.DLL", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); Dll != nullptr)
        {
            CreateDXGIFactoryPtr = reinterpret_cast<decltype(& CreateDXGIFactory1)>(::GetProcAddress(
                Dll,
                "CreateDXGIFactory1"));
        }

        if (!D3D11CreateDevicePtr || !CreateDXGIFactoryPtr)
        {
            LOG_E("Graphics: Failed to load D3D11.DLL or DXGI.DLL");
            return false;
        }

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
                LOG_I("Graphics: Enabling software mode (WARP)");
            }
        }

        if (!D3D11Check(Result))
        {
            return false;
        }

        if (!D3D11Check(Device.As<ID3D11Device1>(AddressOf(mDevice)))
            || !D3D11Check(DeviceImmediate.As<ID3D11DeviceContext1>(AddressOf(mDeviceImmediate)))
            || !D3D11Check(CreateDXGIFactoryPtr(IID_PPV_ARGS(& mDeviceFactory))))
        {
            return false;
        }

        LoadAdapters(D3D11Convert(Config.ColorFormat));
        LoadCapabilities();

        mProfiler.Initialize(mDevice.Get(), mDeviceImmediate.Get());

        if (Output)
        {
            CreateSwapchain(mPasses[kDisplay], static_cast<HWND>(Output), Config);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Reset(UInt16 Width, UInt16 Height, Bool Tearless)
    {
        Ref<Configuration> Config = mDeviceProperties.Config;
        Config.Tearless = Tearless;

        DXGI_SWAP_CHAIN_DESC Description { };
        D3D11Check(mSwapchain->GetDesc(AddressOf(Description)));

        if (Description.BufferDesc.Width != Width || Description.BufferDesc.Height != Height)
        {
            mDeviceImmediate->ClearState();

            // ResizeBuffers fails while any view of the back buffer is alive.
            DeletePass(kDisplay);

            // A zero count and an unknown format keep the buffers' own.
            const UINT Flags = mDeviceProperties.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
            D3D11Check(mSwapchain->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, Flags));

            Config.Width  = Width;
            Config.Height = Height;
            CreateSwapchainResources(mPasses[kDisplay], Config);
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
        const CD3D11_BUFFER_DESC Descriptor(Size, D3D11Convert(Usage), D3D11Convert(Storage), Access);

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
        mDeviceImmediate->UpdateSubresource1(
            mBuffers[ID].Get(),
            0,
            AddressOf(Destination),
            Data.GetData(),
            0,
            0,
            Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeleteBuffer(Object ID)
    {
        mBuffers[ID] = nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> D3D11Driver::MapBuffer(Object ID, UInt32 Offset, UInt32 Size)
    {
        constexpr D3D11_MAP      Mapping = D3D11_MAP_WRITE_DISCARD;
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

        for (const ColorAttachment Color : Colors)
        {
            ConstRef<D3D11Texture> Source = mTextures[Color.Target];

            Ref<D3D11ColorAttachment> Attachment = Target.Colors.Append();
            Attachment.LoadAction  = Color.LoadAction;
            Attachment.StoreAction = Color.StoreAction;

            const D3D11_RTV_DIMENSION Dimension = (Source.Layers > 1)
                ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY
                : D3D11_RTV_DIMENSION_TEXTURE2D;

            const CD3D11_RENDER_TARGET_VIEW_DESC Description(
                Dimension,
                D3D11TranslateSRV(Source.Format),
                Color.TargetLevel,
                Color.TargetLayer,
                1);

            D3D11Check(mDevice->CreateRenderTargetView(
                Source.Object.Get(),
                AddressOf(Description),
                Attachment.Target.GetAddressOf()));
        }

        if (Depth.Target)
        {
            Target.Depth = Depth;

            ConstRef<D3D11Texture> Source = mTextures[Depth.Target];

            const D3D11_DSV_DIMENSION Dimension = (Source.Layers > 1)
                ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY
                : D3D11_DSV_DIMENSION_TEXTURE2D;

            const CD3D11_DEPTH_STENCIL_VIEW_DESC Description(
                Dimension,
                D3D11TranslateDSV(Source.Format),
                Depth.TargetLevel,
                Depth.TargetLayer,
                1);

            D3D11Check(mDevice->CreateDepthStencilView(
                Source.Object.Get(),
                AddressOf(Description),
                Target.DepthStencil.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeletePass(Object ID)
    {
        mPasses[ID] = D3D11Pass();
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

        ComPtr<ID3DBlob> VS;
        VS.Attach(mCompiler.Compile(Program.Modules[0], mDescription.Tier, ShaderStage::Vertex, Macros.GetData()));
        D3D11Check(mDevice->CreateVertexShader(
            VS->GetBufferPointer(),
            VS->GetBufferSize(),
            nullptr,
            Pipeline.VS.GetAddressOf()));

        Macros[Macros.GetSize() - 2].Name = "FRAGMENT_SHADER";

        ComPtr<ID3DBlob> PS;
        PS.Attach(mCompiler.Compile(Program.Modules[1], mDescription.Tier, ShaderStage::Fragment, Macros.GetData()));
        D3D11Check(mDevice->CreatePixelShader(
            PS->GetBufferPointer(),
            PS->GetBufferSize(),
            nullptr,
            Pipeline.PS.GetAddressOf()));

        {
            D3D11_BLEND_DESC Description = CD3D11_BLEND_DESC(CD3D11_DEFAULT());

            Description.RenderTarget[0].BlendEnable           = States.UsesBlending();
            Description.RenderTarget[0].SrcBlend              = D3D11Convert(States.BlendSrcColor);
            Description.RenderTarget[0].DestBlend             = D3D11Convert(States.BlendDstColor);
            Description.RenderTarget[0].BlendOp               = D3D11Convert(States.BlendEquationColor);
            Description.RenderTarget[0].SrcBlendAlpha         = D3D11Convert(States.BlendSrcAlpha);
            Description.RenderTarget[0].DestBlendAlpha        = D3D11Convert(States.BlendDstAlpha);
            Description.RenderTarget[0].BlendOpAlpha          = D3D11Convert(States.BlendEquationAlpha);
            Description.RenderTarget[0].RenderTargetWriteMask = static_cast<D3D11_COLOR_WRITE_ENABLE>(States.Channel);

            D3D11Check(mDevice->CreateBlendState(AddressOf(Description), Pipeline.BS.GetAddressOf()));
        }

        {
            D3D11_DEPTH_STENCIL_DESC Description = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());

            Description.DepthEnable                  = States.UsesDepth();
            Description.DepthFunc                    = D3D11Convert(States.DepthTest);
            Description.DepthWriteMask               = static_cast<D3D11_DEPTH_WRITE_MASK>(States.DepthMask);
            Description.StencilEnable                = States.UsesStencil();
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

            D3D11Check(mDevice->CreateDepthStencilState(AddressOf(Description), Pipeline.DS.GetAddressOf()));
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

            D3D11Check(mDevice->CreateRasterizerState(AddressOf(Description), Pipeline.RS.GetAddressOf()));
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
                D3D11Check(mDevice->CreateInputLayout(
                    Description,
                    Count,
                    VS->GetBufferPointer(),
                    VS->GetBufferSize(),
                    Pipeline.IL.GetAddressOf()));
            }
        }

        Pipeline.PT = D3D11Convert(States.Topology);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeletePipeline(Object ID)
    {
        mPipelines[ID] = D3D11Pipeline();
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

        D3D11_FILTER Filter = D3D11Convert(Descriptor.Filter);

        if (Descriptor.Comparison != TestCondition::None)
        {
            Filter = static_cast<D3D11_FILTER>(Filter | D3D11_COMPARISON_FILTERING_BIT);
        }

        const CD3D11_SAMPLER_DESC SamplerDescriptor(
            Filter,
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
        mSamplers[ID] = nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data)
    {
        const Bool   IsCube = (Layout == TextureLayout::TextureCube);
        const UInt16 Slices = IsCube ? 6 : Max<UInt16>(1, Layers);

        CD3D11_TEXTURE2D_DESC Description(D3D11Convert(Format), Width, Height, Slices, Levels);
        Description.Usage     = D3D11Convert(Storage);
        Description.BindFlags = HasBit(Usage, Usage::Sample) ? D3D11_BIND_SHADER_RESOURCE : 0;
        Description.MiscFlags = IsCube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

        if (HasBit(Usage, Usage::Target))
        {
            Description.BindFlags |= GetTextureMetadata(Format).IsDepth
                ? D3D11_BIND_DEPTH_STENCIL
                : D3D11_BIND_RENDER_TARGET;
        }

        Ref<D3D11Texture> Texture = mTextures[ID];
        Texture.Format = Format;
        Texture.Levels = Levels;
        Texture.Layers = Slices;

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
                Dimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            }
            else
            {
                Dimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            }

            const CD3D11_SHADER_RESOURCE_VIEW_DESC View(Dimension, D3D11TranslateSRV(Format), 0, Levels, 0, Slices);
            D3D11Check(mDevice->CreateShaderResourceView(
                Texture.Object.Get(),
                AddressOf(View),
                Texture.Resource.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::UpdateTexture(Object ID, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
    {
        constexpr D3D11_COPY_FLAGS Flags = D3D11_COPY_NO_OVERWRITE;

        ConstRef<D3D11Texture> Texture     = mTextures[ID];
        const UINT             Subresource = D3D11CalcSubresource(Level, Layer, Texture.Levels);

        const CD3D11_BOX Offset(X, Y, 0, X + Width, Y + Height, 1);
        mDeviceImmediate->UpdateSubresource1(
            Texture.Object.Get(),
            Subresource,
            AddressOf(Offset),
            Data.GetData(),
            Pitch,
            0,
            Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeleteTexture(Object ID)
    {
        mTextures[ID] = D3D11Texture();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Prepare(Object Pass, Text Name, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        Ref<D3D11Pass> Target = mPasses[Pass];

        mProfiler.Open(Name);

        Sequence<Ptr<ID3D11RenderTargetView>, kMaxAttachments> ColorAttachments;
        for (ConstRef<D3D11ColorAttachment> Attachment : Target.Colors)
        {
            ColorAttachments.Append(Attachment.Target.Get());
        }

        const Ptr<ID3D11DepthStencilView> DepthAttachment = Target.DepthStencil.Get();
        mDeviceImmediate->OMSetRenderTargets(ColorAttachments.GetSize(), ColorAttachments.GetData(), DepthAttachment);

        // Clear color attachments as specified, only as far as the clears handed over actually reach.
        for (UInt32 Index = 0; Index < ColorAttachments.GetSize(); ++Index)
        {
            ConstRef<D3D11ColorAttachment> Attachment = Target.Colors[Index];

            if (Attachment.LoadAction == Action::Clear && Index < Colors.GetSize())
            {
                mDeviceImmediate->ClearRenderTargetView(
                    ColorAttachments[Index],
                    reinterpret_cast<ConstPtr<FLOAT>>(AddressOf(Colors[Index])));
            }
        }

        // Clear the depth/stencil attachment as specified (only when the pass actually has one).
        if (DepthAttachment)
        {
            UINT Mode = 0;
            if (Target.Depth.DepthLoadAction == Action::Clear)
            {
                Mode |= D3D11_CLEAR_DEPTH;
            }
            if (Target.Depth.StencilLoadAction == Action::Clear)
            {
                Mode |= D3D11_CLEAR_STENCIL;
            }

            if (Mode != 0)
            {
                mDeviceImmediate->ClearDepthStencilView(DepthAttachment, Mode, Depth, Stencil);
            }
        }

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
            ConstRef<Command> Newest = Commands[Batch];
            ConstRef<Command> Oldest = Batch > 0 ? Commands[Batch - 1] : kEmptyCommand;

            ApplyVertexResources(Oldest, Newest);

            if (Oldest.Indices.Buffer != Newest.Indices.Buffer ||
                Oldest.Indices.Offset != Newest.Indices.Offset ||
                Oldest.Indices.Stride != Newest.Indices.Stride)
            {
                ConstRef<D3D11Buffer> Buffer = mBuffers[Newest.Indices.Buffer];
                const DXGI_FORMAT     Format =
                    Newest.Indices.Stride == 1 ? DXGI_FORMAT_R8_UINT  :
                    Newest.Indices.Stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
                mDeviceImmediate->IASetIndexBuffer(Buffer.Get(), Format, Newest.Indices.Offset);
            }

            const Bool ScissorDirty = (Oldest.Scissor.X      != Newest.Scissor.X)
                                   || (Oldest.Scissor.Y      != Newest.Scissor.Y)
                                   || (Oldest.Scissor.Width  != Newest.Scissor.Width)
                                   || (Oldest.Scissor.Height != Newest.Scissor.Height);

            if (ScissorDirty)
            {
                const RECT Scissor = CD3D11_RECT(
                    Newest.Scissor.X,
                    Newest.Scissor.Y,
                    Newest.Scissor.X + Newest.Scissor.Width,
                    Newest.Scissor.Y + Newest.Scissor.Height);
                mDeviceImmediate->RSSetScissorRects(1, AddressOf(Scissor));
            }

            if (Oldest.Pipeline != Newest.Pipeline)
            {
                ConstRef<D3D11Pipeline> Old = mPipelines[Oldest.Pipeline];
                ConstRef<D3D11Pipeline> New = mPipelines[Newest.Pipeline];

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
                if (Old.DS != New.DS || Oldest.Stencil != Newest.Stencil)
                {
                    mDeviceImmediate->OMSetDepthStencilState(New.DS.Get(), Newest.Stencil);
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
            else if (Oldest.Stencil != Newest.Stencil)
            {
                mDeviceImmediate->OMSetDepthStencilState(
                    mPipelines[Newest.Pipeline].DS.Get(),
                    Newest.Stencil);
            }

            ApplySamplerResources(Oldest, Newest);
            ApplyTextureResources(Oldest, Newest);
            ApplyUniformResources(Oldest, Newest);

            const UInt32 Count     = Newest.Parameters.Count;
            const SInt32 Base      = Newest.Parameters.Base;
            const UInt32 Offset    = Newest.Parameters.Offset;
            const UInt32 Instances = Newest.Parameters.Instances;

            if (Newest.Indices.Buffer)
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
        ConstRef<D3D11Pass> Target = mPasses[Pass];

        // What the pass does not keep is discarded, so the driver need not preserve it past the pass.
        for (ConstRef<D3D11ColorAttachment> Attachment : Target.Colors)
        {
            if (Attachment.StoreAction != Action::Store)
            {
                mDeviceImmediate->DiscardView(Attachment.Target.Get());
            }
        }

        if (Target.DepthStencil
            && (Target.Depth.StencilStoreAction == Action::Discard || Target.Depth.DepthStoreAction == Action::Discard))
        {
            mDeviceImmediate->DiscardView(Target.DepthStencil.Get());
        }

        // Closed once the pass has drawn, so what it timed is the pass and not the wait for the display.
        mProfiler.Close();

        // Present the swap chain if this is the primary rendering pass.
        if (Pass == kDisplay)
        {
            ZY_PROFILE_SCOPE("Driver::Present");

            const UInt Interval = mDeviceProperties.Config.Tearless ? 1 : 0;
            const UInt Flag     = Interval == 0 && mDeviceProperties.Tearing ? DXGI_PRESENT_ALLOW_TEARING : 0;
            D3D11Check(mSwapchain->Present(Interval, Flag));

            // The frame is over, so whatever the queries have answered is gathered before the next one opens.
            mProfiler.Collect();
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

                    for (ConstRef<DXGI_MODE_DESC> Description : Descriptions)
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

        // Tearing presentation needs DXGI 1.5.
        ComPtr<IDXGIFactory5> DXGIFactory5;
        if (SUCCEEDED(mDeviceFactory.As<IDXGIFactory5>(AddressOf(DXGIFactory5))))
        {
            BOOL AllowAdaptive = FALSE;
            D3D11Check(DXGIFactory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                AddressOf(AllowAdaptive),
                sizeof(AllowAdaptive)));
            mDeviceProperties.Tearing = AllowAdaptive;
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

        mDeviceProperties.Config = Config;
        CreateSwapchainResources(Pass, Config);

        // The CPU never queues more frames than the ring it fills.
        if (ComPtr<IDXGIDevice1> DXGIDevice1; SUCCEEDED(mDevice.As(AddressOf(DXGIDevice1))))
        {
            D3D11Check(DXGIDevice1->SetMaximumFrameLatency(kMaxFrames));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateSwapchainResources(Ref<D3D11Pass> Pass, ConstRef<Configuration> Config) const
    {
        // Acquire the swapchain back-buffer and create a render target view for it.
        ComPtr<ID3D11Texture2D> ColorBuffer;
        D3D11Check(mSwapchain->GetBuffer(0, IID_PPV_ARGS(ColorBuffer.GetAddressOf())));

        Ref<D3D11ColorAttachment> Attachment = Pass.Colors.Append();
        Attachment.LoadAction  = Action::Clear;
        Attachment.StoreAction = Action::Store;

        const CD3D11_RENDER_TARGET_VIEW_DESC View(D3D11_RTV_DIMENSION_TEXTURE2D, D3D11Convert(Config.ColorFormat));
        D3D11Check(mDevice->CreateRenderTargetView(
            ColorBuffer.Get(),
            AddressOf(View),
            Attachment.Target.GetAddressOf()));

        // Create a depth-stencil buffer matching the swapchain dimensions.
        if (Config.DepthFormat != TextureFormat::Unspecified)
        {
            const CD3D11_TEXTURE2D_DESC Description(
                D3D11Convert(Config.DepthFormat),
                Config.Width,
                Config.Height,
                1,
                1,
                D3D11_BIND_DEPTH_STENCIL);

            Pass.Depth.DepthLoadAction    = Action::Clear;
            Pass.Depth.DepthStoreAction   = Action::Discard;
            Pass.Depth.StencilLoadAction  = Action::Clear;
            Pass.Depth.StencilStoreAction = Action::Discard;

            ComPtr<ID3D11Texture2D> Depth;
            D3D11Check(mDevice->CreateTexture2D(AddressOf(Description), nullptr, Depth.GetAddressOf()));

            CD3D11_DEPTH_STENCIL_VIEW_DESC DepthViewDescription(
                D3D11_DSV_DIMENSION_TEXTURE2D,
                D3D11TranslateDSV(Config.DepthFormat));
            D3D11Check(mDevice->CreateDepthStencilView(
                Depth.Get(),
                AddressOf(DepthViewDescription),
                Pass.DepthStencil.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyVertexResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        UInt32 Min;
        UInt32 Max;

        if (!Differ<Stream>(Oldest.Vertices, Newest.Vertices, Min, Max))
        {
            return;
        }

        Ptr<ID3D11Buffer> Buffers[Command::kMaxVertices];
        UINT              Offsets[Command::kMaxVertices];
        UINT              Strides[Command::kMaxVertices];

        for (UInt32 Element = Min; Element < Max; ++Element)
        {
            const Stream New = Element < Newest.Vertices.GetSize() ? Newest.Vertices[Element] : Stream { };

            Buffers[Element] = mBuffers[New.Buffer].Get();
            Offsets[Element] = New.Offset;
            Strides[Element] = New.Stride;
        }

        mDeviceImmediate->IASetVertexBuffers(Min, Max - Min, Buffers + Min, Strides + Min, Offsets + Min);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplySamplerResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        UInt32 Min;
        UInt32 Max;

        if (!Differ<Object>(Oldest.Samplers, Newest.Samplers, Min, Max))
        {
            return;
        }

        Ptr<ID3D11SamplerState> Samplers[Command::kMaxSamplers];

        for (UInt32 Element = Min; Element < Max; ++Element)
        {
            const Object New = Element < Newest.Samplers.GetSize() ? Newest.Samplers[Element] : Object { };

            Samplers[Element] = mSamplers[New].Get();
        }

        const UInt32 Count = Max - Min;
        mDeviceImmediate->VSSetSamplers(Min, Count, Samplers + Min);
        mDeviceImmediate->PSSetSamplers(Min, Count, Samplers + Min);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyTextureResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        UInt32 Min;
        UInt32 Max;

        if (!Differ<Object>(Oldest.Textures, Newest.Textures, Min, Max))
        {
            return;
        }

        Ptr<ID3D11ShaderResourceView> Views[Command::kMaxTextures];

        for (UInt32 Element = Min; Element < Max; ++Element)
        {
            const Object New = Element < Newest.Textures.GetSize() ? Newest.Textures[Element] : Object { };

            Views[Element] = mTextures[New].Resource.Get();
        }

        const UInt32 Count = Max - Min;
        mDeviceImmediate->VSSetShaderResources(Min, Count, Views + Min);
        mDeviceImmediate->PSSetShaderResources(Min, Count, Views + Min);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyUniformResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        UInt32 Min;
        UInt32 Max;

        if (!Differ<Stream>(Oldest.Uniforms, Newest.Uniforms, Min, Max))
        {
            return;
        }

        Ptr<ID3D11Buffer> Buffers[Command::kMaxUniforms];
        UINT              Offsets[Command::kMaxUniforms];
        UINT              Lengths[Command::kMaxUniforms];

        for (UInt32 Element = Min; Element < Max; ++Element)
        {
            ConstRef<Stream> New = Newest.Uniforms[Element];

            Buffers[Element] = mBuffers[New.Buffer].Get();
            Offsets[Element] = New.Offset / sizeof(Vector4);
            Lengths[Element] = New.Stride / sizeof(Vector4);
        }

        const UInt32 Count = Max - Min;
        mDeviceImmediate->VSSetConstantBuffers1(Min, Count, Buffers + Min, Offsets + Min, Lengths + Min);
        mDeviceImmediate->PSSetConstantBuffers1(Min, Count, Buffers + Min, Offsets + Min, Lengths + Min);
    }
}