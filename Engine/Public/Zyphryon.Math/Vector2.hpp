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

#include "Zyphryon.Base/Log.hpp"
#include "Zyphryon.Base/Scalar.hpp"

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
        ZYPHRYON_INLINE constexpr AnyVector2()
            : mX { 0 },
              mY { 0 }
        {
        }

        /// \brief Constructor initializing all components to the same value.
        ///
        /// \param Scalar The scalar value used to initialize all components.
        ZYPHRYON_INLINE constexpr explicit AnyVector2(Type Scalar)
            : mX { Scalar },
              mY { Scalar }
        {
        }

        /// \brief Constructor initializing the vector with specified x and y values.
        ///
        /// \param X The x-component of the vector.
        /// \param Y The y-component of the vector.
        ZYPHRYON_INLINE constexpr AnyVector2(Type X, Type Y)
            : mX { X },
              mY { Y }
        {
        }

        /// \brief Conversion constructor from another vector with a different base type.
        ///
        /// \param Other The source vector to convert from.
        template<typename Base>
        ZYPHRYON_INLINE constexpr explicit AnyVector2(ConstRef<AnyVector2<Base>> Other)
            : mX { static_cast<Type>(Other.GetX()) },
              mY { static_cast<Type>(Other.GetY()) }
        {
        }

        /// \brief Checks if the vector is the zero vector.
        ///
        /// \return `true` if all components are approximately zero, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsAlmostZero() const
        {
            return Base::IsAlmostZero(mX) && Base::IsAlmostZero(mY);
        }

        /// \brief Checks if all components are approximately equal to the given values.
        ///
        /// \param X Value to compare with the X component.
        /// \param Y Value to compare with the Y component.
        /// \return `true` if all components are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsAlmostEqual(Type X, Type Y) const
        {
            return Base::IsAlmostEqual(mX, X) && Base::IsAlmostEqual(mY, Y);
        }

        /// \brief Checks if the vector is normalized.
        ///
        /// \return `true` if the vector is normalized, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsNormalized() const
            requires (IsReal<Type>)
        {
            return Base::IsAlmostEqual(GetLengthSquared(), Type(1));
        }

        /// \brief Checks if all components are approximately equal within a tolerance.
        ///
        /// \param Tolerance Maximum allowed difference between components.
        /// \return `true` if all components are within \p Tolerance of each other, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsUniform(Type Tolerance = kEpsilon<Type>) const
        {
            return (Abs(GetX() - GetY()) <= Tolerance);
        }

        /// \brief Checks if this vector is component-wise less than another vector.
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if all components are less, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool IsComponentWiseLess(ConstRef<AnyVector2> Vector) const
        {
            return mX < Vector.mX && mY < Vector.mY;
        }

        /// \brief Checks if this vector is component-wise less than or equal to another vector.
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if all components are less or equal, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool IsComponentWiseLessOrEqual(ConstRef<AnyVector2> Vector) const
        {
            return mX <= Vector.mX && mY <= Vector.mY;
        }

        /// \brief Checks if this vector is component-wise greater than another vector.
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if all components are greater, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool IsComponentWiseGreater(ConstRef<AnyVector2> Vector) const
        {
            return mX > Vector.mX && mY > Vector.mY;
        }

        /// \brief Checks if this vector is component-wise greater than or equal to another vector.
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if all components are greater or equal, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool IsComponentWiseGreaterOrEqual(ConstRef<AnyVector2> Vector) const
        {
            return mX >= Vector.mX && mY >= Vector.mY;
        }

        /// \brief Sets the components of the vector to new values.
        ///
        /// \param X New x value.
        /// \param Y New y value.
        /// \return A reference to this vector after the update.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> Set(Type X, Type Y)
        {
            mX = X;
            mY = Y;
            return (* this);
        }

        /// \brief Sets the x-component of the vector.
        ///
        /// \param X The new x-component.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> SetX(Type X)
        {
            mX = X;
            return (* this);
        }

        /// \brief Gets the x-component of the vector.
        ///
        /// \return The x-component.
        ZYPHRYON_INLINE constexpr Type GetX() const
        {
            return mX;
        }

        /// \brief Sets the y-component of the vector.
        ///
        /// \param Y The new y-component.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> SetY(Type Y)
        {
            mY = Y;
            return (* this);
        }

        /// \brief Gets the y-component of the vector.
        ///
        /// \return The y-component.
        ZYPHRYON_INLINE constexpr Type GetY() const
        {
            return mY;
        }

        /// \brief Calculates the length (magnitude) of the vector.
        ///
        /// \return The length of the vector.
        ZYPHRYON_INLINE constexpr Type GetLength() const
            requires (IsReal<Type>)
        {
            return Sqrt(GetLengthSquared());
        }

        /// \brief Calculates the squared length of the vector.
        ///
        /// \return The squared length of the vector (no square root).
        ZYPHRYON_INLINE constexpr Type GetLengthSquared() const
        {
            return (mX * mX + mY * mY);
        }

        /// \brief Calculates the euclidean distance between this vector and another vector.
        ///
        /// \param Vector The vector to calculate the distance to.
        /// \return The distance between the two vectors.
        ZYPHRYON_INLINE constexpr Type GetDistance(ConstRef<AnyVector2> Vector) const
            requires (IsReal<Type>)
        {
            const AnyVector2 Difference = (* this) - Vector;
            return Difference.GetLength();
        }

        /// \brief Calculates the manhattan distance between this vector and another vector.
        ///
        /// \param Vector The vector to calculate the distance to.
        /// \return The distance between the two vectors.
        ZYPHRYON_INLINE constexpr Type GetDistanceManhattan(ConstRef<AnyVector2> Vector) const
        {
            const AnyVector2 Difference = (* this) - Vector;
            return Abs(Difference.GetX()) + Abs(Difference.GetY());
        }

        /// \brief Calculates the Chebyshev distance between this vector and another vector.
        ///
        /// \param Vector The vector to calculate the distance to.
        /// \return The distance between the two vectors.
        ZYPHRYON_INLINE constexpr Type GetDistanceChebyshev(ConstRef<AnyVector2> Vector) const
        {
            const AnyVector2 Difference = (* this) - Vector;
            return Max(Abs(Difference.GetX()), Abs(Difference.GetY()));
        }

        /// \brief Calculates the squared distance between this vector and another vector.
        ///
        /// \param Vector The vector to calculate the squared distance to.
        /// \return The squared distance between the two vectors.
        ZYPHRYON_INLINE constexpr Type GetDistanceSquared(ConstRef<AnyVector2> Vector) const
        {
            return ((* this) - Vector).GetLengthSquared();
        }

        /// \brief Calculates the angle of the vector relative to the x-axis.
        ///
        /// \return The angle in radians (range [-π, π]).
        ZYPHRYON_INLINE constexpr Type GetAngle() const
            requires (IsReal<Type>)
        {
            return InvTan(mY, mX);
        }

        /// \brief Returns the angle in radians between this vector and another.
        ///
        /// \param Other The other vector to compare with.
        /// \return Angle in radians between the two vectors (range [0, π]).
        ZYPHRYON_INLINE constexpr Type GetAngle(ConstRef<AnyVector2> Other) const
            requires (IsReal<Type>)
        {
            const Type Length = GetLength() * Other.GetLength();
            LOG_ASSERT(!Base::IsAlmostZero(Length), "Cannot compute angle with zero-length vector");

            return InvCos(Dot(* this, Other) / Length);
        }

        /// \brief Adds another vector to this vector.
        ///
        /// \param Vector The vector to add.
        /// \return A new vector that is the sum of this vector and the input vector.
        ZYPHRYON_INLINE constexpr AnyVector2 operator+(ConstRef<AnyVector2> Vector) const
        {
            return AnyVector2(mX + Vector.mX, mY + Vector.mY);
        }

        /// \brief Adds a scalar to all components of this vector.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new vector with the scalar added to all components.
        ZYPHRYON_INLINE constexpr AnyVector2 operator+(Type Scalar) const
        {
            return AnyVector2(mX + Scalar, mY + Scalar);
        }

        /// \brief Negates the vector.
        ///
        /// \return A new vector that is the negation of this vector.
        ZYPHRYON_INLINE constexpr AnyVector2 operator-() const
        {
            return AnyVector2(-mX, -mY);
        }

        /// \brief Subtracts another vector from this vector.
        ///
        /// \param Vector The vector to subtract.
        /// \return A new vector that is the difference of the two vectors.
        ZYPHRYON_INLINE constexpr AnyVector2 operator-(ConstRef<AnyVector2> Vector) const
        {
            return AnyVector2(mX - Vector.mX, mY - Vector.mY);
        }

        /// \brief Subtracts a scalar from all components of this vector.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new vector with the scalar subtracted from all components.
        ZYPHRYON_INLINE constexpr AnyVector2 operator-(Type Scalar) const
        {
            return AnyVector2(mX - Scalar, mY - Scalar);
        }

        /// \brief Multiplies all components of this vector by another vector.
        ///
        /// \param Vector The vector to multiply by.
        /// \return A new vector that is the product of the two vectors.
        ZYPHRYON_INLINE constexpr AnyVector2 operator*(ConstRef<AnyVector2> Vector) const
        {
            return AnyVector2(mX * Vector.mX, mY * Vector.mY);
        }

        /// \brief Multiplies all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new vector with the scalar multiplied by all components.
        ZYPHRYON_INLINE constexpr AnyVector2 operator*(Type Scalar) const
        {
            return AnyVector2(mX * Scalar, mY * Scalar);
        }

        /// \brief Divides all components of this vector by another vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A new vector that is the quotient of the two vectors.
        ZYPHRYON_INLINE constexpr AnyVector2 operator/(ConstRef<AnyVector2> Vector) const
        {
            LOG_ASSERT(!Base::IsAlmostZero(Vector.GetX()), "Division by zero (X)");
            LOG_ASSERT(!Base::IsAlmostZero(Vector.GetY()), "Division by zero (Y)");

            return AnyVector2(mX / Vector.mX, mY / Vector.mY);
        }

        /// \brief Divides all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new vector with the scalar divided into all components.
        ZYPHRYON_INLINE constexpr AnyVector2 operator/(Type Scalar) const
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");

            return AnyVector2(mX / Scalar, mY / Scalar);
        }

        /// \brief Shift left all coordinates of this vector by a scalar.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A new vector with the components shifted.
        ZYPHRYON_INLINE constexpr AnyVector2 operator<<(Type Scalar) const
            requires(IsInteger<Type>)
        {
            LOG_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            return AnyVector2(mX << Scalar, mY << Scalar);
        }

        /// \brief Shift right all coordinates of this vector by a scalar.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A new vector with the components shifted.
        ZYPHRYON_INLINE constexpr AnyVector2 operator>>(Type Scalar) const
            requires(IsInteger<Type>)
        {
            LOG_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            return AnyVector2(mX >> Scalar, mY >> Scalar);
        }

        /// \brief Pre-increments all components of the vector.
        ///
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator++()
        {
            ++mX;
            ++mY;
            return (* this);
        }

        /// \brief Pre-decrements all components of the vector.
        ///
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator--()
        {
            --mX;
            --mY;
            return (* this);
        }

        /// \brief Adds another vector to the current vector.
        ///
        /// \param Vector The vector to add.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator+=(ConstRef<AnyVector2> Vector)
        {
            mX += Vector.mX;
            mY += Vector.mY;
            return (* this);
        }

        /// \brief Adds a scalar value to all components of the vector.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator+=(Type Scalar)
        {
            mX += Scalar;
            mY += Scalar;
            return (* this);
        }

        /// \brief Subtracts another vector from the current vector.
        ///
        /// \param Vector The vector to subtract.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator-=(ConstRef<AnyVector2> Vector)
        {
            mX -= Vector.mX;
            mY -= Vector.mY;
            return (* this);
        }

        /// \brief Subtracts a scalar value from all components of the vector.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator-=(Type Scalar)
        {
            mX -= Scalar;
            mY -= Scalar;
            return (* this);
        }

        /// \brief Multiplies all components of the vector by another vector.
        ///
        /// \param Vector The vector to multiply.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator*=(ConstRef<AnyVector2> Vector)
        {
            mX *= Vector.mX;
            mY *= Vector.mY;
            return (* this);
        }

        /// \brief Multiplies all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator*=(Type Scalar)
        {
            mX *= Scalar;
            mY *= Scalar;
            return (* this);
        }

        /// \brief Divides all components of the vector by another vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator/=(ConstRef<AnyVector2> Vector)
        {
            LOG_ASSERT(!Base::IsAlmostZero(Vector.GetX()), "Division by zero (X)");
            LOG_ASSERT(!Base::IsAlmostZero(Vector.GetY()), "Division by zero (Y)");

            mX /= Vector.mX;
            mY /= Vector.mY;
            return (* this);
        }

        /// \brief Divides all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator/=(Type Scalar)
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");

            mX /= Scalar;
            mY /= Scalar;
            return (* this);
        }

        /// \brief Shifts left all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator<<=(Type Scalar)
            requires(IsInteger<Type>)
        {
            LOG_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            mX <<= Scalar;
            mY <<= Scalar;

            return (* this);
        }

        /// \brief Shifts right all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector2> operator>>=(Type Scalar)
            requires(IsInteger<Type>)
        {
            LOG_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            mX >>= Scalar;
            mY >>= Scalar;

            return (* this);
        }

        /// \brief Checks if this vector is equal to another vector.
        ///
        /// \param Vector The vector to compare to.
        /// \return `true` if all components are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(ConstRef<AnyVector2> Vector) const
        {
            return IsAlmostEqual(Vector.mX, Vector.mY);
        }

        /// \brief Checks if this vector is not equal to another vector.
        ///
        /// \param Other The vector to compare to.
        /// \return `true` if the vectors are not equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(ConstRef<AnyVector2> Other) const
        {
            return !(* this == Other);
        }

        /// \brief Compares this vector with another for less-than relationship (lexicographic ordering).
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if this vector is lexicographically less than the other vector, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator<(ConstRef<AnyVector2> Vector) const
        {
            return (mX < Vector.mX) || (mX == Vector.mX && mY < Vector.mY);
        }

        /// \brief Compares this vector with another for less-than or equal relationship (lexicographic ordering).
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if this vector is lexicographically less than or equal to the other vector, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator<=(ConstRef<AnyVector2> Vector) const
        {
            return (mX < Vector.mX) || (mX == Vector.mX && mY <= Vector.mY);
        }

        /// \brief Compares this vector with another for greater-than relationship (lexicographic ordering).
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if this vector is lexicographically greater than the other vector, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator>(ConstRef<AnyVector2> Vector) const
        {
            return (mX > Vector.mX) || (mX == Vector.mX && mY > Vector.mY);
        }

        /// \brief Compares this vector with another for greater-than or equal relationship (lexicographic ordering).
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if this vector is lexicographically greater than or equal to the other vector, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator>=(ConstRef<AnyVector2> Vector) const
        {
            return (mX > Vector.mX) || (mX == Vector.mX && mY >= Vector.mY);
        }

        /// \brief Computes a hash value for the object.
        ///
        /// \return A hash value uniquely representing the current state of the object.
        ZYPHRYON_INLINE constexpr UInt64 Hash() const
        {
            return HashCombine(this);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mX);
            Archive.SerializeObject(mY);
        }

    public:

        /// \brief Returns the zero vector (0, 0).
        ///
        /// \return The zero vector.
        ZYPHRYON_INLINE constexpr static AnyVector2 Zero()
        {
            return AnyVector2();
        }

        /// \brief Returns the one vector (1, 1).
        ///
        /// \return The one vector.
        ZYPHRYON_INLINE constexpr static AnyVector2 One()
        {
            return AnyVector2(Type(1), Type(1));
        }

        /// \brief Returns the unit vector along the X-axis (1, 0).
        ///
        /// \return The unit vector along the X-axis.
        ZYPHRYON_INLINE constexpr static AnyVector2 UnitX()
        {
            return AnyVector2(Type(1), Type(0));
        }

        /// \brief Returns the unit vector along the Y-axis (0, 1).
        ///
        /// \return The unit vector along the Y-axis.
        ZYPHRYON_INLINE constexpr static AnyVector2 UnitY()
        {
            return AnyVector2(Type(0), Type(1));
        }

        /// \brief Checks if two vectors are parallel.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return `true` if the vectors are parallel, `false` otherwise.
        ZYPHRYON_INLINE constexpr static Bool IsParallel(ConstRef<AnyVector2> P0, ConstRef<AnyVector2> P1)
        {
            return Cross(P0, P1).IsAlmostZero();
        }

        /// \brief Returns a vector perpendicular to the given vector.
        ///
        /// \param Vector The vector to compute the perpendicular of.
        /// \return A vector perpendicular to the input vector.
        ZYPHRYON_INLINE constexpr static AnyVector2 Perpendicular(ConstRef<AnyVector2> Vector)
        {
            return AnyVector2(-Vector.GetY(), Vector.GetX());
        }

        /// \brief Projects the source vector onto the target vector.
        ///
        /// \param Source The vector to be projected.
        /// \param Target The vector onto which the source is projected.
        /// \return The projection of source onto target.
        ZYPHRYON_INLINE constexpr static AnyVector2 Project(ConstRef<AnyVector2> Source, ConstRef<AnyVector2> Target)
            requires (IsReal<Type>)
        {
            const Type Denominator = Dot(Target, Target);
            LOG_ASSERT(!Base::IsAlmostZero(Denominator), "Cannot project onto zero-length vector");

            return Target * (Dot(Source, Target) / Denominator);
        }

        /// \brief Normalizes the given vector.
        ///
        /// \param Vector The vector to normalize.
        /// \return A normalized vector, or the original if its length is too small.
        ZYPHRYON_INLINE constexpr static AnyVector2 Normalize(ConstRef<AnyVector2> Vector)
            requires (IsReal<Type>)
        {
            const Type Length = Vector.GetLength();
            LOG_ASSERT(!Base::IsAlmostZero(Length), "Cannot normalize a zero-length vector");

            return Vector * (Type(1) / Length);
        }

        /// \brief Reflects the incident vector over the given normal.
        ///
        /// \param Incident The incoming vector to reflect.
        /// \param Normal   The surface normal to reflect across (should be normalized).
        /// \return The reflected vector.
        ZYPHRYON_INLINE constexpr static AnyVector2 Reflect(ConstRef<AnyVector2> Incident, ConstRef<AnyVector2> Normal)
            requires (IsReal<Type>)
        {
            LOG_ASSERT(Normal.IsNormalized(), "Normal must be normalized");

            return Incident - Normal * (Type(2) * Dot(Incident, Normal));
        }

        /// \brief Rotates a vector by a given angle in radians.
        ///
        /// \param Vector The vector to rotate.
        /// \param Angles The angle in radians to rotate by (counter-clockwise).
        /// \return A new rotated vector.
        ZYPHRYON_INLINE constexpr static AnyVector2 Rotate(ConstRef<AnyVector2> Vector, Type Angles)
            requires (IsReal<Type>)
        {
            const Type C = Cos(Angles);
            const Type S = Sin(Angles);
            return AnyVector2(Vector.GetX() * C - Vector.GetY() * S, Vector.GetX() * S + Vector.GetY() * C);
        }

        /// \brief Computes the dot product of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The dot product of the two vectors.
        ZYPHRYON_INLINE constexpr static Type Dot(ConstRef<AnyVector2> P0, ConstRef<AnyVector2> P1)
        {
            return (P0.GetX() * P1.GetX()) + (P0.GetY() * P1.GetY());
        }

        /// \brief Computes the cross product of two vectors (scalar in 2D).
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The scalar cross product of the two vectors.
        ZYPHRYON_INLINE constexpr static Type Cross(ConstRef<AnyVector2> P0, ConstRef<AnyVector2> P1)
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
        ZYPHRYON_INLINE constexpr static Type Cross(ConstRef<AnyVector2> P0, ConstRef<AnyVector2> P1, ConstRef<AnyVector2> P2)
        {
            return (P0.GetX() - P2.GetX()) * (P1.GetY() - P2.GetY()) - (P0.GetY() - P2.GetY()) * (P1.GetX() - P2.GetX());
        }

        /// \brief Returns the component-wise minimum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the component-wise minimum values.
        ZYPHRYON_INLINE constexpr static AnyVector2 Min(ConstRef<AnyVector2> P0, ConstRef<AnyVector2> P1)
        {
            return AnyVector2(Base::Min(P0.mX, P1.mX), Base::Min(P0.mY, P1.mY));
        }

        /// \brief Returns the component-wise maximum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the component-wise maximum values.
        ZYPHRYON_INLINE constexpr static AnyVector2 Max(ConstRef<AnyVector2> P0, ConstRef<AnyVector2> P1)
        {
            return AnyVector2(Base::Max(P0.mX, P1.mX), Base::Max(P0.mY, P1.mY));
        }

        /// \brief Returns a vector with each component floored to the nearest integer.
        ///
        /// \param Vector The source vector with real-valued components.
        /// \return A vector with all components rounded down.
        ZYPHRYON_INLINE constexpr static AnyVector2 Floor(ConstRef<AnyVector2> Vector)
            requires(IsReal<Type>)
        {
            return AnyVector2(Base::Floor(Vector.mX), Base::Floor(Vector.mY));
        }

        /// \brief Returns a vector with each component ceiled to the nearest integer.
        ///
        /// \param Vector The source vector with real-valued components.
        /// \return A vector with all components rounded up.
        ZYPHRYON_INLINE constexpr static AnyVector2 Ceil(ConstRef<AnyVector2> Vector)
            requires(IsReal<Type>)
        {
            return AnyVector2(Base::Ceil(Vector.mX), Base::Ceil(Vector.mY));
        }

        /// \brief Linearly interpolates between two vectors.
        ///
        /// \param Start      The starting vector.
        /// \param End        The ending vector.
        /// \param Percentage The interpolation percentage (range between 0 and 1).
        /// \return A vector interpolated between the start and end vectors.
        ZYPHRYON_INLINE constexpr static AnyVector2 Lerp(ConstRef<AnyVector2> Start, ConstRef<AnyVector2> End, Type Percentage)
            requires (IsReal<Type>)
        {
            LOG_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

            return Start + (End - Start) * Percentage;
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