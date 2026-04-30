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

#include "Loader.hpp"
#include "Zyphryon.Content/Service.hpp"

#ifdef    ZYPHRYON_PLATFORM_WINDOWS
    #include <d3dcompiler.h>
#endif // ZYPHRYON_PLATFORM_WINDOWS

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef    ZYPHRYON_PLATFORM_WINDOWS

    static pD3DCompile GetD3DCompileFunction()
    {
        static pD3DCompile D3DCompilePointer = nullptr;

        if (!D3DCompilePointer)
        {
            static constexpr LPCWSTR kFilenames[] = {
                L"D3DCompiler_47",
                L"D3DCompiler_46",
                L"D3DCompiler_43",
                L"D3DCompiler_42",
                L"D3DCompiler_41",
                L"D3DCompiler_40"
            };

            for (const LPCWSTR Library : kFilenames)
            {
                if (const HMODULE Module = ::LoadLibraryW(Library))
                {
                    D3DCompilePointer = reinterpret_cast<pD3DCompile>(GetProcAddress(Module, "D3DCompile"));
                    break;
                }
            }
        }
        return D3DCompilePointer;
    }

#endif // ZYPHRYON_PLATFORM_WINDOWS

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    PipelineLoader::PipelineLoader(Graphic::Backend Backend, Graphic::Language Target)
        : mBackend { Backend },
          mTarget  { Target }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool PipelineLoader::Load(Ref<Service> Service, Ref<Scope> Scope, AnyRef<Blob> Data)
    {
        Graphic::States States;

        TOMLParser        Parser(Data.GetText());
        const TOMLSection Properties = Parser.GetSection("Properties");
        const TOMLSection Program    = Parser.GetSection("Program");

        // Parse 'blend' section
        const TOMLSection Blend = Properties.GetSection("Blend");
        States.AlphaToCoverage     = Blend.GetBool("AlphaToCoverage", false);
        States.Channel             = Blend.GetEnum("Channel", Graphic::Channel::RGBA);
        States.BlendSrcColor       = Blend.GetEnum("SrcColor", Graphic::BlendFactor::One);
        States.BlendDstColor       = Blend.GetEnum("DstColor", Graphic::BlendFactor::Zero);
        States.BlendEquationColor  = Blend.GetEnum("EquationColor", Graphic::BlendFunction::Add);
        States.BlendSrcAlpha       = Blend.GetEnum("SrcAlpha", Graphic::BlendFactor::One);
        States.BlendDstAlpha       = Blend.GetEnum("DstAlpha", Graphic::BlendFactor::Zero);
        States.BlendEquationAlpha  = Blend.GetEnum("EquationAlpha", Graphic::BlendFunction::Add);

        // Parse 'depth' section
        const TOMLSection Depth = Properties.GetSection("Depth");
        States.DepthClip      = Depth.GetBool("Clip", true);
        States.DepthMask      = Depth.GetBool("Mask", true);
        States.DepthTest      = Depth.GetEnum("Condition", Graphic::TestCondition::LessEqual);
        States.DepthBias      = Depth.GetReal("Bias", 0.0);
        States.DepthBiasClamp = Depth.GetReal("BiasClamp", 0.0);
        States.DepthBiasSlope = Depth.GetReal("BiasSlope", 0.0);

        // Parse 'stencil' section
        const TOMLSection Stencil = Properties.GetSection("Stencil");
        States.StencilReadMask       = Stencil.GetInteger("ReadMask", 0);
        States.StencilWriteMask      = Stencil.GetInteger("WriteMask", 0);
        States.StencilBackTest       = Stencil.GetEnum("BackTest", Graphic::TestCondition::Always);
        States.StencilBackFail       = Stencil.GetEnum("BackFail", Graphic::TestAction::Keep);
        States.StencilBackDepthFail  = Stencil.GetEnum("BackDepthFail", Graphic::TestAction::Keep);
        States.StencilBackDepthPass  = Stencil.GetEnum("BackDepthPass", Graphic::TestAction::Keep);
        States.StencilFrontTest      = Stencil.GetEnum("FrontTest", Graphic::TestCondition::Always);
        States.StencilFrontFail      = Stencil.GetEnum("FrontFail", Graphic::TestAction::Keep);
        States.StencilFrontDepthFail = Stencil.GetEnum("FrontDepthFail", Graphic::TestAction::Keep);
        States.StencilFrontDepthPass = Stencil.GetEnum("FrontDepthPass", Graphic::TestAction::Keep);

        // Parse 'rasterizer' section
        const TOMLSection Rasterizer = Properties.GetSection("Rasterizer");
        States.Fill = Rasterizer.GetEnum("Fill", Graphic::Fill::Solid);
        States.Cull = Rasterizer.GetEnum("Cull", Graphic::Cull::Back);

        // Parse 'layout' section
        const TOMLSection Layout     = Properties.GetSection("Layout");
        const TOMLArray   Attributes = Layout.GetArray("Attributes");

        for(UInt Index = 0, Size = Attributes.GetSize(); Index < Size; ++Index)
        {
            const TOMLArray Values = Attributes.GetArray(Index);

            Ref<Graphic::Attribute> Attribute = States.InputAttributes.emplace_back();
            Attribute.Semantic = Values.GetEnum(0, Graphic::VertexSemantic::None);
            Attribute.Format   = Values.GetEnum(1, Graphic::VertexFormat::Float32x4);
            Attribute.Stream   = Values.GetInteger(2);
            Attribute.Offset   = Values.GetInteger(3);
            Attribute.Divisor  = Values.GetInteger(4);
        }

        States.InputPrimitive = Layout.GetEnum("Primitive", Graphic::Primitive::TriangleList);

        // Parse 'textures' section
        const TOMLSection Textures = Program.GetSection("Textures");

        Graphic::EntryList<Graphic::TextureSemantic, Graphic::kMaxResources> TextureBindingList;
        for (const Graphic::TextureSemantic Semantic : Enum::Values<Graphic::TextureSemantic>())
        {
            if (const UInt32 Slot = Textures.GetInteger(Enum::Name(Semantic), ~0); Slot != ~0)
            {
                TextureBindingList.emplace_back(Slot, Semantic);
            }
        }

        // Parse 'shader' section
        Graphic::Program Shaders;
        Shaders[0] = Compile(Service, Scope, Program.GetSection("Vertex"), Graphic::Stage::Vertex);
        Shaders[1] = Compile(Service, Scope, Program.GetSection("Fragment"), Graphic::Stage::Fragment);

        if (Shaders[0] && Shaders[1])
        {
            const Tracker<Graphic::Pipeline> Asset = Tracker<Graphic::Pipeline>::Cast(Scope.GetResource());
            Asset->Load(Move(Shaders), Move(States), Move(TextureBindingList));
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob PipelineLoader::Compile(Ref<Service> Service, ConstRef<Scope> Scope, ConstRef<TOMLSection> Section, Graphic::Stage Stage)
    {
        if (!Section.IsEmpty())
        {
            const Blob         Data     = Service.Find(Section.GetString("Filename"), Scope);
            const ConstStr8    Code     = Data.GetText();
            const ConstStr8    Entry    = Section.GetString("Entry", "main");
            const TOMLArray    Defines  = Section.GetArray("Defines");

            if (Code.empty())
            {
                LOG_WARNING("Can't find shader file '{}'", Section.GetString("Filename"));
                return Blob();
            }

            Vector<Property> Properties;
            Properties.reserve(Defines.GetSize());

            for (UInt32 Index = 0, Limit = Defines.GetSize(); Index < Limit; ++Index)
            {
                const ConstStr8 Definition = Defines.GetString(Index);
                const UInt Delimiter       = Definition.find_first_of('=');

                Properties.emplace_back(
                    Delimiter != ConstStr8::npos ? Definition.substr(0, Delimiter) : Definition,
                    Delimiter != ConstStr8::npos ? Definition.substr(Delimiter) : "true");
            }

#ifdef    ZYPHRYON_PLATFORM_WINDOWS
            return CompileDXBC(Entry, Code, Properties, Stage);
#endif // ZYPHRYON_PLATFORM_WINDOWS
        }
        return Blob();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob PipelineLoader::CompileDXBC(ConstStr8 Entry, ConstStr8 Code, Ref<Vector<Property>> Properties, Graphic::Stage Stage)
    {
        Blob Compilation;

#ifdef    ZYPHRYON_PLATFORM_WINDOWS
        D3D_SHADER_MACRO Defines[128];

        for (UInt Index = 0; Index < Properties.size(); ++Index)
        {
            Defines[Index].Name       = Properties[Index].Key.c_str();
            Defines[Index].Definition = Properties[Index].Value.c_str();
        }
        Defines[Properties.size()] = { nullptr, nullptr };

        Ptr<ID3DBlob> Error    = nullptr;
        Ptr<ID3DBlob> Bytecode = nullptr;

        static constexpr ConstStr8 kShaderProfiles[][2] = {
            { "vs_4_0_level_9_1", "ps_4_0_level_9_1" },
            { "vs_4_0_level_9_1", "ps_4_0_level_9_1" },
            { "vs_4_0_level_9_3", "ps_4_0_level_9_3" },
            { "vs_4_0",           "ps_4_0"           },
            { "vs_5_0",           "ps_5_0"           },
            { "vs_6_0",           "ps_6_0"           }
        };

        ConstRef<ConstStr8> Profile = kShaderProfiles[Enum::Cast(mTarget)][Enum::Cast(Stage)];

        if (const pD3DCompile D3DCompile = GetD3DCompileFunction())
        {
            const HRESULT Result   = D3DCompile(
                Code.data(),
                Code.size(),
                nullptr,
                Defines,
                nullptr,
                Entry.data(),
                Profile.data(),
                D3DCOMPILE_OPTIMIZATION_LEVEL3,
                0,
                &Bytecode,
                &Error);

            if (SUCCEEDED(Result))
            {
                Compilation = Blob::Bytes(Bytecode->GetBufferSize());
                Compilation.Copy(Bytecode->GetBufferPointer(), Bytecode->GetBufferSize());
                Bytecode->Release();
            }
            else
            {
                LOG_ERROR("Failed to compile shader: {}", static_cast<Ptr<Char>>(Error->GetBufferPointer()));
            }
        }
        else
        {
            LOG_ERROR("Can't find D3DCompile function");
        }
#endif // ZYPHRYON_PLATFORM_WINDOWS

        return Compilation;
    }
}