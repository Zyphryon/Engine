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

#include <Zyphryon.Base/Base.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Font
{
    /// \brief A closed span of codepoints to bake.
    struct Interval final
    {
        /// The first codepoint in the span.
        UInt32 Minimum = 0;

        /// The last codepoint in the span, inclusive.
        UInt32 Maximum = 0;
    };

    /// \brief Settings that control how a typeface is baked into the engine's native font format.
    struct Profile final
    {
        /// The em size, in pixels, the glyph fields are generated at.
        Real32 Size      = 40.0f;

        /// The width of the distance transition band, in atlas texels, which is also how far an effect such as
        /// an outline or a glow can reach before the field saturates and the effect clips.
        Real32 Range     = 20.0f;

        /// The angle, in radians, beyond which the join between two edges counts as a corner.
        Real32 Angle     = 3.0f;

        /// The total vertical space an underline occupies, in em units.
        Real32 Underline = 1.2f;

        /// The index of the typeface to read out of a collection.
        UInt32 Face      = 0;

        /// The gap left between neighbouring glyphs in the atlas, in texels.
        UInt32 Padding   = 1;

        /// The largest atlas side, in texels, beyond which a further page is opened rather than a larger sheet.
        UInt32 Limit     = 8192;

        /// The codepoint spans to bake, which need not be contiguous with one another.
        Sequence<Interval> Charset;

        /// \brief Reads these settings from a parsed command line.
        ///
        /// \param Environment The parsed command line.
        /// \return The resolved settings.
        static Profile From(ConstRef<Environment> Environment)
        {
            Profile Result;
            Result.Size      = Environment.GetNumber<Real32>("size",      Result.Size);
            Result.Range     = Environment.GetNumber<Real32>("range",     Result.Range);
            Result.Angle     = Environment.GetNumber<Real32>("angle",     Result.Angle);
            Result.Underline = Environment.GetNumber<Real32>("underline", Result.Underline);
            Result.Face      = Environment.GetNumber<UInt32>("face",      Result.Face);
            Result.Padding   = Environment.GetNumber<UInt32>("padding",   Result.Padding);
            Result.Limit     = Environment.GetNumber<UInt32>("limit",     Result.Limit);

            Parse(Environment.GetText("charset", "ascii"), Result.Charset);

            return Result;
        }

        /// \brief Reads a charset description into a list of spans.
        ///
        /// The description is a comma-separated list, where each entry is either a preset name, a single
        /// codepoint, or a `first-last` span. Numbers may be decimal or, with a `0x` prefix, hexadecimal.
        ///
        /// \param Description The charset description.
        /// \param Output      Receives one span per entry, left untouched when the description is unusable.
        /// \return `true` when every entry was understood, otherwise `false`.
        static Bool Parse(Text Description, Ref<Sequence<Interval>> Output)
        {
            Sequence<Interval> Parsed;

            for (UInt Cursor = 0; Cursor <= Description.GetSize(); )
            {
                // Take everything up to the next separator, which leaves the final entry when there is none.
                UInt Split = Cursor;

                while (Split < Description.GetSize() && Description[Split] != ',')
                {
                    ++Split;
                }

                if (const Text Entry = StrTrim(Description.Slice(Cursor, Split - Cursor)); !Entry.IsEmpty())
                {
                    if (!Append(Entry, Parsed))
                    {
                        LOG_E("Font: '{0}' is not a codepoint, a span, or a known charset", Entry);

                        return false;
                    }
                }
                Cursor = Split + 1;
            }

            if (Parsed.IsEmpty())
            {
                LOG_E("Font: the charset selects no codepoint at all");

                return false;
            }

            Output = Move(Parsed);
            return true;
        }

    private:

        /// \brief Reads one charset entry into a span.
        ///
        /// \param Entry  The entry to read.
        /// \param Output Receives the span the entry describes.
        /// \return `true` when the entry was understood, otherwise `false`.
        static Bool Append(Text Entry, Ref<Sequence<Interval>> Output)
        {
            // A preset stands for the spans a Latin bake almost always wants, which are tedious to spell out.
            if (StrEqualCaseInsensitive(Entry, "ascii"))
            {
                Output.Append(Interval(0x20, 0x7E));
                return true;
            }
            if (StrEqualCaseInsensitive(Entry, "latin1"))
            {
                Output.Append(Interval(0x20, 0x7E));
                Output.Append(Interval(0xA0, 0xFF));
                return true;
            }
            if (StrEqualCaseInsensitive(Entry, "punctuation"))
            {
                Output.Append(Interval(0x2010, 0x205E));
                return true;
            }

            // A span separates its ends with a dash, which cannot be the first character since no codepoint is
            // negative, so finding one past the start is unambiguous.
            UInt Split = 1;

            while (Split < Entry.GetSize() && Entry[Split] != '-')
            {
                ++Split;
            }

            UInt32 Minimum = 0;
            UInt32 Maximum = 0;

            if (!Read(StrTrim(Entry.Slice(0, Split)), Minimum))
            {
                return false;
            }

            if (Split >= Entry.GetSize())
            {
                Maximum = Minimum;
            }
            else if (!Read(StrTrim(Entry.Slice(Split + 1)), Maximum) || Maximum < Minimum)
            {
                return false;
            }

            Output.Append(Interval(Minimum, Maximum));
            return true;
        }

        /// \brief Reads one codepoint, in decimal or in hexadecimal behind a `0x` prefix.
        ///
        /// \param Value  The text to read.
        /// \param Output Receives the codepoint.
        /// \return `true` when the text held a codepoint, otherwise `false`.
        static Bool Read(Text Value, Ref<UInt32> Output)
        {
            if (Value.IsEmpty())
            {
                return false;
            }

            const Bool Hex   = Value.GetSize() > 2 && Value[0] == '0' && (Value[1] == 'x' || Value[1] == 'X');
            const UInt Start = Hex ? 2 : 0;

            UInt32 Result = 0;

            for (UInt Index = Start; Index < Value.GetSize(); ++Index)
            {
                const Char Digit = Value[Index];
                UInt32     Place;

                if (Digit >= '0' && Digit <= '9')
                {
                    Place = static_cast<UInt32>(Digit - '0');
                }
                else if (Hex && Digit >= 'a' && Digit <= 'f')
                {
                    Place = static_cast<UInt32>(Digit - 'a') + 10;
                }
                else if (Hex && Digit >= 'A' && Digit <= 'F')
                {
                    Place = static_cast<UInt32>(Digit - 'A') + 10;
                }
                else
                {
                    return false;
                }
                Result = Result * (Hex ? 16 : 10) + Place;
            }

            Output = Result;
            return true;
        }
    };
}
