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

#include "Zyphryon.Math/Matrix3x2.hpp"
#include "Zyphryon.Math/Matrix4x4.hpp"
#include "Zyphryon.Math/Pivot.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a generic 2D rectangle defined by (X1, Y1, X2, Y2) coordinates.
    template<typename Type>
    class AnyRect final
    {
    public:

        /// \brief Constructor initializing all components zero.
        ZYPHRYON_INLINE constexpr AnyRect()
            : mMinimumX { 0 },
              mMinimumY { 0 },
              mMaximumX { 0 },
              mMaximumY { 0 }
        {
        }

        /// \brief Constructor initializing the rectangle with specified values.
        ///
        /// \param MinimumX The minimum X coordinate of the rectangle.
        /// \param MinimumY The minimum Y coordinate of the rectangle.
        /// \param MaximumX The maximum X coordinate of the rectangle.
        /// \param MaximumY the maximum Y coordinate of the rectangle.
        ZYPHRYON_INLINE constexpr AnyRect(Type MinimumX, Type MinimumY, Type MaximumX, Type MaximumY)
            : mMinimumX { MinimumX },
              mMinimumY { MinimumY },
              mMaximumX { MaximumX },
              mMaximumY { MaximumY }
        {
        }

        /// \brief Constructor initializing the rectangle from minimum and maximum points.
        ///
        /// \param Minimum The minimum point of the rectangle.
        /// \param Maximum The maximum point of the rectangle.
        ZYPHRYON_INLINE constexpr AnyRect(AnyVector2<Type> Minimum, AnyVector2<Type> Maximum)
            : mMinimumX { Minimum.GetX() },
              mMinimumY { Minimum.GetY() },
              mMaximumX { Maximum.GetX() },
              mMaximumY { Maximum.GetY() }
        {
        }

        /// \brief Conversion constructor from another rectangle with a different base type.
        ///
        /// \param Other The source rectangle to convert from.
        template<typename Base>
        ZYPHRYON_INLINE constexpr explicit AnyRect(AnyRect<Base> Other)
            : mMinimumX { static_cast<Type>(Other.GetMinimumX()) },
              mMinimumY { static_cast<Type>(Other.GetMinimumY()) },
              mMaximumX { static_cast<Type>(Other.GetMaximumX()) },
              mMaximumY { static_cast<Type>(Other.GetMaximumY()) }
        {
        }

        /// \brief Checks if the rectangle is valid (X1 <= X2 and Y1 <= Y2).
        ///
        /// \return `true` if the rectangle is valid, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsValid() const
        {
            return mMinimumX <= mMaximumX && mMinimumY <= mMaximumY;
        }

        /// \brief Checks if the rectangle is the zero rectangle.
        ///
        /// \return `true` if all coordinates are approximately zero, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsAlmostZero() const
        {
            return Math::IsAlmostZero(mMinimumX) && Math::IsAlmostZero(mMinimumY) &&
                   Math::IsAlmostZero(mMaximumX) && Math::IsAlmostZero(mMaximumY);
        }

        /// \brief Sets the coordinates of the rectangle.
        ///
        /// \param MinimumX The minimum X coordinate of the rectangle.
        /// \param MinimumY The minimum Y coordinate of the rectangle.
        /// \param MaximumX The maximum X coordinate of the rectangle.
        /// \param MaximumY the maximum Y coordinate of the rectangle.
        ZYPHRYON_INLINE constexpr void Set(Type MinimumX, Type MinimumY, Type MaximumX, Type MaximumY)
        {
            mMinimumX = MinimumX;
            mMinimumY = MinimumY;
            mMaximumX = MaximumX;
            mMaximumY = MaximumY;
        }

        /// \brief Sets the minimum coordinates of the rectangle.
        ///
        /// \param MinimumX The minimum X coordinate of the rectangle.
        /// \param MinimumY The minimum Y coordinate of the rectangle.
        ZYPHRYON_INLINE constexpr void SetMinimum(Type MinimumX, Type MinimumY)
        {
            mMinimumX = MinimumX;
            mMinimumY = MinimumY;
        }

        /// \brief Gets the minimum X coordinate of the rectangle.
        ///
        /// \return The minimum X coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMinimumX() const
        {
            return mMinimumX;
        }

        /// \brief Gets the minimum Y coordinate of the rectangle.
        ///
        /// \return The minimum Y coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMinimumY() const
        {
            return mMinimumY;
        }

        /// \brief Sets the maximum coordinates of the rectangle.
        ///
        /// \param MaximumX The maximum X coordinate of the rectangle.
        /// \param MaximumY The maximum Y coordinate of the rectangle.
        ZYPHRYON_INLINE constexpr void SetMaximum(Type MaximumX, Type MaximumY)
        {
            mMaximumX = MaximumX;
            mMaximumY = MaximumY;
        }

        /// \brief Gets the maximum X coordinate of the rectangle.
        ///
        /// \return The maximum X coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMaximumX() const
        {
            return mMaximumX;
        }

        /// \brief Gets the maximum Y coordinate of the rectangle.
        ///
        /// \return The maximum Y coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMaximumY() const
        {
            return mMaximumY;
        }

        /// \brief Gets the X coordinate of the rectangle.
        ///
        /// \return The X coordinate.
        ZYPHRYON_INLINE constexpr Type GetX() const
        {
            return mMinimumX;
        }

        /// \brief Gets the Y coordinate of the rectangle.
        ///
        /// \return The Y coordinate.
        ZYPHRYON_INLINE constexpr Type GetY() const
        {
            return mMinimumY;
        }

        /// \brief Calculates the width of the rectangle.
        ///
        /// \return The width of the rectangle.
        ZYPHRYON_INLINE constexpr Type GetWidth() const
        {
            return GetMaximumX() - GetMinimumX();
        }

        /// \brief Calculates the height of the rectangle.
        ///
        /// \return The height of the rectangle.
        ZYPHRYON_INLINE constexpr Type GetHeight() const
        {
            return GetMaximumY() - GetMinimumY();
        }

        /// \brief Gets the position of the rectangle.
        ///
        /// \return The position of the rectangle.
        ZYPHRYON_INLINE constexpr AnyVector2<Type> GetPosition() const
        {
            return AnyVector2(mMinimumX, mMinimumY);
        }

        /// \brief Gets the size of the rectangle.
        ///
        /// \return The size of the rectangle.
        ZYPHRYON_INLINE constexpr AnyVector2<Type> GetSize() const
        {
            return AnyVector2(GetWidth(), GetHeight());
        }

        /// \brief Calculates the center point of the rectangle.
        ///
        /// \return The center point of the rectangle.
        ZYPHRYON_INLINE constexpr AnyVector2<Type> GetCenter() const
        {
            if constexpr(IsReal<Type>)
            {
                return AnyVector2<Type>(mMinimumX + GetWidth() * Type(0.5), mMinimumY + GetHeight() * Type(0.5));
            }
            else
            {
                return AnyVector2<Type>(mMinimumX + GetWidth() / Type(2), mMinimumY + GetHeight() / Type(2));
            }
        }

        /// \brief Gets the area of the rectangle.
        ///
        /// \return The area of the rectangle.
        ZYPHRYON_INLINE constexpr Type GetArea() const
        {
            return GetWidth() * GetHeight();
        }

        /// \brief Gets the perimeter of the rectangle.
        ///
        /// \return The perimeter of the rectangle.
        ZYPHRYON_INLINE constexpr Type GetPerimeter() const
        {
            return Type(2) * (GetWidth() + GetHeight());
        }

        /// \brief Returns the nearest point on or inside the rectangle to a given point.
        ///
        /// \param Point The query point in 2D space.
        /// \return The point clamped to the rectangle's boundaries.
        ZYPHRYON_INLINE constexpr AnyVector2<Type> GetNearest(AnyVector2<Type> Point) const
        {
            return AnyVector2<Type>(
                Clamp(Point.GetX(), mMinimumX, mMaximumX),
                Clamp(Point.GetY(), mMinimumY, mMaximumY));
        }

        /// \brief Expands the rectangle by the given amount in all directions.
        ///
        /// \param Amount The amount to expand by.
        /// \return The expanded rectangle.
        ZYPHRYON_INLINE constexpr AnyRect Expand(Type Amount) const
        {
            return AnyRect(mMinimumX - Amount, mMinimumY - Amount, mMaximumX + Amount, mMaximumY + Amount);
        }

        /// \brief Expands the rectangle by the given amounts in the X and Y directions.
        ///
        /// \param AmountX The amount to expand in the X direction.
        /// \param AmountY The amount to expand in the Y direction.
        /// \return The expanded rectangle.
        ZYPHRYON_INLINE constexpr AnyRect Expand(Type AmountX, Type AmountY) const
        {
            return AnyRect(mMinimumX - AmountX, mMinimumY - AmountY, mMaximumX + AmountX, mMaximumY + AmountY);
        }

        /// \brief Contracts the rectangle by the given amount in all directions.
        ///
        /// \param Amount The amount to contract by.
        /// \return The contracted rectangle.
        ZYPHRYON_INLINE constexpr AnyRect Contract(Type Amount) const
        {
            return AnyRect(mMinimumX + Amount, mMinimumY + Amount, mMaximumX - Amount, mMaximumY - Amount);
        }

        /// \brief Checks if this rectangle completely contains another rectangle.
        ///
        /// \param Other The other rectangle to check.
        /// \return `true` if this rectangle contains the other, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(ConstRef<AnyRect> Other) const
        {
            return mMinimumX <= Other.mMinimumX && mMaximumX >= Other.mMaximumX &&
                   mMinimumY <= Other.mMinimumY && mMaximumY >= Other.mMaximumY;
        }

        /// \brief Checks if this rectangle contains a point.
        ///
        /// \param X The X coordinate of the point.
        /// \param Y The Y coordinate of the point.
        /// \return `true` if the point is inside the rectangle, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(Type X, Type Y) const
        {
            return X >= mMinimumX && X < mMaximumX && Y >= mMinimumY && Y < mMaximumY;
        }

        /// \brief Checks if this rectangle contains a point.
        ///
        /// \param Position The position to check.
        /// \return `true` if the point is inside the rectangle, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(AnyVector2<Type> Position) const
        {
            return Contains(Position.GetX(), Position.GetY());
        }

        /// \brief Checks if this rectangle intersects with another rectangle.
        ///
        /// \param Other The other rectangle to check.
        /// \return `true` if the rectangles intersect, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Test(AnyRect Other) const
        {
            return mMinimumX < Other.mMaximumX && mMaximumX > Other.mMinimumX &&
                   mMinimumY < Other.mMaximumY && mMaximumY > Other.mMinimumY;
        }

        /// \brief Checks if this rectangle is equal to another rectangle.
        ///
        /// \param Other The rectangle to compare to.
        /// \return `true` if all coordinates are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(AnyRect Other) const
        {
            return Math::IsAlmostEqual(mMinimumX, Other.mMinimumX) &&
                   Math::IsAlmostEqual(mMinimumY, Other.mMinimumY) &&
                   Math::IsAlmostEqual(mMaximumX, Other.mMaximumX) &&
                   Math::IsAlmostEqual(mMaximumY, Other.mMaximumY);
        }

        /// \brief Checks if this rectangle is not equal to another rectangle.
        ///
        /// \param Other The rectangle to compare to.
        /// \return `true` if the rectangles are not equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(AnyRect Other) const
        {
            return !(* this == Other);
        }

        /// \brief Adds another rectangle to this rectangle.
        ///
        /// \param Other The rectangle to add.
        /// \return A new rectangle that is the sum of the two rectangles.
        ZYPHRYON_INLINE constexpr AnyRect operator+(AnyRect Other) const
        {
            return AnyRect(mMinimumX + Other.mMinimumX,
                           mMinimumY + Other.mMinimumY,
                           mMaximumX + Other.mMaximumX,
                           mMaximumY + Other.mMaximumY);
        }

        /// \brief Adds a scalar to all coordinates of this rectangle.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new rectangle with the scalar added to all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator+(Type Scalar) const
        {
            return AnyRect(mMinimumX + Scalar, mMinimumY + Scalar, mMaximumX + Scalar, mMaximumY + Scalar);
        }

        /// \brief Adds a vector to all coordinates of this rectangle.
        ///
        /// \param Vector The vector to add.
        /// \return A new rectangle with the vector added to all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator+(AnyVector2<Type> Vector) const
        {
            return AnyRect(mMinimumX + Vector.GetX(),
                           mMinimumY + Vector.GetY(),
                           mMaximumX + Vector.GetX(),
                           mMaximumY + Vector.GetY());
        }

        /// \brief Subtracts another rectangle from this rectangle.
        ///
        /// \param Other The rectangle to subtract.
        /// \return A new rectangle that is the difference of the two rectangles.
        ZYPHRYON_INLINE constexpr AnyRect operator-(AnyRect Other) const
        {
            return AnyRect(mMinimumX - Other.mMinimumX,
                           mMinimumY - Other.mMinimumY,
                           mMaximumX - Other.mMaximumX,
                           mMaximumY - Other.mMaximumY);
        }

        /// \brief Subtracts a scalar from all coordinates of this rectangle.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new rectangle with the scalar subtracted from all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator-(Type Scalar) const
        {
            return AnyRect(mMinimumX - Scalar, mMinimumY - Scalar, mMaximumX - Scalar, mMaximumY - Scalar);
        }

        /// \brief Subtracts a vector from all coordinates of this rectangle.
        ///
        /// \param Vector The vector to subtract.
        /// \return A new rectangle with the vector subtracted from all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator-(AnyVector2<Type> Vector) const
        {
            return AnyRect(mMinimumX - Vector.GetX(),
                           mMinimumY - Vector.GetY(),
                           mMaximumX - Vector.GetX(),
                           mMaximumY - Vector.GetY());
        }

        /// \brief Multiplies all coordinates of this rectangle by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new rectangle with the scalar multiplied by all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator*(Type Scalar) const
        {
            return AnyRect(mMinimumX * Scalar, mMinimumY * Scalar, mMaximumX * Scalar, mMaximumY * Scalar);
        }

        /// \brief Multiplies all coordinates of this rectangle by a vector.
        ///
        /// \param Vector The vector to multiply by.
        /// \return A new rectangle with the vector multiplied by all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator*(AnyVector2<Type> Vector) const
        {
            return AnyRect(mMinimumX * Vector.GetX(),
                           mMinimumY * Vector.GetY(),
                           mMaximumX * Vector.GetX(),
                           mMaximumY * Vector.GetY());
        }

        /// \brief Divides all coordinates of this rectangle by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new rectangle with the scalar divided by all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator/(Type Scalar) const
        {
            LOG_ASSERT(!Math::IsAlmostZero(Scalar), "Division by zero");

            return AnyRect(mMinimumX / Scalar, mMinimumY / Scalar, mMaximumX / Scalar, mMaximumY / Scalar);
        }

        /// \brief Divides all coordinates of this rectangle by a vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A new rectangle with the vector divided by all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator/(AnyVector2<Type> Vector) const
        {
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetX()), "Division by zero (X)");
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetY()), "Division by zero (Y)");

            return AnyRect(mMinimumX / Vector.GetX(),
                           mMinimumY / Vector.GetY(),
                           mMaximumX / Vector.GetX(),
                           mMaximumY / Vector.GetY());
        }

        /// \brief Shift left all coordinates of this rectangle by a scalar.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A new rectangle with the scalar shifted by all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator<<(Type Scalar) const
            requires(IsIntegral<Type>)
        {
            LOG_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            return AnyRect(mMinimumX << Scalar,
                           mMinimumY << Scalar,
                           mMaximumX << Scalar,
                           mMaximumY << Scalar);
        }

        /// \brief Shift right all coordinates of this rectangle by a scalar.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A new rectangle with the scalar shifted by all coordinates.
        ZYPHRYON_INLINE constexpr AnyRect operator>>(Type Scalar) const
            requires(IsIntegral<Type>)
        {
            LOG_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            return AnyRect(mMinimumX >> Scalar,
                           mMinimumY >> Scalar,
                           mMaximumX >> Scalar,
                           mMaximumY >> Scalar);
        }

        /// \brief Adds another rectangle to the current rectangle.
        ///
        /// \param Other The rectangle to add.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator+=(AnyRect Other)
        {
            mMinimumX += Other.mMinimumX;
            mMinimumY += Other.mMinimumY;
            mMaximumX += Other.mMaximumX;
            mMaximumY += Other.mMaximumY;
            return (* this);
        }

        /// \brief Adds a scalar value to the rectangle.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator+=(Type Scalar)
        {
            mMinimumX += Scalar;
            mMinimumY += Scalar;
            mMaximumX += Scalar;
            mMaximumY += Scalar;
            return (* this);
        }

        /// \brief Adds a vector to the rectangle.
        ///
        /// \param Vector The vector to add.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator+=(AnyVector2<Type> Vector)
        {
            mMinimumX += Vector.GetX();
            mMinimumY += Vector.GetY();
            mMaximumX += Vector.GetX();
            mMaximumY += Vector.GetY();

            return (* this);
        }

        /// \brief Subtracts another rectangle from the current rectangle.
        ///
        /// \param Other The rectangle to subtract.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator-=(AnyRect Other)
        {
            mMinimumX -= Other.mMinimumX;
            mMinimumY -= Other.mMinimumY;
            mMaximumX -= Other.mMaximumX;
            mMaximumY -= Other.mMaximumY;
            return (* this);
        }

        /// \brief Subtracts a scalar value from the rectangle.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator-=(Type Scalar)
        {
            mMinimumX -= Scalar;
            mMinimumY -= Scalar;
            mMaximumX -= Scalar;
            mMaximumY -= Scalar;
            return (* this);
        }

        /// \brief Subtracts a vector from the rectangle.
        ///
        /// \param Vector The vector to subtract.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator-=(AnyVector2<Type> Vector)
        {
            mMinimumX -= Vector.GetX();
            mMinimumY -= Vector.GetY();
            mMaximumX -= Vector.GetX();
            mMaximumY -= Vector.GetY();

            return (* this);
        }

        /// \brief Multiplies the rectangle coordinates by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator*=(Type Scalar)
        {
            mMinimumX *= Scalar;
            mMinimumY *= Scalar;
            mMaximumX *= Scalar;
            mMaximumY *= Scalar;

            return (* this);
        }

        /// \brief Multiplies the rectangle coordinates by a vector.
        ///
        /// \param Vector The vector to multiply.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator*=(AnyVector2<Type> Vector)
        {
            mMinimumX *= Vector.GetX();
            mMinimumY *= Vector.GetY();
            mMaximumX *= Vector.GetX();
            mMaximumY *= Vector.GetY();

            return (* this);
        }

        /// \brief Divides the rectangle coordinates by a scalar value.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator/=(Type Scalar)
        {
            LOG_ASSERT(!Math::IsAlmostZero(Scalar), "Division by zero");

            mMinimumX /= Scalar;
            mMinimumY /= Scalar;
            mMaximumX /= Scalar;
            mMaximumY /= Scalar;

            return (* this);
        }

        /// \brief Divides the rectangle coordinates by a vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator/=(AnyVector2<Type> Vector)
        {
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetX()), "Division by zero (X)");
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetY()), "Division by zero (Y)");

            mMinimumX /= Vector.GetX();
            mMinimumY /= Vector.GetY();
            mMaximumX /= Vector.GetX();
            mMaximumY /= Vector.GetY();

            return (* this);
        }

        /// \brief Shifts left all coordinates of the rectangle by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator<<=(Type Scalar)
        {
            LOG_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            mMinimumX <<= Scalar;
            mMinimumY <<= Scalar;
            mMaximumX <<= Scalar;
            mMaximumY <<= Scalar;

            return (* this);
        }

        /// \brief Shifts right all coordinates of the rectangle by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated rectangle.
        ZYPHRYON_INLINE constexpr Ref<AnyRect> operator>>=(Type Scalar)
        {
            LOG_ASSERT(Scalar >= 0, "Shift amount must be non-negative");

            mMinimumX >>= Scalar;
            mMinimumY >>= Scalar;
            mMaximumX >>= Scalar;
            mMaximumY >>= Scalar;

            return (* this);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mMinimumX);
            Archive.SerializeObject(mMinimumY);
            Archive.SerializeObject(mMaximumX);
            Archive.SerializeObject(mMaximumY);
        }

    public:

        /// \brief Returns the invalid rect.
        ///
        /// \return An invalid rect.
        ZYPHRYON_INLINE static constexpr AnyRect Invalid()
        {
            return AnyRect(Type(1), Type(1), Type(0), Type(0));
        }

        /// \brief Returns an empty rect.
        ///
        /// \return An empty rect.
        ZYPHRYON_INLINE static constexpr AnyRect Zero()
        {
            return AnyRect(Type(0), Type(0), Type(0), Type(0));
        }

        /// \brief Returns the unit rect with size 1.
        ///
        /// \return An unit rect.
        ZYPHRYON_INLINE static constexpr AnyRect One()
        {
            return AnyRect(Type(0), Type(0), Type(1), Type(1));
        }

        /// \brief Canonicalizes a rectangle by ensuring X1 <= X2 and Y1 <= Y2.
        ///
        /// \param Rectangle The rectangle to canonicalize.
        /// \return A canonicalized rectangle.
        ZYPHRYON_INLINE static constexpr AnyRect Canonicalize(AnyRect Rectangle)
        {
            const Type X1 = Math::Min(Rectangle.mMinimumX, Rectangle.mMaximumX);
            const Type X2 = Math::Max(Rectangle.mMinimumX, Rectangle.mMaximumX);
            const Type Y1 = Math::Min(Rectangle.mMinimumY, Rectangle.mMaximumY);
            const Type Y2 = Math::Max(Rectangle.mMinimumY, Rectangle.mMaximumY);
            return AnyRect(X1, Y1, X2, Y2);
        }

        /// \brief Returns the component-wise minimum of two rectangles.
        ///
        /// \param First  The first rectangle.
        /// \param Second The second rectangle.
        /// \return A rect with the component-wise minimum values.
        ZYPHRYON_INLINE static constexpr AnyRect Min(AnyRect First, AnyRect Second)
        {
            return AnyRect(
                Math::Min(First.mMinimumX, Second.mMinimumX),
                Math::Min(First.mMinimumY, Second.mMinimumY),
                Math::Min(First.mMaximumX, Second.mMaximumX),
                Math::Min(First.mMaximumY, Second.mMaximumY));
        }

        /// \brief Returns the component-wise maximum of two rectangles.
        ///
        /// \param First  The first rectangle.
        /// \param Second The second rectangle.
        /// \return A rectangle with the component-wise maximum values.
        ZYPHRYON_INLINE static constexpr AnyRect Max(AnyRect First, AnyRect Second)
        {
            return AnyRect(
                Math::Max(First.mMinimumX, Second.mMinimumX),
                Math::Max(First.mMinimumY, Second.mMinimumY),
                Math::Max(First.mMaximumX, Second.mMaximumX),
                Math::Max(First.mMaximumY, Second.mMaximumY));
        }

        /// \brief Returns a rectangle with all coordinates floored to the nearest integer.
        ///
        /// \param Rectangle The source rectangle with real-valued coordinates.
        /// \return A rectangle with all coordinates rounded down.
        ZYPHRYON_INLINE static constexpr AnyRect Floor(AnyRect Rectangle)
            requires(IsReal<Type>)
        {
            return AnyRect(
                Math::Floor(Rectangle.mMinimumX),
                Math::Floor(Rectangle.mMinimumY),
                Math::Floor(Rectangle.mMaximumX),
                Math::Floor(Rectangle.mMaximumY));
        }

        /// \brief Returns a rectangle with all coordinates ceiled to the nearest integer.
        ///
        /// \param Rectangle The source rectangle with real-valued coordinates.
        /// \return A rectangle with all coordinates rounded up.
        ZYPHRYON_INLINE static constexpr AnyRect Ceil(AnyRect Rectangle)
            requires(IsReal<Type>)
        {
            return AnyRect(
                Math::Ceil(Rectangle.mMinimumX),
                Math::Ceil(Rectangle.mMinimumY),
                Math::Ceil(Rectangle.mMaximumX),
                Math::Ceil(Rectangle.mMaximumY));
        }

        /// \brief Returns the intersection of two rectangles.
        ///
        /// \param First  The first rectangle.
        /// \param Second The second rectangle.
        /// \return A rectangle representing the overlapping region. If they don't overlap, the rect may be invalid.
        ZYPHRYON_INLINE static constexpr AnyRect Intersection(AnyRect First, AnyRect Second)
        {
            const Type MinimumX = Math::Max(First.mMinimumX, Second.mMinimumX);
            const Type MinimumY = Math::Max(First.mMinimumY, Second.mMinimumY);
            const Type MaximumX = Math::Min(First.mMaximumX, Second.mMaximumX);
            const Type MaximumY = Math::Min(First.mMaximumY, Second.mMaximumY);

            if (MaximumX > MinimumX && MaximumY > MinimumY)
            {
                return AnyRect(MinimumX, MinimumY, MaximumX, MaximumY);
            }
            return AnyRect::Zero();
        }

        /// \brief Returns the union (bounding box) of two rectangles.
        ///
        /// \param First  The first rectangle.
        /// \param Second The second rectangle.
        /// \return A rectangle that fully contains both inputs.
        ZYPHRYON_INLINE static constexpr AnyRect Union(AnyRect First, AnyRect Second)
        {
            return AnyRect(
                Math::Min(First.mMinimumX, Second.mMinimumX),
                Math::Min(First.mMinimumY, Second.mMinimumY),
                Math::Max(First.mMaximumX, Second.mMaximumX),
                Math::Max(First.mMaximumY, Second.mMaximumY));
        }

        /// \brief Computes an integer-aligned rectangle that fully encloses the given rectangle.
        ///
        /// \param Rectangle The source rectangle with real-valued coordinates.
        /// \return A rectangle with integer-aligned edges that fully contains the input.
        template<typename Target>
        ZYPHRYON_INLINE static constexpr AnyRect<Target> Enclose(AnyRect Rectangle)
            requires(IsReal<Type> && IsIntegral<Target>)
        {
            const Target MinimumX = static_cast<Target>(Math::Floor(Rectangle.mMinimumX));
            const Target MinimumY = static_cast<Target>(Math::Floor(Rectangle.mMinimumY));
            const Target MaximumX = static_cast<Target>(Math::Ceil(Rectangle.mMaximumX));
            const Target MaximumY = static_cast<Target>(Math::Ceil(Rectangle.mMaximumY));

            return AnyRect<Target>(MinimumX, MinimumY, MaximumX, MaximumY);
        }

        /// \brief Anchors a rectangle relative to a pivot point.
        ///
        /// \param Rectangle The source rectangle.
        /// \param Origin    The pivot point for anchoring.
        /// \return A rectangle anchored according to the pivot.
        ZYPHRYON_INLINE static constexpr AnyRect Anchor(AnyRect Rectangle, Pivot Origin)
            requires (IsReal<Type>)
        {
            const Type OffsetX = Rectangle.GetMinimumX() - Origin.GetX() * Rectangle.GetWidth();
            const Type OffsetY = Rectangle.GetMinimumY() - Origin.GetY() * Rectangle.GetHeight();

            return AnyRect(OffsetX, OffsetY, Rectangle.GetWidth(), Rectangle.GetHeight());
        }

        /// \brief Linearly interpolates between two rectangles.
        ///
        /// \param Start      The starting rectangle.
        /// \param End        The ending rectangle.
        /// \param Percentage The interpolation percentage (range between 0 and 1).
        /// \return A rectangle interpolated between the start and end rectangles.
        ZYPHRYON_INLINE static constexpr AnyRect Lerp(AnyRect Start, AnyRect End, Type Percentage)
            requires (IsReal<Type>)
        {
            LOG_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

            return Start + (End - Start) * Percentage;
        }

        /// \brief Transform a rectangle using a 4x4 transformation matrix.
        ///
        /// \param Rectangle The rectangle to transform.
        /// \param Matrix    The 4x4 transformation matrix to apply.
        /// \return A rectangle resulting from transforming the rectangle with the matrix.
        ZYPHRYON_INLINE static AnyRect Transform(AnyRect Rectangle, ConstRef<Matrix4x4> Matrix)
            requires (IsReal<Type>)
        {
            const Vector4 CornerX(Rectangle.GetMinimumX(), Rectangle.GetMaximumX(),
                                  Rectangle.GetMaximumX(), Rectangle.GetMinimumX());
            const Vector4 CornerY(Rectangle.GetMaximumY(), Rectangle.GetMaximumY(),
                                  Rectangle.GetMinimumY(), Rectangle.GetMinimumY());

            const Vector4 C0 = Matrix.GetColumn(0);
            const Vector4 C1 = Matrix.GetColumn(1);
            const Vector4 C3 = Matrix.GetColumn(3);

            const Vector4 PX = CornerX * Vector4::SplatX(C0) + CornerY * Vector4::SplatX(C1) + Vector4::SplatX(C3);
            const Vector4 PY = CornerX * Vector4::SplatY(C0) + CornerY * Vector4::SplatY(C1) + Vector4::SplatY(C3);

            return AnyRect(Vector4::HorizontalMin(PX), Vector4::HorizontalMin(PY),
                           Vector4::HorizontalMax(PX), Vector4::HorizontalMax(PY));
        }

        /// \brief Transform a rectangle using a 3x2 transformation matrix.
        ///
        /// \param Rectangle The rectangle to transform.
        /// \param Matrix    The 3x2 transformation matrix to apply.
        /// \return A rectangle resulting from transforming the rectangle with the matrix.
        ZYPHRYON_INLINE static AnyRect Transform(AnyRect Rectangle, ConstRef<Matrix3x2> Matrix)
            requires (IsReal<Type>)
        {
            const Type MinX = Rectangle.GetMinimumX();
            const Type MaxX = Rectangle.GetMaximumX();
            const Type MinY = Rectangle.GetMinimumY();
            const Type MaxY = Rectangle.GetMaximumY();

            const Vector2 P0 = Matrix3x2::Project(Matrix, Vector2(MinX, MaxY));
            const Vector2 P1 = Matrix3x2::Project(Matrix, Vector2(MaxX, MaxY));
            const Vector2 P2 = Matrix3x2::Project(Matrix, Vector2(MaxX, MinY));
            const Vector2 P3 = Matrix3x2::Project(Matrix, Vector2(MinX, MinY));

            const Type OutMinX = Math::Min(P0.GetX(), P1.GetX(), P2.GetX(), P3.GetX());
            const Type OutMaxX = Math::Max(P0.GetX(), P1.GetX(), P2.GetX(), P3.GetX());
            const Type OutMinY = Math::Min(P0.GetY(), P1.GetY(), P2.GetY(), P3.GetY());
            const Type OutMaxY = Math::Max(P0.GetY(), P1.GetY(), P2.GetY(), P3.GetY());

            return AnyRect(OutMinX, OutMinY, OutMaxX, OutMaxY);
        }

        /// \brief Iterates over the difference bands between two rectangles, invoking a callback for each band.
        ///
        /// \param First  The first rectangle.
        /// \param Second The second rectangle.
        /// \param Action The callback function to invoke for each difference band.
        template<typename Function>
        ZYPHRYON_INLINE static void ForEachRectDiff(AnyRect First, AnyRect Second, AnyRef<Function> Action)
            requires (IsIntegral<Type>)
        {
            if (First == Second)
            {
                return;
            }

            const AnyRect Intersect = AnyRect::Intersection(First, Second);

            // No intersection, so just return the first rect as-is.
            if (Intersect.IsAlmostZero())
            {
                Action(First);
                return;
            }

            // Top band (above intersection)
            if (Intersect.GetMinimumY() > First.GetMinimumY())
            {
                Action(AnyRect(First.GetMinimumX(), First.GetMinimumY(), First.GetMaximumX(), Intersect.GetMinimumY()));
            }

            // Bottom band (below intersection)
            if (Intersect.GetMaximumY() < First.GetMaximumY())
            {
                Action(AnyRect(First.GetMinimumX(), Intersect.GetMaximumY(), First.GetMaximumX(), First.GetMaximumY()));
            }

            // Left band (left of intersection)
            if (Intersect.GetMinimumX() > First.GetMinimumX())
            {
                Action(AnyRect(First.GetMinimumX(), Intersect.GetMinimumY(), Intersect.GetMinimumX(), Intersect.GetMaximumY()));
            }

            // Right band (right of intersection)
            if (Intersect.GetMaximumX() < First.GetMaximumX())
            {
                Action(AnyRect(Intersect.GetMaximumX(), Intersect.GetMinimumY(), First.GetMaximumX(), Intersect.GetMaximumY()));
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type mMinimumX;
        Type mMinimumY;
        Type mMaximumX;
        Type mMaximumY;
    };

    /// \brief Represents a 2D rectangle defined by (X1, Y1, X2, Y2) floating-point coordinates.
    using Rect     = AnyRect<Real32>;

    /// \brief Represents a 2D rectangle defined by (X1, Y1, X2, Y2) signed integer coordinates.
    using IntRect  = AnyRect<SInt32>;

    /// \brief Represents a 2D rectangle defined by (X1, Y1, X2, Y2) unsigned integer coordinates.
    using UIntRect = AnyRect<UInt32>;
}