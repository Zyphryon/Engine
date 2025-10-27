// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
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
        Graphic::Descriptor Description;

        TOMLParser        Parser(Data.GetText());
        const TOMLSection Properties = Parser.GetSection("Properties");
        const TOMLSection Program    = Parser.GetSection("Program");

        // Parse 'blend' section
        const TOMLSection Blend = Properties.GetSection("Blend");
        Description.BlendMask           = Blend.GetEnum("Mask", Graphic::BlendMask::RGBA);
        Description.BlendSrcColor       = Blend.GetEnum("SrcColor", Graphic::BlendFactor::One);
        Description.BlendDstColor       = Blend.GetEnum("DstColor", Graphic::BlendFactor::Zero);
        Description.BlendEquationColor  = Blend.GetEnum("EquationColor", Graphic::BlendFunction::Add);
        Description.BlendSrcAlpha       = Blend.GetEnum("SrcAlpha", Graphic::BlendFactor::One);
        Description.BlendDstAlpha       = Blend.GetEnum("DstAlpha", Graphic::BlendFactor::Zero);
        Description.BlendEquationAlpha  = Blend.GetEnum("EquationAlpha", Graphic::BlendFunction::Add);

        // Parse 'depth' section
        const TOMLSection Depth = Properties.GetSection("Depth");
        Description.DepthClip      = Depth.GetBool("Clip", true);
        Description.DepthMask      = Depth.GetBool("Mask", true);
        Description.DepthTest      = Depth.GetEnum("Condition", Graphic::TestCondition::LessEqual);
        Description.DepthBias      = Depth.GetReal("Bias", 0.0);
        Description.DepthBiasClamp = Depth.GetReal("BiasClamp", 0.0);
        Description.DepthBiasSlope = Depth.GetReal("BiasSlope", 0.0);

        // Parse 'stencil' section
        const TOMLSection Stencil = Properties.GetSection("Stencil");
        Description.StencilReadMask       = Stencil.GetInteger("ReadMask", 0);
        Description.StencilWriteMask      = Stencil.GetInteger("WriteMask", 0);
        Description.StencilBackTest       = Stencil.GetEnum("BackTest", Graphic::TestCondition::Always);
        Description.StencilBackFail       = Stencil.GetEnum("BackFail", Graphic::TestAction::Keep);
        Description.StencilBackDepthFail  = Stencil.GetEnum("BackDepthFail", Graphic::TestAction::Keep);
        Description.StencilBackDepthPass  = Stencil.GetEnum("BackDepthPass", Graphic::TestAction::Keep);
        Description.StencilFrontTest      = Stencil.GetEnum("FrontTest", Graphic::TestCondition::Always);
        Description.StencilFrontFail      = Stencil.GetEnum("FrontFail", Graphic::TestAction::Keep);
        Description.StencilFrontDepthFail = Stencil.GetEnum("FrontDepthFail", Graphic::TestAction::Keep);
        Description.StencilFrontDepthPass = Stencil.GetEnum("FrontDepthPass", Graphic::TestAction::Keep);

        // Parse 'rasterizer' section
        const TOMLSection Rasterizer = Properties.GetSection("Rasterizer");
        Description.Fill = Rasterizer.GetEnum("Fill", Graphic::Fill::Solid);
        Description.Cull = Rasterizer.GetEnum("Cull", Graphic::Cull::Back);

        // Parse 'layout' section
        const TOMLSection Layout     = Properties.GetSection("Layout");
        const TOMLArray   Attributes = Layout.GetArray("Attributes");

        for(UInt Index = 0, Size = Attributes.GetSize(); Index < Size; ++Index)
        {
            const TOMLArray Values = Attributes.GetArray(Index);

            Ref<Graphic::Attribute> Attribute = Description.InputAttributes.emplace_back();
            Attribute.Semantic = Values.GetEnum(0, Graphic::VertexSemantic::None);
            Attribute.Format   = Values.GetEnum(1, Graphic::VertexFormat::Float32x4);
            Attribute.Stream   = Values.GetInteger(2);
            Attribute.Offset   = Values.GetInteger(3);
            Attribute.Divisor  = Values.GetInteger(4);
        }

        Description.InputPrimitive = Layout.GetEnum("Primitive", Graphic::Primitive::TriangleList);

        // Parse 'textures' section
        const TOMLSection Textures = Program.GetSection("Textures");

        Graphic::Pipeline::MappingList<Graphic::TextureSemantic, Graphic::kMaxSlots> TextureMappingList;
        for (const Graphic::TextureSemantic Semantic : Enum::GetValues<Graphic::TextureSemantic>())
        {
            if (const UInt32 Slot = Textures.GetInteger(Enum::GetName(Semantic), ~0); Slot != ~0)
            {
                TextureMappingList.emplace_back(Semantic, Slot);
            }
        }

        // Parse 'shader' section
        Array<Blob, Graphic::kMaxStages> Stages;
        Stages[0] = Compile(Service, Scope, Program.GetSection("Vertex"), Graphic::Stage::Vertex);
        Stages[1] = Compile(Service, Scope, Program.GetSection("Fragment"), Graphic::Stage::Fragment);

        if (Stages[0] && Stages[1])
        {
            const Tracker<Graphic::Pipeline> Asset = Tracker<Graphic::Pipeline>::Cast(Scope.GetResource());
            Asset->Load(Move(Stages), Move(TextureMappingList), Move(Description));
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

        constexpr ConstStr8 kShaderProfiles[][3] = {
            { "vs_4_0_level_9_1", "ps_4_0_level_9_1", ""       },
            { "vs_4_0_level_9_1", "ps_4_0_level_9_1", ""       },
            { "vs_4_0_level_9_3", "ps_4_0_level_9_3", ""       },
            { "vs_4_0",           "ps_4_0",           "gs_4_0" },
            { "vs_5_0",           "ps_5_0",           "gs_5_0" },
            { "vs_6_0",           "ps_6_0",           "gs_6_0" }
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
                Compilation = Blob(Bytecode->GetBufferSize());
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