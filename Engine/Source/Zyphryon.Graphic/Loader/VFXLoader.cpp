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
#include "Zyphryon.Graphic/Technique.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
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
        Technique::Description Description;
        ZyGraphic::Schema      Schema;

        JsonValue JsonDocument = JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()));
        const JsonObject JsonRoot(JsonDocument);

        if (!JsonRoot.IsValid())
        {
            LOG_W("'{0}' is not a valid technique document", Scope.GetResource()->GetKey());
            return false;
        }

        // Parse 'Properties' section
        if (const JsonObject JsonProperties = JsonRoot.GetObject("Properties"); JsonProperties.IsValid())
        {
            LoadProperties(JsonProperties, Description.Base.States, Description.Base.Attributes);
        }

        // Parse 'Signature' section
        if (const JsonObject JsonProperties = JsonRoot.GetObject("Signature"); JsonProperties.IsValid())
        {
            // Parse 'Textures' section
            if (const JsonArray JsonTextures = JsonProperties.GetArray("Textures"); !JsonTextures.IsNullOrEmpty())
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
                            "A fallback texel is four bytes of colour, stored linear or sRGB");

                        Schema.SetFallback(Register, Texel, JsonTexture.GetEnum("Layout", TextureLayout::Texture2D), Format);
                    }
                }
            }

            // Parse 'Samplers' section
            if (const JsonArray JsonSamplers = JsonProperties.GetArray("Samplers"); !JsonSamplers.IsNullOrEmpty())
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

            // Parse 'Uniforms' section
            if (const JsonArray JsonUniforms = JsonProperties.GetArray("Uniforms"); !JsonUniforms.IsNullOrEmpty())
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
                            Str64 Buffer(Name);
                            Buffer.Append('[');
                            Buffer.AppendInteger(Element, CountDigits<10>(Element), 10, false);
                            Buffer.Append(']');

                            Schema.AddUniform(Frequency, Buffer, Type, Parameter());
                        }
                    }
                }
            }
        }

        // Every program is told the tier the device reaches, numbered from one, which the shared shaders branch on.
        constexpr Text kTiers[] = { "1", "2", "3", "4", "5" };
        Description.Base.Macros.Append(Text("ZY_TIER"), kTiers[ZyEnum::Cast(mTier)]);

        // Parse 'Program' section
        if (const JsonObject JsonProgram = JsonRoot.GetObject("Program"); JsonProgram.IsValid())
        {
            LoadProgram(Service, Scope, JsonProgram, Description.Base.Macros, Description.Base.Shaders);
        }

        // Parse 'Features' section, ordered so each entry's index is the bit it occupies in a key.
        if (const JsonArray JsonFeatures = JsonRoot.GetArray("Features"); !JsonFeatures.IsNullOrEmpty())
        {
            for (UInt Index = 0, Limit = JsonFeatures.GetSize(); Index < Limit; ++Index)
            {
                const JsonObject JsonFeature = JsonFeatures.GetObject(Index);

                Ref<Technique::Feature> Feature = Description.Features.Append();
                Feature.Name = JsonFeature.GetString("Name");

                // Parse 'Enable' section, naming what turns the feature on without the caller asking for it.
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
                    LOG_W("'{0}' has feature '{1}' patching the signature, which every variant shares",
                          Scope.GetResource()->GetKey(), Feature.Name);
                }

                if (const JsonObject JsonProgram = JsonFeature.GetObject("Program"); JsonProgram.IsValid())
                {
                    LoadDefines(JsonProgram, Feature.Macros);

                    if (JsonProgram.GetObject("Shaders").IsValid())
                    {
                        LOG_W("'{0}' has feature '{1}' replacing shaders, which every variant shares",
                              Scope.GetResource()->GetKey(), Feature.Name);
                    }
                }
            }
        }

        // Parse 'Preload' section, resolving each combination of feature names into the key selecting them.
        if (const JsonArray JsonPreload = JsonRoot.GetArray("Preload"); !JsonPreload.IsNullOrEmpty())
        {
            const auto GetFeatureKey = [&Description](Text Name)
            {
                for (UInt Index = 0, Limit = Description.Features.GetSize(); Index < Limit; ++Index)
                {
                    if (Description.Features[Index].Name == Name)
                    {
                        return static_cast<Technique::Key>(1u << Index);
                    }
                }
                return static_cast<Technique::Key>(0);
            };

            for (UInt Index = 0, Limit = JsonPreload.GetSize(); Index < Limit; ++Index)
            {
                const JsonArray JsonVariant = JsonPreload.GetArray(Index);

                Technique::Key Key = 0;

                for (UInt Slot = 0, Count = JsonVariant.GetSize(); Slot < Count; ++Slot)
                {
                    const Text Name = JsonVariant.GetString(Slot);

                    if (const Technique::Key Bit = GetFeatureKey(Name))
                    {
                        Key = SetBit(Key, Bit);
                    }
                    else
                    {
                        LOG_W("'{0}' preloads unknown feature '{1}'", Scope.GetResource()->GetKey(), Name);
                    }
                }

                Description.Preload.Append(Key);
            }
        }

        const Retainer<Technique> Asset = Retainer<Technique>::Cast(Scope.GetResource());
        Asset->Setup(Move(Description), Move(Schema));
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void VFXLoader::LoadProperties(JsonObject Section, Ref<States> States, Ref<Attributes> Attributes)
    {
        LoadPatch(Section, States);

        // Parse 'Stencil' section
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

        // Parse 'Layout' section
        if (const JsonObject JsonLayout = Section.GetObject("Layout"); JsonLayout.IsValid())
        {
            if (const JsonArray JsonAttributes = JsonLayout.GetArray("Attributes"); !JsonAttributes.IsNullOrEmpty())
            {
                Attributes.Clear();

                for (UInt Index = 0, Size = JsonAttributes.GetSize(); Index < Size; ++Index)
                {
                    const JsonArray Values = JsonAttributes.GetArray(Index);

                    Ref<Attribute> Attribute = Attributes.Append();
                    Attribute.Location = Values.GetNumber(0, 0);
                    Attribute.Format   = Values.GetEnum(1, VertexFormat::Float32x4);
                    Attribute.Stream   = Values.GetNumber<UInt32>(2);
                    Attribute.Offset   = Values.GetNumber<UInt32>(3);
                    Attribute.Divisor  = Values.GetNumber<UInt32>(4);
                }
            }

            States.Topology = JsonLayout.GetEnum("Primitive", States.Topology);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt8 VFXLoader::LoadPatch(JsonObject Section, Ref<States> States)
    {
        UInt8 Blocks = 0;

        // Parse 'Blend' section
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

        // Parse 'Depth' section
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

        // Parse 'Rasterizer' section
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

    void VFXLoader::LoadProgram(
        Ref<ZyContent::Service> Service,
        Ref<ZyContent::Scope>   Scope,
        JsonObject              Section,
        Ref<Sequence<Macro>>    Macros,
        Ref<Technique::Shaders> Shaders)
    {
        LoadDefines(Section, Macros);

        // Parse 'Shaders' section
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

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void VFXLoader::LoadDefines(JsonObject Section, Ref<Sequence<Macro>> Macros)
    {
        // Parse 'Defines' section
        if (const JsonArray JsonDefines = Section.GetArray("Defines"); !JsonDefines.IsNullOrEmpty())
        {
            for (UInt Index = 0, Limit = JsonDefines.GetSize(); Index < Limit; ++Index)
            {
                const Text Definition = JsonDefines.GetString(Index);

                if (const SInt32 Position = StrFind(Definition, '='); Position != -1)
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
}