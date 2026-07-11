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

#include "Expression.hpp"
#include "Zyphryon.Base/Container/Sequence.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Format
{
    /// \brief A compiled format pattern containing parsed expressions for formatting.
    ///
    /// \tparam Capacity The maximum number of expressions the pattern can hold.
    template<UInt Capacity = 24>
    class Pattern final
    {
    public:

        /// \brief Default constructs an empty pattern.
        ZY_INLINE constexpr Pattern() = default;

        /// \brief Constructs a pattern by parsing a format string.
        ///
        /// \param Format The format string to parse (e.g., "Hello {0}, value is {1:.2f}").
        ZY_INLINE constexpr Pattern(Text Format)
            : mFormat { Format }
        {
            Parse(Format);
        }

        /// \brief Gets a pointer to the parsed expressions.
        ///
        /// \return A pointer to the array of parsed expressions.
        ZY_INLINE constexpr Ptr<Expression> GetData()
        {
            return mBuffer.GetData();
        }

        /// \brief Gets a pointer to the parsed expressions.
        ///
        /// \return A pointer to the array of parsed expressions.
        ZY_INLINE constexpr ConstPtr<Expression> GetData() const
        {
            return mBuffer.GetData();
        }

        /// \brief Gets the number of parsed expressions in the pattern.
        ///
        /// \return The number of expressions.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return mBuffer.GetSize();
        }

        /// \brief Gets the maximum capacity of the pattern.
        ///
        /// \return The maximum number of expressions the pattern can hold.
        ZY_INLINE constexpr UInt GetCapacity() const
        {
            return mBuffer.GetCapacity();
        }

        /// \brief Gets the original format string.
        ///
        /// \return The original format string.
        ZY_INLINE constexpr Text GetFormat() const
        {
            return mFormat;
        }

    private:

        /// \brief Parses a format string into a sequence of expressions.
        ///
        /// \param Format The format string to parse.
        ZY_INLINE constexpr void Parse(Text Format)
        {
            UInt Cursor     = 0;
            UInt Offset     = 0;
            const UInt Size = Format.GetSize();

            while (Cursor < Size)
            {
                const SInt Braces = StrFindAny(Format, "{}", Cursor);

                if (Braces < 0)
                {
                    break;
                }

                Cursor = static_cast<UInt>(Braces);

                if (Cursor > Offset)
                {
                    mBuffer.Append(static_cast<UInt16>(Offset), static_cast<UInt16>(Cursor - Offset));
                }

                if (Format[Cursor] == '{')
                {
                    if (Cursor + 1 < Size && Format[Cursor + 1] == '{')
                    {
                        mBuffer.Append(static_cast<UInt16>(Cursor), 1);
                        Cursor += 2;
                        Offset  = Cursor;
                        continue;
                    }

                    const Text Specification = StrExtractText(Format, ++Cursor, '}');

                    if (!StrConsume(Format, Cursor, '}') || Specification.IsEmpty())
                    {
                        mBuffer.Append(static_cast<UInt16>(Cursor - 1), 1);
                    }
                    else
                    {
                        mBuffer.Append(Placeholder(Specification));
                    }

                    Offset = Cursor;
                }
                else
                {
                    mBuffer.Append(static_cast<UInt16>(Cursor), 1);

                    if (Cursor + 1 < Size && Format[Cursor + 1] == '}')
                    {
                        Cursor += 2;
                    }
                    else
                    {
                        ++Cursor;
                    }
                    Offset = Cursor;
                }
            }

            if (Offset < Size)
            {
                mBuffer.Append(static_cast<UInt16>(Offset), static_cast<UInt16>(Size - Offset));
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Text                           mFormat;
        Sequence<Expression, Capacity> mBuffer;
    };
}