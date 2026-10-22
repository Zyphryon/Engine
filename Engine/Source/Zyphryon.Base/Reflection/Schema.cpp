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

#include "Schema.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReflection
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Schema::Decode(ConstRef<JsonValue> Fields, Ptr<void> Instance) const
    {
        if (!Fields.IsObject())
        {
            return Fields.IsNull();
        }

        Bool Whole = true;

        for (ConstRef<Field> Each : GetFields())
        {
            const ConstPtr<JsonValue> Given = Fields.GetObject().Find(Each.GetName());

            if (Given == nullptr)
            {
                continue;
            }

            Bool Written = false;

            switch (Each.GetKind())
            {
            case Kind::Boolean:
                if ((Written = Given->IsBool()))
                {
                    Each.Write(Instance, Value::FromBoolean(Given->GetBool()));
                }
                break;
            case Kind::Integer:
                if ((Written = Given->IsNumber()))
                {
                    Each.Write(Instance, Value::FromInteger(Kind::Integer, Given->GetNumber<SInt64>()));
                }
                break;
            case Kind::Real:
                if ((Written = Given->IsNumber()))
                {
                    Each.Write(Instance, Value::FromReal(Given->GetNumber<Real64>()));
                }
                break;
            case Kind::Text:
                if ((Written = Given->IsString()))
                {
                    Each.Write(Instance, Value::FromText(Given->GetString()));
                }
                break;
            case Kind::Enumeration:
                if (Given->IsString())
                {
                    for (ConstRef<Option> Choice : Each.GetOptions())
                    {
                        if ((Written = (Choice.Name == Given->GetString())))
                        {
                            Each.Write(Instance, Value::FromInteger(Kind::Enumeration, Choice.Content));
                            break;
                        }
                    }
                }
                break;
            default:
                break;
            }
            Whole = Whole && Written;
        }
        return Whole;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Schema::Write(Ptr<void> Instance, Ref<Writer> Output) const
    {
        for (ConstRef<Field> Each : GetFields())
        {
            const Value Content = Each.Read(Instance);

            switch (Each.GetKind())
            {
            case Kind::Boolean:
                Output.Write<UInt8>(Content.GetBoolean() ? 1 : 0);
                break;
            case Kind::Integer:
            case Kind::Enumeration:
                Output.WriteInt<SInt64>(Content.GetInteger());
                break;
            case Kind::Real:
                Output.Write<Real32>(static_cast<Real32>(Content.GetReal()));
                break;
            case Kind::Text:
                Output.WriteText(Content.GetText());
                break;
            case Kind::Structure:
                if (!Of(Each).Write(Content.GetAddress(), Output))
                {
                    return false;
                }
                break;
            default:
                return false;
            }
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Schema::Read(Ref<Reader> Input, Ptr<void> Instance) const
    {
        const auto Bounded = [](ConstRef<Field> Each, Real64 Number)
        {
            return Each.GetMinimum() >= Each.GetMaximum() || (Number >= Each.GetMinimum() && Number <= Each.GetMaximum());
        };

        for (ConstRef<Field> Each : GetFields())
        {
            Value Content;

            switch (Each.GetKind())
            {
            case Kind::Boolean:
                Content = Value::FromBoolean(Input.Read<UInt8>() != 0);
                break;
            case Kind::Integer:
            {
                const SInt64 Number = Input.ReadInt<SInt64>();

                if (!Bounded(Each, static_cast<Real64>(Number)))
                {
                    return false;
                }
                Content = Value::FromInteger(Kind::Integer, Number);
                break;
            }
            case Kind::Enumeration:
            {
                const SInt64 Number = Input.ReadInt<SInt64>();
                Bool         Named  = false;

                for (ConstRef<Option> Choice : Each.GetOptions())
                {
                    Named = Named || Choice.Content == Number;
                }

                if (!Named)
                {
                    return false;
                }
                Content = Value::FromInteger(Kind::Enumeration, Number);
                break;
            }
            case Kind::Real:
            {
                const Real32 Number = Input.Read<Real32>();

                if (!(Abs(Number) <= kMaximum<Real32>) || !Bounded(Each, Number))
                {
                    return false;
                }
                Content = Value::FromReal(Number);
                break;
            }
            case Kind::Text:
                Content = Value::FromText(Input.ReadText());
                break;
            case Kind::Structure:
                if (!Of(Each).Read(Input, Each.Read(Instance).GetAddress()))
                {
                    return false;
                }
                continue;
            default:
                return false;
            }
            Each.Write(Instance, Content);
        }
        return true;
    }
}