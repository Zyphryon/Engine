// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Quaternion.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Quaternion Quaternion::FromEulerAngles(Vector3 Angles)
    {
        const Angle HalfPitch(Angles.GetX() * 0.5f);
        const Angle HalfYaw(Angles.GetY() * 0.5f);
        const Angle HalfRoll(Angles.GetZ() * 0.5f);

        const Real32 CX = Angle::Cosine(HalfPitch);
        const Real32 SX = Angle::Sine(HalfPitch);
        const Real32 CY = Angle::Cosine(HalfYaw);
        const Real32 SY = Angle::Sine(HalfYaw);
        const Real32 CZ = Angle::Cosine(HalfRoll);
        const Real32 SZ = Angle::Sine(HalfRoll);

        return Quaternion(SX * CY * CZ + CX * SY * SZ,
                          CX * SY * CZ - SX * CY * SZ,
                          CX * CY * SZ + SX * SY * CZ,
                          CX * CY * CZ - SX * SY * SZ);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Quaternion Quaternion::FromDirection(Vector3 Direction, Vector3 Up)
    {
        LOG_ASSERT(Direction.IsNormalized(), "Direction must be normalized");
        LOG_ASSERT(Up.IsNormalized(), "Up must be normalized");
        LOG_ASSERT(!Vector3::IsParallel(Direction, Up), "Direction and Up cannot be parallel");

        const Vector3 vRight = Vector3::Normalize(Vector3::Cross(Up, Direction));
        const Vector3 vCross = Vector3::Cross(Direction, vRight);

        const Real32 M00 = vRight.GetX(), M01 = vCross.GetX(), M02 = Direction.GetX();
        const Real32 M10 = vRight.GetY(), M11 = vCross.GetY(), M12 = Direction.GetY();
        const Real32 M20 = vRight.GetZ(), M21 = vCross.GetZ(), M22 = Direction.GetZ();

        const Real32 Trace = M00 + M11 + M22;
        if (Trace > 0.0f)
        {
            const Real32 S  = 0.5f / Sqrt(Trace + 1.0f);
            const Real32 QW = 0.25f / S;
            const Real32 QX = (M21 - M12) * S;
            const Real32 QY = (M02 - M20) * S;
            const Real32 QZ = (M10 - M01) * S;

            return Quaternion(QX, QY, QZ, QW);
        }
        if (M00 > M11 && M00 > M22)
        {
            const Real32 S  = 2.0f * Sqrt(1.0f + M00 - M11 - M22);
            const Real32 QW = (M21 - M12) / S;
            const Real32 QX = 0.25f * S;
            const Real32 QY = (M01 + M10) / S;
            const Real32 QZ = (M02 + M20) / S;

            return Quaternion(QX, QY, QZ, QW);
        }
        if (M11 > M22)
        {
            const Real32 S  = 2.0f * Sqrt(1.0f + M11 - M00 - M22);
            const Real32 QW = (M02 - M20) / S;
            const Real32 QX = (M01 + M10) / S;
            const Real32 QY = 0.25f * S;
            const Real32 QZ = (M12 + M21) / S;

            return Quaternion(QX, QY, QZ, QW);
        }

        const Real32 S  = 2.0f * Sqrt(1.0f + M22 - M00 - M11);
        const Real32 QW = (M10 - M01) / S;
        const Real32 QX = (M02 + M20) / S;
        const Real32 QY = (M12 + M21) / S;
        const Real32 QZ = 0.25f * S;
        return Quaternion(QX, QY, QZ, QW);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Quaternion Quaternion::Slerp(Quaternion Start, Quaternion End, Real32 Percentage)
    {
        LOG_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

        if (const Real32 Dot = Clamp(Quaternion::Dot(Start, End), -1.0f, +1.0f); Dot > 0.9995f)
        {
            return NLerp(Start, End, Percentage);
        }
        else
        {
            const Angle Theta = Angle::FromCosine(Dot) * Percentage;
            return (Start * Angle::Cosine(Theta)) + (Normalize(End - Start * Dot) * Angle::Sine(Theta));
        }
    }
}
