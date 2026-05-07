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

#include "Filesystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Filesystem
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool CopyAll(ConstStr8 Source, ConstStr8 Destination)
    {
        if (Make(Destination))
        {
            constexpr auto OnEnumerate = [](Ptr<void> Context, ConstPtr<Char> Dirname, ConstPtr<Char> Filename)
            {
                if (Filename[0] == '.' && (Filename[1] == '\0' || (Filename[1] == '.' && Filename[2] == '\0')))
                {
                    return SDL_ENUM_CONTINUE;
                }

                const ConstPtr<Char> Path = static_cast<ConstPtr<Char>>(Context);

                const Str8 Current = Str8(Dirname) + "/" + Filename;
                const Str8 Target  = Str8(Path)    + "/" + Filename;

                SDL_PathInfo Info;
                if (!SDL_GetPathInfo(Current.data(), &Info))
                {
                    return SDL_ENUM_FAILURE;
                }

                if (Info.type == SDL_PATHTYPE_DIRECTORY)
                {
                    if (!CopyAll(Current, Target))
                    {
                        return SDL_ENUM_FAILURE;
                    }
                }
                else if (Info.type == SDL_PATHTYPE_FILE)
                {
                    if (!Copy(Current, Target))
                    {
                        return SDL_ENUM_FAILURE;
                    }
                }

                return SDL_ENUM_CONTINUE;
            };

            return SDL_EnumerateDirectory(Source.data(), OnEnumerate, const_cast<Ptr<Char>>(Destination.data()));
        }
        return false;
    }
}