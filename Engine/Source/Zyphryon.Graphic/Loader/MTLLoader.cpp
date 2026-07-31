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

        if (!JsonRoot.IsValid())
        {
            LOG_W("'{0}' is not a valid material document", Scope.GetResource()->GetKey());
            return false;
        }

        Parse(Service, Scope, JsonRoot, * Asset);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void MTLLoader::Parse(Ref<Content::Service> Service, Ref<Content::Scope> Scope, ConstRef<JsonObject> Root, Ref<Material> Asset)
    {
        // Parse 'Images' section, keyed by the texture name the technique declares.
        if (const JsonObject JsonImages = Root.GetObject("Images"); JsonImages.IsValid())
        {
            for (ConstRef<JsonValue::Object::Pair> Entry : JsonImages.GetNode()->GetObject())
            {
                const JsonObject JsonImage = JsonImages.GetObject(Entry.First);
                const Text       Path      = JsonImage.GetString("Path");

                Asset.SetImage(Hash(Entry.First), Service.Load<Image>(Path, AddressOf(Scope)));
            }
        }

        // Parse 'Samplers' section, keyed by the sampler name the technique declares.
        if (const JsonObject JsonSamplers = Root.GetObject("Samplers"); JsonSamplers.IsValid())
        {
            for (ConstRef<JsonValue::Object::Pair> Entry : JsonSamplers.GetNode()->GetObject())
            {
                const JsonObject JsonSampler = JsonSamplers.GetObject(Entry.First);

                Sampler Descriptor;
                Descriptor.AddressModeU = JsonSampler.GetEnum("AddressModeU", TextureAddress::Clamp);
                Descriptor.AddressModeV = JsonSampler.GetEnum("AddressModeV", TextureAddress::Clamp);
                Descriptor.AddressModeW = JsonSampler.GetEnum("AddressModeW", TextureAddress::Clamp);
                Descriptor.Filter       = JsonSampler.GetEnum("Filter",       TextureFilter::Point);
                Descriptor.Comparison   = JsonSampler.GetEnum("Comparison",   TestCondition::None);
                Descriptor.Border       = JsonSampler.GetEnum("Border",       TextureBorder::OpaqueBlack);

                Asset.SetSampler(Hash(Entry.First), Descriptor);
            }
        }

        // Parse 'Parameters' section
        if (const JsonObject JsonParameters = Root.GetObject("Parameters"); JsonParameters.IsValid())
        {
            for (ConstRef<JsonValue::Object::Pair> Entry : JsonParameters.GetNode()->GetObject())
            {
                const JsonObject JsonParameter = JsonParameters.GetObject(Entry.First);

                const Text Name = Entry.First;
                const Text Type = JsonParameter.GetString("Type");

                switch (Enum::Cast(Type, Uniform::Float))
                {
                case Uniform::Bool:
                {
                    Asset.SetParameter(Hash(Name), JsonParameter.GetBool("Value"));
                    break;
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

                    Asset.SetParameter(Hash(Name), Result);
                    break;
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

                    Asset.SetParameter(Hash(Name), Result);
                    break;
                }
                case Uniform::Float:
                {
                    Asset.SetParameter(Hash(Name), JsonParameter.GetNumber<Real32>("Value"));
                    break;
                }
                case Uniform::Float2:
                {
                    Vector2 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1));
                    }
                    Asset.SetParameter(Hash(Name), Result);
                    break;
                }
                case Uniform::Float3:
                {
                    Vector3 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1), Value.GetNumber<Real32>(2));
                    }
                    Asset.SetParameter(Hash(Name), Result);
                    break;
                }
                case Uniform::Float4:
                {
                    Array<Real32, 4> Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result[0] = Value.GetNumber<Real32>(0);
                        Result[1] = Value.GetNumber<Real32>(1);
                        Result[2] = Value.GetNumber<Real32>(2);
                        Result[3] = Value.GetNumber<Real32>(3);
                    }
                    Asset.SetParameter(Hash(Name), Move(Result));
                    break;
                }
                case Uniform::Int:

                {
                    Asset.SetParameter(Hash(Name), JsonParameter.GetNumber<SInt32>("Value"));
                    break;
                }
                case Uniform::Int2:
                {
                    IntVector2 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1));
                    }
                    Asset.SetParameter(Hash(Name), Result);
                    break;
                }
                case Uniform::Int3:
                {
                    IntVector3 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1), Value.GetNumber<SInt32>(2));
                    }
                    Asset.SetParameter(Hash(Name), Result);
                    break;
                }
                case Uniform::Int4:
                {
                    Array<SInt32, 4> Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result[0] = Value.GetNumber<SInt32>(0);
                        Result[1] = Value.GetNumber<SInt32>(1);
                        Result[2] = Value.GetNumber<SInt32>(2);
                        Result[3] = Value.GetNumber<SInt32>(3);
                    }
                    Asset.SetParameter(Hash(Name), Move(Result));
                    break;
                }
                case Uniform::UInt:
                {
                    Asset.SetParameter(Hash(Name), JsonParameter.GetNumber<UInt32>("Value"));
                    break;
                }
                case Uniform::UInt2:
                {
                    UIntVector2 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1));
                    }
                    Asset.SetParameter(Hash(Name), Result);
                    break;
                }
                case Uniform::UInt3:
                {
                    UIntVector3 Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1), Value.GetNumber<UInt32>(2));
                    }
                    Asset.SetParameter(Hash(Name), Result);
                    break;
                }
                case Uniform::UInt4:
                {
                    Array<UInt32, 4> Result;

                    if (const JsonArray Value = JsonParameter.GetArray("Value"); !Value.IsNullOrEmpty())
                    {
                        Result[0] = Value.GetNumber<UInt32>(0);
                        Result[1] = Value.GetNumber<UInt32>(1);
                        Result[2] = Value.GetNumber<UInt32>(2);
                        Result[3] = Value.GetNumber<UInt32>(3);
                    }
                    Asset.SetParameter(Hash(Name), Move(Result));
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
    }
}