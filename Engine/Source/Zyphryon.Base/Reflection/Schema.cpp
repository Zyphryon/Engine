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
}