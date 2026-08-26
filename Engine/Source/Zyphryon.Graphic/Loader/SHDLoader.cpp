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

#include "SHDLoader.hpp"
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Graphic/Shader.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Text ParseInclude(Text Line)
    {
        const Text Body = StrTrim(Line);

        if (!StrStartsWith(Body, "#include"))
        {
            return Text::Empty();
        }

        const SInt Opening = StrFind(Body, '"');

        if (Opening < 0)
        {
            return Text::Empty();
        }

        const Text Quoted  = Body.Slice(static_cast<UInt>(Opening) + 1);
        const SInt Closing = StrFind(Quoted, '"');

        return (Closing < 0 ? Text::Empty() : Quoted.Slice(0, static_cast<UInt>(Closing)));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool SHDLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Shader> Asset = Retainer<Shader>::Cast(Scope.GetResource());

        const Text Source(Data.GetData<Char>(), Data.GetSize());

        StrSplit(Source, '\n', [&](Text Line)
        {
            if (const Text Path = ParseInclude(Line); !Path.IsEmpty())
            {
                const Retainer<Shader> Module = Service.Load<Shader>(Content::Uri(Path), AddressOf(Scope));

                Asset->AddInclude(Module,
                    static_cast<UInt32>(Line.GetData() - Source.GetData()),
                    static_cast<UInt32>(Line.GetSize()));
            }
        });

        Asset->SetSource(Move(Data));
        return true;
    }
}