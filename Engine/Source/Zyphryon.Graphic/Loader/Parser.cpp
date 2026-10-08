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

#include "Parser.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Parameter ParseParameter(ConstRef<JsonObject> Json)
    {
        switch (Json.GetEnum("Type", Uniform::Float))
        {
        case Uniform::Bool:
        {
            return Json.GetBool("Value");
        }
        case Uniform::Color:
        {
            Color Result = Color::Transparent();

            if (const ConstPtr<JsonValue> Value = Json.GetValue("Value"))
            {
                if (Value->IsString())
                {
                    Result = Color::FromHexadecimal(Value->GetString());
                }
                else
                {
                    const JsonArray Array = Json.GetArray("Value");

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

            if (const ConstPtr<JsonValue> Value = Json.GetValue("Value"))
            {
                if (Value->IsString())
                {
                    Result = IntColor8::FromHexadecimal(Value->GetString());
                }
                else
                {
                    const JsonArray Array = Json.GetArray("Value");

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
            return Json.GetNumber<Real32>("Value");
        }
        case Uniform::Float2:
        {
            Vector2 Result;

            if (const JsonArray Value = Json.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1));
            }
            return Result;
        }
        case Uniform::Float3:
        {
            Vector3 Result;

            if (const JsonArray Value = Json.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<Real32>(0), Value.GetNumber<Real32>(1), Value.GetNumber<Real32>(2));
            }
            return Result;
        }
        case Uniform::Float4:
        {
            if (const JsonArray Value = Json.GetArray("Value"); !Value.IsNullOrEmpty())
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
            return Json.GetNumber<SInt32>("Value");
        }
        case Uniform::Int2:
        {
            IntVector2 Result;

            if (const JsonArray Value = Json.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1));
            }
            return Result;
        }
        case Uniform::Int3:
        {
            IntVector3 Result;

            if (const JsonArray Value = Json.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<SInt32>(0), Value.GetNumber<SInt32>(1), Value.GetNumber<SInt32>(2));
            }
            return Result;
        }
        case Uniform::Int4:
        {
            if (const JsonArray Value = Json.GetArray("Value"); !Value.IsNullOrEmpty())
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
            return Json.GetNumber<UInt32>("Value");
        }
        case Uniform::UInt2:
        {
            UIntVector2 Result;

            if (const JsonArray Value = Json.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1));
            }
            return Result;
        }
        case Uniform::UInt3:
        {
            UIntVector3 Result;

            if (const JsonArray Value = Json.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                Result.Set(Value.GetNumber<UInt32>(0), Value.GetNumber<UInt32>(1), Value.GetNumber<UInt32>(2));
            }
            return Result;
        }
        case Uniform::UInt4:
        {
            if (const JsonArray Value = Json.GetArray("Value"); !Value.IsNullOrEmpty())
            {
                return Array(Value.GetNumber<UInt32>(0),
                             Value.GetNumber<UInt32>(1),
                             Value.GetNumber<UInt32>(2),
                             Value.GetNumber<UInt32>(3));
            }
            return Array<UInt32, 4>();
        }
        }
        return Parameter();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sampler ParseSampler(ConstRef<JsonObject> Json, ConstRef<Sampler> Defaults)
    {
        Sampler Result;
        Result.AddressModeU = Json.GetEnum("AddressModeU", Defaults.AddressModeU);
        Result.AddressModeV = Json.GetEnum("AddressModeV", Defaults.AddressModeV);
        Result.AddressModeW = Json.GetEnum("AddressModeW", Defaults.AddressModeW);
        Result.Filter       = Json.GetEnum("Filter",       Defaults.Filter);
        Result.Comparison   = Json.GetEnum("Comparison",   Defaults.Comparison);
        Result.Border       = Json.GetEnum("Border",       Defaults.Border);
        return Result;
    }
}