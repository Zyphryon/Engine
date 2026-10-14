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

#include "D3D11Pipeline.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool CreateVertex(Ptr<ID3D11Device1> Device, Ptr<ID3DBlob> Bytecode, Ref<ComPtr<ID3D11VertexShader>> Output)
    {
        const ConstPtr<void> Data = Bytecode->GetBufferPointer();
        const SIZE_T         Size = Bytecode->GetBufferSize();

        return Check(Device->CreateVertexShader(Data, Size, nullptr, Output.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool CreatePixel(Ptr<ID3D11Device1> Device, Ptr<ID3DBlob> Bytecode, Ref<ComPtr<ID3D11PixelShader>> Output)
    {
        const ConstPtr<void> Data = Bytecode->GetBufferPointer();
        const SIZE_T         Size = Bytecode->GetBufferSize();

        return Check(Device->CreatePixelShader(Data, Size, nullptr, Output.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool CreateBlend(Ptr<ID3D11Device1> Device, ConstRef<States> States, Ref<ComPtr<ID3D11BlendState>> Output)
    {
        D3D11_BLEND_DESC Description = CD3D11_BLEND_DESC(CD3D11_DEFAULT());

        Description.RenderTarget[0].BlendEnable           = States.UsesBlending();
        Description.RenderTarget[0].SrcBlend              = Convert(States.BlendSrcColor);
        Description.RenderTarget[0].DestBlend             = Convert(States.BlendDstColor);
        Description.RenderTarget[0].BlendOp               = Convert(States.BlendEquationColor);
        Description.RenderTarget[0].SrcBlendAlpha         = Convert(States.BlendSrcAlpha);
        Description.RenderTarget[0].DestBlendAlpha        = Convert(States.BlendDstAlpha);
        Description.RenderTarget[0].BlendOpAlpha          = Convert(States.BlendEquationAlpha);
        Description.RenderTarget[0].RenderTargetWriteMask = static_cast<D3D11_COLOR_WRITE_ENABLE>(States.Channel);

        return Check(Device->CreateBlendState(AddressOf(Description), Output.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool CreateDepth(Ptr<ID3D11Device1> Device, ConstRef<States> States, Ref<ComPtr<ID3D11DepthStencilState>> Output)
    {
        D3D11_DEPTH_STENCIL_DESC Description = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());

        Description.DepthEnable                  = States.UsesDepth();
        Description.DepthFunc                    = Convert(States.DepthTest);
        Description.DepthWriteMask               = static_cast<D3D11_DEPTH_WRITE_MASK>(States.DepthMask);
        Description.StencilEnable                = States.UsesStencil();
        Description.StencilReadMask              = States.StencilReadMask;
        Description.StencilWriteMask             = States.StencilWriteMask;
        Description.FrontFace.StencilFunc        = Convert(States.StencilFrontTest);
        Description.FrontFace.StencilFailOp      = Convert(States.StencilFrontFail);
        Description.FrontFace.StencilDepthFailOp = Convert(States.StencilFrontDepthFail);
        Description.FrontFace.StencilPassOp      = Convert(States.StencilFrontDepthPass);
        Description.BackFace.StencilFunc         = Convert(States.StencilBackTest);
        Description.BackFace.StencilFailOp       = Convert(States.StencilBackFail);
        Description.BackFace.StencilDepthFailOp  = Convert(States.StencilBackDepthFail);
        Description.BackFace.StencilPassOp       = Convert(States.StencilBackDepthPass);

        return Check(Device->CreateDepthStencilState(AddressOf(Description), Output.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool CreateRaster(Ptr<ID3D11Device1> Device, ConstRef<States> States, Ref<ComPtr<ID3D11RasterizerState>> Output)
    {
        D3D11_RASTERIZER_DESC Description = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());

        Description.FrontCounterClockwise = FALSE;
        Description.DepthBias             = States.DepthBias;
        Description.DepthBiasClamp        = States.DepthBiasClamp;
        Description.SlopeScaledDepthBias  = States.DepthBiasSlope;
        Description.DepthClipEnable       = States.DepthClip;
        Description.CullMode              = Convert(States.Cull);
        Description.ScissorEnable         = States.Scissor;
        Description.MultisampleEnable     = FALSE;
        Description.AntialiasedLineEnable = FALSE;
        Description.FillMode              = Convert(States.Fill);

        return Check(Device->CreateRasterizerState(AddressOf(Description), Output.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool CreateLayout(Ptr<ID3D11Device1> Device, ConstRef<Signature> Signature, Ptr<ID3DBlob> Bytecode, Ref<ComPtr<ID3D11InputLayout>> Output)
    {
        D3D11_INPUT_ELEMENT_DESC Description[kMaxAttributes];
        UInt                     Count = 0;

        for (ConstRef<Attribute> Attribute : Signature.Attributes)
        {
            Ref<D3D11_INPUT_ELEMENT_DESC> Element = Description[Count++];
            Element.SemanticName         = "SLOT";
            Element.SemanticIndex        = Attribute.Location;
            Element.Format               = Convert(Attribute.Format);
            Element.AlignedByteOffset    = Attribute.Offset;
            Element.InputSlot            = Attribute.Stream;
            Element.InputSlotClass       = Attribute.Divisor
                ? D3D11_INPUT_PER_INSTANCE_DATA
                : D3D11_INPUT_PER_VERTEX_DATA;
            Element.InstanceDataStepRate = Attribute.Divisor;
        }

        // A pipeline that reads no attributes binds no layout, as its vertices come from the vertex index alone.
        if (Count == 0)
        {
            return true;
        }

        return Check(Device->CreateInputLayout(
            Description,
            Count,
            Bytecode->GetBufferPointer(),
            Bytecode->GetBufferSize(),
            Output.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool CreateUsage(ConstRef<Compiler> Compiler, Ptr<ID3DBlob> Bytecode, Ref<Pipeline::Usage> Output)
    {
        const ComPtr<ID3D11ShaderReflection> Reflection = Compiler.Reflect(Bytecode);
        D3D11_SHADER_DESC                    Description;

        if (!Reflection || !Check(Reflection->GetDesc(AddressOf(Description))))
        {
            return false;
        }

        // Only what the stage reads is listed, as the compiler strips every resource it declares but never touches.
        for (UINT Index = 0; Index < Description.BoundResources; ++Index)
        {
            D3D11_SHADER_INPUT_BIND_DESC Resource;
            Reflection->GetResourceBindingDesc(Index, AddressOf(Resource));

            UInt32 Mask = 0;

            for (UINT Slot = Resource.BindPoint; Slot < Resource.BindPoint + Resource.BindCount && Slot < 32; ++Slot)
            {
                Mask |= 1u << Slot;
            }

            switch (Resource.Type)
            {
            case D3D_SIT_CBUFFER:
                Output.Uniforms |= Mask;
                break;
            case D3D_SIT_SAMPLER:
                Output.Samplers |= Mask;
                break;
            case D3D_SIT_TEXTURE:
            case D3D_SIT_TBUFFER:
            case D3D_SIT_STRUCTURED:
            case D3D_SIT_BYTEADDRESS:
                Output.Textures |= Mask;
                break;
            default:
                break;
            }
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Pipeline::Pipeline()
        : mTopology { D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Pipeline::~Pipeline()
    {
        Await();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pipeline::Create(ConstRef<Device> Device, ConstRef<Compiler> Compiler, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States)
    {
        Delete();

        mBuild = Unique<Build>::Create();
        mBuild->Compiler  = AddressOf(Compiler);
        mBuild->Tier      = Device.GetDescription().Tier;
        mBuild->Macros    = Program.Macros;
        mBuild->Signature = Signature;
        mBuild->States    = States;

        for (UInt32 Stage = 0; Stage < ZyEnum::Count<ShaderStage>(); ++Stage)
        {
            mBuild->Sources[Stage].Append(Program.Modules[Stage]);
        }

        // Only the compile leaves the graphics thread, since the device that creates the shaders is single-threaded.
        if (!::TrySubmitThreadpoolCallback(& OnCompile, mBuild.Grab(), nullptr))
        {
            OnCompile(nullptr, mBuild.Grab());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pipeline::Delete()
    {
        Await();

        mBuild    = nullptr;
        mVertex   = nullptr;
        mFragment = nullptr;
        mBlend    = nullptr;
        mDepth    = nullptr;
        mRaster   = nullptr;
        mLayout   = nullptr;
        mTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        mUsages   = { };
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Pipeline::Resolve(ConstRef<Device> Device, ConstRef<Compiler> Compiler)
    {
        if (mBuild && mBuild->Done.load(std::memory_order_acquire))
        {
            const Unique<Build> Finished = Move(mBuild);

            if (!Assemble(Device, Compiler, * Finished))
            {
                LOG_E("Graphics: Failed to create a pipeline, its draws are skipped");

                Delete();
            }
        }
        return mVertex != nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void CALLBACK Pipeline::OnCompile(PTP_CALLBACK_INSTANCE Instance, PVOID Context)
    {
        ZY_PROFILE_SCOPE("Graphics::Compile");

        const Ptr<Build> Source = static_cast<Ptr<Build>>(Context);

        Sequence<D3D_SHADER_MACRO, 64> Macros;

        for (ConstRef<Macro> Macro : Source->Macros)
        {
            Macros.Append(Macro.Name.GetData(), Macro.Value.GetData());
        }
        Macros.Append("VERTEX_SHADER", nullptr);
        Macros.Append(nullptr, nullptr);

        Source->Vertex = Source->Compiler->Compile(Source->Sources[0], Source->Tier, ShaderStage::Vertex, Macros.GetData());

        // A vertex stage that failed leaves nothing worth compiling the fragment stage for.
        if (Source->Vertex)
        {
            Macros[Macros.GetSize() - 2].Name = "FRAGMENT_SHADER";
            Source->Fragment = Source->Compiler->Compile(Source->Sources[1], Source->Tier, ShaderStage::Fragment, Macros.GetData());
        }
        Source->Done.store(true, std::memory_order_release);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Pipeline::Assemble(ConstRef<Device> Device, ConstRef<Compiler> Compiler, ConstRef<Build> Source)
    {
        const Ptr<ID3D11Device1> Handle = Device.GetHandle();
        const Ptr<ID3DBlob>      VSCode = Source.Vertex.Get();
        const Ptr<ID3DBlob>      PSCode = Source.Fragment.Get();

        if (!VSCode || !PSCode)
        {
            return false;
        }

        if (!CreateVertex(Handle, VSCode, mVertex) || !CreatePixel(Handle, PSCode, mFragment))
        {
            return false;
        }

        if (!CreateUsage(Compiler, VSCode, mUsages[ZyEnum::Cast(ShaderStage::Vertex)]))
        {
            return false;
        }

        if (!CreateUsage(Compiler, PSCode, mUsages[ZyEnum::Cast(ShaderStage::Fragment)]))
        {
            return false;
        }

        mTopology = Convert(Source.States.Topology);

        return CreateBlend(Handle, Source.States, mBlend)
            && CreateDepth(Handle, Source.States, mDepth)
            && CreateRaster(Handle, Source.States, mRaster)
            && CreateLayout(Handle, Source.Signature, VSCode, mLayout);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pipeline::Await() const
    {
        while (mBuild && !mBuild->Done.load(std::memory_order_acquire))
        {
            ::SwitchToThread();
        }
    }
}