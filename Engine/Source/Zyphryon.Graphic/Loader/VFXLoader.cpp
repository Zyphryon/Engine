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
#include "Zyphryon.Graphic/Technique.hpp"
#include "Zyphryon.Content/Service.hpp"

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

        JsonValue JsonDocument = JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()));
        const JsonObject JsonRoot(JsonDocument);

        // Parse 'Properties' section
        if (const JsonObject JsonProperties = JsonRoot.GetObject("Properties"); JsonProperties.IsValid())
        {
            // Parse 'Blend' section
            if (const JsonObject JsonBlend = JsonProperties.GetObject("Blend"); JsonBlend.IsValid())
            {
                Description.States.AlphaToCoverage     = JsonBlend.GetBool("AlphaToCoverage", false);
                Description.States.Channel             = JsonBlend.GetEnum("Channel", Channel::RGBA);
                Description.States.BlendSrcColor       = JsonBlend.GetEnum("SrcColor", BlendFactor::One);
                Description.States.BlendDstColor       = JsonBlend.GetEnum("DstColor", BlendFactor::Zero);
                Description.States.BlendEquationColor  = JsonBlend.GetEnum("EquationColor", BlendFunction::Add);
                Description.States.BlendSrcAlpha       = JsonBlend.GetEnum("SrcAlpha", BlendFactor::One);
                Description.States.BlendDstAlpha       = JsonBlend.GetEnum("DstAlpha", BlendFactor::Zero);
                Description.States.BlendEquationAlpha  = JsonBlend.GetEnum("EquationAlpha", BlendFunction::Add);
            }

            // Parse 'Depth' section
            if (const JsonObject JsonDepth = JsonProperties.GetObject("Depth"); JsonDepth.IsValid())
            {
                Description.States.DepthClip      = JsonDepth.GetBool("Clip", true);
                Description.States.DepthMask      = JsonDepth.GetBool("Mask", true);
                Description.States.DepthTest      = JsonDepth.GetEnum("Condition", TestCondition::LessEqual);
                Description.States.DepthBias      = JsonDepth.GetNumber<Real32>("Bias", 0.0f);
                Description.States.DepthBiasClamp = JsonDepth.GetNumber<Real32>("BiasClamp", 0.0f);
                Description.States.DepthBiasSlope = JsonDepth.GetNumber<Real32>("BiasSlope", 0.0f);
            }

            // Parse 'Stencil' section
            if (const JsonObject JsonStencil = JsonProperties.GetObject("Stencil"); JsonStencil.IsValid())
            {
                Description.States.StencilReadMask       = JsonStencil.GetNumber<UInt8>("ReadMask", 0);
                Description.States.StencilWriteMask      = JsonStencil.GetNumber<UInt8>("WriteMask", 0);
                Description.States.StencilBackTest       = JsonStencil.GetEnum("BackTest", TestCondition::Always);
                Description.States.StencilBackFail       = JsonStencil.GetEnum("BackFail", TestAction::Keep);
                Description.States.StencilBackDepthFail  = JsonStencil.GetEnum("BackDepthFail", TestAction::Keep);
                Description.States.StencilBackDepthPass  = JsonStencil.GetEnum("BackDepthPass", TestAction::Keep);
                Description.States.StencilFrontTest      = JsonStencil.GetEnum("FrontTest", TestCondition::Always);
                Description.States.StencilFrontFail      = JsonStencil.GetEnum("FrontFail", TestAction::Keep);
                Description.States.StencilFrontDepthFail = JsonStencil.GetEnum("FrontDepthFail", TestAction::Keep);
                Description.States.StencilFrontDepthPass = JsonStencil.GetEnum("FrontDepthPass", TestAction::Keep);
            }

            // Parse 'Rasterizer' section
            if (const JsonObject JsonRasterizer = JsonProperties.GetObject("Rasterizer"); JsonRasterizer.IsValid())
            {
                Description.States.Fill    = JsonRasterizer.GetEnum("Fill", Fill::Solid);
                Description.States.Cull    = JsonRasterizer.GetEnum("Cull", Cull::Back);
                Description.States.Scissor = JsonRasterizer.GetBool("Scissor", false);
            }

            // Parse 'Layout' section
            if (const JsonObject JsonLayout = JsonProperties.GetObject("Layout"); JsonLayout.IsValid())
            {
                if (const JsonArray JsonAttributes = JsonLayout.GetArray("Attributes"); !JsonAttributes.IsNullOrEmpty())
                {
                    for (UInt Index = 0, Size = JsonAttributes.GetSize(); Index < Size; ++Index)
                    {
                        const JsonArray Values = JsonAttributes.GetArray(Index);

                        Ref<Attribute> Attribute = Description.States.InputAttributes.Append();
                        Attribute.Location = Values.GetNumber(0, 0);
                        Attribute.Format   = Values.GetEnum(1, VertexFormat::Float32x4);
                        Attribute.Stream   = Values.GetNumber<UInt32>(2);
                        Attribute.Offset   = Values.GetNumber<UInt32>(3);
                        Attribute.Divisor  = Values.GetNumber<UInt32>(4);
                    }
                }

                Description.States.InputPrimitive = JsonLayout.GetEnum("Primitive", Primitive::TriangleList);
            }
        }

        // Parse 'Signature' section
        if (const JsonObject JsonProperties = JsonRoot.GetObject("Signature"); JsonProperties.IsValid())
        {
            // Parse 'Textures' section
            if (const JsonArray JsonTextures = JsonProperties.GetArray("Textures"); !JsonTextures.IsNullOrEmpty())
            {
                for (UInt Index = 0, Limit = JsonTextures.GetSize(); Index < Limit; ++Index)
                {
                    Description.Schema.AddTexture(Enum::Cast(JsonTextures.GetString(Index), TextureSlot::Albedo));
                }
            }

            // Parse 'Samplers' section
            if (const JsonArray JsonSamplers = JsonProperties.GetArray("Samplers"); !JsonSamplers.IsNullOrEmpty())
            {
                for (UInt Index = 0, Limit = JsonSamplers.GetSize(); Index < Limit; ++Index)
                {
                    const JsonObject JsonSampler = JsonSamplers.GetObject(Index);

                    const TextureSlot Slot = JsonSampler.GetEnum("Slot", TextureSlot::Albedo);

                    Sampler Sampler;
                    Sampler.AddressModeU = JsonSampler.GetEnum("AddressModeU", TextureAddress::Clamp);
                    Sampler.AddressModeV = JsonSampler.GetEnum("AddressModeV", TextureAddress::Clamp);
                    Sampler.AddressModeW = JsonSampler.GetEnum("AddressModeW", TextureAddress::Clamp);
                    Sampler.Filter       = JsonSampler.GetEnum("Filter",       TextureFilter::LinearMipLinear);
                    Sampler.Comparison   = JsonSampler.GetEnum("Comparison",   TestCondition::Always);
                    Sampler.Border       = JsonSampler.GetEnum("Border",       TextureBorder::OpaqueBlack);

                    Description.Schema.AddSampler(Slot, Sampler);
                }
            }

            // Parse 'Uniforms' section
            if (const JsonArray JsonUniforms = JsonProperties.GetArray("Uniforms"); !JsonUniforms.IsNullOrEmpty())
            {
                for (UInt Index = 0, Limit = JsonUniforms.GetSize(); Index < Limit; ++Index)
                {
                    const JsonObject JsonUniform = JsonUniforms.GetObject(Index);

                    const Text         Name = JsonUniform.GetString("Name");
                    const UniformScope Slot = JsonUniform.GetEnum("Slot", UniformScope::Global);
                    const UniformType  Type = JsonUniform.GetEnum("Type", UniformType::Float);

                    if (const UInt32 Count = JsonUniform.GetNumber<UInt32>("Count", 1); Count == 1)
                    {
                        Description.Schema.AddUniform(Slot, Name, Type, LoadParameter(JsonUniform));
                    }
                    else
                    {
                        for (UInt32 Element = 0; Element < Count; ++Element)
                        {
                            Str64 Buffer(Name);
                            Buffer.Append('[');
                            Buffer.AppendInteger(Element, CountDigits<10>(Element), 10, false);
                            Buffer.Append(']');

                            Description.Schema.AddUniform(Slot, Buffer, Type, Parameter());
                        }
                    }
                }
            }
        }

        // Parse 'Program' section
        if (const JsonObject JsonProgram = JsonRoot.GetObject("Program"); JsonProgram.IsValid())
        {
            // Parse 'Defines' section
            if (const JsonArray JsonDefines = JsonProgram.GetArray("Defines"); !JsonDefines.IsNullOrEmpty())
            {
                for (UInt Index = 0, Limit = JsonDefines.GetSize(); Index < Limit; ++Index)
                {
                    const Text Definition = JsonDefines.GetString(Index);

                    if (const SInt32 Position = StrFind(Definition, '='); Position != -1)
                    {
                        Description.Macros.Append(Definition.Slice(0, Position), Definition.Slice(Position + 1));
                    }
                    else
                    {
                        Description.Macros.Append(Definition, Text::Empty());
                    }
                }
            }

            // Parse 'Shaders' section
            if (const JsonObject JsonShaders = JsonProgram.GetObject("Shaders"); JsonShaders.IsValid())
            {
                if (const JsonArray JsonStages = JsonShaders.GetArray(mLanguage); !JsonStages.IsNullOrEmpty())
                {
                    for (UInt Index = 0, Limit = JsonStages.GetSize(); Index < Limit; ++Index)
                    {
                        const JsonObject JsonShader = JsonStages.GetObject(Index);

                        const Text        Path  = JsonShader.GetString("Path");
                        const ShaderStage Stage = JsonShader.GetEnum("Stage", ShaderStage::Vertex);

                        Description.Shaders[Enum::Cast(Stage)] = Service.Load<Shader>(Path, AddressOf(Scope));
                    }
                }
            }
        }

        const Retainer<Technique> Asset = Retainer<Technique>::Cast(Scope.GetResource());
        Asset->Setup(Move(Description));
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Parameter VFXLoader::LoadParameter(JsonObject JsonParameter)
    {
        switch (const UniformType Type = JsonParameter.GetEnum("Type", UniformType::Float))
        {
        case UniformType::Bool:
        {
            return JsonParameter.GetBool("Value");
        }
        case UniformType::Color:
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
        case UniformType::IntColor8:
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
        case UniformType::Float:
        {
            return JsonParameter.GetNumber<Real32>("Value");
        }
        case UniformType::Float2:
        {
            Vector2 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1));
            }
            return Result;
        }
        case UniformType::Float3:
        {
            Vector3 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1), Value.GetNumber<Real32>(2));
            }
            return Result;
        }
        case UniformType::Float4:
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
        case UniformType::Int:
        {
            return JsonParameter.GetNumber<SInt32>("Value");
        }
        case UniformType::Int2:
        {
            IntVector2 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1));
            }
            return Result;
        }
        case UniformType::Int3:
        {
            IntVector3 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1), Value.GetNumber<SInt32>(2));
            }
            return Result;
        }
        case UniformType::Int4:
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
        case UniformType::UInt:
        {
            return JsonParameter.GetNumber<UInt32>("Value");
        }
        case UniformType::UInt2:
        {
            UIntVector2 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1));
            }
            return Result;
        }
        case UniformType::UInt3:
        {
            UIntVector3 Result;

            if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1), Value.GetNumber<UInt32>(2));
            }
            return Result;
        }
        case UniformType::UInt4:
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
            LOG_W("Unknown parameter type '{0}'", Type);
            break;
        }
        }
        return Parameter();
    }
}