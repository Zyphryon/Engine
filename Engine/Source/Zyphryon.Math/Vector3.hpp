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

#include "Vector2.hpp"

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
        ZY_INLINE constexpr AnyVector3()
            : mX { 0 },
              mY { 0 },
              mZ { 0 }
        {
        }

        /// \brief Constructor initializing all components to the same value.
        ///
        /// \param Scalar The scalar value used to initialize all components.
        ZY_INLINE constexpr explicit AnyVector3(Type Scalar)
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
        ZY_INLINE constexpr AnyVector3(Type X, Type Y, Type Z)
            : mX { X },
              mY { Y },
              mZ { Z }
        {
        }

        /// \brief Upward conversion constructor from a 2D vector, setting z to zero.
        ///
        /// \param XY The 2D vector to use for x and y components.
        /// \param Z  The z-component to use (default is zero).
        template<typename Base>
        ZY_INLINE constexpr explicit AnyVector3(AnyVector2<Base> XY, Type Z = Type(0))
            : mX { static_cast<Type>(XY.GetX()) },
              mY { static_cast<Type>(XY.GetY()) },
              mZ { Z }
        {
        }

        /// \brief Conversion constructor from another vector with a different base type.
        ///
        /// \param Other The source vector to convert from.
        template<typename Base>
        ZY_INLINE constexpr explicit AnyVector3(AnyVector3<Base> Other)
            : mX { static_cast<Type>(Other.GetX()) },
              mY { static_cast<Type>(Other.GetY()) },
              mZ { static_cast<Type>(Other.GetZ()) }
        {
        }

        /// \brief Checks if the vector is the zero vector.
        ///
        /// \return `true` if all components are approximately zero, `false` otherwise.
        ZY_INLINE constexpr Bool IsAlmostZero() const
        {
            return ::IsAlmostZero(mX) && ::IsAlmostZero(mY) && ::IsAlmostZero(mZ);
        }

        /// \brief Checks if all components are approximately equal to the given values.
        ///
        /// \param X The value to compare with the X component.
        /// \param Y The value to compare with the Y component.
        /// \param Z The value to compare with the Z component.
        /// \return `true` if all components are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool IsAlmostEqual(Type X, Type Y, Type Z) const
        {
            return ::IsAlmostEqual(mX, X) && ::IsAlmostEqual(mY, Y) && ::IsAlmostEqual(mZ, Z);
        }

        /// \brief Checks if the vector is normalized.
        ///
        /// \return `true` if the vector is normalized, `false` otherwise.
        ZY_INLINE constexpr Bool IsNormalized() const
            requires(IsReal<Type>)
        {
            return ::IsAlmostEqual(GetLengthSquared(), Type(1));
        }

        /// \brief Checks if all components are approximately equal within a tolerance.
        ///
        /// \param Tolerance The maximum allowed difference between components.
        /// \return `true` if all components are within \p Tolerance of each other, `false` otherwise.
        ZY_INLINE constexpr Bool IsUniform(Type Tolerance = kEpsilon<Type>) const
        {
            return (Abs(GetX() - GetY()) <= Tolerance) &&
                   (Abs(GetY() - GetZ()) <= Tolerance);
        }

        /// \brief Checks if this vector is component-wise less than another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if all components are less, otherwise `false`.
        ZY_INLINE constexpr Bool IsComponentWiseLess(AnyVector3 Other) const
        {
            return mX < Other.mX && mY < Other.mY && mZ < Other.mZ;
        }

        /// \brief Checks if this vector is component-wise less than or equal to another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if all components are less or equal, otherwise `false`.
        ZY_INLINE constexpr Bool IsComponentWiseLessOrEqual(AnyVector3 Other) const
        {
            return mX <= Other.mX && mY <= Other.mY && mZ <= Other.mZ;
        }

        /// \brief Checks if this vector is component-wise greater than another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if all components are greater, otherwise `false`.
        ZY_INLINE constexpr Bool IsComponentWiseGreater(AnyVector3 Other) const
        {
            return mX > Other.mX && mY > Other.mY && mZ > Other.mZ;
        }

        /// \brief Checks if this vector is component-wise greater than or equal to another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if all components are greater or equal, otherwise `false`.
        ZY_INLINE constexpr Bool IsComponentWiseGreaterOrEqual(AnyVector3 Other) const
        {
            return mX >= Other.mX && mY >= Other.mY && mZ >= Other.mZ;
        }

        /// \brief Checks if this vector is parallel to another vector.
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if the vectors are parallel, `false` otherwise.
        ZY_INLINE constexpr Bool IsParallel(AnyVector3 Other) const
        {
            return Cross(* this, Other).IsAlmostZero();
        }

        /// \brief Sets the components of the vector to new values.
        ///
        /// \param X The new x value.
        /// \param Y The new y value.
        /// \param Z The new z value.
        /// \return A reference to this vector after the update.
        ZY_INLINE constexpr Ref<AnyVector3> Set(Type X, Type Y, Type Z)
        {
            mX = X;
            mY = Y;
            mZ = Z;
            return (* this);
        }

        /// \brief Sets the x and y components from a 2D vector, and z to a specified value.
        ///
        /// \param XY The 2D vector to use for x and y components.
        /// \param Z  The z-component to use (default is zero).
        template<typename Base>
        ZY_INLINE constexpr Ref<AnyVector3> Set(AnyVector2<Base> XY, Type Z = Type(0))
        {
            mX = static_cast<Type>(XY.GetX());
            mY = static_cast<Type>(XY.GetY());
            mZ = Z;
            return (* this);
        }

        /// \brief Sets the x-component of the vector.
        ///
        /// \param X The new x-component.
        /// \return A reference to this vector after the update.
        ZY_INLINE constexpr Ref<AnyVector3> SetX(Type X)
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
        ZY_INLINE constexpr Ref<AnyVector3> SetY(Type Y)
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

        /// \brief Sets the z-component of the vector.
        ///
        /// \param Z The new z-component.
        /// \return A reference to this vector after the update.
        ZY_INLINE constexpr Ref<AnyVector3> SetZ(Type Z)
        {
            mZ = Z;
            return (* this);
        }

        /// \brief Gets the z-component of the vector.
        ///
        /// \return The z-component.
        ZY_INLINE constexpr Type GetZ() const
        {
            return mZ;
        }

        /// \brief Gets the (x, y) components as a 2D vector.
        ///
        /// \return A new 2D vector containing the x and y components.
        ZY_INLINE constexpr AnyVector2<Type> GetXY() const
        {
            return AnyVector2(mX, mY);
        }

        /// \brief Gets the (x, z) components as a 2D vector.
        ///
        /// \return A new 2D vector containing the x and z components.
        ZY_INLINE constexpr AnyVector2<Type> GetXZ() const
        {
            return AnyVector2(mX, mZ);
        }

        /// \brief Calculates the length (magnitude) of the vector.
        ///
        /// \return The length of the vector.
        ZY_INLINE Type GetLength() const
            requires(IsReal<Type>)
        {
            return Sqrt(GetLengthSquared());
        }

        /// \brief Calculates the squared length of the vector.
        ///
        /// \return The squared length of the vector (no square root).
        ZY_INLINE constexpr Type GetLengthSquared() const
        {
            return (mX * mX + mY * mY + mZ * mZ);
        }

        /// \brief Calculates the distance between this vector and another vector.
        ///
        /// \param Target The vector to calculate the distance to.
        /// \return The distance between the two vectors.
        ZY_INLINE Type GetDistance(AnyVector3 Target) const
            requires(IsReal<Type>)
        {
            return ((* this) - Target).GetLength();
        }

        /// \brief Calculates the squared distance between this vector and another vector.
        ///
        /// \param Target The vector to calculate the squared distance to.
        /// \return The squared distance between the two vectors.
        ZY_INLINE constexpr Type GetDistanceSquared(AnyVector3 Target) const
        {
            return ((* this) - Target).GetLengthSquared();
        }

        /// \brief Gets the angle in radians between this vector and another.
        ///
        /// \param Other The other vector to compare with.
        /// \return The angle in radians between the two vectors (range [0, π]).
        ZY_INLINE Angle GetAngle(AnyVector3 Other) const
            requires(IsReal<Type>)
        {
            const Type Length = GetLength() * Other.GetLength();
            ZY_ASSERT(!::IsAlmostZero(Length), "Cannot compute angle with zero-length vector");

            return Angle::FromCosine(Dot(* this, Other) / Length);
        }

        /// \brief Adds another vector to this vector.
        ///
        /// \param Other The vector to add.
        /// \return A new vector that is the sum of this vector and the input vector.
        ZY_INLINE constexpr AnyVector3 operator+(AnyVector3 Other) const
        {
            return AnyVector3(mX + Other.mX, mY + Other.mY, mZ + Other.mZ);
        }

        /// \brief Adds a scalar to all components of this vector.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new vector with the scalar added to all components.
        ZY_INLINE constexpr AnyVector3 operator+(Type Scalar) const
        {
            return AnyVector3(mX + Scalar, mY + Scalar, mZ + Scalar);
        }

        /// \brief Negates the vector.
        ///
        /// \return A new vector that is the negation of this vector.
        ZY_INLINE constexpr AnyVector3 operator-() const
        {
            return AnyVector3(-mX, -mY, -mZ);
        }

        /// \brief Subtracts another vector from this vector.
        ///
        /// \param Other The vector to subtract.
        /// \return A new vector that is the difference of the two vectors.
        ZY_INLINE constexpr AnyVector3 operator-(AnyVector3 Other) const
        {
            return AnyVector3(mX - Other.mX, mY - Other.mY, mZ - Other.mZ);
        }

        /// \brief Subtracts a scalar from all components of this vector.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new vector with the scalar subtracted from all components.
        ZY_INLINE constexpr AnyVector3 operator-(Type Scalar) const
        {
            return AnyVector3(mX - Scalar, mY - Scalar, mZ - Scalar);
        }

        /// \brief Multiplies all components of this vector by another vector.
        ///
        /// \param Other The vector to multiply by.
        /// \return A new vector that is the product of the two vectors.
        ZY_INLINE constexpr AnyVector3 operator*(AnyVector3 Other) const
        {
            return AnyVector3(mX * Other.mX, mY * Other.mY, mZ * Other.mZ);
        }

        /// \brief Multiplies all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new vector with the scalar multiplied by all components.
        ZY_INLINE constexpr AnyVector3 operator*(Type Scalar) const
        {
            return AnyVector3(mX * Scalar, mY * Scalar, mZ * Scalar);
        }

        /// \brief Divides all components of this vector by another vector.
        ///
        /// \param Other The vector to divide by.
        /// \return A new vector that is the quotient of the two vectors.
        ZY_INLINE constexpr AnyVector3 operator/(AnyVector3 Other) const
        {
            ZY_ASSERT(!::IsAlmostZero(Other.GetX()), "Division by zero (X)");
            ZY_ASSERT(!::IsAlmostZero(Other.GetY()), "Division by zero (Y)");
            ZY_ASSERT(!::IsAlmostZero(Other.GetZ()), "Division by zero (Z)");

            return AnyVector3(mX / Other.mX, mY / Other.mY, mZ / Other.mZ);
        }

        /// \brief Divides all components of this vector by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new vector with the scalar divided into all components.
        ZY_INLINE constexpr AnyVector3 operator/(Type Scalar) const
        {
            ZY_ASSERT(!::IsAlmostZero(Scalar), "Division by zero");

            return AnyVector3(mX / Scalar, mY / Scalar, mZ / Scalar);
        }

        /// \brief Shift left all coordinates of this vector by a scalar.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A new vector with the components shifted.
        ZY_INLINE constexpr AnyVector3 operator<<(Type Scalar) const
            requires(IsIntegral<Type>)
        {
            ZY_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            return AnyVector3(mX << Scalar, mY << Scalar, mZ << Scalar);
        }

        /// \brief Shift right all coordinates of this vector by a scalar.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A new vector with the components shifted.
        ZY_INLINE constexpr AnyVector3 operator>>(Type Scalar) const
            requires(IsIntegral<Type>)
        {
            ZY_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            return AnyVector3(mX >> Scalar, mY >> Scalar, mZ >> Scalar);
        }

        /// \brief Pre-increments all components of the vector.
        ///
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator++()
        {
            ++mX;
            ++mY;
            ++mZ;
            return (* this);
        }

        /// \brief Pre-decrements all components of the vector.
        ///
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator--()
        {
            --mX;
            --mY;
            --mZ;
            return (* this);
        }

        /// \brief Adds another vector to the current vector.
        ///
        /// \param Other The vector to add.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator+=(AnyVector3 Other)
        {
            mX += Other.mX;
            mY += Other.mY;
            mZ += Other.mZ;
            return (* this);
        }

        /// \brief Adds a scalar value to all components of the vector.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator+=(Type Scalar)
        {
            mX += Scalar;
            mY += Scalar;
            mZ += Scalar;
            return (* this);
        }

        /// \brief Subtracts another vector from the current vector.
        ///
        /// \param Other The vector to subtract.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator-=(AnyVector3 Other)
        {
            mX -= Other.mX;
            mY -= Other.mY;
            mZ -= Other.mZ;
            return (* this);
        }

        /// \brief Subtracts a scalar value from all components of the vector.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator-=(Type Scalar)
        {
            mX -= Scalar;
            mY -= Scalar;
            mZ -= Scalar;
            return (* this);
        }

        /// \brief Multiplies all components of the vector by another vector.
        ///
        /// \param Other The vector to multiply.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator*=(AnyVector3 Other)
        {
            mX *= Other.mX;
            mY *= Other.mY;
            mZ *= Other.mZ;
            return (* this);
        }

        /// \brief Multiplies all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator*=(Type Scalar)
        {
            mX *= Scalar;
            mY *= Scalar;
            mZ *= Scalar;
            return (* this);
        }

        /// \brief Divides all components of the vector by another vector.
        ///
        /// \param Other The vector to divide by.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator/=(AnyVector3 Other)
        {
            ZY_ASSERT(!::IsAlmostZero(Other.GetX()), "Division by zero (X)");
            ZY_ASSERT(!::IsAlmostZero(Other.GetY()), "Division by zero (Y)");
            ZY_ASSERT(!::IsAlmostZero(Other.GetZ()), "Division by zero (Z)");

            mX /= Other.mX;
            mY /= Other.mY;
            mZ /= Other.mZ;
            return (* this);
        }

        /// \brief Divides all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator/=(Type Scalar)
        {
            ZY_ASSERT(!::IsAlmostZero(Scalar), "Division by zero");

            mX /= Scalar;
            mY /= Scalar;
            mZ /= Scalar;
            return (* this);
        }

        /// \brief Shifts left all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator<<=(Type Scalar)
            requires(IsIntegral<Type>)
        {
            ZY_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            mX <<= Scalar;
            mY <<= Scalar;
            mZ <<= Scalar;

            return (* this);
        }

        /// \brief Shifts right all components of the vector by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated vector.
        ZY_INLINE constexpr Ref<AnyVector3> operator>>=(Type Scalar)
            requires(IsIntegral<Type>)
        {
            ZY_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            mX >>= Scalar;
            mY >>= Scalar;
            mZ >>= Scalar;

            return (* this);
        }

        /// \brief Checks if this vector is equal to another vector.
        ///
        /// \param Other The vector to compare to.
        /// \return `true` if all components are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(AnyVector3 Other) const
        {
            return IsAlmostEqual(Other.mX, Other.mY, Other.mZ);
        }

        /// \brief Checks if this vector is not equal to another vector.
        ///
        /// \param Other The vector to compare to.
        /// \return `true` if the vectors are not equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(AnyVector3 Other) const
        {
            return !(* this == Other);
        }

        /// \brief Compares this vector with another for less-than relationship (lexicographic ordering).
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if this vector is lexicographically less than the other vector, otherwise `false`.
        ZY_INLINE constexpr Bool operator<(AnyVector3 Other) const
        {
            return (mX < Other.mX) || (mX == Other.mX && mY < Other.mY) || (mX == Other.mX && mY == Other.mY && mZ < Other.mZ);
        }

        /// \brief Compares this vector with another for less-than or equal relationship (lexicographic ordering).
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if this vector is lexicographically less than or equal to the other vector, otherwise `false`.
        ZY_INLINE constexpr Bool operator<=(AnyVector3 Other) const
        {
            return (mX < Other.mX) || (mX == Other.mX && mY < Other.mY) || (mX == Other.mX && mY == Other.mY && mZ <= Other.mZ);
        }

        /// \brief Compares this vector with another for greater-than relationship (lexicographic ordering).
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if this vector is lexicographically greater than the other vector, otherwise `false`.
        ZY_INLINE constexpr Bool operator>(AnyVector3 Other) const
        {
            return (mX > Other.mX) || (mX == Other.mX && mY > Other.mY) || (mX == Other.mX && mY == Other.mY && mZ > Other.mZ);
        }

        /// \brief Compares this vector with another for greater-than or equal relationship (lexicographic ordering).
        ///
        /// \param Other The vector to compare against.
        /// \return `true` if this vector is lexicographically greater than or equal to the other vector, otherwise `false`.
        ZY_INLINE constexpr Bool operator>=(AnyVector3 Other) const
        {
            return (mX > Other.mX) || (mX == Other.mX && mY > Other.mY) || (mX == Other.mX && mY == Other.mY && mZ >= Other.mZ);
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<7> kPattern("({0}, {1}, {2})");
            Format::Processor<Output>::Format(Buffer, kPattern, mX, mY, mZ);
        }

    public:

        /// \brief Gets the zero vector (0, 0, 0).
        ///
        /// \return The zero vector.
        ZY_INLINE static constexpr AnyVector3 Zero()
        {
            return AnyVector3();
        }

        /// \brief Gets the one vector (1, 1, 1).
        ///
        /// \return The one vector.
        ZY_INLINE static constexpr AnyVector3 One()
        {
            return AnyVector3(Type(1), Type(1), Type(1));
        }

        /// \brief Gets the unit vector along the X-axis (1, 0, 0).
        ///
        /// \return The unit vector along the X-axis.
        ZY_INLINE static constexpr AnyVector3 UnitX()
        {
            return AnyVector3(Type(1), Type(0), Type(0));
        }

        /// \brief Gets the unit vector along the Y-axis (0, 1, 0).
        ///
        /// \return The unit vector along the Y-axis.
        ZY_INLINE static constexpr AnyVector3 UnitY()
        {
            return AnyVector3(Type(0), Type(1), Type(0));
        }

        /// \brief Gets the unit vector along the Z-axis (0, 0, 1).
        ///
        /// \return The unit vector along the Z-axis.
        ZY_INLINE static constexpr AnyVector3 UnitZ()
        {
            return AnyVector3(Type(0), Type(0), Type(1));
        }

        /// \brief Projects the source vector onto the target vector.
        ///
        /// \param Source The vector to be projected.
        /// \param Target The vector onto which the source is projected.
        /// \return The projection of source onto target.
        ZY_INLINE static constexpr AnyVector3 Project(AnyVector3 Source, AnyVector3 Target)
            requires(IsReal<Type>)
        {
            const Type Denominator = Dot(Target, Target);
            ZY_ASSERT(!::IsAlmostZero(Denominator), "Cannot project onto zero-length vector");

            return Target * (Dot(Source, Target) / Denominator);
        }

        /// \brief Normalizes the given vector.
        ///
        /// \param Vector The vector to normalize.
        /// \return A normalized vector, or the original if its length is too small.
        ZY_INLINE static AnyVector3 Normalize(AnyVector3 Vector)
            requires(IsReal<Type>)
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
        ZY_INLINE static constexpr AnyVector3 Reflect(AnyVector3 Incident, AnyVector3 Normal)
            requires(IsReal<Type>)
        {
            ZY_ASSERT(Normal.IsNormalized(), "Normal vector must be normalized");

            return Incident - Normal * (Type(2) * Dot(Incident, Normal));
        }

        /// \brief Computes the dot product of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The dot product of the two vectors.
        ZY_INLINE static constexpr Type Dot(AnyVector3 P0, AnyVector3 P1)
        {
            return (P0.GetX() * P1.GetX()) + (P0.GetY() * P1.GetY()) + (P0.GetZ() * P1.GetZ());
        }

        /// \brief Computes the cross product of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return The cross product of the two vectors.
        ZY_INLINE static constexpr AnyVector3 Cross(AnyVector3 P0, AnyVector3 P1)
        {
            const Type X = P0.GetY() * P1.GetZ() - P0.GetZ() * P1.GetY();
            const Type Y = P0.GetZ() * P1.GetX() - P0.GetX() * P1.GetZ();
            const Type Z = P0.GetX() * P1.GetY() - P0.GetY() * P1.GetX();

            return AnyVector3(X, Y, Z);
        }

        /// \brief Gets the component-wise minimum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the component-wise minimum values.
        ZY_INLINE static constexpr AnyVector3 Min(AnyVector3 P0, AnyVector3 P1)
        {
            return AnyVector3(::Min(P0.mX, P1.mX), ::Min(P0.mY, P1.mY), ::Min(P0.mZ, P1.mZ));
        }

        /// \brief Gets the component-wise maximum of two vectors.
        ///
        /// \param P0 The first vector.
        /// \param P1 The second vector.
        /// \return A vector with the component-wise maximum values.
        ZY_INLINE static constexpr AnyVector3 Max(AnyVector3 P0, AnyVector3 P1)
        {
            return AnyVector3(::Max(P0.mX, P1.mX), ::Max(P0.mY, P1.mY), ::Max(P0.mZ, P1.mZ));
        }

        /// \brief Clamps each component of a vector between the corresponding components of min and max vectors.
        ///
        /// \param Vector The source vector to clamp.
        /// \param Min    The vector specifying the minimum bounds.
        /// \param Max    The vector specifying the maximum bounds.
        /// \return A vector with each component clamped between the min and max values.
        ZY_INLINE static constexpr AnyVector3 Clamp(AnyVector3 Vector, AnyVector3 Min, AnyVector3 Max)
        {
            return AnyVector3(
                ::Clamp(Vector.mX, Min.mX, Max.mX),
                ::Clamp(Vector.mY, Min.mY, Max.mY),
                ::Clamp(Vector.mZ, Min.mZ, Max.mZ));
        }

        /// \brief Gets a vector with each component floored to the nearest integer.
        ///
        /// \param Vector The source vector with real-valued components.
        /// \return A vector with all components rounded down.
        ZY_INLINE static constexpr AnyVector3 Floor(AnyVector3 Vector)
            requires(IsReal<Type>)
        {
            return AnyVector3(::Floor(Vector.mX), ::Floor(Vector.mY), ::Floor(Vector.mZ));
        }

        /// \brief Gets a vector with each component ceiled to the nearest integer.
        ///
        /// \param Vector The source vector with real-valued components.
        /// \return A vector with all components rounded up.
        ZY_INLINE static constexpr AnyVector3 Ceil(AnyVector3 Vector)
            requires(IsReal<Type>)
        {
            return AnyVector3(::Ceil(Vector.mX), ::Ceil(Vector.mY), ::Ceil(Vector.mZ));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type mX;
        Type mY;
        Type mZ;
    };

    /// \brief Represents a vector with (x, y, z) floating-point components.
    using Vector3     = AnyVector3<Real32>;

    /// \brief Represents a vector with (x, y, z) signed integer components.
    using IntVector3  = AnyVector3<SInt32>;

    /// \brief Represents a vector with (x, y, z) unsigned integer components.
    using UIntVector3 = AnyVector3<UInt32>;
}