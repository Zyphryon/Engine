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

#include "Output.hpp"
#include "Placeholder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyFormat
{
    /// \brief Concept that is satisfied when \p Type can be formatted into an output via an `OnFormat` method.
    template<typename Type>
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

    /// \brief Represents one value a pattern places, held without its type so the format compiles once.
    class Argument final
    {
    public:

        /// \brief Constructs an argument over a value.
        ///
        /// \param Value The value to place, which must outlive the argument unless it is a number or a flag.
        template<typename Type>
        ZY_INLINE Argument(ConstRef<Type> Value)
            : mKind    { Kind::None },
              mInteger { 0 },
              mData    { nullptr }
        {
            if constexpr (IsIntegral<Type> && !IsAnyOf<Type, Bool>)
            {
                mKind    = IsSigned<Type> ? Kind::Signed : Kind::Unsigned;
                mInteger = static_cast<UInt64>(Value);
            }
            else if constexpr (IsReal<Type>)
            {
                mKind = sizeof(Type) == sizeof(Real32) ? Kind::Real32 : Kind::Real64;
                mReal = static_cast<Real64>(Value);
            }
            else if constexpr (IsAnyOf<Type, Bool>)
            {
                mKind    = Kind::Flag;
                mInteger = Value;
            }
            else if constexpr (IsContiguousOf<Type, Char>)
            {
                mKind = Kind::Text;
                mData = Value.GetData();
                mSize = Value.GetSize();
            }
            else if constexpr (IsFormatteable<Type> || IsContiguous<Type>)
            {
                mKind  = Kind::Custom;
                mData  = AddressOf(Value);
                mRelay = AddressOf(Argument::Compose<Type>);
            }
        }

        /// \brief Writes the value into an output the way a placeholder asks.
        ///
        /// \param Buffer     The output to write to.
        /// \param Properties The placeholder giving the type, width, alignment, padding, sign and precision.
        ZY_API void Write(Output Buffer, Placeholder Properties) const;

    private:

        /// \brief Enumerates what an argument holds.
        enum class Kind : UInt8
        {
            None,     ///< Holds a value no pattern can place, which writes nothing.
            Signed,   ///< Holds a signed integer.
            Unsigned, ///< Holds an unsigned integer.
            Real32,   ///< Holds a real of single precision.
            Real64,   ///< Holds a real of double precision.
            Flag,     ///< Holds a flag.
            Text,     ///< Holds a view of characters.
            Custom,   ///< Holds the address of a value that formats itself, or of a container.
        };

        /// \brief Signature of the handler that writes a value of a type only its header knows.
        using Relay = void (*)(Output Buffer, Placeholder Properties, ConstPtr<void> Value);

        /// \brief Writes a value that formats itself, or every element of a container between its brackets.
        ///
        /// \param Buffer     The output to write to.
        /// \param Properties The placeholder every element is written by.
        /// \param Value      The address of the value, which is of \p Type.
        template<typename Type>
        static void Compose(Output Buffer, Placeholder Properties, ConstPtr<void> Value)
        {
            ConstRef<Type> Object = * static_cast<ConstPtr<Type>>(Value);

            if constexpr (IsFormatteable<Type>)
            {
                Object.OnFormat(Buffer);
            }
            else if constexpr (IsAssociative<Type>)
            {
                ConstPtr<typename Type::Pair> Data = Object.GetData();

                Buffer.Append('{');

                for (UInt Index = 0, Limit = Object.GetSize(); Index < Limit; ++Index)
                {
                    Buffer.Append(Index > 0 ? Text(", \"") : Text("\""));
                    Argument(Data[Index].First).Write(Buffer, Properties);
                    Buffer.Append("\": ");
                    Argument(Data[Index].Second).Write(Buffer, Properties);
                }

                Buffer.Append('}');
            }
            else
            {
                ConstPtr<typename Type::Element> Data = Object.GetData();

                Buffer.Append('[');

                for (UInt Index = 0, Limit = Object.GetSize(); Index < Limit; ++Index)
                {
                    if (Index > 0)
                    {
                        Buffer.Append(", ");
                    }
                    Argument(Data[Index]).Write(Buffer, Properties);
                }

                Buffer.Append(']');
            }
        }

        /// \brief Writes an integer in the base a placeholder asks for, or as the character it stands for.
        ///
        /// \param Buffer     The output to write to.
        /// \param Properties The placeholder giving the type, width, alignment, padding and sign.
        /// \param Number     The integer, in two's complement when negative.
        /// \param Negative   `true` to write the magnitude behind a minus, `false` when the integer is not negative.
        static void WriteInteger(Output Buffer, Placeholder Properties, UInt64 Number, Bool Negative);

        /// \brief Writes a real in fixed decimals under `f`, and in the significant digits its type keeps otherwise.
        ///
        /// \param Buffer     The output to write to.
        /// \param Properties The placeholder giving the type, precision, width, alignment, padding and sign.
        /// \param Number     The real to write.
        template<typename Type>
        static void WriteReal(Output Buffer, Placeholder Properties, Type Number);

        /// \brief Writes content behind its prefix, padded out to the width a placeholder asks for.
        ///
        /// \param Buffer     The output to write to.
        /// \param Properties The placeholder giving the width, alignment and padding.
        /// \param Prefix     The sign written before the content, or `\0` for none.
        /// \param Content    The digits or the text to write.
        static void WritePadded(Output Buffer, Placeholder Properties, Char Prefix, Text Content);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Kind           mKind;
        union
        {
            UInt64     mInteger;
            Real64     mReal;
            UInt       mSize;
            Relay      mRelay;
        };
        ConstPtr<void> mData;
    };
}