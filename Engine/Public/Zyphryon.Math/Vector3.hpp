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

#include "Zyphryon.Base/Scalar.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a generic vector with (x, y, z) components.
    template<typename Type>
    class AnyVector3 final
    {
    public:

        /// \brief Constructor initializing all components zero.
        ZYPHRYON_INLINE constexpr AnyVector3()
            : mX { 0 },
              mY { 0 },
              mZ { 0 }
        {
        }

        /// \brief Constructor initializing all components to the same value.
        ///
        /// \param Scalar The scalar value used to initialize all components.
        ZYPHRYON_INLINE constexpr explicit AnyVector3(Type Scalar)
            : mX { Scalar },
              mY { Scalar },
              mZ { Scalar }
        {
        }

        /// \brief Constructor initializing the vector with specified x, y and z values.
        ///
        /// \param X The x-component of the vector.
        /// \param Y The y-component of the vector.
        /// \param Z The z-component of the vector.
        ZYPHRYON_INLINE constexpr AnyVector3(Type X, Type Y, Type Z)
            : mX { X },
              mY { Y },
              mZ { Z }
        {
        }

        /// \brief Copy constructor.
        ///
        /// \param Other The vector to copy.
        ZYPHRYON_INLINE constexpr AnyVector3(ConstRef<AnyVector3> Other)
            : mX { Other.GetX() },
              mY { Other.GetY() },
              mZ { Other.GetZ() }
        {
        }

        /// \brief Checks if the vector is the zero vector.
        ///
        /// \return `true` if all components are approximately zero, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsAlmostZero() const
        {
            return Base::IsAlmostZero(mX) && Base::IsAlmostZero(mY) && Base::IsAlmostZero(mZ);
        }

        /// \brief Checks if all components are approximately equal to the given values.
        ///
        /// \param X Value to compare with the X component.
        /// \param Y Value to compare with the Y component.
        /// \param Z Value to compare with the Z component.
        /// \return `true` if all components are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsAlmostEqual(Type X, Type Y, Type Z) const
        {
            return Base::IsAlmostEqual(mX, X) && Base::IsAlmostEqual(mY, Y) && Base::IsAlmostEqual(mZ, Z);
        }

        /// \brief Checks if the vector is normalized.
        ///
        /// \return `true` if the vector is normalized, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsNormalized() const
            requires(IsReal<Type>)
        {
            return Base::IsAlmostEqual(GetLengthSquared(), Type(1));
        }

        /// \brief Checks if all components are approximately equal within a tolerance.
        ///
        /// \param Tolerance Maximum allowed difference between components.
        /// \return `true` if all components are within \p Tolerance of each other, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsUniform(Type Tolerance = kEpsilon<Type>) const
        {
            return (Abs(GetX() - GetY()) <= Tolerance) &&
                   (Abs(GetY() - GetZ()) <= Tolerance);
        }

        /// \brief Checks if this vector is component-wise less than another vector.
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if all components are less, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool IsComponentWiseLess(ConstRef<AnyVector3> Vector) const
        {
            return mX < Vector.mX && mY < Vector.mY && mZ < Vector.mZ;
        }

        /// \brief Checks if this vector is component-wise less than or equal to another vector.
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if all components are less or equal, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool IsComponentWiseLessOrEqual(ConstRef<AnyVector3> Vector) const
        {
            return mX <= Vector.mX && mY <= Vector.mY && mZ <= Vector.mZ;
        }

        /// \brief Checks if this vector is component-wise greater than another vector.
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if all components are greater, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool IsComponentWiseGreater(ConstRef<AnyVector3> Vector) const
        {
            return mX > Vector.mX && mY > Vector.mY && mZ > Vector.mZ;
        }

        /// \brief Checks if this vector is component-wise greater than or equal to another vector.
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if all components are greater or equal, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool IsComponentWiseGreaterOrEqual(ConstRef<AnyVector3> Vector) const
        {
            return mX >= Vector.mX && mY >= Vector.mY && mZ >= Vector.mZ;
        }

        /// \brief Sets the components of the vector to new values.
        ///
        /// \param X New x value.
        /// \param Y New y value.
        /// \param Z New z value.
        /// \return A reference to this vector after the update.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> Set(Type X, Type Y, Type Z)
        {
            mX = X;
            mY = Y;
            mZ = Z;
            return (* this);
        }

        /// \brief Sets the x-component of the vector.
        ///
        /// \param X The new x-component.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> SetX(Type X)
        {
            mX = X;
            return (*this);
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
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> SetY(Type Y)
        {
            mY = Y;
            return (*this);
        }

        /// \brief Gets the y-component of the vector.
        ///
        /// \return The y-component.
        ZYPHRYON_INLINE constexpr Type GetY() const
        {
            return mY;
        }

        /// \brief Sets the z-component of the vector.
        ///
        /// \param Z The new z-component.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> SetZ(Type Z)
        {
            mZ = Z;
            return (*this);
        }

        /// \brief Gets the z-component of the vector.
        ///
        /// \return The z-component.
        ZYPHRYON_INLINE constexpr Type GetZ() const
        {
            return mZ;
        }

        /// \brief Calculates the length (magnitude) of the vector.
        ///
        /// \return The length of the vector.
        ZYPHRYON_INLINE constexpr Type GetLength() const
            requires(IsReal<Type>)
        {
            return Sqrt(GetLengthSquared());
        }

        /// \brief Calculates the squared length of the vector.
        ///
        /// \return The squared length of the vector (no square root).
        ZYPHRYON_INLINE constexpr Type GetLengthSquared() const
        {
            return (mX * mX + mY * mY + mZ * mZ);
        }

        /// \brief Calculates the distance between this vector and another vector.
        ///
        /// \param Vector The vector to calculate the distance to.
        /// \return The distance between the two vectors.
        ZYPHRYON_INLINE constexpr Type GetDistance(ConstRef<AnyVector3> Vector) const
            requires(IsReal<Type>)
        {
            return (* this - Vector).GetLength();
        }

        /// \brief Calculates the squared distance between this vector and another vector.
        ///
        /// \param Vector The vector to calculate the squared distance to.
        /// \return The squared distance between the two vectors.
        ZYPHRYON_INLINE constexpr Type GetDistanceSquared(ConstRef<AnyVector3> Vector) const
        {
            return (* this - Vector).GetLengthSquared();
        }

        /// \brief Returns the angle in radians between this vector and another.
        ///
        /// \param Other The other vector to compare with.
        /// \return Angle in radians between the two vectors (range [0, π]).
        ZYPHRYON_INLINE constexpr Type GetAngle(ConstRef<AnyVector3> Other) const
            requires(IsReal<Type>)
        {
            const Type Length = GetLength() * Other.GetLength();
            LOG_ASSERT(!Base::IsAlmostZero(Length), "Cannot compute angle with zero-length vector");

            return InvCos(Dot(* this, Other) / Length);
        }

        /// \brief Adds another vector to this vector.
        ///
        /// \param Vector The vector to add.
        /// \return A new vector that is the sum of this vector and the input vector.
        ZYPHRYON_INLINE constexpr AnyVector3 operator+(ConstRef<AnyVector3> Vector) const
        {
            return AnyVector3(mX + Vector.mX, mY + Vector.mY, mZ + Vector.mZ);
        }

        /// \brief Adds a scalar to all components of this vector.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new vector with the scalar added to all components.
        ZYPHRYON_INLINE constexpr AnyVector3 operator+(Type Scalar) const
        {
            return AnyVector3(mX + Scalar, mY + Scalar, mZ + Scalar);
        }

        /// \brief Negates the vector.
        ///
        /// \return A new vector that is the negation of this vector.
        ZYPHRYON_INLINE constexpr AnyVector3 operator-() const
        {
            return AnyVector3(-mX, -mY, -mZ);
        }

        /// \brief Subtracts another vector from this vector.
        ///
        /// \param Vector The vector to subtract.
        /// \return A new vector that is the difference of the two vectors.
        ZYPHRYON_INLINE constexpr AnyVector3 operator-(ConstRef<AnyVector3> Vector) const
        {
            return AnyVector3(mX - Vector.mX, mY - Vector.mY, mZ - Vector.mZ);
        }

        /// \brief Subtracts a scalar from all components of this vector.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new vector with the scalar subtracted from all components.
        ZYPHRYON_INLINE constexpr AnyVector3 operator-(Type Scalar) const
        {
            return AnyVector3(mX - Scalar, mY - Scalar, mZ - Scalar);
        }

        /// \brief Multiplies all components of this vector by another vector.
        ///
        /// \param Vector The vector to multiply by.
        /// \return A new vector that is the product of the two vectors.
        ZYPHRYON_INLINE constexpr AnyVector3 operator*(ConstRef<AnyVector3> Vector) const
        {
            return AnyVector3(mX * Vector.mX, mY * Vector.mY, mZ * Vector.mZ);
        }

        /// \brief Multiplies all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new vector with the scalar multiplied by all components.
        ZYPHRYON_INLINE constexpr AnyVector3 operator*(Type Scalar) const
        {
            return AnyVector3(mX * Scalar, mY * Scalar, mZ * Scalar);
        }

        /// \brief Divides all components of this vector by another vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A new vector that is the quotient of the two vectors.
        ZYPHRYON_INLINE constexpr AnyVector3 operator/(ConstRef<AnyVector3> Vector) const
        {
            LOG_ASSERT(!Base::IsAlmostZero(Vector.mX), "Division by zero (X)");
            LOG_ASSERT(!Base::IsAlmostZero(Vector.mY), "Division by zero (Y)");
            LOG_ASSERT(!Base::IsAlmostZero(Vector.mZ), "Division by zero (Z)");

            return AnyVector3(mX / Vector.mX, mY / Vector.mY, mZ / Vector.mZ);
        }

        /// \brief Divides all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new vector with the scalar divided into all components.
        ZYPHRYON_INLINE constexpr AnyVector3 operator/(Type Scalar) const
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");

            return AnyVector3(mX / Scalar, mY / Scalar, mZ / Scalar);
        }

        /// \brief Pre-increments all components of the vector.
        ///
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator++()
        {
            ++mX;
            ++mY;
            ++mZ;
            return (* this);
        }

        /// \brief Pre-decrements all components of the vector.
        ///
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator--()
        {
            --mX;
            --mY;
            --mZ;
            return (* this);
        }

        /// \brief Adds another vector to the current vector.
        ///
        /// \param Vector The vector to add.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator+=(ConstRef<AnyVector3> Vector)
        {
            mX += Vector.mX;
            mY += Vector.mY;
            mZ += Vector.mZ;
            return (* this);
        }

        /// \brief Adds a scalar value to all components of the vector.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator+=(Type Scalar)
        {
            mX += Scalar;
            mY += Scalar;
            mZ += Scalar;
            return (* this);
        }

        /// \brief Subtracts another vector from the current vector.
        ///
        /// \param Vector The vector to subtract.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator-=(ConstRef<AnyVector3> Vector)
        {
            mX -= Vector.mX;
            mY -= Vector.mY;
            mZ -= Vector.mZ;
            return (* this);
        }

        /// \brief Subtracts a scalar value from all components of the vector.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator-=(Type Scalar)
        {
            mX -= Scalar;
            mY -= Scalar;
            mZ -= Scalar;
            return (* this);
        }

        /// \brief Multiplies all components of the vector by another vector.
        ///
        /// \param Vector The vector to multiply.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator*=(ConstRef<AnyVector3> Vector)
        {
            mX *= Vector.mX;
            mY *= Vector.mY;
            mZ *= Vector.mZ;
            return (* this);
        }

        /// \brief Multiplies all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator*=(Type Scalar)
        {
            mX *= Scalar;
            mY *= Scalar;
            mZ *= Scalar;
            return (* this);
        }

        /// \brief Divides all components of the vector by another vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator/=(ConstRef<AnyVector3> Vector)
        {
            LOG_ASSERT(!Base::IsAlmostZero(Vector.mX), "Division by zero (X)");
            LOG_ASSERT(!Base::IsAlmostZero(Vector.mY), "Division by zero (Y)");
            LOG_ASSERT(!Base::IsAlmostZero(Vector.mZ), "Division by zero (Z)");

            mX /= Vector.mX;
            mY /= Vector.mY;
            mZ /= Vector.mZ;
            return (* this);
        }

        /// \brief Divides all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A reference to the updated vector.
        ZYPHRYON_INLINE constexpr Ref<AnyVector3> operator/=(Type Scalar)
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");

            mX /= Scalar;
            mY /= Scalar;
            mZ /= Scalar;
            return (* this);
        }

        /// \brief Checks if this vector is equal to another vector.
        ///
        /// \param Vector The vector to compare to.
        /// \return `true` if all components are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(ConstRef<AnyVector3> Vector) const
        {
            return IsAlmostEqual(Vector.mX, Vector.mY, Vector.mZ);
        }

        /// \brief Checks if this vector is not equal to another vector.
        ///
        /// \param Other The vector to compare to.
        /// \return `true` if the vectors are not equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(ConstRef<AnyVector3> Other) const = default;

        /// \brief Compares this vector with another for less-than relationship (lexicographic ordering).
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if this vector is lexicographically less than the other vector, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator<(ConstRef<AnyVector3> Vector) const
        {
            return (mX < Vector.mX) ||
                   (mX == Vector.mX && mY < Vector.mY) ||
                   (mX == Vector.mX && mY == Vector.mY && mZ < Vector.mZ);
        }

        /// \brief Compares this vector with another for less-than or equal relationship (lexicographic ordering).
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if this vector is lexicographically less than or equal to the other vector, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator<=(ConstRef<AnyVector3> Vector) const
        {
            return (mX < Vector.mX) ||
                   (mX == Vector.mX && mY < Vector.mY) ||
                   (mX == Vector.mX && mY == Vector.mY && mZ <= Vector.mZ);
        }

        /// \brief Compares this vector with another for greater-than relationship (lexicographic ordering).
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if this vector is lexicographically greater than the other vector, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator>(ConstRef<AnyVector3> Vector) const
        {
            return (mX > Vector.mX) ||
                   (mX == Vector.mX && mY > Vector.mY) ||
                   (mX == Vector.mX && mY == Vector.mY && mZ > Vector.mZ);
        }

        /// \brief Compares this vector with another for greater-than or equal relationship (lexicographic ordering).
        ///
        /// \param Vector The vector to compare against.
        /// \return `true` if this vector is lexicographically greater than or equal to the other vector, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator>=(ConstRef<AnyVector3> Vector) const
        {
            return (mX > Vector.mX) ||
                   (mX == Vector.mX && mY > Vector.mY) ||
                   (mX == Vector.mX && mY == Vector.mY && mZ >= Vector.mZ);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mX);
            Archive.SerializeObject(mY);
            Archive.SerializeObject(mZ);
        }

    public:

        /// \brief Returns the zero vector (Type(0), Type(0), Type(0)).
        ///
        /// \return The zero vector.
        ZYPHRYON_INLINE constexpr static AnyVector3 Zero()
        {
            return AnyVector3();
        }

        /// \brief Returns the one vector (1, 1, 1).
        ///
        /// \return The one vector.
        ZYPHRYON_INLINE constexpr static AnyVector3 One()
        {
            return AnyVector3(Type(1), Type(1), Type(1));
        }

        /// \brief Returns the unit vector along the X-axis (1, 0, 0).
        ///
        /// \return The unit vector along the X-axis.
        ZYPHRYON_INLINE constexpr static AnyVector3 UnitX()
        {
            return AnyVector3(Type(1), Type(0), Type(0));
        }

        /// \brief Returns the unit vector along the Y-axis (0, 1, 0).
        ///
        /// \return The unit vector along the Y-axis.
        ZYPHRYON_INLINE constexpr static AnyVector3 UnitY()
        {
            return AnyVector3(Type(0), Type(1), Type(0));
        }

        /// \brief Returns the unit vector along the Z-axis (0, 0, 1).
        ///
        /// \return The unit vector along the Z-axis.
        ZYPHRYON_INLINE constexpr static AnyVector3 UnitZ()
        {
            return AnyVector3(Type(0), Type(0), Type(1));
        }

        /// \brief Normalizes the given vector.
        ///
        /// \param Vector The vector to normalize.
        /// \return A normalized vector, or the original if its length is too small.
        ZYPHRYON_INLINE constexpr static AnyVector3 Normalize(ConstRef<AnyVector3> Vector)
            requires(IsReal<Type>)
        {
            const Type Length = Vector.GetLength();
            LOG_ASSERT(!Base::IsAlmostZero(Length), "Cannot normalize a zero-length vector");

            return Vector * (Type(1) / Length);
        }

        /// \brief Checks if two vectors are parallel.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return `true` if the vectors are parallel, `false` otherwise.
        ZYPHRYON_INLINE constexpr static Bool IsParallel(ConstRef<AnyVector3> P0, ConstRef<AnyVector3> P1)
        {
            return Cross(P0, P1).IsAlmostZero();
        }

        /// \brief Projects the source vector onto the target vector.
        ///
        /// \param Source The vector to be projected.
        /// \param Target The vector onto which the source is projected.
        /// \return The projection of source onto target.
        ZYPHRYON_INLINE constexpr static AnyVector3 Project(ConstRef<AnyVector3> Source, ConstRef<AnyVector3> Target)
            requires(IsReal<Type>)
        {
            const Type Denominator = Dot(Target, Target);
            LOG_ASSERT(!Base::IsAlmostZero(Denominator), "Cannot project onto zero-length vector");

            return Target * (Dot(Source, Target) / Denominator);
        }

        /// \brief Reflects the incident vector over the given normal.
        ///
        /// \param Incident The incoming vector to reflect.
        /// \param Normal   The surface normal to reflect across (should be normalized).
        /// \return The reflected vector.
        ZYPHRYON_INLINE constexpr static AnyVector3 Reflect(ConstRef<AnyVector3> Incident, ConstRef<AnyVector3> Normal)
        {
            LOG_ASSERT(Normal.IsNormalized(), "Normal vector must be normalized");

            return Incident - Normal * (Type(2) * Dot(Incident, Normal));
        }

        /// \brief Computes the dot product of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The dot product of the two vectors.
        ZYPHRYON_INLINE constexpr static Type Dot(ConstRef<AnyVector3> P0, ConstRef<AnyVector3> P1)
        {
            return (P0.GetX() * P1.GetX()) + (P0.GetY() * P1.GetY()) + (P0.GetZ() * P1.GetZ());
        }

        /// \brief Computes the cross product of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The cross product of the two vectors.
        ZYPHRYON_INLINE constexpr static AnyVector3 Cross(ConstRef<AnyVector3> P0, ConstRef<AnyVector3> P1)
        {
            const Type X = P0.GetY() * P1.GetZ() - P0.GetZ() * P1.GetY();
            const Type Y = P0.GetZ() * P1.GetX() - P0.GetX() * P1.GetZ();
            const Type Z = P0.GetX() * P1.GetY() - P0.GetY() * P1.GetX();

            return AnyVector3(X, Y, Z);
        }

        /// \brief Returns the component-wise minimum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the component-wise minimum values.
        ZYPHRYON_INLINE constexpr static AnyVector3 Min(ConstRef<AnyVector3> P0, ConstRef<AnyVector3> P1)
        {
            return AnyVector3(Base::Min(P0.mX, P1.mX), Base::Min(P0.mY, P1.mY), Base::Min(P0.mZ, P1.mZ));
        }

        /// \brief Returns the component-wise maximum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the component-wise maximum values.
        ZYPHRYON_INLINE constexpr static AnyVector3 Max(ConstRef<AnyVector3> P0, ConstRef<AnyVector3> P1)
        {
            return AnyVector3(Base::Max(P0.mX, P1.mX), Base::Max(P0.mY, P1.mY), Base::Max(P0.mZ, P1.mZ));
        }

        /// \brief Floors each component of the given vector.
        ///
        /// \param Vector The vector to floor.
        /// \return A vector with floored components.
        ZYPHRYON_INLINE constexpr static AnyVector3 Floor(ConstRef<AnyVector3> Vector)
            requires(IsReal<Type>)
        {
            return AnyVector3(Base::Floor(Vector.mX), Base::Floor(Vector.mY), Base::Floor(Vector.mZ));
        }

        /// \brief Ceils each component of the given vector.
        ///
        /// \param Vector The vector to ceil.
        /// \return A vector with ceiled components.
        ZYPHRYON_INLINE constexpr static AnyVector3 Ceil(ConstRef<AnyVector3> Vector)
            requires(IsReal<Type>)
        {
            return AnyVector3(Base::Ceil(Vector.mX), Base::Ceil(Vector.mY), Base::Ceil(Vector.mZ));
        }

        /// \brief Linearly interpolates between two vectors.
        ///
        /// \param Start      The starting vector.
        /// \param End        The ending vector.
        /// \param Percentage The interpolation percentage (range between 0 and 1).
        /// \return A vector interpolated between the start and end vectors.
        ZYPHRYON_INLINE constexpr static AnyVector3 Lerp(ConstRef<AnyVector3> Start, ConstRef<AnyVector3> End, Type Percentage)
            requires(IsReal<Type>)
        {
            LOG_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

            return Start + (End - Start) * Percentage;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type mX;
        Type mY;
        Type mZ;
    };

    /// \brief Represents a vector with (x, y, z) floating-point components.
    using Vector3    = AnyVector3<Real32>;

    /// \brief Represents a vector with (x, y, z) signed integer components.
    using IntVector3 = AnyVector3<SInt32>;
}
