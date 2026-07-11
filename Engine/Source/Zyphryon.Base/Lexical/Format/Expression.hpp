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

#include "Placeholder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Format
{
    /// \brief A parsed format expression representing either a placeholder or literal text.
    struct Expression final
    {
        /// The offset of the literal text in the original format string.
        UInt16      Offset;

        /// The length of the literal text.
        UInt16      Length;

        /// The parsed placeholder properties, active when this expression is a placeholder.
        Placeholder Properties;

        /// \brief Default constructs an empty expression.
        ZY_INLINE constexpr Expression() = default;

        /// \brief Constructs an expression from a placeholder.
        ///
        /// \param Properties The placeholder properties.
        ZY_INLINE constexpr Expression(Placeholder Properties)
            : Offset     { 0 },
              Length     { 0 },
              Properties { Properties }
        {
        }

        /// \brief Constructs an expression from literal text.
        ///
        /// \param Offset The offset of the literal text in the original format string.
        /// \param Length The length of the literal text.
        ZY_INLINE constexpr Expression(UInt16 Offset, UInt16 Length)
            : Offset { Offset },
              Length { Length }
        {
        }

        /// \brief Checks if this expression is a placeholder.
        ///
        /// \return  `true` if this expression is a placeholder, `false` if it is literal text.
        ZY_INLINE constexpr Bool IsPlaceholder() const
        {
            return (Offset == 0 && Length == 0);
        }
    };
}