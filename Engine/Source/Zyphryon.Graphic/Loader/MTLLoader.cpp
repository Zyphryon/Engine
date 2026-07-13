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

#include "MTLLoader.hpp"
#include "Zyphryon.Graphic/Material.hpp"
#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MTLLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Material> Asset = Retainer<Material>::Cast(Scope.GetResource());

        // Parse Json document
        JsonValue JsonDocument = JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()));
        const JsonObject JsonRoot(JsonDocument);

        // Parse 'textures' and 'samplers' section
        const JsonObject JsonImages   = JsonRoot.GetObject("Images");
        const JsonObject JsonSamplers = JsonRoot.GetObject("Samplers");

        for (const TextureSlot Slot : Enum::GetValues<TextureSlot>())
        {
            if (const JsonObject JsonImage = JsonImages.GetObject(Enum::GetName(Slot)); JsonImage.IsValid())
            {
                // Parse image
                const Text Path = JsonImage.GetString("Path");
                Asset->SetImage(Slot, Service.Load<Image>(Path, AddressOf(Scope)));

                // Parse sampler
                if (JsonSamplers.IsValid())
                {
                    const JsonObject JsonSampler = JsonSamplers.GetObject(JsonImage.GetString("Sampler"));

                    if (JsonSampler.IsValid())
                    {
                        Sampler Description;
                        Description.AddressModeU = JsonSampler.GetEnum("AddressModeU", TextureAddress::Clamp);
                        Description.AddressModeV = JsonSampler.GetEnum("AddressModeV", TextureAddress::Clamp);
                        Description.AddressModeW = JsonSampler.GetEnum("AddressModeW", TextureAddress::Clamp);
                        Description.Filter       = JsonSampler.GetEnum("Filter",       TextureFilter::Point);
                        Description.Comparison   = JsonSampler.GetEnum("Comparison",   TestCondition::Always);
                        Description.Border       = JsonSampler.GetEnum("Border",       TextureBorder::OpaqueBlack);

                        Asset->SetSampler(Slot, Description);
                    }
                }
            }
        }

        // Parse 'parameters' section
        if (const JsonObject JsonParameters = JsonRoot.GetObject("Parameters"); JsonParameters.IsValid())
        {
            for (ConstRef<JsonValue::Object::Pair> Entry : JsonParameters.GetNode()->GetObject())
            {
                const JsonObject JsonParameter = JsonParameters.GetObject(Entry.First);

                const Text Name = Entry.First;
                const Text Type = JsonParameter.GetString("Type");

                switch (Enum::Cast(Type, UniformType::Float))
                {
                case UniformType::Bool:
                {
                    Asset->SetParameter(Hash(Name), JsonParameter.GetBool("Value"));
                    break;
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

                    Asset->SetParameter(Hash(Name), Result);
                    break;
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

                    Asset->SetParameter(Hash(Name), Result);
                    break;
                }
                case UniformType::Float:
                {
                    Asset->SetParameter(Hash(Name), JsonParameter.GetNumber<Real32>("Value"));
                    break;
                }
                case UniformType::Float2:
                {
                    Vector2 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1));
                    }
                    Asset->SetParameter(Hash(Name), Result);
                    break;
                }
                case UniformType::Float3:
                {
                    Vector3 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1), Value.GetNumber<Real32>(2));
                    }
                    Asset->SetParameter(Hash(Name), Result);
                    break;
                }
                case UniformType::Float4:
                {
                    Array<Real32, 4> Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result[0] = Value.GetNumber<Real32>(0);
                        Result[1] = Value.GetNumber<Real32>(1);
                        Result[2] = Value.GetNumber<Real32>(2);
                        Result[3] = Value.GetNumber<Real32>(3);
                    }
                    Asset->SetParameter(Hash(Name), Move(Result));
                    break;
                }
                case UniformType::Int:

                {
                    Asset->SetParameter(Hash(Name), JsonParameter.GetNumber<SInt32>("Value"));
                    break;
                }
                case UniformType::Int2:
                {
                    IntVector2 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1));
                    }
                    Asset->SetParameter(Hash(Name), Result);
                    break;
                }
                case UniformType::Int3:
                {
                    IntVector3 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1), Value.GetNumber<SInt32>(2));
                    }
                    Asset->SetParameter(Hash(Name), Result);
                    break;
                }
                case UniformType::Int4:
                {
                    Array<SInt32, 4> Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result[0] = Value.GetNumber<SInt32>(0);
                        Result[1] = Value.GetNumber<SInt32>(1);
                        Result[2] = Value.GetNumber<SInt32>(2);
                        Result[3] = Value.GetNumber<SInt32>(3);
                    }
                    Asset->SetParameter(Hash(Name), Move(Result));
                    break;
                }
                case UniformType::UInt:
                {
                    Asset->SetParameter(Hash(Name), JsonParameter.GetNumber<UInt32>("Value"));
                    break;
                }
                case UniformType::UInt2:
                {
                    UIntVector2 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1));
                    }
                    Asset->SetParameter(Hash(Name), Result);
                    break;
                }
                case UniformType::UInt3:
                {
                    UIntVector3 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1), Value.GetNumber<UInt32>(2));
                    }
                    Asset->SetParameter(Hash(Name), Result);
                    break;
                }
                case UniformType::UInt4:
                {
                    Array<UInt32, 4> Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result[0] = Value.GetNumber<UInt32>(0);
                        Result[1] = Value.GetNumber<UInt32>(1);
                        Result[2] = Value.GetNumber<UInt32>(2);
                        Result[3] = Value.GetNumber<UInt32>(3);
                    }
                    Asset->SetParameter(Hash(Name), Move(Result));
                    break;
                }
                default:
                {
                    LOG_W("Unknown parameter type '{0}' for parameter '{1}'.", Type, Name);
                    break;
                }
                }
            }
        }
        return true;
    }
}