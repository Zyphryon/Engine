// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Graphic/Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Describes a texture a \ref Pipeline declares and every \ref Graph realizes one of its own.
    struct Target final
    {
        /// \brief Describes how a target's dimensions track the frame's output size.
        enum class Scale : UInt8
        {
            Full,    ///< Full output resolution.
            Half,    ///< Half the output resolution in each axis.
            Quarter, ///< Quarter the output resolution in each axis.
            Fixed,   ///< Explicit, independent of the output size.
        };

        /// The pixel format of the target texture.
        Graphic::TextureFormat Format;

        /// How the target's dimensions track the frame's output size.
        Scale                  Sizing  = Scale::Full;

        /// The width, in pixels, used only when \ref Sizing is \ref Scale::Fixed.
        UInt16                 Width   = 0;

        /// The height, in pixels, used only when \ref Sizing is \ref Scale::Fixed.
        UInt16                 Height  = 0;

        /// The multisample count of the target texture.
        Graphic::Multisample   Samples = Graphic::Multisample::X1;
    };
}