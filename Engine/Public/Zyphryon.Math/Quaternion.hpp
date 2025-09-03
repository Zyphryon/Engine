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

#include "Vector3.hpp"
#include "Vector4.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a quaternion for 3D rotations.
    class ZYPHRYON_ALIGN(16) Quaternion final
    {
        friend class Matrix4x4;

    public:

        /// \brief Constructs an identity quaternion.
        ZYPHRYON_INLINE Quaternion()
            : mData { 0.0f, 0.0f, 0.0f, 1.0f }
        {
        }

        /// \brief Constructs a quaternion from explicit components.
        ///
        /// \param X X value.
        /// \param Y Y value.
        /// \param Z Z value.
        /// \param W W value.
        ZYPHRYON_INLINE Quaternion(Real32 X, Real32 Y, Real32 Z, Real32 W)
            : mData { X, Y, Z, W }
        {
        }

        /// \brief Constructs a quaternion from its imaginary (vector) part and real (scalar) part.
        ///
        /// \param Imaginary The vector part of the quaternion (x, y, z components).
        /// \param Real      The scalar (real) part of the quaternion (w component).
        ZYPHRYON_INLINE Quaternion(ConstRef<Vector3> Imaginary, Real32 Real)
            : mData { Imaginary.GetX(), Imaginary.GetY(), Imaginary.GetZ(), Real }
        {
        }

        /// \brief Copy constructor.
        /// 
        /// \param Other The quaternion to copy.
        ZYPHRYON_INLINE Quaternion(ConstRef<Quaternion> Other) = default;

        /// \brief Checks if the quaternion is the zero quaternion.
        /// 
        /// \return `true` if all components are approximately zero, `false` otherwise.
        ZYPHRYON_INLINE Bool IsAlmostZero() const
        {
            return mData.IsAlmostZero();
        }

        /// \brief Checks if all components are approximately equal to the given values.
        /// 
        /// \param X Value to compare with the X component.
        /// \param Y Value to compare with the Y component.
        /// \param Z Value to compare with the Z component.
        /// \param W Value to compare with the W component.
        /// \return `true` if all components are within epsilon, `false` otherwise.
        ZYPHRYON_INLINE Bool IsAlmostEqual(Real32 X, Real32 Y, Real32 Z, Real32 W) const
        {
            return mData.IsAlmostEqual(X, Y, Z, W);
        }

        /// \brief Checks if the quaternion is normalized.
        /// 
        /// \return `true` if the quaternion is normalized, `false` otherwise.
        ZYPHRYON_INLINE Bool IsNormalized() const
        {
            return mData.IsNormalized();
        }

        /// \brief Checks if the quaternion is finite.
        /// 
        /// \return `true` if the quaternion is finite, `false` otherwise.
        ZYPHRYON_INLINE Bool IsFinite() const
        {
            return mData.IsFinite();
        }

        /// \brief Gets the x-component of the quaternion.
        /// 
        /// \return The x-component.
        ZYPHRYON_INLINE Real32 GetX() const
        {
            return mData.GetX();
        }

        /// \brief Gets the y-component of the quaternion.
        /// 
        /// \return The y-component.
        ZYPHRYON_INLINE Real32 GetY() const
        {
            return mData.GetY();
        }

        /// \brief Gets the z-component of the quaternion.
        /// 
        /// \return The z-component.
        ZYPHRYON_INLINE Real32 GetZ() const
        {
            return mData.GetZ();
        }

        /// \brief Gets the w-component of the quaternion.
        /// 
        /// \return The w-component.
        ZYPHRYON_INLINE Real32 GetW() const
        {
            return mData.GetW();
        }

        /// \brief Stores the quaternion components into a float array.
        ///
        /// \tparam Align  When `true`, `Output` must be 16-byte aligned. When `false`, any alignment is allowed.
        /// \param  Output Pointer to four 32-bit floats. Must be 16-byte aligned if `Align` is `true`.
        template<Bool Align = true>
        ZYPHRYON_INLINE void Store(Ptr<Real32> Output) const
        {
            mData.Store<Align>(Output);
        }

        /// \brief Calculates the length (magnitude) of the quaternion.
        /// 
        /// \return The length of the quaternion.
        ZYPHRYON_INLINE Real32 GetLength() const
        {
            return mData.GetLength();
        }

        /// \brief Calculates the squared length of the quaternion.
        /// 
        /// \return The squared length of the quaternion (no square root).
        ZYPHRYON_INLINE Real32 GetLengthSquared() const
        {
            return mData.GetLengthSquared();
        }

        /// \brief Returns the forward (Z+) direction vector rotated by this quaternion.
        /// 
        /// \return The forward direction in world space as a Vector3.
        ZYPHRYON_INLINE Vector3 GetForward() const
        {
            return Rotate(* this, Vector3::UnitZ());
        }

        /// \brief Returns the back (Z–) direction vector rotated by this quaternion.
        /// 
        /// \return The back direction in world space as a Vector3.
        ZYPHRYON_INLINE Vector3 GetBack() const
        {
            return -GetForward();
        }

        /// \brief Returns the up (Y+) direction vector rotated by this quaternion.
        /// 
        /// \return The up direction in world space as a Vector3.
        ZYPHRYON_INLINE Vector3 GetUp() const
        {
            return Rotate(* this, Vector3::UnitY());
        }

        /// \brief Returns the down (Y–) direction vector rotated by this quaternion.
        /// 
        /// \return The down direction in world space as a Vector3.
        ZYPHRYON_INLINE Vector3 GetDown() const
        {
            return -GetUp();
        }

        /// \brief Returns the right (X+) direction vector rotated by this quaternion.
        /// 
        /// \return The right direction in world space as a Vector3.
        ZYPHRYON_INLINE Vector3 GetRight() const
        {
            return Rotate(* this, Vector3::UnitX());
        }

        /// \brief Returns the left (X–) direction vector rotated by this quaternion.
        /// 
        /// \return The left direction in world space as a Vector3.
        ZYPHRYON_INLINE Vector3 GetLeft() const
        {
            return -GetRight();
        }

        /// \brief Adds another quaternion to this quaternion.
        /// 
        /// \param Other The quaternion to add.
        /// \return A new quaternion that is the sum of this quaternion and the input quaternion.
        ZYPHRYON_INLINE Quaternion operator+(ConstRef<Quaternion> Other) const
        {
            return Quaternion(mData + Other.mData);
        }

        /// \brief Negates the quaternion.
        /// 
        /// \return A new quaternion that is the negation of this quaternion.
        ZYPHRYON_INLINE Quaternion operator-() const
        {
            return Quaternion(- mData);
        }

        /// \brief Subtracts another quaternion from this quaternion.
        /// 
        /// \param Other The quaternion to subtract.
        /// \return A new quaternion that is the difference of the two vectors.
        ZYPHRYON_INLINE Quaternion operator-(ConstRef<Quaternion> Other) const
        {
            return Quaternion(mData - Other.mData);
        }

        /// \brief Multiplies the quaternion components by another quaternion (Hamilton product).
        /// 
        /// \param Other The quaternion to multiply.
        /// \return A new quaternion with the new rotation.
        ZYPHRYON_INLINE Quaternion operator*(ConstRef<Quaternion> Other) const
        {
            LOG_ASSERT(IsNormalized(), "Quaternion must be normalized before rotating");

            const Vector4 V1 = Vector4::Blend<0b1000>(mData, Vector4::Zero());
            const Vector4 V2 = Vector4::Blend<0b1000>(Other.mData, Vector4::Zero());
            const Vector4 W1 = Vector4::SplatW(mData);
            const Vector4 W2 = Vector4::SplatW(Other.mData);

            const Vector4 XYZ = W1 * V2 + W2 * V1 + Vector4::Cross(V1, V2);
            const Vector4 W   = W1 * W2 - Vector4(Vector4::Dot3(V1, V2));

            return Quaternion(Vector4::Blend<0b1000>(XYZ, W * Vector4::UnitW()));
        }

        /// \brief Multiplies all components of this quaternion by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new quaternion with the scalar multiplied by all components.
        ZYPHRYON_INLINE Quaternion operator*(Real32 Scalar) const
        {
            return Quaternion(mData * Scalar);
        }

        /// \brief Adds another quaternion to the current quaternion.
        /// 
        /// \param Other The quaternion to add.
        /// \return A reference to the updated quaternion.
        ZYPHRYON_INLINE Ref<Quaternion> operator+=(ConstRef<Quaternion> Other)
        {
            mData += Other.mData;
            return (* this);
        }

        /// \brief Subtracts another quaternion from the current quaternion.
        /// 
        /// \param Other The quaternion to subtract.
        /// \return A reference to the updated quaternion.
        ZYPHRYON_INLINE Ref<Quaternion> operator-=(ConstRef<Quaternion> Other)
        {
            mData -= Other.mData;
            return (* this);
        }

        /// \brief Multiplies the quaternion components by another quaternion (Hamilton product).
        /// 
        /// \param Other The quaternion to multiply.
        /// \return A reference to the updated quaternion.
        ZYPHRYON_INLINE Ref<Quaternion> operator*=(ConstRef<Quaternion> Other)
        {
            const Vector4 V1 = Vector4::Blend<0b1000>(mData, Vector4::Zero());
            const Vector4 V2 = Vector4::Blend<0b1000>(Other.mData, Vector4::Zero());
            const Vector4 W1 = Vector4::SplatW(mData);
            const Vector4 W2 = Vector4::SplatW(Other.mData);

            const Vector4 XYZ = W1 * V2 + W2 * V1 + Vector4::Cross(V1, V2);
            const Vector4 W   = W1 * W2 - Vector4(Vector4::Dot3(V1, V2));

            mData =Vector4::Blend<0b1000>(XYZ, W * Vector4::UnitW());
            return (* this);
        }

        /// \brief Multiplies all components of this quaternion by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A reference to the updated quaternion.
        ZYPHRYON_INLINE Ref<Quaternion> operator*=(Real32 Scalar)
        {
            mData *= Scalar;
            return (* this);
        }

        /// \brief Checks if this quaternion is equal to another quaternion.
        /// 
        /// \param Other The quaternion to compare to.
        /// \return `true` if all vectors are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE Bool operator==(ConstRef<Quaternion> Other) const
        {
            return mData == Other.mData || mData == (-Other.mData);
        }

        /// \brief Checks if this quaternion is not equal to another quaternion.
        /// 
        /// \param Other The quaternion to compare to.
        /// \return `true` if the vectors are not equal, `false` otherwise.
        ZYPHRYON_INLINE Bool operator!=(ConstRef<Quaternion> Other) const = default;

        /// \brief Computes a hash value for the object.
        ///
        /// \return A hash value uniquely representing the current state of the object.
        ZYPHRYON_INLINE UInt Hash() const
        {
            return mData.Hash();
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        /// 
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mData);
        }

    public:

        /// \brief Normalizes the given quaternion.
        /// 
        /// \param Quaternion The quaternion to normalize.
        /// \return A normalized quaternion.
        ZYPHRYON_INLINE static Quaternion Normalize(ConstRef<Quaternion> Quaternion)
        {
            return ::Quaternion(Vector4::Normalize(Quaternion.mData));
        }

        /// \brief Conjugate the given quaternion.
        /// 
        /// \param Quaternion The quaternion to conjugate.
        /// \return A conjugated quaternion.
        ZYPHRYON_INLINE static Quaternion Conjugate(ConstRef<Quaternion> Quaternion)
        {
            return Math::Quaternion(Quaternion.mData * Vector4(-1.0f, -1.0f, -1.0f, 1.0f));
        }

        /// \brief Invert the given quaternion.
        /// 
        /// \param Quaternion The quaternion to invert. Must be normalized if representing a rotation.
        /// \return A inverse quaternion.
        ZYPHRYON_INLINE static Quaternion Inverse(ConstRef<Quaternion> Quaternion)
        {
            const Real32 LengthSquared = Quaternion.GetLengthSquared();
            LOG_ASSERT(LengthSquared > kEpsilon<Real32>, "Cannot invert a zero-length quaternion");

            return Math::Quaternion(Conjugate(Quaternion).mData / LengthSquared);
        }

        /// \brief Computes the dot product of two quaternions.
        /// 
        /// \param P0 The first quaternion.
        /// \param P1 The second quaternion.
        /// \return The dot product of the two quaternions.
        ZYPHRYON_INLINE static Real32 Dot(ConstRef<Quaternion> P0, ConstRef<Quaternion> P1)
        {
            return Vector4::Dot(P0.mData, P1.mData);
        }

        /// \brief Rotates a 3D vector by the given quaternion.
        ///
        /// \param Rotation The quaternion that defines the rotation. Must be normalized.
        /// \param Vector   The vector to rotate (treated as a direction, w = 0).
        /// \return The rotated vector.
        ZYPHRYON_INLINE static Vector3 Rotate(ConstRef<Quaternion> Rotation, ConstRef<Vector3> Vector)
        {
            LOG_ASSERT(Rotation.IsNormalized(), "Quaternion must be normalized before rotating");

            const Vector4 AV = Vector4::Blend<0b0111>(Rotation.mData, Vector4::Zero());
            const Vector4 BV = Vector4(Vector.GetX(), Vector.GetY(), Vector.GetZ(), 0.0f);
            const Vector4 T  = Vector4::Cross(AV, BV) * 2.0f;
            const Vector4 VP = BV + Vector4::SplatW(Rotation.mData) * T + Vector4::Cross(AV, T);

            return VP.GetXYZ();
        }

        /// \brief Creates a quaternion from an angle and an axis of rotation.
        /// 
        /// \param Angle The angle of rotation (in radians).
        /// \param Axis  The axis to rotate around (normalized).
        /// \return A quaternion representing the rotation.
        ZYPHRYON_INLINE static Quaternion FromAngles(Real32 Angle, ConstRef<Vector3> Axis)
        {
            LOG_ASSERT(Axis.IsNormalized(), "Axis must be normalized before constructing a quaternion");

            const Real32 HalfAngle = 0.5f * Angle;
            return Quaternion(Axis * Sin(HalfAngle), Cos(HalfAngle));
        }

        /// \brief Creates a quaternion from Euler angles.
        /// 
        /// \param Angles A vector where X = pitch, Y = yaw, Z = roll (in radians).
        /// \return A quaternion representing the combined rotation.
        ZYPHRYON_INLINE static Quaternion FromEulerAngles(ConstRef<Vector3> Angles)
        {
            const Real32 HalfPitch = Angles.GetX() * 0.5f;
            const Real32 HalfYaw   = Angles.GetY() * 0.5f;
            const Real32 HalfRoll  = Angles.GetZ() * 0.5f;

            const Real32 CX = Cos(HalfPitch);
            const Real32 SX = Sin(HalfPitch);
            const Real32 CY = Cos(HalfYaw);
            const Real32 SY = Sin(HalfYaw);
            const Real32 CZ = Cos(HalfRoll);
            const Real32 SZ = Sin(HalfRoll);

            return Quaternion(SX * CY * CZ + CX * SY * SZ,
                              CX * SY * CZ - SX * CY * SZ,
                              CX * CY * SZ + SX * SY * CZ,
                              CX * CY * CZ - SX * SY * SZ);
        }

        /// @brief Generates a quaternion from the given direction and up vectors.
        /// 
        /// @param Direction The desired forward (normalized) direction vector.
        /// @param Up        The desired up (normalized) vector.
        /// 
        /// @return A quaternion representing the rotation from the given direction and up vectors.
        ZYPHRYON_INLINE static Quaternion FromDirection(ConstRef<Vector3> Direction, ConstRef<Vector3> Up)
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

        /// \brief Performs a linear interpolation between two quaternions.
        /// 
        /// \param Start      The starting quaternion.
        /// \param End        The ending quaternion.
        /// \param Percentage A value between 0 and 1 representing interpolation amount.
        /// \return A quaternion interpolated between Start and End.
        ZYPHRYON_INLINE static Quaternion Lerp(ConstRef<Quaternion> Start, ConstRef<Quaternion> End, Real32 Percentage)
        {
            LOG_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

            const Real32 Product = Dot(Start, End);
            return Start + ((Product < 0.0f ? -End : End) - Start) * Percentage;
        }

        /// \brief Performs a normalized linear interpolation between two quaternions.
        /// 
        /// \param Start      The starting quaternion.
        /// \param End        The ending quaternion.
        /// \param Percentage A value between 0 and 1 representing interpolation amount.
        /// \return A normalized quaternion interpolated between Start and End.
        ZYPHRYON_INLINE static Quaternion NLerp(ConstRef<Quaternion> Start, ConstRef<Quaternion> End, Real32 Percentage)
        {
            return Normalize(Lerp(Start, End, Percentage));
        }

        /// \brief Performs a spherical linear interpolation between two quaternions.
        /// 
        /// \param Start      The starting quaternion.
        /// \param End        The ending quaternion.
        /// \param Percentage A value between 0 and 1 representing interpolation amount.
        /// \return A normalized quaternion interpolated between Start and End.
        ZYPHRYON_INLINE static Quaternion Slerp(ConstRef<Quaternion> Start, ConstRef<Quaternion> End, Real32 Percentage)
        {
            LOG_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

            if (const Real32 Dot = Clamp(Quaternion::Dot(Start, End), -1.0f, +1.0f); Dot > 0.9995f)
            {
                return NLerp(Start, End, Percentage);
            }
            else
            {
                const Real32 Theta = InvCos(Dot) * Percentage;
                return (Start * Cos(Theta)) + (Normalize(End - Start * Dot) * Sin(Theta));
            }
        }

    private:

        /// \brief Constructs a quaternion from an existing vector.
        /// 
        /// \param Data A vector containing (x, y, z, w) components.
        ZYPHRYON_INLINE explicit Quaternion(Vector4 Data)
            : mData { Data }
        {
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector4 mData;
    };
}