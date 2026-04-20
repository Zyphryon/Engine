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

#include <cmath>
#include <limits>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Mathematical constant representing the value of pi.
    template<typename Type>
    inline constexpr Type kPI       = Type(3.141592653589793238462643383279502884e+00);

    /// \brief Mathematical constant representing the value of two times pi.
    template<typename Type>
    inline constexpr Type kEpsilon  = std::numeric_limits<Type>::epsilon();

    /// \brief Mathematical constant representing positive infinity.
    template<typename Type>
    inline constexpr Type kInfinity = std::numeric_limits<Type>::infinity();

    /// \brief Mathematical constant representing the minimum finite value for the given type.
    template<typename Type>
    inline constexpr Type kMinimum  = std::numeric_limits<Type>::min();

    /// \brief Mathematical constant representing the maximum finite value for the given type.
    template<typename Type>
    inline constexpr Type kMaximum  = std::numeric_limits<Type>::max();

    /// \brief Computes the remainder of the division of two real numbers.
    ///
    /// \param Dividend The dividend value.
    /// \param Divisor  The divisor value.
    /// \return The remainder of the division of \a Dividend by \a Divisor.
    template<typename Type>
    constexpr Type Remainder(Type Dividend, Type Divisor)
        requires(IsReal<Type>)
    {
        LOG_ASSERT(Divisor != 0, "Divisor cannot be zero");

        return std::fmod(Dividend, Divisor);
    }

    /// \brief Computes the absolute value of the given number.
    ///
    /// \param Value The value to compute the absolute for.
    /// \return The absolute value of \a Value.
    template<typename Type>
    constexpr Type Abs(Type Value)
    {
        return Value < static_cast<Type>(0) ? -Value : Value;
    }

    /// \brief Computes the power of a base raised to an exponent.
    ///
    /// \param Base     The base value.
    /// \param Exponent The exponent value.
    /// \return The result of raising \a Base to the power of \a Exponent.
    template<typename Type>
    constexpr Type Pow(Type Base, Type Exponent)
    {
        return std::pow(Base, Exponent);
    }

    /// \brief Computes the square root of the given number.
    ///
    /// \param Number The number to compute the square root for.
    /// \return The square root of \a Number.
    template<typename Type>
    constexpr Type Sqrt(Type Number)
    {
        return std::sqrt(Number);
    }

    /// \brief Computes the inverse square root of the given number.
    ///
    /// \param Number The number to compute the inverse square root for.
    /// \return The inverse square root of \a Number.
    template<typename Type>
    constexpr Type InvSqrt(Type Number)
    {
        LOG_ASSERT(Number > 0, "Number must be positive");

        return static_cast<Type>(1) / std::sqrt(Number);
    }

    /// \brief Determines the sign of the given number.
    ///
    /// \param Value The value to determine the sign for.
    /// \return `1` if \a Value is positive, `-1` if negative, and `0` if zero.
    template<typename Type>
    constexpr Type Sign(Type Value)
    {
        return (static_cast<Type>(0) < Value) - (Value < static_cast<Type>(0));
    }

    /// \brief Computes the largest integer less than or equal to the given value.
    ///
    /// \param Value The value to floor.
    /// \return The largest integer less than or equal to \a Value.
    template<typename Type>
    constexpr Type Floor(Type Value)
    {
        return std::floor(Value);
    }

    /// \brief Computes the smallest integer greater than or equal to the given value.
    ///
    /// \param Value The value to ceil.
    /// \return The smallest integer greater than or equal to \a Value.
    template<typename Type>
    constexpr Type Ceil(Type Value)
    {
        return std::ceil(Value);
    }

    /// \brief Rounds the given value to the nearest integer.
    ///
    /// \param Value The value to round.
    /// \return The nearest integer to \a Value.
    template<typename Type>
    constexpr Type Round(Type Value)
    {
        return std::round(Value);
    }

    /// \brief Computes the logarithm of a number with a given base.
    ///
    /// \param Number The number to compute the logarithm for.
    /// \param Base   The base of the logarithm (default is 2).
    /// \return The logarithm of \a Number to the base \a Base.
    template<typename Type, typename Other = UInt32>
    constexpr Type Log(Type Number, Other Base = Other(2))
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

    /// \brief Linearly interpolates between two scalar.
    ///
    /// \param Start      The starting scalar.
    /// \param End        The ending scalar.
    /// \param Percentage The interpolation percentage (range between 0 and 1).
    /// \return A scalar interpolated between the start and end scalar.
    template<typename Type>
    constexpr Type Lerp(Type Start, Type End, Real32 Percentage)
    {
        return Start + Percentage * (End - Start);
    }

    //// \brief Computes the minimum value among the provided arguments.
    ///
    /// \param Left       The first value to compare.
    /// \param Right      The second value to compare.
    /// \param Additional The additional values to compare.
    /// \return The minimum value among all provided arguments.
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

    /// \brief Computes the maximum value among the provided arguments.
    ///
    /// \param Left       The first value to compare.
    /// \param Right      The second value to compare.
    /// \param Additional The additional values to compare.
    /// \return The maximum value among all provided arguments.
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
    /// \param Value   Input value to clamp.
    /// \param Minimum Lower bound of range.
    /// \param Maximum Upper bound of range.
    /// \return The restricted value within lower and upper bounds.
    template<typename T0, typename T1, typename T2>
    constexpr auto Clamp(T0 Value, T1 Minimum, T2 Maximum)
    {
        LOG_ASSERT(Minimum <= Maximum, "Minimum must be less or equal than Maximum");

        return Min(Max(Value, Minimum), Maximum);
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

    /// \brief Checks if a floating-point number is approximately zero within a specified tolerance.
    ///
    /// \param Value The floating-point number to check.
    /// \return `true` if the number is approximately zero, `false` otherwise.
    template<typename Type>
    constexpr Bool IsAlmostZero(Type Value)
    {
        if constexpr (IsReal<Type>)
        {
            return Abs(Value) <= kEpsilon<Type>;
        }
        else
        {
            return (Value == static_cast<Type>(0));
        }
    }

    /// \brief Checks if a value is within a specified range, inclusive, considering floating-point precision.
    ///
    /// \param Value   The value to check.
    /// \param Minimum The minimum bound of the range.
    /// \param Maximum The maximum bound of the range.
    /// \return `true` if the value is within the range, `false` otherwise
    template<typename Type>
    constexpr Bool IsBetween(Type Value, Type Minimum, Type Maximum)
    {
        return (Value + kEpsilon<Type> >= Minimum && Value - kEpsilon<Type> <= Maximum);
    }
}