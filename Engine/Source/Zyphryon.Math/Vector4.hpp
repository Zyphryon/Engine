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

#include "Vector3.hpp"

#if defined(ZY_ARCH_X86_64)
#   include <immintrin.h>
#elif defined(ZY_ARCH_ARM64)
#   include <arm_neon.h>
#elif defined(ZY_ARCH_WASM32) || defined(ZY_ARCH_WASM64)
#   include <wasm_simd128.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a generic vector with (x, y, z, w) floating-point components.
    class ZY_ALIGN(16) Vector4 final
    {
    public:
 
        /// \brief Constructor initializing all components to zero.
        ZY_INLINE Vector4();

        /// \brief Constructor initializing all components to the same scalar value.
        ///
        /// \param Scalar The scalar value used to initialize all components.
        ZY_INLINE Vector4(Real32 Scalar);

        /// \brief Constructor initializing the vector with specified x, y, z, and w values.
        ///
        /// \param X The x-component of the vector.
        /// \param Y The y-component of the vector.
        /// \param Z The z-component of the vector.
        /// \param W The w-component of the vector (default is 0.0f).
        ZY_INLINE Vector4(Real32 X, Real32 Y, Real32 Z, Real32 W = 0.0f);

        /// \brief Stores the vector components into a float array.
        ///
        /// \param Output Pointer to four 32-bit floats. Must be 16-byte aligned.
        ZY_INLINE void Store(Ptr<Real32> Output) const;

        /// \brief Checks if all components are true when interpreted as booleans.
        ///
        /// \return `true` if all components contain non-zero values, `false` otherwise.
        ZY_INLINE Bool IsAllTrue() const;

        /// \brief Checks if any component is true when interpreted as a boolean.
        ///
        /// \return `true` if any component contains a non-zero value, `false` otherwise.
        ZY_INLINE Bool IsAnyTrue() const;

        /// \brief Checks if the vector is approximately a zero vector.
        ///
        /// \return `true` if the squared length is within the tolerance, `false` otherwise.
        template<Real32 Tolerance = kEpsilon<Real32>>
        ZY_INLINE Bool IsAlmostZero() const
        {
            return GetLengthSquared() <= (Tolerance * Tolerance);
        }

        /// \brief Checks if this vector is approximately equal to another vector.
        ///
        /// \param Other The vector to compare to.
        /// \return `true` if all components are within the tolerance, `false` otherwise.
        template<Real32 Tolerance = kEpsilon<Real32>>
        ZY_INLINE Bool IsAlmostEqual(Vector4 Other) const;

        /// \brief Checks if the vector is normalized.
        ///
        /// \return `true` if the squared length is approximately 1.0f, `false` otherwise.
        template<Real32 Tolerance = kEpsilon<Real32>>
        ZY_INLINE Bool IsNormalized() const
        {
            return std::fabsf(GetLengthSquared() - 1.0f) <= Tolerance;
        }

        /// \brief Checks if this vector is parallel to another vector (based on the XYZ components).
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if the vectors are parallel, `false` otherwise.
        ZY_INLINE Bool IsParallel(Vector4 Other) const
        {
            return Cross3(* this, Other).IsAlmostZero();
        }

        /// \brief Checks if all components are finite values.
        ///
        /// \return `true` if all components are finite (not infinity or NaN), `false` otherwise.
        ZY_INLINE Bool IsFinite() const;

        /// \brief Gets the x-component of the vector.
        ///
        /// \return The x-component.
        ZY_INLINE Real32 GetX() const;

        /// \brief Gets the y-component of the vector.
        ///
        /// \return The y-component.
        ZY_INLINE Real32 GetY() const;

        /// \brief Gets the z-component of the vector.
        ///
        /// \return The z-component.
        ZY_INLINE Real32 GetZ() const;

        /// \brief Gets the w-component of the vector.
        ///
        /// \return The w-component.
        ZY_INLINE Real32 GetW() const;

        /// \brief Gets the (x, y) components as a 2D vector.
        ///
        /// \return A new 2D vector containing the x and y components.
        ZY_INLINE Vector2 GetXY() const
        {
            return Vector2(GetX(), GetY());
        }

        /// \brief Gets the (x, y, z) components as a 3D vector.
        ///
        /// \return A new 3D vector containing the x, y, and z components.
        ZY_INLINE Vector3 GetXYZ() const
        {
            return Vector3(GetX(), GetY(), GetZ());
        }

        /// \brief Calculates the length (magnitude) of the vector.
        ///
        /// \return The length of the vector.
        ZY_INLINE Real32 GetLength() const;

        /// \brief Calculates the squared length of the vector.
        ///
        /// \return The squared length of the vector (no square root).
        ZY_INLINE Real32 GetLengthSquared() const
        {
            return Dot(* this, * this);
        }

        /// \brief Calculates the Euclidean distance between this vector and another vector.
        ///
        /// \param Target The vector to calculate the distance to.
        /// \return The distance between the two vectors.
        ZY_INLINE Real32 GetDistance(Vector4 Target) const
        {
            return (* this - Target).GetLength();
        }

        /// \brief Calculates the squared distance between this vector and another vector.
        ///
        /// \param Target The vector to calculate the squared distance to.
        /// \return The squared distance between the two vectors.
        ZY_INLINE Real32 GetDistanceSquared(Vector4 Target) const
        {
            return (* this - Target).GetLengthSquared();
        }

        /// \brief Gets the angle in radians between this vector and another.
        ///
        /// \param Other The other vector to compare with.
        /// \return The angle in radians between the two vectors (range [0, π]).
        ZY_INLINE Angle GetAngle(Vector4 Other) const
        {
            const Real32 Length = GetLength() * Other.GetLength();
            ZY_ASSERT(!::IsAlmostZero(Length), "Cannot compute angle with zero-length vector");

            return Angle::FromCosine(Dot(* this, Other) / Length);
        }

        /// \brief Reduces the vector by summing all components.
        ///
        /// \return The sum of all four components (X + Y + Z + W).
        ZY_INLINE Real32 ReduceSum() const;

        /// \brief Reduces the vector by subtracting components sequentially.
        ///
        /// \return The result of subtracting components sequentially (X - Y - Z - W).
        ZY_INLINE Real32 ReduceSubtract() const;

        /// \brief Reduces the vector by finding the minimum component.
        ///
        /// \return The minimum value among all four components.
        ZY_INLINE Real32 ReduceMin() const;

        /// \brief Reduces the vector by finding the maximum component.
        ///
        /// \return The maximum value among all four components.
        ZY_INLINE Real32 ReduceMax() const;

        /// \brief Adds a scalar to all components of this vector.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A new vector with the scalar added to all components.
        ZY_INLINE Vector4 operator+(Real32 Scalar) const
        {
            return (* this) + Vector4(Scalar);
        }

        /// \brief Subtracts a scalar from all components of this vector.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A new vector with the scalar subtracted from all components.
        ZY_INLINE Vector4 operator-(Real32 Scalar) const
        {
            return (* this) - Vector4(Scalar);
        }

        /// \brief Multiplies all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A new vector with all components multiplied by the scalar.
        ZY_INLINE Vector4 operator*(Real32 Scalar) const
        {
            return (* this) * Vector4(Scalar);
        }

        /// \brief Divides all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A new vector with all components divided by the scalar.
        ZY_INLINE Vector4 operator/(Real32 Scalar) const
        {
            return (* this) / Vector4(Scalar);
        }

        /// \brief Adds another vector to this vector.
        ///
        /// \param Other The vector to add.
        /// \return A new vector that is the sum of this vector and the input vector.
        ZY_INLINE Vector4 operator+(Vector4 Other) const;

        /// \brief Subtracts another vector from this vector.
        ///
        /// \param Other The vector to subtract.
        /// \return A new vector that is the difference of the two vectors.
        ZY_INLINE Vector4 operator-(Vector4 Other) const;

        /// \brief Multiplies all components of this vector by another vector.
        ///
        /// \param Other The vector to multiply by.
        /// \return A new vector with component-wise multiplication.
        ZY_INLINE Vector4 operator*(Vector4 Other) const;

        /// \brief Divides all components of this vector by another vector.
        ///
        /// \param Other The vector to divide by.
        /// \return A new vector with component-wise division.
        ZY_INLINE Vector4 operator/(Vector4 Other) const;

        /// \brief Negates the vector.
        ///
        /// \return A new vector that is the negation of this vector.
        ZY_INLINE Vector4 operator-() const;

        /// \brief Computes the bitwise NOT of this vector.
        ///
        /// \return A new vector with all bits inverted.
        ZY_INLINE Vector4 operator~() const;

        /// \brief Computes the bitwise AND of this vector with another vector.
        ///
        /// \param Other The vector to AND with.
        /// \return A new vector with component-wise bitwise AND.
        ZY_INLINE Vector4 operator&(Vector4 Other) const;

        /// \brief Computes the bitwise OR of this vector with another vector.
        ///
        /// \param Other The vector to OR with.
        /// \return A new vector with component-wise bitwise OR.
        ZY_INLINE Vector4 operator|(Vector4 Other) const;

        /// \brief Computes the bitwise XOR of this vector with another vector.
        ///
        /// \param Other The vector to XOR with.
        /// \return A new vector with component-wise bitwise XOR.
        ZY_INLINE Vector4 operator^(Vector4 Other) const;

        /// \brief Adds a scalar value to all components of this vector in-place.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator+=(Real32 Scalar)
        {
            return (* this) = (* this) + Scalar;
        }

        /// \brief Subtracts a scalar value from all components of this vector in-place.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator-=(Real32 Scalar)
        {
            return (* this) = (* this) - Scalar;
        }

        /// \brief Multiplies all components of this vector by a scalar value in-place.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator*=(Real32 Scalar)
        {
            return (* this) = (* this) * Scalar;
        }

        /// \brief Divides all components of this vector by a scalar value in-place.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator/=(Real32 Scalar)
        {
            return (* this) = (* this) / Scalar;
        }

        /// \brief Adds another vector to this vector in-place.
        ///
        /// \param Other The vector to add.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator+=(Vector4 Other)
        {
            return (* this) = (* this) + Other;
        }

        /// \brief Subtracts another vector from this vector in-place.
        ///
        /// \param Other The vector to subtract.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator-=(Vector4 Other)
        {
            return (* this) = (* this) - Other;
        }

        /// \brief Multiplies this vector by another vector in-place.
        ///
        /// \param Other The vector to multiply by.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator*=(Vector4 Other)
        {
            return (* this) = (* this) * Other;
        }

        /// \brief Divides this vector by another vector in-place.
        ///
        /// \param Other The vector to divide by.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator/=(Vector4 Other)
        {
            return (* this) = (* this) / Other;
        }

        /// \brief Computes the bitwise AND of this vector with another vector in-place.
        ///
        /// \param Other The vector to AND with.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator&=(Vector4 Other)
        {
            return (* this) = (* this) & Other;
        }

        /// \brief Computes the bitwise OR of this vector with another vector in-place.
        ///
        /// \param Other The vector to OR with.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator|=(Vector4 Other)
        {
            return (* this) = (* this) | Other;
        }

        /// \brief Computes the bitwise XOR of this vector with another vector in-place.
        ///
        /// \param Other The vector to XOR with.
        /// \return A reference to the updated vector.
        ZY_INLINE Ref<Vector4> operator^=(Vector4 Other)
        {
            return (* this) = (* this) ^ Other;
        }

        /// \brief Compares this vector with another for component-wise less-than relationship.
        ///
        /// \param Other The vector to compare against.
        /// \return A new vector with component-wise comparison results (all bits set if true).
        ZY_INLINE Vector4 operator<(Vector4 Other) const;

        /// \brief Compares this vector with another for component-wise greater-than relationship.
        ///
        /// \param Other The vector to compare against.
        /// \return A new vector with component-wise comparison results (all bits set if true).
        ZY_INLINE Vector4 operator>(Vector4 Other) const;

        /// \brief Compares this vector with another for component-wise less-than or equal relationship.
        ///
        /// \param Other The vector to compare against.
        /// \return A new vector with component-wise comparison results (all bits set if true).
        ZY_INLINE Vector4 operator<=(Vector4 Other) const;

        /// \brief Compares this vector with another for component-wise greater-than or equal relationship.
        ///
        /// \param Other The vector to compare against.
        /// \return A new vector with component-wise comparison results (all bits set if true).
        ZY_INLINE Vector4 operator>=(Vector4 Other) const;

        /// \brief Compares this vector with another for component-wise equality.
        ///
        /// \param Other The vector to compare against.
        /// \return A new vector with component-wise comparison results (all bits set if equal).
        ZY_INLINE Vector4 operator==(Vector4 Other) const;

        /// \brief Compares this vector with another for component-wise inequality.
        ///
        /// \param Other The vector to compare against.
        /// \return A new vector with component-wise comparison results (all bits set if unequal).
        ZY_INLINE Vector4 operator!=(Vector4 Other) const;

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            Array<Real32, 4> Result;
            Store(Result.GetData());

            static constexpr Format::Pattern<9> kPattern("({0}, {1}, {2}, {3})");
            Format::Processor<Output>::Format(Buffer, kPattern, Result[0], Result[1], Result[2], Result[3]);
        }

    public:

        /// \brief Gets the zero vector (0.0f, 0.0f, 0.0f, 0.0f).
        ///
        /// \return The zero vector.
        ZY_INLINE static Vector4 Zero()
        {
            return Vector4();
        }

        /// \brief Gets the one vector (1.0f, 1.0f, 1.0f, 1.0f).
        ///
        /// \return The one vector.
        ZY_INLINE static Vector4 One()
        {
            return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        /// \brief Gets the unit vector along the X-axis (1.0f, 0.0f, 0.0f, 0.0f).
        ///
        /// \return The unit vector along the X-axis.
        ZY_INLINE static Vector4 UnitX()
        {
            return Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        }

        /// \brief Gets the unit vector along the Y-axis (0.0f, 1.0f, 0.0f, 0.0f).
        ///
        /// \return The unit vector along the Y-axis.
        ZY_INLINE static Vector4 UnitY()
        {
            return Vector4(0.0f, 1.0f, 0.0f, 0.0f);
        }

        /// \brief Gets the unit vector along the Z-axis (0.0f, 0.0f, 1.0f, 0.0f).
        ///
        /// \return The unit vector along the Z-axis.
        ZY_INLINE static Vector4 UnitZ()
        {
            return Vector4(0.0f, 0.0f, 1.0f, 0.0f);
        }

        /// \brief Gets the unit vector along the W-axis (0.0f, 0.0f, 0.0f, 1.0f).
        ///
        /// \return The unit vector along the W-axis.
        ZY_INLINE static Vector4 UnitW()
        {
            return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        }

        /// \brief Projects the source vector onto the target vector using XYZ components only (W ignored).
        ///
        /// \param Source The vector to be projected.
        /// \param Target The vector onto which the source is projected.
        /// \return The projection of source onto target.
        ZY_INLINE static Vector4 Project(Vector4 Source, Vector4 Target)
        {
            const Real32 Denominator = Dot3(Target, Target);
            ZY_ASSERT(!::IsAlmostZero(Denominator), "Cannot project onto zero-length vector");

            return Target * (Dot3(Source, Target) / Denominator);
        }

        /// \brief Normalizes the given vector to unit length.
        ///
        /// \param Vector The vector to normalize.
        /// \return A normalized vector with magnitude 1.0f.
        ZY_INLINE static Vector4 Normalize(Vector4 Vector);

        /// \brief Reflects the incident vector over the given normal using XYZ components only (W ignored).
        ///
        /// \param Incident The incoming vector to reflect.
        /// \param Normal   The surface normal to reflect across (should be normalized).
        /// \return The reflected vector.
        ZY_INLINE static Vector4 Reflect(Vector4 Incident, Vector4 Normal)
        {
            ZY_ASSERT(Normal.IsNormalized(), "Normal must be normalized before reflection");

            return Incident - Normal * (2.0f * Dot3(Incident, Normal));
        }

        /// \brief Computes the dot product of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The dot product of the two vectors.
        ZY_INLINE static Real32 Dot(Vector4 P0, Vector4 P1);

        /// \brief Computes the dot product of the XYZ components of two vectors, ignoring W.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The dot product of the XYZ components of the two vectors.
        ZY_INLINE static Real32 Dot3(Vector4 P0, Vector4 P1);

        /// \brief Computes the cross product of the XYZ components of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector containing the cross product of the XYZ components, with W = 0.
        ZY_INLINE static Vector4 Cross3(Vector4 P0, Vector4 P1);

        /// \brief Gets the component-wise minimum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the minimum values from each component.
        ZY_INLINE static Vector4 Min(Vector4 P0, Vector4 P1);

        /// \brief Gets the component-wise maximum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the maximum values from each component.
        ZY_INLINE static Vector4 Max(Vector4 P0, Vector4 P1);

        /// \brief Gets the absolute value of each component.
        ///
        /// \param Vector The vector to compute absolute value of.
        /// \return A vector with the absolute value of each component.
        ZY_INLINE static Vector4 Abs(Vector4 Vector);

        /// \brief Clamps each component of a vector between corresponding min and max components.
        ///
        /// \param Vector The source vector to clamp.
        /// \param Min    The vector specifying the minimum bounds.
        /// \param Max    The vector specifying the maximum bounds.
        /// \return A vector with each component clamped between the min and max values.
        ZY_INLINE static Vector4 Clamp(Vector4 Vector, Vector4 Min, Vector4 Max)
        {
            return Vector4::Max(Vector4::Min(Vector, Max), Min);
        }

        /// \brief Gets a vector with each component floored to the nearest integer.
        ///
        /// \param Vector The vector to floor.
        /// \return A vector with all components rounded down.
        ZY_INLINE static Vector4 Floor(Vector4 Vector);

        /// \brief Gets a vector with each component ceiled to the nearest integer.
        ///
        /// \param Vector The vector to ceil.
        /// \return A vector with all components rounded up.
        ZY_INLINE static Vector4 Ceil(Vector4 Vector);

        /// \brief Computes the reciprocal square root of each component.
        ///
        /// \param Vector The vector to compute reciprocal square root of.
        /// \return A vector with the reciprocal square root of each component.
        ZY_INLINE static Vector4 ReciprocalSqrt(Vector4 Vector);

        /// \brief Computes the fast reciprocal square root of each component.
        ///
        /// \param Vector The vector to compute fast reciprocal square root of.
        /// \return A vector with the fast reciprocal square root of each component.
        ZY_INLINE static Vector4 ReciprocalSqrtFast(Vector4 Vector);

        /// \brief Computes the square root of each component.
        ///
        /// \param Vector The vector to compute square root of.
        /// \return A vector with the square root of each component.
        ZY_INLINE static Vector4 Sqrt(Vector4 Vector);

        /// \brief Computes the fast square root of each component.
        ///
        /// \param Vector The vector to compute fast square root of.
        /// \return A vector with the fast square root of each component.
        ZY_INLINE static Vector4 SqrtFast(Vector4 Vector);

        /// \brief Shuffles components with another vector using template indices.
        ///
        /// \tparam X The index for the X component (0-3 are from the first vector, 4-7 from the second).
        /// \tparam Y The index for the Y component (0-3 are from the first vector, 4-7 from the second).
        /// \tparam Z The index for the Z component (0-3 are from the first vector, 4-7 from the second).
        /// \tparam W The index for the W component (0-3 are from the first vector, 4-7 from the second).
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A new vector with shuffled components.
        template<SInt X, SInt Y, SInt Z, SInt W>
        ZY_INLINE static Vector4 Shuffle(Vector4 P0, Vector4 P1);

        /// \brief Swizzles components of a vector using template indices.
        ///
        /// \tparam X The index for the X component (0-3).
        /// \tparam Y The index for the Y component (0-3).
        /// \tparam Z The index for the Z component (0-3).
        /// \tparam W The index for the W component (0-3).
        /// \param Vector The vector to swizzle.
        /// \return A new vector with swizzled components from input vector.
        template<SInt X, SInt Y, SInt Z, SInt W>
        ZY_INLINE static Vector4 Swizzle(Vector4 Vector)
        {
            return Shuffle<X, Y, Z, W>(Vector, Vector);
        }

        /// \brief Replicates the X component to all components.
        ///
        /// \param Vector The vector to splat.
        /// \return A vector with all components set to the X value.
        ZY_INLINE static Vector4 SplatX(Vector4 Vector)
        {
            return Swizzle<0, 0, 0, 0>(Vector);
        }

        /// \brief Replicates the Y component to all components.
        ///
        /// \param Vector The vector to splat.
        /// \return A vector with all components set to the Y value.
        ZY_INLINE static Vector4 SplatY(Vector4 Vector)
        {
            return Swizzle<1, 1, 1, 1>(Vector);
        }

        /// \brief Replicates the Z component to all components.
        ///
        /// \param Vector The vector to splat.
        /// \return A vector with all components set to the Z value.
        ZY_INLINE static Vector4 SplatZ(Vector4 Vector)
        {
            return Swizzle<2, 2, 2, 2>(Vector);
        }

        /// \brief Replicates the W component to all components.
        ///
        /// \param Vector The vector to splat.
        /// \return A vector with all components set to the W value.
        ZY_INLINE static Vector4 SplatW(Vector4 Vector)
        {
            return Swizzle<3, 3, 3, 3>(Vector);
        }

        /// \brief Selects components from two vectors based on a compile-time mask.
        ///
        /// \tparam Mask The selection mask (bit i selects from P1 if set, else from P0).
        /// \param P0 The vector to select from when mask bits are clear.
        /// \param P1 The vector to select from when mask bits are set.
        /// \return A new vector with selected components.
        template<UInt8 Mask>
        ZY_INLINE static Vector4 Select(Vector4 P0, Vector4 P1);

        /// \brief Selects components from two vectors based on a dynamic mask.
        ///
        /// \param P0 The vector to select from when mask components are zero.
        /// \param P1 The vector to select from when mask components are non-zero.
        /// \param Mask The mask vector (non-zero components select from P1).
        /// \return A new vector with selected components.
        ZY_INLINE static Vector4 Select(Vector4 P0, Vector4 P1, Vector4 Mask);

        /// \brief Merges the lower components of two vectors.
        ///
        /// \param P0 The first vector providing lower components.
        /// \param P1 The second vector providing lower components.
        /// \return A new vector with lower components from both vectors merged.
        ZY_INLINE static Vector4 MergeLow(Vector4 P0, Vector4 P1);

        /// \brief Merges the higher components of two vectors.
        ///
        /// \param P0 The first vector providing higher components.
        /// \param P1 The second vector providing higher components.
        /// \return A new vector with higher components from both vectors merged.
        ZY_INLINE static Vector4 MergeHigh(Vector4 P0, Vector4 P1);

        /// \brief Zips the lower components of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A new vector with lower components from both vectors zipped.
        ZY_INLINE static Vector4 ZipLow(Vector4 P0, Vector4 P1);

        /// \brief Zips the higher components of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A new vector with higher components from both vectors zipped.
        ZY_INLINE static Vector4 ZipHigh(Vector4 P0, Vector4 P1);

        /// \brief Swizzles a vector to arrange components as (0, 0, 2, 2).
        ///
        /// \param Vector The vector to swizzle.
        /// \return A new vector with components rearranged.
        ZY_INLINE static Vector4 Swizzle0022(Vector4 Vector);

        /// \brief Swizzles a vector to arrange components as (1, 1, 3, 3).
        ///
        /// \param Vector The vector to swizzle.
        /// \return A new vector with components rearranged.
        ZY_INLINE static Vector4 Swizzle1133(Vector4 Vector);

    private:

#if   defined(ZY_ARCH_X86_64)
        /// \brief Underlying platform SIMD register type.
        using Type = __m128;
#elif defined(ZY_ARCH_ARM64)
        /// \brief Underlying platform SIMD register type.
        using Type = float32x4_t;
#elif defined(ZY_ARCH_WASM32) || defined(ZY_ARCH_WASM64)
        /// \brief Underlying platform SIMD register type.
        using Type = v128_t;
#endif

        /// \brief Constructs a vector from a platform-specific SIMD register.
        ///
        /// \param Value The SIMD register value to initialize the vector with.
        ZY_INLINE Vector4(Type Value)
            : mData { Value }
        {
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

         Type mData;
    };
}

#if   defined(ZY_ARCH_X86_64)
#   include "Vector4_SSE4.inl"
#elif defined(ZY_ARCH_ARM64)
#   include "Vector4_NEON.inl"
#elif defined(ZY_ARCH_WASM32) || defined(ZY_ARCH_WASM64)
#   include "Vector4_WASM.inl"
#endif
