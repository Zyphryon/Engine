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

#include "Pattern.hpp"
#include "Zyphryon.Base/Container/Adaptor.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Format
{
    /// \brief Concept that is satisfied when \p Type can be formatted into \p Output via an `OnFormat` method.
    template<typename Type, typename Output>
    concept IsFormatteable = requires(ConstRef<Type> Value, Ref<Output> Buffer)
    {
        { Value.OnFormat(Buffer) } -> IsAnyOf<void>;
    };

    /// \brief Concept that is satisfied when \p Type is an associative container with key-value pairs.
    template<typename Type>
    concept IsAssociative = IsContiguous<Type> && requires (ConstRef<Type> Value)
    {
        typename Type::Pair;
        requires requires(typename Type::Pair Pair)
        {
            { Pair.First  };
            { Pair.Second };
        };
    };

    /// \brief Static processor that formats arguments into an output buffer according to a pattern.
    ///
    /// \tparam Type The output buffer type (must support `Append`, `AppendInteger`, `AppendReal`).
    template<typename Type>
    class Processor
    {
    public:

        /// \brief The output buffer type.
        using Output = Type;

    public:

        /// \brief Formats arguments into the buffer using the given pattern.
        ///
        /// \param Buffer     The output buffer to write to.
        /// \param Pattern    The format pattern containing literal text and placeholders.
        /// \param Parameters The arguments to substitute into the pattern.
        template<typename Input, typename... Arguments>
        ZY_INLINE static constexpr void Format(Ref<Output> Buffer, ConstRef<Input> Pattern, AnyRef<Arguments>... Parameters)
        {
            for (ConstRef<Expression> Expression : ConstSpan<Expression>(Pattern))
            {
                if (Expression.IsPlaceholder())
                {
                    AppendPlaceholder(Buffer, Expression.Properties, Expression.Properties.Parameter, Parameters...);
                }
                else
                {
                    Buffer.Append(Pattern.GetFormat().Slice(Expression.Offset, Expression.Length));
                }
            }
        }

        /// \brief Formats arguments into the buffer using the given pattern.
        ///
        /// \param Buffer  The output buffer to write to.
        /// \param Pattern The format pattern containing literal text and placeholders.
        template<typename Input>
        ZY_INLINE static constexpr void Format(Ref<Output> Buffer, ConstRef<Input> Pattern)
        {
            for (ConstRef<Expression> Expression : ConstSpan<Expression>(Pattern))
            {
                Buffer.Append(Pattern.GetFormat().Slice(Expression.Offset, Expression.Length));
            }
        }

    private:

        /// \brief Stores left and right padding counts for alignment.
        struct Padding
        {
            UInt Left;
            UInt Right;
        };

        /// \brief Computes left and right padding based on placeholder properties and required width.
        ///
        /// \param Properties The placeholder formatting properties (width, alignment, padding char).
        /// \param Required   The minimum required width of the content (excluding padding).
        /// \return A Padding struct with Left and Right padding counts.
        ZY_INLINE static constexpr Padding Compute(Placeholder Properties, UInt Required)
        {
            const UInt Width = Properties.Width;

            if (Width == 0 || Width <= Required)
            {
                return { 0, 0 };
            }

            const UInt Count = Width - Required;

            if (Properties.Padding == '0')
            {
                return { Count, 0 };
            }

            switch (Properties.Alignment)
            {
            case '<':
                return { 0, Count };
            case '^':
            {
                const UInt Left = Count / 2;
                return { Left, Count - Left };
            }
            case '>':
            default:
                return { Count, 0 };
            }
        }

        /// \brief Appends a formatted integer to the buffer with padding and prefix.
        ///
        /// \param Buffer     The output buffer.
        /// \param Properties The placeholder formatting properties.
        /// \param Prefix     The sign prefix character ('-', '+', ' ', or '\0').
        /// \param Number     The unsigned integer value to format.
        /// \param Digits     The number of digits in the value.
        /// \param Base       The numeric base (2, 10, 16).
        /// \param Uppercase  Whether to use uppercase letters for bases > 10.
        template<typename Unsigned>
        ZY_INLINE static constexpr void AppendInt(Ref<Output> Buffer, Placeholder Properties, Char Prefix, Unsigned Number, UInt Digits, UInt Base, Bool Uppercase)
        {
            const Padding Pad = Compute(Properties, (Prefix ? 1 : 0) + Digits);

            if (Properties.Padding == '0')
            {
                if (Prefix)
                {
                    Buffer.Append(Prefix);
                }
                Buffer.Append('0', Pad.Left);
                Buffer.AppendInteger(Number, Digits, Base, Uppercase);
                return;
            }

            if (Pad.Left > 0)
            {
                Buffer.Append(Properties.Padding, Pad.Left);
            }

            if (Prefix)
            {
                Buffer.Append(Prefix);
            }

            Buffer.AppendInteger(Number, Digits, Base, Uppercase);

            if (Pad.Right > 0)
            {
                Buffer.Append(Properties.Padding, Pad.Right);
            }
        }

        /// \brief Appends a formatted floating-point value to the buffer with padding and prefix.
        ///
        /// \param Buffer     The output buffer.
        /// \param Properties The placeholder formatting properties.
        /// \param Prefix     The sign prefix character ('-', '+', ' ', or '\0').
        /// \param Length     The total length of the formatted number.
        /// \param Value      The floating-point value to format.
        /// \param Precision  The number of decimal places.
        template<typename Argument>
        ZY_INLINE static constexpr void AppendReal(Ref<Output> Buffer, Placeholder Properties, Char Prefix, UInt Length, Argument Value, UInt Precision)
        {
            const Padding Pad = Compute(Properties, (Prefix ? 1 : 0) + Length);

            if (Pad.Left > 0)
            {
                Buffer.Append(Properties.Padding, Pad.Left);
            }

            if (Prefix)
            {
                Buffer.Append(Prefix);
            }

            if (Properties.Padding == '0')
            {
                Buffer.Append('0', Pad.Left);
            }

            Buffer.AppendReal(Value, Precision);

            if (Pad.Right > 0)
            {
                Buffer.Append(Properties.Padding, Pad.Right);
            }
        }

        /// \brief Appends a text string to the buffer with padding and optional prefix.
        ///
        /// \param Buffer     The output buffer.
        /// \param Properties The placeholder formatting properties.
        /// \param Prefix     The prefix character (typically '\0' for text).
        /// \param Length     The length of the text.
        /// \param Parameter  The text to append.
        template<typename Argument>
        ZY_INLINE static constexpr void AppendText(Ref<Output> Buffer, Placeholder Properties, Char Prefix, UInt Length, AnyRef<Argument> Parameter)
        {
            const Padding Pad = Compute(Properties, (Prefix ? 1 : 0) + Length);

            if (Pad.Left > 0)
            {
                Buffer.Append(Properties.Padding, Pad.Left);
            }

            if (Prefix)
            {
                Buffer.Append(Prefix);
            }

            Buffer.Append(Parameter);

            if (Pad.Right > 0)
            {
                Buffer.Append(Properties.Padding, Pad.Right);
            }
        }

        /// \brief Appends a formatted argument to the buffer based on its type.
        ///
        /// \param Buffer     The output buffer.
        /// \param Properties The placeholder formatting properties.
        /// \param Parameter  The argument to format and append.
        template<typename Argument>
        ZY_INLINE static constexpr void AppendArgument(Ref<Output> Buffer, Placeholder Properties, AnyRef<Argument> Parameter)
        {
            using Value = StripAll<Argument>;

            if constexpr (IsIntegral<Value> && !IsAnyOf<Value, Bool>)
            {
                using Unsigned = Unsigned<Value>;

                if (Properties.Type == 'c')
                {
                    AppendText(Buffer, Properties, '\0', 1, static_cast<Char>(Parameter));
                    return;
                }

                Bool IsNegative = false;

                if constexpr (IsSigned<Value>)
                {
                    IsNegative = Parameter < Value(0);
                }

                const Unsigned Number = IsNegative ? Unsigned(0) - static_cast<Unsigned>(Parameter) : static_cast<Unsigned>(Parameter);
                const Char     Prefix = IsNegative ? '-' : (Properties.Sign == '+' ? '+' : (Properties.Sign == ' ' ? ' ' : '\0'));

                switch (Properties.Type)
                {
                case 'x':
                    AppendInt(Buffer, Properties, Prefix, Number, CountDigits<16>(Number), 16, false);
                    break;
                case 'X':
                    AppendInt(Buffer, Properties, Prefix, Number, CountDigits<16>(Number), 16, true);
                    break;
                case 'b':
                    AppendInt(Buffer, Properties, Prefix, Number, CountDigits<2>(Number), 2, false);
                    break;
                case 'B':
                    AppendInt(Buffer, Properties, Prefix, Number, CountDigits<2>(Number), 2, true);
                    break;
                default:
                    AppendInt(Buffer, Properties, Prefix, Number, CountDigits<10>(Number), 10, true);
                    break;
                }
            }
            else if constexpr (IsReal<Value>)
            {
                const auto Number = Abs(Parameter);
                const UInt Length = CountDigits(Number, Properties.Precision);
                const Char Prefix = (Parameter < 0) ? '-' : (Properties.Sign == '+' ? '+' : (Properties.Sign == ' ' ? ' ' : '\0'));

                AppendReal(Buffer, Properties, Prefix, Length, Number, Properties.Precision);
            }
            else if constexpr (IsAnyOf<Value, Bool>)
            {
                if (Properties.Type == 'b' || Properties.Type == 'B')
                {
                    const Text Data = Parameter ? "True"_Text : "False"_Text;
                    AppendText(Buffer, Properties, '\0', Data.GetSize(), Data);
                }
                else
                {
                    AppendText(Buffer, Properties, '\0', 1, Parameter ? '1' : '0');
                }
            }
            else if constexpr (IsContiguousOf<Value, Char>)
            {
                AppendText(Buffer, Properties, '\0', Parameter.GetSize(), Parameter);
            }
            else if constexpr (IsFormatteable<Value, Output>)
            {
                Parameter.OnFormat(Buffer);
            }
            else if constexpr (IsAssociative<Value>)
            {
                Buffer.Append('{');

                if (const UInt Limit = Parameter.GetSize(); Limit > 0)
                {
                    ConstPtr<typename Value::Pair> Data = Parameter.GetData();

                    Buffer.Append('"');
                    AppendArgument(Buffer, Properties, Data[0].First);
                    Buffer.Append('"');
                    Buffer.Append(':');
                    Buffer.Append(' ');
                    AppendArgument(Buffer, Properties, Data[0].Second);

                    for (UInt Index = 1; Index < Limit; ++Index)
                    {
                        Buffer.Append(',');
                        Buffer.Append(' ');
                        Buffer.Append('"');
                        AppendArgument(Buffer, Properties, Data[Index].First);
                        Buffer.Append('"');
                        Buffer.Append(':');
                        Buffer.Append(' ');
                        AppendArgument(Buffer, Properties, Data[Index].Second);
                    }
                }

                Buffer.Append('}');
            }
            else if constexpr (IsContiguous<Value>)
            {
                Buffer.Append('[');

                if (const UInt Limit = Parameter.GetSize(); Limit > 0)
                {
                    ConstPtr<typename Value::Element> Data = Parameter.GetData();

                    AppendArgument(Buffer, Properties, Data[0]);

                    for (UInt Index = 1; Index < Limit; ++Index)
                    {
                        Buffer.Append(", ");
                        AppendArgument(Buffer, Properties, Data[Index]);
                    }
                }

                Buffer.Append(']');
            }
        }

        /// \brief Recursively selects and appends the argument at the specified index.
        ///
        /// \param Buffer     The output buffer.
        /// \param Properties The placeholder formatting properties.
        /// \param Index      The zero-based index of the argument to append.
        /// \param First      The first argument in the parameter pack.
        /// \param Parameters The remaining arguments.
        template<typename Argument, typename... Arguments>
        ZY_INLINE static constexpr void AppendPlaceholder(Ref<Output> Buffer, Placeholder Properties, UInt Index, AnyRef<Argument> First, AnyRef<Arguments>... Parameters)
        {
            if (Index == 0)
            {
                AppendArgument(Buffer, Properties, First);
            }
            else if constexpr (sizeof...(Parameters) > 0)
            {
                AppendPlaceholder(Buffer, Properties, Index - 1, Parameters...);
            }
        }
    };

    /// \brief Formats arguments into the buffer using the given pattern and appends a null terminator.
    ///
    /// \param Buffer     The output buffer to write to.
    /// \param Format     The format pattern containing literal text and placeholders.
    /// \param Parameters The arguments to substitute into the pattern.
    template<typename Output, UInt Capacity, typename ...Arguments>
    ZY_INLINE void Print(Ref<Output> Buffer, AnyRef<Pattern<Capacity>> Format, AnyRef<Arguments>... Parameters)
    {
        Processor<Output>::Format(Buffer, Format, Parameters...);
    }
}