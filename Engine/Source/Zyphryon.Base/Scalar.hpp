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

#include "Bit.hpp"
#include <cmath>
#include <limits>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Mathematical constant representing the value of pi.
    template<typename Type>
    inline constexpr Type kPI       = Type(3.141592653589793238462643383279502884e+00);

    /// \brief Mathematical constant representing the minimum finite value for the given type.
    template<typename Type>
    inline constexpr Type kMinimum  = std::numeric_limits<Type>::min();

    /// \brief Mathematical constant representing the maximum finite value for the given type.
    template<typename Type>
    inline constexpr Type kMaximum  = std::numeric_limits<Type>::max();

    /// \brief Mathematical constant representing the machine epsilon (smallest difference from 1.0).
    template<typename Type>
    inline constexpr Type kEpsilon  = std::numeric_limits<Type>::epsilon();

    /// \brief Mathematical constant representing positive infinity.
    template<typename Type>
    inline constexpr Type kInfinity = std::numeric_limits<Type>::infinity();

    /// \brief Computes the absolute value of the given number.
    ///
    /// \param Value The value to compute the absolute for.
    /// \return A value equal to |\p Value|.
    template<typename Type>
    constexpr Type Abs(Type Value)
        requires IsSigned<Type>
    {
        return Value < Type(0) ? -Value : Value;
    }

    /// \brief Computes the largest integer less than or equal to the given value.
    ///
    /// \param Value The value to floor.
    /// \return The largest integer less than or equal to \a Value.
    template<typename Type>
    ZY_INLINE Type Floor(Type Value)
        requires IsReal<Type>
    {
        return std::floor(Value);
    }

    /// \brief Computes the smallest integer greater than or equal to the given value.
    ///
    /// \param Value The value to ceil.
    /// \return The smallest integer greater than or equal to \a Value.
    template<typename Type>
    ZY_INLINE Type Ceil(Type Value)
        requires IsReal<Type>
    {
        return std::ceil(Value);
    }

    /// \brief Rounds the given value to the nearest integer.
    ///
    /// \param Value The value to round.
    /// \return The nearest integer to \a Value.
    template<typename Type>
    ZY_INLINE Type Round(Type Value)
        requires IsReal<Type>
    {
        return std::round(Value);
    }

    /// \brief Computes the remainder of the division of two real numbers.
    ///
    /// \param Dividend The dividend value.
    /// \param Divisor  The divisor value.
    /// \return The remainder of the division of \a Dividend by \a Divisor.
    template<typename Type>
    ZY_INLINE Type Remainder(Type Dividend, Type Divisor)
        requires(IsReal<Type>)
    {
        ZY_ASSERT(Divisor != 0, "Divisor cannot be zero");

        return std::fmod(Dividend, Divisor);
    }

    /// \brief Computes the power of a base raised to an exponent for real numbers.
    ///
    /// \param Base     The base value.
    /// \param Exponent The exponent value.
    /// \return The result of raising \p Base to the power of \p Exponent.
    template<typename Type>
    ZY_INLINE Type Pow(Type Base, Type Exponent)
        requires IsReal<Type>
    {
        return std::pow(Base, Exponent);
    }

    /// \brief Computes the power of a base raised to an exponent for integer numbers.
    ///
    /// \param Base     The base value.
    /// \param Exponent The exponent value.
    /// \return The result of raising \p Base to the power of \p Exponent.
    template<typename Type, typename Power>
    constexpr UInt Pow(Type Base, Power Exponent)
        requires (IsIntegral<Type> && IsUnsigned<Power>)
    {
        return Exponent == 0 ? Type(1) : (Exponent & 1 ? Base : 1) * Pow(Base * Base, Exponent >> 1);
    }

    /// \brief Computes 2 raised to the power of the given exponent.
    ///
    /// \param Exponent The exponent value.
    /// \return The result of 2 ^ \p Exponent.
    template<typename Type>
    constexpr UInt Pow2(Type Exponent)
        requires (IsIntegral<Type>)
    {
        return Type(1) << Exponent;
    }

    /// \brief Computes 10 raised to the power of the given exponent.
    ///
    /// \param Exponent The exponent value.
    /// \return The result of 10 ^ \p Exponent.
    template<typename Type>
    constexpr UInt Pow10(Type Exponent)
        requires IsIntegral<Type>
    {
        static constexpr UInt kPow10Table[20] =
        {
            1ull,                   10ull,                  100ull,
            1000ull,                10000ull,               100000ull,
            1000000ull,             10000000ull,            100000000ull,
            1000000000ull,          10000000000ull,         100000000000ull,
            1000000000000ull,       10000000000000ull,      100000000000000ull,
            1000000000000000ull,    10000000000000000ull,   100000000000000000ull,
            1000000000000000000ull, 10000000000000000000ull
        };
        return kPow10Table[Exponent];
    }

    /// \brief Computes the square root of the given number.
    ///
    /// \param Value The number to compute the square root for.
    /// \return The square root of \a Value.
    template<typename Type>
    ZY_INLINE Type Sqrt(Type Value)
    {
        return std::sqrt(Value);
    }

    /// \brief Computes the inverse square root of the given number.
    ///
    /// \param Value The number to compute the inverse square root for.
    /// \return The inverse square root of \a Value.
    template<typename Type>
    ZY_INLINE Type ReciprocalSqrt(Type Value)
    {
        ZY_ASSERT(Value > 0, "Value must be positive");

        return static_cast<Type>(1) / Sqrt(Value);
    }

    /// \brief Computes the logarithm of a number with a given base.
    ///
    /// \param Number The number to compute the logarithm for.
    /// \param Base   The base of the logarithm (default is 2).
    /// \return The logarithm of \a Number to the base \a Base.
    template<typename Type, typename Other = UInt32>
    ZY_INLINE Type Log(Type Number, Other Base = Other(2))
        requires (IsReal<Type> && IsIntegral<Other>)
    {
        return std::log(Number) / std::log(static_cast<Type>(Base));
    }

    /// \brief Computes the integer logarithm of a number with a given base.
    ///
    /// \param Number The number to compute the integer logarithm for.
    /// \param Base   The base of the logarithm (default is 2).
    /// \return The integer logarithm of \a Number to the base \a Base.
    template<typename Type, typename Other = Type>
    constexpr Type Log(Type Number, Other Base = Other(2))
        requires (IsIntegral<Type> && IsIntegral<Other>)
    {
        Type Result = Type(0);

        while (Number >= Base)
        {
            Number /= Base;
            ++Result;
        }
        return Result;
    }

    /// \brief Returns the smallest of the provided values.
    ///
    /// \param Left       The first value to compare.
    /// \param Right      The second value to compare.
    /// \param Additional The remaining values to compare.
    /// \return A copy of the smallest value among all arguments.
    template <typename T0, typename T1, typename... Tail>
    constexpr auto Min(T0 Left, T1 Right, Tail... Additional)
    {
        if constexpr (sizeof...(Tail) == 0)
        {
            return Left < Right ? Left : Right;
        }
        else
        {
            return Min(Min(Left, Right), Additional...);
        }
    }

    /// \brief Returns the largest of the provided values.
    ///
    /// \param Left       The first value to compare.
    /// \param Right      The second value to compare.
    /// \param Additional The remaining values to compare.
    /// \return A copy of the largest value among all arguments.
    template <typename T0, typename T1, typename... Tail>
    constexpr auto Max(T0 Left, T1 Right, Tail... Additional)
    {
        if constexpr (sizeof...(Tail) == 0)
        {
            return Left > Right ? Left : Right;
        }
        else
        {
            return Max(Max(Left, Right), Additional...);
        }
    }

    /// \brief Restricts a value to be within a specified range.
    ///
    /// \param Value   The value to clamp.
    /// \param Minimum The lower bound of the range.
    /// \param Maximum The upper bound of the range.
    /// \return A value clamped to the range [\p Minimum, \p Maximum].
    template<typename T0, typename T1, typename T2>
    constexpr auto Clamp(T0 Value, T1 Minimum, T2 Maximum)
    {
        ZY_ASSERT(Minimum <= Maximum, "Minimum must be less than or equal to Maximum");

        return Min(Max(Value, Minimum), Maximum);
    }

    /// \brief Linearly interpolates between two scalar values.
    ///
    /// \param Start      The starting scalar value.
    /// \param End        The ending scalar value.
    /// \param Percentage The interpolation factor, in the range [0, 1].
    /// \return The scalar value interpolated between \a Start and \a End.
    template<typename Value, typename Delta>
    constexpr Value Lerp(Value Start, Value End, Delta Percentage)
        requires IsReal<Delta>
    {
        if constexpr (IsReal<Value>)
        {
            return static_cast<Value>(std::lerp(Start, End, Percentage));
        }
        else
        {
            return static_cast<Value>(Start + (End - Start) * Percentage);
        }
    }

    /// \brief Checks if a floating-point number is approximately zero within a specified tolerance.
    ///
    /// \param Value     The floating-point number to check.
    /// \param Tolerance The tolerance within which the two numbers are considered equal.
    /// \return `true` if the number is approximately zero, `false` otherwise.
    template<typename Type>
    constexpr Bool IsAlmostZero(Type Value, Type Tolerance = kEpsilon<Type>)
    {
        if constexpr (IsReal<Type>)
        {
            return Abs(Value) <= Tolerance;
        }
        else
        {
            return (Value == Type(0));
        }
    }

    /// \brief Checks if two floating-point numbers are approximately equal within a specified tolerance.
    ///
    /// \param Left      The first floating-point number to compare.
    /// \param Right     The second floating-point number to compare.
    /// \param Tolerance The tolerance within which the two numbers are considered equal.
    /// \return `true` if the numbers are approximately equal, `false` otherwise.
    template<typename Type>
    constexpr Bool IsAlmostEqual(Type Left, Type Right, Type Tolerance = kEpsilon<Type>)
    {
        if constexpr (IsReal<Type>)
        {
            const Type Difference = Abs(Left - Right);
            const Type Scale      = Max(Abs(Left), Abs(Right), Type(1));
            return Difference <= Tolerance * Scale;
        }
        else
        {
            return (Left == Right);
        }
    }

    /// \brief Checks if a value is within a specified range, inclusive, considering floating-point precision.
    ///
    /// \param Value   The value to check.
    /// \param Minimum The minimum bound of the range.
    /// \param Maximum The maximum bound of the range.
    /// \return `true` if the value is within the range, `false` otherwise.
    template<typename Type>
    constexpr Bool IsBetween(Type Value, Type Minimum, Type Maximum)
    {
        if constexpr (IsReal<Type>)
        {
            const Type ToleranceMin = kEpsilon<Type> * Max(Abs(Value), Abs(Minimum), Type(1));
            const Type ToleranceMax = kEpsilon<Type> * Max(Abs(Value), Abs(Maximum), Type(1));
            return (Value >= Minimum - ToleranceMin && Value <= Maximum + ToleranceMax);
        }
        else
        {
            return (Value >= Minimum && Value <= Maximum);
        }
    }

    /// \brief Checks whether \p Value is a Not-a-Number (NaN) via its IEEE 754 bit pattern.
    ///
    /// \param Value The value to inspect.
    /// \return `true` if \p Value is NaN, `false` otherwise.
    template<typename Type>
    constexpr Bool IsNaN(Type Value)
        requires IsReal<Type>
    {
        if constexpr (sizeof(Type) == sizeof(Real32))
        {
            return (std::bit_cast<UInt32>(Value) & 0x7FFFFFFFu) > 0x7F800000u;
        }
        else
        {
            return (std::bit_cast<UInt64>(Value) & 0x7FFFFFFFFFFFFFFFull) > 0x7FF0000000000000ull;
        }
    }

    /// \brief Checks whether \p Value is positive or negative infinity via its IEEE 754 bit pattern.
    ///
    /// \param Value The value to inspect.
    /// \return `true` if \p Value is infinite, `false` otherwise.
    template<typename Type>
    constexpr Bool IsInf(Type Value)
        requires IsReal<Type>
    {
        if constexpr (sizeof(Type) == sizeof(Real32))
        {
            return (std::bit_cast<UInt32>(Value) & 0x7FFFFFFFu) == 0x7F800000u;
        }
        else
        {
            return (std::bit_cast<UInt64>(Value) & 0x7FFFFFFFFFFFFFFFull) == 0x7FF0000000000000ull;
        }
    }

    /// \brief Returns the number of decimal digits required to represent \p Value.
    ///
    /// Uses a leading-zero-count table combined with a power-of-ten boundary check to
    /// determine the digit count in O(1) without any division or loop.
    ///
    /// \param Value The integral value to inspect.
    /// \return The number of decimal digits in the base-10 representation of \p Value.
    template<typename Type>
    constexpr UInt CountDigitsDecimal(Type Value)
        requires IsIntegral<Type>
    {
        using Unsigned = Unsigned<Type>;

        static constexpr UInt8 kTable[64] =
        {
            20, 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15,
            15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10,
            10, 10, 10,  9,  9,  9,  8,  8,  8,  7,  7,  7,  7,  6,  6,  6,
             5,  5,  5,  4,  4,  4,  4,  3,  3,  3,  2,  2,  2,  1,  1,  1
        };

        const UInt64 Number = static_cast<UInt64>(static_cast<Unsigned>(Value));
        const UInt64 Digits = kTable[CountLeadingZeros(Number)];
        return Digits - (Number < Pow10(Digits - 1));
    }

    /// \brief Returns the number of binary digits required to represent \p Value.
    ///
    /// \param Value The integral value to inspect.
    /// \return The number of bits in the binary representation of \p Value.
    template<typename Type>
    constexpr Type CountDigitsBinary(Type Value)
        requires IsIntegral<Type>
    {
        return (sizeof(Type) * 8) - CountLeadingZeros(Value);
    }

    /// \brief Returns the number of hexadecimal digits required to represent \p Value.
    ///
    /// \param Value The integral value to inspect.
    /// \return The number of hex digits in the base-16 representation of \p Value.
    template<typename Type>
    constexpr UInt CountDigitsHexadecimal(Type Value)
        requires IsIntegral<Type>
    {
        return (CountDigitsBinary(Value) + 3) >> 2;
    }

    /// \brief Returns the number of digits required to represent \p Value in the given \p Base.
    ///
    /// Dispatches at compile time to `CountDigitsDecimal`, `CountDigitsBinary`, or  `CountDigitsHexadecimal`.
    ///
    /// \param Value The integral value to inspect.
    /// \return The number of digits needed to represent \p Value in base \p Base.
    template<UInt Base, typename Type>
    constexpr UInt CountDigits(Type Value)
        requires IsIntegral<Type>
    {
        if (Value == Type(0))
        {
            return 1;
        }

        if constexpr (Base == 2)
        {
            return ::CountDigitsBinary(Value);
        }
        else if constexpr (Base == 10)
        {
            return ::CountDigitsDecimal(Value);
        }
        else if constexpr (Base == 16)
        {
            return ::CountDigitsHexadecimal(Value);
        }
        else
        {
            static_assert(false, "Unsupported base. Only bases 2, 10, and 16 are supported.");

            return 0u;
        }
    }

    /// \brief Returns the number of digits required to represent \p Value with \p Precision precision.
    ///
    /// \param Value     The floating-point value to inspect.
    /// \param Precision The number of digits after the decimal point.
    /// \return The number of digits needed to represent \p Value with \p Precision precision.
    template<typename Type>
    constexpr UInt CountDigits(Type Value, UInt Precision = 6)
        requires IsReal<Type>
    {
        if (IsNaN(Value))
        {
            return 3; // "NaN"
        }

        if (IsInf(Value))
        {
            return 3; // "Inf"
        }

        if (Precision > 0)
        {
            const UInt64 Scale  = Pow10(Precision);
            const UInt64 Scaled = Abs(Value) * Scale + 0.5;
            return CountDigits<10>(Scaled / Scale) + 1 + Precision;
        }
        return CountDigits<10>(static_cast<UInt>(::Round(Value)));
    }

    /// \brief Converts 2D coordinates to a 1D index based on the width of the 2D structure.
    ///
    /// \param X     The x-coordinate (column index).
    /// \param Y     The y-coordinate (row index).
    /// \param Width The width of the 2D structure (number of columns).
    /// \return The corresponding 1D index for the given 2D coordinates.
    template<typename Type>
    constexpr auto ConvertTo1D(Type X, Type Y, Type Width)
    {
        return Y * Width + X;
    }

    /// \brief Converts 3D coordinates to a 1D index based on the width and height of the 3D structure.
    ///
    /// \param X      The x-coordinate (column index).
    /// \param Y      The y-coordinate (row index).
    /// \param Z      The z-coordinate (depth index).
    /// \param Width  The width of the 3D structure (number of columns).
    /// \param Height The height of the 3D structure (number of rows).
    /// \return The corresponding 1D index for the given 3D coordinates.
    template<typename Type>
    constexpr auto ConvertTo1D(Type X, Type Y, Type Z, Type Width, Type Height)
    {
        return Z * (Width * Height) + Y * Width + X;
    }
}