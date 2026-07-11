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

#include "Vector4.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a quaternion for 3D rotations.
    class ZY_ALIGN(16) Quaternion final
    {
        friend class Matrix4x4;

    public:

        /// \brief Constructs an identity quaternion.
        ZY_INLINE Quaternion()
            : mData { 0.0f, 0.0f, 0.0f, 1.0f }
        {
        }

        /// \brief Constructs a quaternion from an existing vector.
        ///
        /// \param Data A vector containing (x, y, z, w) components.
        ZY_INLINE explicit Quaternion(Vector4 Data)
            : mData { Data }
        {
        }

        /// \brief Constructs a quaternion from explicit components.
        ///
        /// \param X The x-component of the quaternion.
        /// \param Y The y-component of the quaternion.
        /// \param Z The z-component of the quaternion.
        /// \param W The w-component of the quaternion.
        ZY_INLINE Quaternion(Real32 X, Real32 Y, Real32 Z, Real32 W)
            : mData { X, Y, Z, W }
        {
        }

        /// \brief Constructs a quaternion from its imaginary (vector) part and real (scalar) part.
        ///
        /// \param Imaginary The vector part of the quaternion (x, y, z components).
        /// \param Real      The scalar (real) part of the quaternion (w component).
        ZY_INLINE Quaternion(Vector3 Imaginary, Real32 Real)
            : mData { Imaginary.GetX(), Imaginary.GetY(), Imaginary.GetZ(), Real }
        {
        }

        /// \brief Stores the quaternion components into a float array.
        ///
        /// \param  Output Pointer to four 32-bit floats. Must be 16-byte aligned.
        ZY_INLINE void Store(Ptr<Real32> Output) const
        {
            mData.Store(Output);
        }

        /// \brief Checks if the quaternion is the zero quaternion.
        /// 
        /// \return `true` if all components are approximately zero, `false` otherwise.
        ZY_INLINE Bool IsAlmostZero() const
        {
            return mData.IsAlmostZero();
        }

        /// \brief Checks if all components are approximately equal to the given values.
        /// 
        /// \param X The value to compare with the X component.
        /// \param Y The value to compare with the Y component.
        /// \param Z The value to compare with the Z component.
        /// \param W The value to compare with the W component.
        /// \return `true` if all components are within epsilon, `false` otherwise.
        ZY_INLINE Bool IsAlmostEqual(Real32 X, Real32 Y, Real32 Z, Real32 W) const
        {
            return mData.IsAlmostEqual(Vector4(X, Y, Z, W));
        }

        /// \brief Checks if the quaternion is normalized.
        /// 
        /// \return `true` if the quaternion is normalized, `false` otherwise.
        ZY_INLINE Bool IsNormalized() const
        {
            return mData.IsNormalized();
        }

        /// \brief Checks if the quaternion is finite.
        /// 
        /// \return `true` if the quaternion is finite, `false` otherwise.
        ZY_INLINE Bool IsFinite() const
        {
            return mData.IsFinite();
        }

        /// \brief Gets the x-component of the quaternion.
        /// 
        /// \return The x-component.
        ZY_INLINE Real32 GetX() const
        {
            return mData.GetX();
        }

        /// \brief Gets the y-component of the quaternion.
        /// 
        /// \return The y-component.
        ZY_INLINE Real32 GetY() const
        {
            return mData.GetY();
        }

        /// \brief Gets the z-component of the quaternion.
        /// 
        /// \return The z-component.
        ZY_INLINE Real32 GetZ() const
        {
            return mData.GetZ();
        }

        /// \brief Gets the w-component of the quaternion.
        /// 
        /// \return The w-component.
        ZY_INLINE Real32 GetW() const
        {
            return mData.GetW();
        }

        /// \brief Calculates the length (magnitude) of the quaternion.
        /// 
        /// \return The length of the quaternion.
        ZY_INLINE Real32 GetLength() const
        {
            return mData.GetLength();
        }

        /// \brief Calculates the squared length of the quaternion.
        /// 
        /// \return The squared length of the quaternion (no square root).
        ZY_INLINE Real32 GetLengthSquared() const
        {
            return mData.GetLengthSquared();
        }

        /// \brief Gets the forward (Z+) direction vector rotated by this quaternion.
        /// 
        /// \return The forward direction in world space as a Vector3.
        ZY_INLINE Vector3 GetForward() const
        {
            return Rotate(* this, Vector3::UnitZ());
        }

        /// \brief Gets the backward (Z–) direction vector rotated by this quaternion.
        /// 
        /// \return The backward direction in world space as a Vector3.
        ZY_INLINE Vector3 GetBackward() const
        {
            return -GetForward();
        }

        /// \brief Gets the up (Y+) direction vector rotated by this quaternion.
        /// 
        /// \return The up direction in world space as a Vector3.
        ZY_INLINE Vector3 GetUp() const
        {
            return Rotate(* this, Vector3::UnitY());
        }

        /// \brief Gets the down (Y–) direction vector rotated by this quaternion.
        /// 
        /// \return The down direction in world space as a Vector3.
        ZY_INLINE Vector3 GetDown() const
        {
            return -GetUp();
        }

        /// \brief Gets the right (X+) direction vector rotated by this quaternion.
        /// 
        /// \return The right direction in world space as a Vector3.
        ZY_INLINE Vector3 GetRight() const
        {
            return Rotate(* this, Vector3::UnitX());
        }

        /// \brief Gets the left (X–) direction vector rotated by this quaternion.
        /// 
        /// \return The left direction in world space as a Vector3.
        ZY_INLINE Vector3 GetLeft() const
        {
            return -GetRight();
        }

        /// \brief Extracts the axis-angle representation from this quaternion.
        ///
        /// \param Rotation The output angle of rotation in radians.
        /// \return The normalized rotation axis as a Vector3.
        ZY_INLINE Vector3 GetAxis(Ref<Angle> Rotation) const
        {
            ZY_ASSERT(IsNormalized(), "Quaternion must be normalized to extract axis-angle");

            const Real32 CosHalfAngle = Clamp(GetW(), -1.0f, 1.0f);
            const Real32 Sin          = Sqrt(1.0f - CosHalfAngle * CosHalfAngle);

            Rotation = 2.0f * Angle::FromCosine(CosHalfAngle).GetRadians();
            return (Sin > kEpsilon<Real32>) ? Vector3(GetX(), GetY(), GetZ()) / Sin : Vector3::UnitX();
        }

        /// \brief Decomposes the quaternion into Euler angles (pitch, yaw, roll).
        ///
        /// \return A vector where X = pitch, Y = yaw, Z = roll (in radians).
        Vector3 ToEulerAngles() const;

        /// \brief Adds another quaternion to this quaternion.
        /// 
        /// \param Other The quaternion to add.
        /// \return A new quaternion that is the sum of this quaternion and the input quaternion.
        ZY_INLINE Quaternion operator+(Quaternion Other) const
        {
            return Quaternion(mData + Other.mData);
        }

        /// \brief Negates the quaternion.
        /// 
        /// \return A new quaternion that is the negation of this quaternion.
        ZY_INLINE Quaternion operator-() const
        {
            return Quaternion(- mData);
        }

        /// \brief Subtracts another quaternion from this quaternion.
        /// 
        /// \param Other The quaternion to subtract.
        /// \return A new quaternion that is the difference of the two quaternions.
        ZY_INLINE Quaternion operator-(Quaternion Other) const
        {
            return Quaternion(mData - Other.mData);
        }

        /// \brief Multiplies the quaternion components by another quaternion (Hamilton product).
        ///
        /// \param Other The quaternion to multiply.
        /// \return A new quaternion with the combined rotation.
        ZY_INLINE Quaternion operator*(Quaternion Other) const
        {
            ZY_ASSERT(IsNormalized(), "Quaternion must be normalized before rotating");

            const Vector4 V1 = Vector4::Select<0b1000>(mData, Vector4::Zero());
            const Vector4 V2 = Vector4::Select<0b1000>(Other.mData, Vector4::Zero());
            const Vector4 W1 = Vector4::SplatW(mData);
            const Vector4 W2 = Vector4::SplatW(Other.mData);

            const Vector4 XYZ = W1 * V2 + W2 * V1 + Vector4::Cross3(V1, V2);
            const Vector4 W   = W1 * W2 - Vector4(Vector4::Dot3(V1, V2));

            return Quaternion(Vector4::Select<0b1000>(XYZ, W * Vector4::UnitW()));
        }

        /// \brief Multiplies all components of this quaternion by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new quaternion with the scalar multiplied by all components.
        ZY_INLINE Quaternion operator*(Real32 Scalar) const
        {
            return Quaternion(mData * Scalar);
        }

        /// \brief Adds another quaternion to the current quaternion.
        ///
        /// \param Other The quaternion to add.
        /// \return A reference to the updated quaternion.
        ZY_INLINE Ref<Quaternion> operator+=(Quaternion Other)
        {
            mData += Other.mData;
            return (* this);
        }

        /// \brief Subtracts another quaternion from the current quaternion.
        ///
        /// \param Other The quaternion to subtract.
        /// \return A reference to the updated quaternion.
        ZY_INLINE Ref<Quaternion> operator-=(Quaternion Other)
        {
            mData -= Other.mData;
            return (* this);
        }

        /// \brief Multiplies the quaternion components by another quaternion (Hamilton product).
        /// 
        /// \param Other The quaternion to multiply.
        /// \return A reference to the updated quaternion.
        ZY_INLINE Ref<Quaternion> operator*=(Quaternion Other)
        {
            ZY_ASSERT(IsNormalized(), "Quaternion must be normalized before rotating");

            const Vector4 V1 = Vector4::Select<0b1000>(mData, Vector4::Zero());
            const Vector4 V2 = Vector4::Select<0b1000>(Other.mData, Vector4::Zero());
            const Vector4 W1 = Vector4::SplatW(mData);
            const Vector4 W2 = Vector4::SplatW(Other.mData);

            const Vector4 XYZ = W1 * V2 + W2 * V1 + Vector4::Cross3(V1, V2);
            const Vector4 W   = W1 * W2 - Vector4(Vector4::Dot3(V1, V2));

            mData = Vector4::Select<0b1000>(XYZ, W * Vector4::UnitW());
            return (* this);
        }

        /// \brief Multiplies all components of this quaternion by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A reference to the updated quaternion.
        ZY_INLINE Ref<Quaternion> operator*=(Real32 Scalar)
        {
            mData *= Scalar;
            return (* this);
        }

        /// \brief Checks if this quaternion is equal to another quaternion.
        /// 
        /// \param Other The quaternion to compare to.
        /// \return `true` if all vectors are approximately equal, `false` otherwise.
        ZY_INLINE Bool operator==(Quaternion Other) const
        {
            return mData.IsAlmostEqual(Other.mData) || mData.IsAlmostEqual(-Other.mData);
        }

        /// \brief Checks if this quaternion is not equal to another quaternion.
        /// 
        /// \param Other The quaternion to compare to.
        /// \return `true` if the vectors are not equal, `false` otherwise.
        ZY_INLINE Bool operator!=(Quaternion Other) const
        {
            return !(* this == Other);
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            Angle Rotation;
            const Vector3 Axis = GetAxis(Rotation);

            static constexpr Format::Pattern<5> kPattern("(Axis: {0} Angle: {1})");
            Format::Processor<Output>::Format(Buffer, kPattern, Axis, Rotation);
        }

    public:

        /// \brief Normalizes the given quaternion.
        /// 
        /// \param Quaternion The quaternion to normalize.
        /// \return The normalized quaternion.
        ZY_INLINE static Quaternion Normalize(Quaternion Quaternion)
        {
            return ::Quaternion(Vector4::Normalize(Quaternion.mData));
        }

        /// \brief Conjugate the given quaternion.
        /// 
        /// \param Quaternion The quaternion to conjugate.
        /// \return The conjugated quaternion.
        ZY_INLINE static Quaternion Conjugate(Quaternion Quaternion)
        {
            return Math::Quaternion(Quaternion.mData * Vector4(-1.0f, -1.0f, -1.0f, 1.0f));
        }

        /// \brief Invert the given quaternion.
        /// 
        /// \param Quaternion The quaternion to invert. Must be normalized if representing a rotation.
        /// \return The inverse quaternion.
        ZY_INLINE static Quaternion Inverse(Quaternion Quaternion)
        {
            const Real32 LengthSquared = Quaternion.GetLengthSquared();
            ZY_ASSERT(LengthSquared > kEpsilon<Real32>, "Cannot invert a zero-length quaternion");

            return Math::Quaternion(Conjugate(Quaternion).mData / LengthSquared);
        }

        /// \brief Computes the dot product of two quaternions.
        /// 
        /// \param P0 The first quaternion.
        /// \param P1 The second quaternion.
        /// \return The dot product of the two quaternions.
        ZY_INLINE static Real32 Dot(Quaternion P0, Quaternion P1)
        {
            return Vector4::Dot(P0.mData, P1.mData);
        }

        /// \brief Rotates a 3D vector by the given quaternion.
        ///
        /// \param Rotation The quaternion that defines the rotation. Must be normalized.
        /// \param Vector   The vector to rotate (treated as a direction, w = 0).
        /// \return The rotated vector.
        ZY_INLINE static Vector3 Rotate(Quaternion Rotation, Vector3 Vector)
        {
            ZY_ASSERT(Rotation.IsNormalized(), "Quaternion must be normalized before rotating");

            const Vector4 AV = Vector4::Select<0b1000>(Rotation.mData, Vector4::Zero());
            const Vector4 BV = Vector4(Vector.GetX(), Vector.GetY(), Vector.GetZ(), 0.0f);
            const Vector4 T  = Vector4::Cross3(AV, BV) * 2.0f;
            const Vector4 VP = BV + Vector4::SplatW(Rotation.mData) * T + Vector4::Cross3(AV, T);

            return VP.GetXYZ();
        }

        /// \brief Creates a quaternion from an angle and an axis of rotation.
        /// 
        /// \param Rotation The rotation angle (in radians).
        /// \param Axis     The axis to rotate around (normalized).
        /// \return The quaternion representing the rotation.
        ZY_INLINE static Quaternion FromAngles(Angle Rotation, Vector3 Axis)
        {
            ZY_ASSERT(Axis.IsNormalized(), "Axis must be normalized before constructing a quaternion");

            const Angle HalfAngle = Rotation * 0.5f;
            return Quaternion(Axis * Angle::Sine(HalfAngle), Angle::Cosine(HalfAngle));
        }

        /// \brief Creates a quaternion from Euler angles.
        /// 
        /// \param Angles A vector where X = pitch, Y = yaw, Z = roll (in radians).
        /// \return The quaternion representing the combined rotation.
        static Quaternion FromEulerAngles(Vector3 Angles);

        /// \brief Generates a quaternion from the given direction and up vectors.
        /// 
        /// \param Direction The desired forward (normalized) direction vector.
        /// \param Up        The desired up (normalized) vector.
        /// 
        /// \return The quaternion representing the rotation from the given direction and up vectors.
        static Quaternion FromDirection(Vector3 Direction, Vector3 Up);

        /// \brief Performs a spherical linear interpolation between two quaternions.
        ///
        /// \param Start      The starting quaternion.
        /// \param End        The ending quaternion.
        /// \param Percentage A value between 0 and 1 representing interpolation amount.
        /// \return The normalized quaternion interpolated between Start and End.
        static Quaternion Slerp(Quaternion Start, Quaternion End, Real32 Percentage);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector4 mData;
    };
}