// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Trait.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief The mathematical constant PI (π), templated for any real type.
    template<typename Type>
    inline constexpr Type kPI      = Type(3.141592653589793238462643383279502884e+00);

    /// \brief Relative epsilon for floating-point comparisons.
    template<typename Type>
    inline constexpr Type kEpsilon =
        IsEqual<Type, Real32> ? Type(1e-5f) :
        IsEqual<Type, Real64> ? Type(1e-12) :
        Type(0);

    /// \brief Converts degrees to radians.
    ///
    /// \param Degrees Angle in degrees.
    /// \return Angle in radians.
    template<typename Type>
    constexpr Type DegreesToRadians(Type Degrees)
    {
        return Degrees * (kPI<Type> / Type(180));
    }

    /// \brief Converts radians to degrees.
    ///
    /// \param Radians Angle in radians.
    /// \return Angle in degrees.
    template<typename Type>
    constexpr Type RadiansToDegrees(Type Radians)
    {
        return Radians * (Type(180) / kPI<Type>);
    }

    /// \brief Returns the absolute (non-negative) value of a number.
    ///
    /// \param Value The input number.
    /// \return The absolute value of the input.
    template<typename Type>
    constexpr Type Abs(Type Value)
    {
        return std::abs(Value);
    }

    /// \brief Returns the sign of a number: -1 for negative, 0 for zero, and +1 for positive.
    ///
    /// \param Value The input number.
    /// \return -1, 0, or +1 depending on the sign of the input.
    template<typename Type>
    constexpr Type Sign(Type Value)
    {
        return (Type(0) < Value) - (Value < Type(0));
    }

    /// \brief Computes the cosine of an angle in radians.
    ///
    /// \param Angle The angle in radians.
    /// \return Cosine of the angle.
    template<typename Type>
    constexpr Type Cos(Type Angle)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::cos(Angle);
        }
        else
        {
            return std::cosf(Angle);
        }
    }

    /// \brief Computes the inverse cosine (arccos) of a value.
    ///
    /// \param Value The cosine value in the range [-1, 1].
    /// \return Angle in radians.
    template<typename Type>
    constexpr Type InvCos(Type Value)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::acos(Value);
        }
        else
        {
            return std::acosf(Value);
        }
    }

    /// \brief Computes the sine of an angle in radians.
    ///
    /// \param Angle Angle in radians.
    /// \return Sine of the angle.
    template<typename Type>
    constexpr Type Sin(Type Angle)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::sin(Angle);
        }
        else
        {
            return std::sinf(Angle);
        }
    }

    /// \brief Computes the inverse sine (arcsin) of a value.
    ///
    /// \param Value The sine value in the range [-1, 1].
    /// \return Angle in radians.
    template<typename Type>
    constexpr Type InvSin(Type Value)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::asin(Value);
        }
        else
        {
            return std::asinf(Value);
        }
    }

    /// \brief Computes the tangent of an angle in radians.
    ///
    /// \param Angle Angle in radians.
    /// \return Tangent of the angle.
    template<typename Type>
    constexpr Type Tan(Type Angle)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::tan(Angle);
        }
        else
        {
            return std::tanf(Angle);
        }
    }

    /// \brief Computes the inverse tangent (arctangent) of a value.
    ///
    /// \param Value The tangent value in the range [-1, 1].
    /// \return Angle in radians.
    template<typename Type>
    constexpr Type InvTan(Type Value)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::atan(Value);
        }
        else
        {
            return std::atanf(Value);
        }
    }

    /// \brief Computes the four-quadrant inverse tangent of Y and X.
    ///
    /// \param X The X-coordinate.
    /// \param Y The Y-coordinate.
    /// \return The angle in radians between the positive X-axis and the point (X, Y).
    template<typename Type>
    constexpr Type InvTan(Type Y, Type X)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::atan2(Y, X);
        }
        else
        {
            return std::atan2f(Y, X);
        }
    }

    /// \brief Computes the square root of a number.
    ///
    /// \param Number The value to compute the square root of.
    /// \return The square root of the number.
    template<typename Type>
    constexpr Type Sqrt(Type Number)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::sqrt(Number);
        }
        else
        {
            return std::sqrtf(Number);
        }
    }

    /// \brief Computes the inverse square root of a number.
    ///
    /// \param Number The value to compute the inverse square root of.
    /// \return The inverse square root of the number.
    template<typename Type>
    constexpr Type InvSqrt(Type Number)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return Type(1) / std::sqrt(Number);
        }
        else
        {
            return Type(1) / std::sqrtf(Number);
        }
    }

    /// \brief Computes the power of a base raised to an exponent.
    ///
    /// \param Base     The base value.
    /// \param Exponent The exponent value.
    /// \return The result of base raised to the power of exponent.
    template<typename Type>
    constexpr Type Pow(Type Base, Type Exponent)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::pow(Base, Exponent);
        }
        else
        {
            return std::powf(Base, Exponent);
        }
    }

    /// \brief Computes the largest integer less than or equal to the given value.
    ///
    /// \param Value The value to floor.
    /// \return The largest integer less than or equal to Value.
    template<typename Type>
    constexpr Type Floor(Type Value)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::floor(Value);
        }
        else
        {
            return std::floorf(Value);
        }
    }

    /// \brief Computes the smallest integer greater than or equal to the given value.
    ///
    /// \param Value The value to ceil.
    /// \return The smallest integer greater than or equal to Value.
    template<typename Type>
    constexpr Type Ceil(Type Value)
    {
        if constexpr (IsEqual<Real64, Type>)
        {
            return std::ceil(Value);
        }
        else
        {
            return std::ceilf(Value);
        }
    }

    /// \brief Computes the logarithm of a number with a specified base.
    ///
    /// \param Number The number to compute the logarithm for.
    /// \param Base   The logarithmic base (default is 2).
    /// \return The logarithm of number to the specified base.
    template<typename Type, typename Other = UInt32>
    constexpr Type Log(Type Number, Other Base = Other(2))
        requires (IsReal<Type> && IsInteger<Other>)
    {
        return std::log(Number) / std::log(static_cast<Type>(Base));
    }

    /// \brief Computes the integer logarithm of a number with a given base.
    ///
    /// \param Number The number to compute the logarithm for.
    /// \param Base   The logarithmic base (default is 2).
    /// \return The integer logarithm of number to the specified base.
    template<typename Type, typename Other = Type>
    constexpr Type Log(Type Number, Other Base = Other(2))
        requires (IsInteger<Type> && IsInteger<Other>)
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

    /// \brief Finds the smallest value among all provided arguments.
    ///
    /// \param First  First value to compare.
    /// \param Second Second value to compare.
    /// \param Third  Additional values to compare (variadic).
    /// \return The smallest value among all inputs.
    template <typename T0, typename T1, typename... Tail>
    constexpr auto Min(T0 First, T1 Second, Tail... Third)
    {
        if constexpr (sizeof...(Tail) == 0)
        {
            return First < Second ? First : Second;
        }
        else
        {
            return Min(Min(First, Second), Third...);
        }
    }

    /// \brief Finds the largest  value among all provided arguments.
    ///
    /// \param First  First value to compare.
    /// \param Second Second value to compare.
    /// \param Third  Additional values to compare (variadic).
    /// \return The largest value among all inputs.
    template <typename T0, typename T1, typename... Tail>
    constexpr auto Max(T0 First, T1 Second, Tail... Third)
    {
        if constexpr (sizeof...(Tail) == 0)
        {
            return First > Second ? First : Second;
        }
        else
        {
            return Max(Max(First, Second), Third...);
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
        return Min(Max(Value, Minimum), Maximum);
    }

    /// \brief Compares two values for approximate equality within a specified tolerance.
    ///
    /// \param First     Left-hand value.
    /// \param Second    Right-hand value.
    /// \param Tolerance Optional tolerance for comparison (default is machine epsilon).
    /// \return `true` if the values are considered almost equal; otherwise `false`.
    template<typename Type>
    constexpr Bool IsAlmostEqual(Type First, Type Second, Type Tolerance = kEpsilon<Type>)
    {
        if constexpr (IsReal<Type>)
        {
            const Type Difference = Abs(First - Second);
            const Type Scale      = Max(Abs(First), Abs(Second), Type(1));
            return Difference <= Tolerance * Scale;
        }
        else
        {
            return (First == Second);
        }
    }

    /// \brief Checks if a value is approximately zero.
    ///
    /// \param First Input value.
    /// \return `true` if the value is near zero, `false` otherwise.
    template<typename Type>
    constexpr Bool IsAlmostZero(Type First)
    {
        if constexpr (IsReal<Type>)
        {
            return Abs(First) <= kEpsilon<Type>;
        }
        else
        {
            return IsAlmostEqual(First, Type(0));
        }
    }
}