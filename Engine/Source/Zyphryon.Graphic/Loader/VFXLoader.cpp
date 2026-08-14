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
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Graphic/Technique.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    VFXLoader::VFXLoader(ShaderLanguage Language)
        : mLanguage { Enum::GetName(Language) }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool VFXLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        Technique::Description Description;
        Graphic::Schema        Schema;

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

                    const Text       Name       = JsonTexture.GetString("Name");
                    const Frequency  Frequency  = JsonTexture.GetEnum("Frequency", Frequency::Material);
                    const Visibility Visibility = JsonTexture.GetEnum("Visibility", Visibility::All);

                    // The schema hands back the register it took, which the authored one only has to agree with.
                    const UInt8 Register = Schema.AddTexture(Name);

                    ZY_ASSERT(JsonTexture.GetNumber<UInt8>("Register", Index) == Register,
                        "Texture registers must be dense and ordered");

                    Description.Signature.Bindings[Enum::Cast(Frequency)].Append(
                        Resource::Texture, Register, 1, Visibility);
                }
            }

            // Parse 'Samplers' section
            if (const JsonArray JsonSamplers = JsonProperties.GetArray("Samplers"); !JsonSamplers.IsNullOrEmpty())
            {
                for (UInt Index = 0, Limit = JsonSamplers.GetSize(); Index < Limit; ++Index)
                {
                    const JsonObject JsonSampler = JsonSamplers.GetObject(Index);

                    const Text       Name       = JsonSampler.GetString("Name");
                    const Frequency  Frequency  = JsonSampler.GetEnum("Frequency", Frequency::Material);
                    const Visibility Visibility = JsonSampler.GetEnum("Visibility", Visibility::All);

                    Sampler Descriptor;
                    Descriptor.AddressModeU = JsonSampler.GetEnum("AddressModeU", TextureAddress::Clamp);
                    Descriptor.AddressModeV = JsonSampler.GetEnum("AddressModeV", TextureAddress::Clamp);
                    Descriptor.AddressModeW = JsonSampler.GetEnum("AddressModeW", TextureAddress::Clamp);
                    Descriptor.Filter       = JsonSampler.GetEnum("Filter",       TextureFilter::LinearMipLinear);
                    Descriptor.Comparison   = JsonSampler.GetEnum("Comparison",   TestCondition::None);
                    Descriptor.Border       = JsonSampler.GetEnum("Border",       TextureBorder::OpaqueBlack);

                    const UInt8 Register = Schema.AddSampler(Name, Descriptor);

                    ZY_ASSERT(JsonSampler.GetNumber<UInt8>("Register", Index) == Register,
                        "Sampler registers must be dense and ordered");

                    Description.Signature.Bindings[Enum::Cast(Frequency)].Append(
                        Resource::Sampler, Register, 1, Visibility);
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
                        Schema.AddUniform(Frequency, Name, Type, LoadParameter(JsonUniform));
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

            // Declare one uniform block per frequency that declared any field, at the register matching it.
            for (const Frequency Frequency : Enum::GetValues<Frequency>())
            {
                if (Schema.GetUniforms(Frequency).Size > 0)
                {
                    Description.Signature.Bindings[Enum::Cast(Frequency)].Append(
                        Resource::Uniform, Enum::Cast(Frequency), 1, Visibility::All);
                }
            }
        }

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

                if (const Text Definition = JsonFeature.GetString("Define"); !Definition.IsEmpty())
                {
                    if (const SInt32 Position = StrFind(Definition, '='); Position != -1)
                    {
                        Feature.Patch.Macros.Append(Definition.Slice(0, Position), Definition.Slice(Position + 1));
                    }
                    else
                    {
                        Feature.Patch.Macros.Append(Definition, Text::Empty());
                    }
                }

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
                Feature.Patch.States = Description.Base.States;

                if (const JsonObject JsonProperties = JsonFeature.GetObject("Properties"); JsonProperties.IsValid())
                {
                    Feature.Blocks = LoadProperties(JsonProperties, Feature.Patch.States, Feature.Patch.Attributes);
                }

                if (JsonFeature.GetObject("Signature").IsValid())
                {
                    LOG_W("'{0}' has feature '{1}' patching the signature, which every variant shares",
                          Scope.GetResource()->GetKey(), Feature.Name);
                }

                if (const JsonObject JsonProgram = JsonFeature.GetObject("Program"); JsonProgram.IsValid())
                {
                    LoadProgram(Service, Scope, JsonProgram, Feature.Patch.Macros, Feature.Patch.Shaders);
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

    UInt8 VFXLoader::LoadProperties(JsonObject Section, Ref<States> States, Ref<Attributes> Attributes)
    {
        UInt8 Blocks = 0;

        // Parse 'Blend' section
        if (const JsonObject JsonBlend = Section.GetObject("Blend"); JsonBlend.IsValid())
        {
            States.AlphaToCoverage     = JsonBlend.GetBool("AlphaToCoverage", States.AlphaToCoverage);
            States.Channel             = JsonBlend.GetEnum("Channel", States.Channel);
            States.BlendSrcColor       = JsonBlend.GetEnum("SrcColor", States.BlendSrcColor);
            States.BlendDstColor       = JsonBlend.GetEnum("DstColor", States.BlendDstColor);
            States.BlendEquationColor  = JsonBlend.GetEnum("EquationColor", States.BlendEquationColor);
            States.BlendSrcAlpha       = JsonBlend.GetEnum("SrcAlpha", States.BlendSrcAlpha);
            States.BlendDstAlpha       = JsonBlend.GetEnum("DstAlpha", States.BlendDstAlpha);
            States.BlendEquationAlpha  = JsonBlend.GetEnum("EquationAlpha", States.BlendEquationAlpha);

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

            Blocks = SetBit(Blocks, Technique::GetBlockMask(Technique::Block::Stencil));
        }

        // Parse 'Rasterizer' section
        if (const JsonObject JsonRasterizer = Section.GetObject("Rasterizer"); JsonRasterizer.IsValid())
        {
            States.Fill    = JsonRasterizer.GetEnum("Fill", States.Fill);
            States.Cull    = JsonRasterizer.GetEnum("Cull", States.Cull);
            States.Scissor = JsonRasterizer.GetBool("Scissor", States.Scissor);

            Blocks = SetBit(Blocks, Technique::GetBlockMask(Technique::Block::Rasterizer));
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

            Blocks = SetBit(Blocks, Technique::GetBlockMask(Technique::Block::Layout));
        }
        return Blocks;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void VFXLoader::LoadProgram(
        Ref<Content::Service>   Service,
        Ref<Content::Scope>     Scope,
        JsonObject              Section,
        Ref<Sequence<Macro>>    Macros,
        Ref<Technique::Shaders> Shaders)
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

                    Shaders[Enum::Cast(Stage)] = Service.Load<Shader>(Path, AddressOf(Scope));
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Parameter VFXLoader::LoadParameter(JsonObject JsonParameter)
    {
        switch (const Uniform Type = JsonParameter.GetEnum("Type", Uniform::Float))
        {
        case Uniform::Bool:
        {
            return JsonParameter.GetBool("Value");
        }
        case Uniform::Color:
        {
            Color Result = Color::Transparent();

            if (const ConstPtr<JsonValue> Value = JsonParameter.GetValue("Value"))
            {
                if (Value->IsString())
                {
                    Result = Color::FromHexadecimal(Value->GetString());
                }
                else
                {
                    const JsonArray Array = JsonParameter.GetArray("Value");

                    Result = Color(Array.GetNumber<Real32>(0),
                                   Array.GetNumber<Real32>(1),
                                   Array.GetNumber<Real32>(2),
                                   Array.GetNumber<Real32>(3));
                }
            }
            return Result;
        }
        case Uniform::IntColor8:
        {
            IntColor8 Result = IntColor8::Transparent();

            if (const ConstPtr<JsonValue> Value = JsonParameter.GetValue("Value"))
            {
                if (Value->IsString())
                {
                    Result = IntColor8::FromHexadecimal(Value->GetString());
                }
                else
                {
                    const JsonArray Array = JsonParameter.GetArray("Value");

                    Result = IntColor8(Array.GetNumber<UInt8>(0),
                                       Array.GetNumber<UInt8>(1),
                                       Array.GetNumber<UInt8>(2),
                                       Array.GetNumber<UInt8>(3));
                }
            }
            return Result;
        }
        case Uniform::Float:
        {
            return JsonParameter.GetNumber<Real32>("Value");
        }
        case Uniform::Float2:
        {
            Vector2 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1));
            }
            return Result;
        }
        case Uniform::Float3:
        {
            Vector3 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1), Value.GetNumber<Real32>(2));
            }
            return Result;
        }
        case Uniform::Float4:
        {
            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                return Array(Value.GetNumber<Real32>(0),
                             Value.GetNumber<Real32>(1),
                             Value.GetNumber<Real32>(2),
                             Value.GetNumber<Real32>(3));
            }
            return Array<Real32, 4>();
        }
        case Uniform::Int:
        {
            return JsonParameter.GetNumber<SInt32>("Value");
        }
        case Uniform::Int2:
        {
            IntVector2 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1));
            }
            return Result;
        }
        case Uniform::Int3:
        {
            IntVector3 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1), Value.GetNumber<SInt32>(2));
            }
            return Result;
        }
        case Uniform::Int4:
        {
            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                return Array(Value.GetNumber<SInt32>(0),
                             Value.GetNumber<SInt32>(1),
                             Value.GetNumber<SInt32>(2),
                             Value.GetNumber<SInt32>(3));
            }
            return Array<SInt32, 4>();
        }
        case Uniform::UInt:
        {
            return JsonParameter.GetNumber<UInt32>("Value");
        }
        case Uniform::UInt2:
        {
            UIntVector2 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1));
            }
            return Result;
        }
        case Uniform::UInt3:
        {
            UIntVector3 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1), Value.GetNumber<UInt32>(2));
            }
            return Result;
        }
        case Uniform::UInt4:
        {
            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                return Array(Value.GetNumber<UInt32>(0),
                             Value.GetNumber<UInt32>(1),
                             Value.GetNumber<UInt32>(2),
                             Value.GetNumber<UInt32>(3));
            }
            return Array<UInt32, 4>();
        }
        default:
        {
            LOG_W("VFXLoader: Unknown parameter type '{0}'", Type);
            break;
        }
        }
        return Parameter();
    }
}