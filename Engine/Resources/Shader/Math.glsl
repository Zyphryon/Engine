// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_MATH_INCLUDED
#define ZY_MATH_INCLUDED

/// The half turn, in radians.
#define ZY_PI            3.14159265359

/// The full turn, in radians.
#define ZY_TWO_PI        6.28318530718

/// The quarter turn, in radians.
#define ZY_HALF_PI       1.57079632679

/// The reciprocal of the half turn, for trading a divide against a multiply.
#define ZY_INV_PI        0.31830988618

/// The reciprocal of the full turn, for trading a divide against a multiply.
#define ZY_INV_TWO_PI    0.15915494309

/// Small enough to stand in for zero, large enough to keep a divide from running away.
#define ZY_EPSILON       0.0001

/// \brief Holds a value to the closed range zero through one.
///
/// \param Value The value to hold.
///
/// \return The value, brought inside the range.
float ZySaturate(float Value)
{
    return clamp(Value, 0.0, 1.0);
}

/// \brief Multiplies a value by itself.
///
/// \param Value The value to multiply.
///
/// \return The value squared.
float ZySquare(float Value)
{
    return Value * Value;
}

/// \brief Picks the smallest of three values.
///
/// \param First  The first value to weigh.
/// \param Second The second value to weigh.
/// \param Third  The third value to weigh.
///
/// \return The smallest of the three.
float ZyMin3(float First, float Second, float Third)
{
    return min(First, min(Second, Third));
}

/// \brief Picks the largest of three values.
///
/// \param First  The first value to weigh.
/// \param Second The second value to weigh.
/// \param Third  The third value to weigh.
///
/// \return The largest of the three.
float ZyMax3(float First, float Second, float Third)
{
    return max(First, max(Second, Third));
}

/// \brief Carries a value out of one range and into another, keeping its place between the ends.
///
/// \note Either range may descend, but a source range narrower than \ref ZY_EPSILON is widened to it.
///
/// \param Value   The value to carry.
/// \param FromMin The end of the range the value starts at.
/// \param FromMax The end of the range the value stops at.
/// \param ToMin   The end of the range the value is carried to.
/// \param ToMax   The end of the range the value is carried against.
///
/// \return The value, holding the same place in the range it was carried to.
float ZyRemap(float Value, float FromMin, float FromMax, float ToMin, float ToMax)
{
    float Range = FromMax - FromMin;

    return ToMin + (Value - FromMin) * (ToMax - ToMin) / (abs(Range) > ZY_EPSILON ? Range : ZY_EPSILON);
}

/// \brief Folds an angle back into the half turn either side of zero.
///
/// \param Radians The angle to fold.
///
/// \return The angle, inside the range negative through positive half turn.
float ZyWrapAngle(float Radians)
{
    return Radians - ZY_TWO_PI * round(Radians * ZY_INV_TWO_PI);
}

/// \brief Normalizes a vector, giving back zero rather than dividing by a length it does not have.
///
/// \param Vector The vector to normalize.
///
/// \return The vector at unit length, or zero when it is too short to carry a direction.
vec3 ZySafeNormalize(vec3 Vector)
{
    float Length = dot(Vector, Vector);

    return (Length > ZY_EPSILON * ZY_EPSILON) ? Vector * inversesqrt(Length) : vec3(0.0);
}

#endif // ZY_MATH_INCLUDED