// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_MATH_INCLUDED
#define ZY_MATH_INCLUDED

/// The engine tier the program is built for, counted from one; the technique loader sets it.
#ifndef ZY_TIER
#error "ZY_TIER must be named by the technique loader"
#endif

/// Pi.
#define ZY_PI            3.14159265359

/// Two pi.
#define ZY_TWO_PI        6.28318530718

/// Half pi.
#define ZY_HALF_PI       1.57079632679

/// One over pi.
#define ZY_INV_PI        0.31830988618

/// One over two pi.
#define ZY_INV_TWO_PI    0.15915494309

/// A tiny value that stands in for zero and keeps divides safe.
#define ZY_EPSILON       0.0001

/// Clamps a value to 0..1.
float ZySaturate(float Value)
{
    return clamp(Value, 0.0, 1.0);
}

/// Squares a value.
float ZySquare(float Value)
{
    return Value * Value;
}

/// The smallest of three values.
float ZyMin3(float First, float Second, float Third)
{
    return min(First, min(Second, Third));
}

/// The largest of three values.
float ZyMax3(float First, float Second, float Third)
{
    return max(First, max(Second, Third));
}

/// Remaps a value from one range to another; either range may run backwards.
float ZyRemap(float Value, float FromMin, float FromMax, float ToMin, float ToMax)
{
    float Range = FromMax - FromMin;

    return ToMin + (Value - FromMin) * (ToMax - ToMin) / (abs(Range) > ZY_EPSILON ? Range : ZY_EPSILON);
}

/// Wraps an angle into -pi..pi.
float ZyWrapAngle(float Radians)
{
    return Radians - ZY_TWO_PI * round(Radians * ZY_INV_TWO_PI);
}

/// Rotates a 2D vector by an angle given as its cosine and sine.
vec2 ZyRotate(vec2 Vector, vec2 Turn)
{
    return vec2(Vector.x * Turn.x - Vector.y * Turn.y, Vector.x * Turn.y + Vector.y * Turn.x);
}

/// Normalizes a vector, or returns zero when it is too short to have a direction.
vec3 ZySafeNormalize(vec3 Vector)
{
    float Length = dot(Vector, Vector);

    return (Length > ZY_EPSILON * ZY_EPSILON) ? Vector * inversesqrt(Length) : vec3(0.0);
}

#endif // ZY_MATH_INCLUDED