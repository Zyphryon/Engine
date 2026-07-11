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
    /// \brief Represents a generic vector with (x, y) components.
    template<typename Type>
    class AnyVector2 final
    {
    public:

        /// \brief Constructor initializing all components zero.
        ZY_INLINE constexpr AnyVector2()
            : mX { 0 },
              mY { 0 }
        {
        }

        /// \brief Constructor initializing all components to the same value.
        ///
        /// \param Scalar The scalar value used to initialize all components.
        ZY_INLINE constexpr explicit AnyVector2(Type Scalar)
            : mX { Scalar },
              mY { Scalar }
        {
        }

        /// \brief Constructor initializing the vector with specified x and y values.
        ///
        /// \param X The x-component of the vector.
        /// \param Y The y-component of the vector.
        ZY_INLINE constexpr AnyVector2(Type X, Type Y)
            : mX { X },
              mY { Y }
        {
        }

        /// \brief Conversion constructor from another vector with a different base type.
        ///
        /// \param Other The source vector to convert from.
        template<typename Base>
        ZY_INLINE constexpr explicit AnyVector2(AnyVector2<Base> Other)
            : mX { static_cast<Type>(Other.GetX()) },
              mY { static_cast<Type>(Other.GetY()) }
        {
        }

        /// \brief Checks if the vector is the zero vector.
        ///
        /// \return `true` if all components are approximately zero, `false` otherwise.
        ZY_INLINE constexpr Bool IsAlmostZero() const
        {
            return ::IsAlmostZero(mX) && ::IsAlmostZero(mY);
        }

        /// \brief Checks if all components are approximately equal to the given values.
        ///
        /// \param X The value to compare with the X component.
        /// \param Y The value to compare with the Y component.
        /// \return `true` if all components are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool IsAlmostEqual(Type X, Type Y) const
        {
            return ::IsAlmostEqual(mX, X) && ::IsAlmostEqual(mY, Y);
        }

        /// \brief Checks if the vector is normalized.
        ///
        /// \return `true` if the vector is normalized, `false` otherwise.
        ZY_INLINE constexpr Bool IsNormalized() const
            requires (IsReal<Type>)
        {
            return ::IsAlmostEqual(GetLengthSquared(), Type(1));
        }

        /// \brief Checks if all components are approximately equal within a tolerance.
        ///
        /// \param Tolerance The maximum allowed difference between components.
        /// \return `true` if all components are within \p Tolerance of each other, `false` otherwise.
        ZY_INLINE constexpr Bool IsUniform(Type Tolerance = kEpsilon<Type>) const
        {
            return (Abs(GetX() - GetY()) <= Tolerance);
        }

        /// \brief Checks if this vector is component-wise less than another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if all components are less, otherwise `false`.
        ZY_INLINE constexpr Bool IsComponentWiseLess(AnyVector2 Other) const
        {
            return mX < Other.mX && mY < Other.mY;
        }

        /// \brief Checks if this vector is component-wise less than or equal to another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if all components are less or equal, otherwise `false`.
        ZY_INLINE constexpr Bool IsComponentWiseLessOrEqual(AnyVector2 Other) const
        {
            return mX <= Other.mX && mY <= Other.mY;
        }

        /// \brief Checks if this vector is component-wise greater than another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if all components are greater, otherwise `false`.
        ZY_INLINE constexpr Bool IsComponentWiseGreater(AnyVector2 Other) const
        {
            return mX > Other.mX && mY > Other.mY;
        }

        /// \brief Checks if this vector is component-wise greater than or equal to another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if all components are greater or equal, otherwise `false`.
        ZY_INLINE constexpr Bool IsComponentWiseGreaterOrEqual(AnyVector2 Other) const
        {
            return mX >= Other.mX && mY >= Other.mY;
        }

        /// \brief Checks if this vector is parallel to another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if the vectors are parallel, `false` otherwise.
        ZY_INLINE constexpr Bool IsParallel(AnyVector2 Other) const
        {
            return ::IsAlmostZero(Cross(* this, Other));
        }

        /// \brief Sets the components of the vector to new values.
        ///
        /// \param X The new x value.
        /// \param Y The new y value.
        /// \return A reference to this vector after the update.
        ZY_INLINE constexpr Ref<AnyVector2> Set(Type X, Type Y)
        {
            mX = X;
            mY = Y;
            return (* this);
        }

        /// \brief Sets the x-component of the vector.
        ///
        /// \param X The new x-component.
        /// \return A reference to this vector after the update.
        ZY_INLINE constexpr Ref<AnyVector2> SetX(Type X)
        {
            mX = X;
            return (* this);
        }

        /// \brief Gets the x-component of the vector.
        ///
        /// \return The x-component.
        ZY_INLINE constexpr Type GetX() const
        {
            return mX;
        }

        /// \brief Sets the y-component of the vector.
        ///
        /// \param Y The new y-component.
        /// \return A reference to this vector after the update.
        ZY_INLINE constexpr Ref<AnyVector2> SetY(Type Y)
        {
            mY = Y;
            return (* this);
        }

        /// \brief Gets the y-component of the vector.
        ///
        /// \return The y-component.
        ZY_INLINE constexpr Type GetY() const
        {
            return mY;
        }

        /// \brief Calculates the length (magnitude) of the vector.
        ///
        /// \return The length of the vector.
        ZY_INLINE Type GetLength() const
            requires (IsReal<Type>)
        {
            return Sqrt(GetLengthSquared());
        }

        /// \brief Calculates the squared length of the vector.
        ///
        /// \return The squared length of the vector (no square root).
        ZY_INLINE constexpr Type GetLengthSquared() const
        {
            return (mX * mX + mY * mY);
        }

        /// \brief Calculates the Euclidean distance between this vector and another vector.
        ///
        /// \param Target The vector to calculate the distance to.
        /// \return The distance between the two vectors.
        ZY_INLINE Type GetDistance(AnyVector2 Target) const
            requires (IsReal<Type>)
        {
            const AnyVector2 Difference = (* this) - Target;
            return Difference.GetLength();
        }

        /// \brief Calculates the Manhattan distance between this vector and another vector.
        ///
        /// \param Target The vector to calculate the distance to.
        /// \return The distance between the two vectors.
        ZY_INLINE constexpr Type GetDistanceManhattan(AnyVector2 Target) const
        {
            const AnyVector2 Difference = (* this) - Target;
            return Abs(Difference.GetX()) + Abs(Difference.GetY());
        }

        /// \brief Calculates the Chebyshev distance between this vector and another vector.
        ///
        /// \param Target The vector to calculate the distance to.
        /// \return The distance between the two vectors.
        ZY_INLINE constexpr Type GetDistanceChebyshev(AnyVector2 Target) const
        {
            const AnyVector2 Difference = (* this) - Target;
            return Max(Abs(Difference.GetX()), Abs(Difference.GetY()));
        }

        /// \brief Calculates the squared distance between this vector and another vector.
        ///
        /// \param Target The vector to calculate the squared distance to.
        /// \return The squared distance between the two vectors.
        ZY_INLINE constexpr Type GetDistanceSquared(AnyVector2 Target) const
        {
            return ((* this) - Target).GetLengthSquared();
        }

        /// \brief Calculates the angle of the vector relative to the x-axis.
        ///
        /// \return The angle in radians (range [-π, π]).
        ZY_INLINE Angle GetAngle() const
            requires (IsReal<Type>)
        {
            return Angle::FromCartesian(mX, mY);
        }

        /// \brief Gets the angle in radians between this vector and another.
        ///
        /// \param Other The other vector to compare with.
        /// \return The angle in radians between the two vectors (range [0, π]).
        ZY_INLINE Angle GetAngle(AnyVector2 Other) const
            requires (IsReal<Type>)
        {
            const Type Length = GetLength() * Other.GetLength();
            ZY_ASSERT(!::IsAlmostZero(Length), "Cannot compute angle with zero-length vector");

            return Angle::FromCosine(Dot(* this, Other) / Length);
        }

        /// \brief Adds another vector to this vector.
        ///
        /// \param Other The vector to add.
        /// \return A new vector that is the sum of this vector and the input vector.
        ZY_INLINE constexpr AnyVector2 operator+(AnyVector2 Other) const
        {
            return AnyVector2(mX + Other.mX, mY + Other.mY);
        }

        /// \brief Adds a scalar to all components of this vector.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new vector with the scalar added to all components.
        ZY_INLINE constexpr AnyVector2 operator+(Type Scalar) const
        {
            return AnyVector2(mX + Scalar, mY + Scalar);
        }

        /// \brief Negates the vector.
        ///
        /// \return A new vector that is the negation of this vector.
        ZY_INLINE constexpr AnyVector2 operator-() const
        {
            return AnyVector2(-mX, -mY);
        }

        /// \brief Subtracts another vector from this vector.
        ///
        /// \param Other The vector to subtract.
        /// \return A new vector that is the difference of the two vectors.
        ZY_INLINE constexpr AnyVector2 operator-(AnyVector2 Other) const
        {
            return AnyVector2(mX - Other.mX, mY - Other.mY);
        }

        /// \brief Subtracts a scalar from all components of this vector.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new vector with the scalar subtracted from all components.
        ZY_INLINE constexpr AnyVector2 operator-(Type Scalar) const
        {
            return AnyVector2(mX - Scalar, mY - Scalar);
        }

        /// \brief Multiplies all components of this vector by another vector.
        ///
        /// \param Other The vector to multiply by.
        /// \return A new vector that is the product of the two vectors.
        ZY_INLINE constexpr AnyVector2 operator*(AnyVector2 Other) const
        {
            return AnyVector2(mX * Other.mX, mY * Other.mY);
        }

        /// \brief Multiplies all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new vector with the scalar multiplied by all components.
        ZY_INLINE constexpr AnyVector2 operator*(Type Scalar) const
        {
            return AnyVector2(mX * Scalar, mY * Scalar);
        }

        /// \brief Divides all components of this vector by another vector.
        ///
        /// \param Other The vector to divide by.
        /// \return A new vector that is the quotient of the two vectors.
        ZY_INLINE constexpr AnyVector2 operator/(AnyVector2 Other) const
        {
            ZY_ASSERT(!::IsAlmostZero(Other.GetX()), "Division by zero (X)");
            ZY_ASSERT(!::IsAlmostZero(Other.GetY()), "Division by zero (Y)");

            return AnyVector2(mX / Other.mX, mY / Other.mY);
        }

        /// \brief Divides all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new vector with the scalar divided into all components.
        ZY_INLINE constexpr AnyVector2 operator/(Type Scalar) const
        {
            ZY_ASSERT(!::IsAlmostZero(Scalar), "Division by zero");

            return AnyVector2(mX / Scalar, mY / Scalar);
        }

        /// \brief Shift left all coordinates of this vector by a scalar.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A new vector with the components shifted.
        ZY_INLINE constexpr AnyVector2 operator<<(Type Scalar) const
            requires(IsIntegral<Type>)
        {
            ZY_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            return AnyVector2(mX << Scalar, mY << Scalar);
        }

        /// \brief Shift right all coordinates of this vector by a scalar.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A new vector with the components shifted.
        ZY_INLINE constexpr AnyVector2 operator>>(Type Scalar) const
            requires(IsIntegral<Type>)
        {
            ZY_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            return AnyVector2(mX >> Scalar, mY >> Scalar);
        }

        /// \brief Pre-increments all components of the vector.
        ///
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator++()
        {
            ++mX;
            ++mY;
            return (* this);
        }

        /// \brief Pre-decrements all components of the vector.
        ///
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator--()
        {
            --mX;
            --mY;
            return (* this);
        }

        /// \brief Adds another vector to the current vector.
        ///
        /// \param Other The vector to add.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator+=(AnyVector2 Other)
        {
            mX += Other.mX;
            mY += Other.mY;
            return (* this);
        }

        /// \brief Adds a scalar value to all components of the vector.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator+=(Type Scalar)
        {
            mX += Scalar;
            mY += Scalar;
            return (* this);
        }

        /// \brief Subtracts another vector from the current vector.
        ///
        /// \param Other The vector to subtract.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator-=(AnyVector2 Other)
        {
            mX -= Other.mX;
            mY -= Other.mY;
            return (* this);
        }

        /// \brief Subtracts a scalar value from all components of the vector.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator-=(Type Scalar)
        {
            mX -= Scalar;
            mY -= Scalar;
            return (* this);
        }

        /// \brief Multiplies all components of the vector by another vector.
        ///
        /// \param Other The vector to multiply.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator*=(AnyVector2 Other)
        {
            mX *= Other.mX;
            mY *= Other.mY;
            return (* this);
        }

        /// \brief Multiplies all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator*=(Type Scalar)
        {
            mX *= Scalar;
            mY *= Scalar;
            return (* this);
        }

        /// \brief Divides all components of the vector by another vector.
        ///
        /// \param Other The vector to divide by.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator/=(AnyVector2 Other)
        {
            ZY_ASSERT(!::IsAlmostZero(Other.GetX()), "Division by zero (X)");
            ZY_ASSERT(!::IsAlmostZero(Other.GetY()), "Division by zero (Y)");

            mX /= Other.mX;
            mY /= Other.mY;
            return (* this);
        }

        /// \brief Divides all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator/=(Type Scalar)
        {
            ZY_ASSERT(!::IsAlmostZero(Scalar), "Division by zero");

            mX /= Scalar;
            mY /= Scalar;
            return (* this);
        }

        /// \brief Shifts left all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator<<=(Type Scalar)
            requires(IsIntegral<Type>)
        {
            ZY_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            mX <<= Scalar;
            mY <<= Scalar;

            return (* this);
        }

        /// \brief Shifts right all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector2> operator>>=(Type Scalar)
            requires(IsIntegral<Type>)
        {
            ZY_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            mX >>= Scalar;
            mY >>= Scalar;

            return (* this);
        }

        /// \brief Checks if this vector is equal to another vector.
        ///
        /// \param Other The vector to compare to.
        /// \return `true` if all components are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(AnyVector2 Other) const
        {
            return IsAlmostEqual(Other.mX, Other.mY);
        }

        /// \brief Checks if this vector is not equal to another vector.
        ///
        /// \param Other The vector to compare to.
        /// \return `true` if the vectors are not equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(AnyVector2 Other) const
        {
            return !(* this == Other);
        }

        /// \brief Compares this vector with another for less-than relationship (lexicographic ordering).
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if this vector is lexicographically less than the other vector, otherwise `false`.
        ZY_INLINE constexpr Bool operator<(AnyVector2 Other) const
        {
            return (mX < Other.mX) || (mX == Other.mX && mY < Other.mY);
        }

        /// \brief Compares this vector with another for less-than or equal relationship (lexicographic ordering).
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if this vector is lexicographically less than or equal to the other vector, otherwise `false`.
        ZY_INLINE constexpr Bool operator<=(AnyVector2 Other) const
        {
            return (mX < Other.mX) || (mX == Other.mX && mY <= Other.mY);
        }

        /// \brief Compares this vector with another for greater-than relationship (lexicographic ordering).
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if this vector is lexicographically greater than the other vector, otherwise `false`.
        ZY_INLINE constexpr Bool operator>(AnyVector2 Other) const
        {
            return (mX > Other.mX) || (mX == Other.mX && mY > Other.mY);
        }

        /// \brief Compares this vector with another for greater-than or equal relationship (lexicographic ordering).
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if this vector is lexicographically greater than or equal to the other vector, otherwise `false`.
        ZY_INLINE constexpr Bool operator>=(AnyVector2 Other) const
        {
            return (mX > Other.mX) || (mX == Other.mX && mY >= Other.mY);
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<5> kPattern("({0}, {1})");
            Format::Processor<Output>::Format(Buffer, kPattern, mX, mY);
        }

    public:

        /// \brief Gets the zero vector (0, 0).
        ///
        /// \return The zero vector.
        ZY_INLINE static constexpr AnyVector2 Zero()
        {
            return AnyVector2();
        }

        /// \brief Gets the one vector (1, 1).
        ///
        /// \return The one vector.
        ZY_INLINE static constexpr AnyVector2 One()
        {
            return AnyVector2(Type(1), Type(1));
        }

        /// \brief Gets the unit vector along the X-axis (1, 0).
        ///
        /// \return The unit vector along the X-axis.
        ZY_INLINE static constexpr AnyVector2 UnitX()
        {
            return AnyVector2(Type(1), Type(0));
        }

        /// \brief Gets the unit vector along the Y-axis (0, 1).
        ///
        /// \return The unit vector along the Y-axis.
        ZY_INLINE static constexpr AnyVector2 UnitY()
        {
            return AnyVector2(Type(0), Type(1));
        }


        /// \brief Gets a vector perpendicular to the given vector.
        ///
        /// \param Vector The vector to compute the perpendicular of.
        /// \return A vector perpendicular to the input vector.
        ZY_INLINE static constexpr AnyVector2 Perpendicular(AnyVector2 Vector)
        {
            return AnyVector2(-Vector.GetY(), Vector.GetX());
        }

        /// \brief Projects the source vector onto the target vector.
        ///
        /// \param Source The vector to be projected.
        /// \param Target The vector onto which the source is projected.
        /// \return The projection of source onto target.
        ZY_INLINE static constexpr AnyVector2 Project(AnyVector2 Source, AnyVector2 Target)
            requires (IsReal<Type>)
        {
            const Type Denominator = Dot(Target, Target);
            ZY_ASSERT(!::IsAlmostZero(Denominator), "Cannot project onto zero-length vector");

            return Target * (Dot(Source, Target) / Denominator);
        }

        /// \brief Normalizes the given vector.
        ///
        /// \param Vector The vector to normalize.
        /// \return A normalized vector, or the original if its length is too small.
        ZY_INLINE static AnyVector2 Normalize(AnyVector2 Vector)
            requires (IsReal<Type>)
        {
            const Type Length = Vector.GetLength();
            ZY_ASSERT(!::IsAlmostZero(Length), "Cannot normalize a zero-length vector");

            return Vector * (Type(1) / Length);
        }

        /// \brief Reflects the incident vector over the given normal.
        ///
        /// \param Incident The incoming vector to reflect.
        /// \param Normal   The surface normal to reflect across (should be normalized).
        /// \return The reflected vector.
        ZY_INLINE static constexpr AnyVector2 Reflect(AnyVector2 Incident, AnyVector2 Normal)
            requires (IsReal<Type>)
        {
            ZY_ASSERT(Normal.IsNormalized(), "Normal must be normalized");

            return Incident - Normal * (Type(2) * Dot(Incident, Normal));
        }

        /// \brief Rotates a vector by a given angle in radians.
        ///
        /// \param Vector   The vector to rotate.
        /// \param Rotation The angles to rotate the vector.
        /// \return A new rotated vector.
        ZY_INLINE static AnyVector2 Rotate(AnyVector2 Vector, Angle Rotation)
            requires (IsReal<Type>)
        {
            const Type C = Angle::Cosine(Rotation);
            const Type S = Angle::Sine(Rotation);
            return AnyVector2(Vector.GetX() * C - Vector.GetY() * S, Vector.GetX() * S + Vector.GetY() * C);
        }

        /// \brief Computes the dot product of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The dot product of the two vectors.
        ZY_INLINE static constexpr Type Dot(AnyVector2 P0, AnyVector2 P1)
        {
            return (P0.GetX() * P1.GetX()) + (P0.GetY() * P1.GetY());
        }

        /// \brief Computes the cross product of two vectors (scalar in 2D).
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The scalar cross product of the two vectors.
        ZY_INLINE static constexpr Type Cross(AnyVector2 P0, AnyVector2 P1)
        {
            return P0.GetX() * P1.GetY() - P0.GetY() * P1.GetX();
        }

        /// \brief Computes the 2D equivalent of a scalar triple product for three points.
        ///
        /// This calculates the signed area of the parallelogram formed by (P1-P0) and (P2-P0).
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \param P2 The third vector.
        /// \return The scalar triple product value.
        ZY_INLINE static constexpr Type Cross(AnyVector2 P0, AnyVector2 P1, AnyVector2 P2)
        {
            return (P0.GetX() - P2.GetX()) * (P1.GetY() - P2.GetY()) - (P0.GetY() - P2.GetY()) * (P1.GetX() - P2.GetX());
        }

        /// \brief Gets the component-wise minimum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the component-wise minimum values.
        ZY_INLINE static constexpr AnyVector2 Min(AnyVector2 P0, AnyVector2 P1)
        {
            return AnyVector2(::Min(P0.mX, P1.mX), ::Min(P0.mY, P1.mY));
        }

        /// \brief Gets the component-wise maximum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the component-wise maximum values.
        ZY_INLINE static constexpr AnyVector2 Max(AnyVector2 P0, AnyVector2 P1)
        {
            return AnyVector2(::Max(P0.mX, P1.mX), ::Max(P0.mY, P1.mY));
        }

        /// \brief Clamps each component of a vector between the corresponding components of min and max vectors.
        ///
        /// \param Vector The source vector to clamp.
        /// \param Min    The vector specifying the minimum bounds.
        /// \param Max    The vector specifying the maximum bounds.
        /// \return A vector with each component clamped between the min and max values.
        ZY_INLINE static constexpr AnyVector2 Clamp(AnyVector2 Vector, AnyVector2 Min, AnyVector2 Max)
        {
            return AnyVector2(::Clamp(Vector.mX, Min.mX, Max.mX), ::Clamp(Vector.mY, Min.mY, Max.mY));
        }

        /// \brief Gets a vector with each component floored to the nearest integer.
        ///
        /// \param Vector The source vector with real-valued components.
        /// \return A vector with all components rounded down.
        ZY_INLINE static constexpr AnyVector2 Floor(AnyVector2 Vector)
            requires(IsReal<Type>)
        {
            return AnyVector2(::Floor(Vector.mX), ::Floor(Vector.mY));
        }

        /// \brief Gets a vector with each component ceiled to the nearest integer.
        ///
        /// \param Vector The source vector with real-valued components.
        /// \return A vector with all components rounded up.
        ZY_INLINE static constexpr AnyVector2 Ceil(AnyVector2 Vector)
            requires(IsReal<Type>)
        {
            return AnyVector2(::Ceil(Vector.mX), ::Ceil(Vector.mY));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type mX;
        Type mY;
    };

    /// \brief Represents a vector with (x, y) floating-point components.
    using Vector2     = AnyVector2<Real32>;

    /// \brief Represents a vector with (x, y) signed integer components.
    using IntVector2  = AnyVector2<SInt32>;

    /// \brief Represents a vector with (x, y) unsigned integer components.
    using UIntVector2 = AnyVector2<UInt32>;
}