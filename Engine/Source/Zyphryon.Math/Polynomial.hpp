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

#include "Angle.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Finds the real roots of the linear equation `Ax + B`.
    ///
    /// \param Result   The span that receives the root, which needs room for one.
    /// \param Linear   The coefficient of the linear term.
    /// \param Constant The constant term.
    /// \return The number of real roots written, which is zero when the equation is constant.
    template<IsReal Type>
    ZY_INLINE UInt32 SolveLinear(Span<Type> Result, Type Linear, Type Constant)
    {
        ZY_ASSERT(Result.GetSize() >= 1, "Solving a linear equation needs room for one root");

        if (Abs(Linear) <= kEpsilon<Type> * Max(Abs(Constant), Type(1)))
        {
            return 0;
        }

        Result[0] = -Constant / Linear;
        return 1;
    }

    /// \brief Finds the real roots of the quadratic equation `Ax² + Bx + C`.
    ///
    /// \param Result    The span that receives the roots in no particular order, which needs room for two.
    /// \param Quadratic The coefficient of the squared term.
    /// \param Linear    The coefficient of the linear term.
    /// \param Constant  The constant term.
    /// \return The number of real roots written, counting a repeated root once.
    template<IsReal Type>
    ZY_INLINE UInt32 SolveQuadratic(Span<Type> Result, Type Quadratic, Type Linear, Type Constant)
    {
        ZY_ASSERT(Result.GetSize() >= 2, "Solving a quadratic equation needs room for two roots");

        if (Abs(Quadratic) <= kEpsilon<Type> * Max(Abs(Linear), Abs(Constant), Type(1)))
        {
            return SolveLinear(Result, Linear, Constant);
        }

        const Type Discriminant = Linear * Linear - 4 * Quadratic * Constant;

        if (Discriminant > 0)
        {
            const Type Root = Sqrt(Discriminant);

            Result[0] = (-Linear + Root) / (2 * Quadratic);
            Result[1] = (-Linear - Root) / (2 * Quadratic);
            return 2;
        }

        // A vanishing discriminant means the curve grazes the axis, which is one root rather than two.
        if (Discriminant == 0)
        {
            Result[0] = -Linear / (2 * Quadratic);
            return 1;
        }
        return 0;
    }

    /// \brief Finds the real roots of the cubic equation `Ax³ + Bx² + Cx + D`.
    ///
    /// \param Result    The span that receives the roots in no particular order, which needs room for three.
    /// \param Cubic     The coefficient of the cubed term.
    /// \param Quadratic The coefficient of the squared term.
    /// \param Linear    The coefficient of the linear term.
    /// \param Constant  The constant term.
    /// \return The number of real roots written, counting a repeated root once.
    template<IsReal Type>
    ZY_INLINE UInt32 SolveCubic(Span<Type> Result, Type Cubic, Type Quadratic, Type Linear, Type Constant)
    {
        ZY_ASSERT(Result.GetSize() >= 3, "Solving a cubic equation needs room for three roots");

        if (Abs(Cubic) <= kEpsilon<Type> * Max(Abs(Quadratic), Abs(Linear), Abs(Constant), Type(1)))
        {
            return SolveQuadratic(Result, Quadratic, Linear, Constant);
        }

        constexpr Type kThird      = Type(1) / Type(3);
        constexpr Type kHalf       = Type(1) / Type(2);
        constexpr Type kTwoNinths  = Type(2) / Type(27);

        const Type Normal   = Quadratic / Cubic;
        const Type Slope    = Linear / Cubic;
        const Type Squared  = Normal * Normal;
        const Type Pressure = kThird * (Slope - kThird * Squared);
        const Type Offset   = kHalf * (kTwoNinths * Normal * Squared - kThird * Normal * Slope + Constant / Cubic);

        const Type Cubed        = Pressure * Pressure * Pressure;
        const Type Discriminant = Offset * Offset + Cubed;
        const Type Shift        = kThird * Normal;

        if (Discriminant < 0)
        {
            const Real32 Ratio = static_cast<Real32>(Clamp(-Offset / Sqrt(-Cubed), Type(-1), Type(1)));
            const Angle  Phi   = Angle::FromCosine(Ratio) * (1.0f / 3.0f);
            const Type   Ray   = 2 * Sqrt(-Pressure);

            Result[0] =  Ray * Angle::Cosine(Phi) - Shift;
            Result[1] = -Ray * Angle::Cosine(Phi + Angle(kPI<Real32> / 3.0f)) - Shift;
            Result[2] = -Ray * Angle::Cosine(Phi - Angle(kPI<Real32> / 3.0f)) - Shift;
            return 3;
        }

        if (Discriminant > 0)
        {
            const Type Root = Sqrt(Discriminant);

            Result[0] = Cbrt(Root - Offset) - Cbrt(Root + Offset) - Shift;
            return 1;
        }

        const Type Single = -Cbrt(Offset);

        Result[0] = 2 * Single - Shift;
        Result[1] = -Single - Shift;
        return 2;
    }
}