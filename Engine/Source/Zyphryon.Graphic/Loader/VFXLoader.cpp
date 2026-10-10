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

#include "VFXLoader.hpp"
#include "Parser.hpp"
#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt8 LoadPatch(JsonObject Section, Ref<States> States)
    {
        UInt8 Blocks = 0;

        if (const JsonObject JsonBlend = Section.GetObject("Blend"); JsonBlend.IsValid())
        {
            States.Channel            = JsonBlend.GetEnum("Channel", States.Channel);
            States.BlendSrcColor      = JsonBlend.GetEnum("SrcColor", States.BlendSrcColor);
            States.BlendDstColor      = JsonBlend.GetEnum("DstColor", States.BlendDstColor);
            States.BlendEquationColor = JsonBlend.GetEnum("EquationColor", States.BlendEquationColor);
            States.BlendSrcAlpha      = JsonBlend.GetEnum("SrcAlpha", States.BlendSrcAlpha);
            States.BlendDstAlpha      = JsonBlend.GetEnum("DstAlpha", States.BlendDstAlpha);
            States.BlendEquationAlpha = JsonBlend.GetEnum("EquationAlpha", States.BlendEquationAlpha);

            Blocks = SetBit(Blocks, Technique::GetBlockMask(Technique::Block::Blend));
        }

        if (const JsonObject JsonDepth = Section.GetObject("Depth"); JsonDepth.IsValid())
        {
            States.DepthClip      = JsonDepth.GetBool("Clip", States.DepthClip);
            States.DepthMask      = JsonDepth.GetBool("Mask", States.DepthMask);
            States.DepthTest      = JsonDepth.GetEnum("Condition", States.DepthTest);
            States.DepthBias      = JsonDepth.GetNumber<Real32>("Bias", States.DepthBias);
            States.DepthBiasClamp = JsonDepth.GetNumber<Real32>("BiasClamp", States.DepthBiasClamp);
            States.DepthBiasSlope = JsonDepth.GetNumber<Real32>("BiasSlope", States.DepthBiasSlope);

            Blocks = SetBit(Blocks, Technique::GetBlockMask(Technique::Block::Depth));
        }

        if (const JsonObject JsonRasterizer = Section.GetObject("Rasterizer"); JsonRasterizer.IsValid())
        {
            States.Fill    = JsonRasterizer.GetEnum("Fill", States.Fill);
            States.Cull    = JsonRasterizer.GetEnum("Cull", States.Cull);
            States.Scissor = JsonRasterizer.GetBool("Scissor", States.Scissor);

            Blocks = SetBit(Blocks, Technique::GetBlockMask(Technique::Block::Rasterizer));
        }
        return Blocks;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LoadProperties(JsonObject Section, Ref<States> States, Ref<Attributes> Attributes)
    {
        LoadPatch(Section, States);

        if (const JsonObject JsonStencil = Section.GetObject("Stencil"); JsonStencil.IsValid())
        {
            States.StencilReadMask       = JsonStencil.GetNumber<UInt8>("ReadMask", States.StencilReadMask);
            States.StencilWriteMask      = JsonStencil.GetNumber<UInt8>("WriteMask", States.StencilWriteMask);
            States.StencilBackTest       = JsonStencil.GetEnum("BackTest", States.StencilBackTest);
            States.StencilBackFail       = JsonStencil.GetEnum("BackFail", States.StencilBackFail);
            States.StencilBackDepthFail  = JsonStencil.GetEnum("BackDepthFail", States.StencilBackDepthFail);
            States.StencilBackDepthPass  = JsonStencil.GetEnum("BackDepthPass", States.StencilBackDepthPass);
            States.StencilFrontTest      = JsonStencil.GetEnum("FrontTest", States.StencilFrontTest);
            States.StencilFrontFail      = JsonStencil.GetEnum("FrontFail", States.StencilFrontFail);
            States.StencilFrontDepthFail = JsonStencil.GetEnum("FrontDepthFail", States.StencilFrontDepthFail);
            States.StencilFrontDepthPass = JsonStencil.GetEnum("FrontDepthPass", States.StencilFrontDepthPass);
        }

        if (const JsonObject JsonLayout = Section.GetObject("Layout"); JsonLayout.IsValid())
        {
            if (const JsonArray JsonAttributes = JsonLayout.GetArray("Attributes"); !JsonAttributes.IsNullOrEmpty())
            {
                Attributes.Clear();

                for (UInt Index = 0, Limit = JsonAttributes.GetSize(); Index < Limit; ++Index)
                {
                    const JsonArray Values = JsonAttributes.GetArray(Index);

                    Ref<Attribute> Attribute = Attributes.Append();
                    Attribute.Location = Values.GetNumber<UInt8>(0);
                    Attribute.Format   = Values.GetEnum(1, VertexFormat::Float32x4);
                    Attribute.Stream   = Values.GetNumber<UInt8>(2);
                    Attribute.Offset   = Values.GetNumber<UInt16>(3);
                    Attribute.Divisor  = Values.GetNumber<UInt8>(4);
                }
            }

            States.Topology = JsonLayout.GetEnum("Primitive", States.Topology);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LoadDefines(JsonObject Section, Ref<Sequence<Macro>> Macros)
    {
        if (const JsonArray JsonDefines = Section.GetArray("Defines"); !JsonDefines.IsNullOrEmpty())
        {
            for (UInt Index = 0, Limit = JsonDefines.GetSize(); Index < Limit; ++Index)
            {
                const Text Definition = JsonDefines.GetString(Index);

                if (const SInt Position = StrFind(Definition, '='); Position != -1)
                {
                    Macros.Append(Definition.Slice(0, Position), Definition.Slice(Position + 1));
                }
                else
                {
                    Macros.Append(Definition, Text::Empty());
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LoadSignature(JsonObject Section, Ref<Schema> Schema)
    {
        if (const JsonArray JsonTextures = Section.GetArray("Textures"); !JsonTextures.IsNullOrEmpty())
        {
            for (UInt Index = 0, Limit = JsonTextures.GetSize(); Index < Limit; ++Index)
            {
                const JsonObject JsonTexture = JsonTextures.GetObject(Index);

                const Text Name = JsonTexture.GetString("Name");

                // The schema hands back the register it took, which the authored one only has to agree with.
                const UInt8 Register = Schema.AddTexture(Name);

                ZY_ASSERT(JsonTexture.GetNumber<UInt8>("Register", Index) == Register,
                    "Texture registers must be dense and ordered");

                // A fallback is one texel, given as four bytes, bound wherever a material leaves the texture out.
                if (const JsonArray JsonFallback = JsonTexture.GetArray("Fallback"); !JsonFallback.IsNullOrEmpty())
                {
                    const UInt32 Texel = (JsonFallback.GetNumber<UInt32>(0) & 0xFFu)
                                       | (JsonFallback.GetNumber<UInt32>(1) & 0xFFu) << 8
                                       | (JsonFallback.GetNumber<UInt32>(2) & 0xFFu) << 16
                                       | (JsonFallback.GetNumber<UInt32>(3) & 0xFFu) << 24;

                    const TextureFormat Format = JsonTexture.GetEnum("Format", TextureFormat::RGBA8UIntNorm);

                    ZY_ASSERT(Format == TextureFormat::RGBA8UIntNorm || Format == TextureFormat::RGBA8UIntNorm_sRGB,
                        "A fallback texel is four bytes of color, stored linear or sRGB");

                    Schema.SetFallback(Register, Texel, JsonTexture.GetEnum("Layout", TextureLayout::Texture2D), Format);
                }
            }
        }

        if (const JsonArray JsonSamplers = Section.GetArray("Samplers"); !JsonSamplers.IsNullOrEmpty())
        {
            for (UInt Index = 0, Limit = JsonSamplers.GetSize(); Index < Limit; ++Index)
            {
                const JsonObject JsonSampler = JsonSamplers.GetObject(Index);

                const Text  Name     = JsonSampler.GetString("Name");
                const UInt8 Register = Schema.AddSampler(Name, ParseSampler(JsonSampler, Sampler()));

                ZY_ASSERT(JsonSampler.GetNumber<UInt8>("Register", Index) == Register,
                    "Sampler registers must be dense and ordered");
            }
        }

        if (const JsonArray JsonUniforms = Section.GetArray("Uniforms"); !JsonUniforms.IsNullOrEmpty())
        {
            for (UInt Index = 0, Limit = JsonUniforms.GetSize(); Index < Limit; ++Index)
            {
                const JsonObject JsonUniform = JsonUniforms.GetObject(Index);

                const Text      Name      = JsonUniform.GetString("Name");
                const Frequency Frequency = JsonUniform.GetEnum("Frequency", Frequency::Frame);
                const Uniform   Type      = JsonUniform.GetEnum("Type", Uniform::Float);

                if (const UInt32 Count = JsonUniform.GetNumber<UInt32>("Count", 1); Count == 1)
                {
                    Schema.AddUniform(Frequency, Name, Type, ParseParameter(JsonUniform));
                }
                else
                {
                    for (UInt32 Element = 0; Element < Count; ++Element)
                    {
                        Schema.AddUniform(Frequency, Str64::Print<"{0}[{1}]">(Name, Element), Type, Parameter());
                    }
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LoadFeatures(
        ConstRef<ZyContent::Uri>    Key,
        JsonArray                   Section,
        Ref<Technique::Description> Description)
    {
        for (UInt Index = 0, Limit = Section.GetSize(); Index < Limit; ++Index)
        {
            const JsonObject JsonFeature = Section.GetObject(Index);

            Ref<Technique::Feature> Feature = Description.Features.Append();
            Feature.Name = JsonFeature.GetString("Name");

            // 'Enable' names what turns the feature on without the caller asking for it.
            if (const JsonObject JsonEnable = JsonFeature.GetObject("Enable"); JsonEnable.IsValid())
            {
                if (const Text Texture = JsonEnable.GetString("Texture"); !Texture.IsEmpty())
                {
                    Feature.Texture = Hash(Texture);
                }

                if (const Text Parameter = JsonEnable.GetString("Parameter"); !Parameter.IsEmpty())
                {
                    Feature.Parameter = Hash(Parameter);
                }
            }

            // Parsing the patch over the base leaves every block it declares complete rather than partial.
            Feature.States = Description.Base.States;

            if (const JsonObject JsonProperties = JsonFeature.GetObject("Properties"); JsonProperties.IsValid())
            {
                Feature.Blocks = LoadPatch(JsonProperties, Feature.States);
            }

            if (JsonFeature.GetObject("Signature").IsValid())
            {
                LOG_W(
                    "'{0}' has feature '{1}' patching the signature, which every variant shares",
                    Key,
                    Feature.Name);
            }

            if (const JsonObject JsonProgram = JsonFeature.GetObject("Program"); JsonProgram.IsValid())
            {
                LoadDefines(JsonProgram, Feature.Macros);

                if (JsonProgram.GetObject("Shaders").IsValid())
                {
                    LOG_W(
                        "'{0}' has feature '{1}' replacing shaders, which every variant shares",
                        Key,
                        Feature.Name);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LoadPreload(
        ConstRef<ZyContent::Uri>    Key,
        JsonArray                   Section,
        Ref<Technique::Description> Description)
    {
        for (UInt Index = 0, Limit = Section.GetSize(); Index < Limit; ++Index)
        {
            const JsonArray JsonVariant = Section.GetArray(Index);

            Technique::Key Mask = 0;

            for (UInt Slot = 0, Count = JsonVariant.GetSize(); Slot < Count; ++Slot)
            {
                const Text Name  = JsonVariant.GetString(Slot);
                const SInt Found = Description.Features.Find([Name](ConstRef<Technique::Feature> Entry)
                {
                    return Entry.Name == Name;
                });

                if (Found >= 0)
                {
                    Mask = SetBit(Mask, 1u << Found);
                }
                else
                {
                    LOG_W("'{0}' preloads unknown feature '{1}'", Key, Name);
                }
            }

            Description.Preload.Append(Mask);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    VFXLoader::VFXLoader(ShaderLanguage Language, Tier Tier)
        : mLanguage { ZyEnum::GetName(Language) },
          mTier     { Tier }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool VFXLoader::Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Technique> Asset = Retainer<Technique>::Cast(Scope.GetResource());

        Technique::Description Description;
        ZyGraphic::Schema      Schema;

        JsonValue Document;
        JsonError Error;

        if (!JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()), Document, Error))
        {
            LOG_W("'{0}' is not valid JSON: {1}", Asset->GetKey(), Error);
            return false;
        }

        if (!Document.IsObject())
        {
            LOG_W("'{0}' is not a valid technique document", Asset->GetKey());
            return false;
        }

        const JsonObject JsonRoot(Document);

        if (const JsonObject JsonProperties = JsonRoot.GetObject("Properties"); JsonProperties.IsValid())
        {
            LoadProperties(JsonProperties, Description.Base.States, Description.Base.Attributes);
        }

        if (const JsonObject JsonSignature = JsonRoot.GetObject("Signature"); JsonSignature.IsValid())
        {
            LoadSignature(JsonSignature, Schema);
        }

        // Every program is told the tier the device reaches, numbered from one, which the shared shaders branch on.
        constexpr Text kTiers[] = { "1", "2", "3", "4", "5" };
        Description.Base.Macros.Append(Text("ZY_TIER"), kTiers[ZyEnum::Cast(mTier)]);

        if (const JsonObject JsonProgram = JsonRoot.GetObject("Program"); JsonProgram.IsValid())
        {
            LoadProgram(Service, Scope, JsonProgram, Description.Base.Macros, Description.Base.Shaders);
        }

        if (const JsonArray JsonFeatures = JsonRoot.GetArray("Features"); !JsonFeatures.IsNullOrEmpty())
        {
            LoadFeatures(Asset->GetKey(), JsonFeatures, Description);
        }

        if (const JsonArray JsonPreload = JsonRoot.GetArray("Preload"); !JsonPreload.IsNullOrEmpty())
        {
            LoadPreload(Asset->GetKey(), JsonPreload, Description);
        }

        Asset->Setup(Move(Description), Move(Schema));
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void VFXLoader::LoadProgram(
        Ref<ZyContent::Service> Service,
        Ref<ZyContent::Scope>   Scope,
        JsonObject              Section,
        Ref<Sequence<Macro>>    Macros,
        Ref<Technique::Shaders> Shaders)
    {
        LoadDefines(Section, Macros);

        if (const JsonObject JsonShaders = Section.GetObject("Shaders"); JsonShaders.IsValid())
        {
            if (const JsonArray JsonStages = JsonShaders.GetArray(mLanguage); !JsonStages.IsNullOrEmpty())
            {
                for (UInt Index = 0, Limit = JsonStages.GetSize(); Index < Limit; ++Index)
                {
                    const JsonObject JsonShader = JsonStages.GetObject(Index);

                    const Text        Path  = JsonShader.GetString("Path");
                    const ShaderStage Stage = JsonShader.GetEnum("Stage", ShaderStage::Vertex);

                    Shaders[ZyEnum::Cast(Stage)] = Service.Load<Shader>(Path, AddressOf(Scope));
                }
            }
        }
    }
}