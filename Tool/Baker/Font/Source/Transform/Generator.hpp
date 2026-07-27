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

#include "Shape.hpp"
#include <Zyphryon.Math/Geometry/Rect.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Font
{
    /// \brief The generated distance field of a single glyph.
    struct Field final
    {
        /// The width of the field, in texels.
        UInt32         Width  = 0;

        /// The height of the field, in texels.
        UInt32         Height = 0;

        /// The box the field covers, in pixels, with the origin on the baseline and Y pointing up.
        Rect           Bounds;

        /// The interleaved texels, four channels each, with the first row along the bottom of the glyph.
        Sequence<Byte> Texels;
    };

    /// \brief Turns a glyph outline into the multi-channel distance field the engine's text shader samples.
    ///
    /// Three channels each carry the field of a different part of the outline, so their median reproduces a
    /// corner that a single channel would round off. The fourth channel carries the plain distance, which the
    /// shader blends towards for deliberately rounded text and uses for the glow and blur effects.
    ///
    /// \note The field is produced by `msdfgen`, so this only translates the outline into that library's shape,
    ///       decides how large a cell the glyph needs, and quantizes the result.
    class Generator final
    {
    public:

        /// \brief Generates the field of one outline.
        ///
        /// \param Outline The glyph outline, in pixels.
        /// \param Range   The width of the distance transition band, in texels.
        /// \param Angle   The angle, in radians, beyond which a join counts as a corner.
        /// \param Output  Receives the generated field, left empty when the outline encloses nothing.
        static void Generate(ConstRef<Shape> Outline, Real32 Range, Real32 Angle, Ref<Field> Output);
    };
}
