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

#include "Zyphryon.Math/Matrix4x4.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a generic 3D axis-aligned bounding box (AABB) defined by a minimum and maximum point.
    template<typename Type>
    class AnyBox final
    {
    public:

        /// \brief Constructor initializing all components to zero.
        ZYPHRYON_INLINE constexpr AnyBox()
            : mMinimum { 0, 0, 0 },
              mMaximum { 0, 0, 0 }
        {
        }

        /// \brief Constructor initializing the box with specified minimum and maximum values.
        ///
        /// \param MinimumX The minimum X coordinate of the box.
        /// \param MinimumY The minimum Y coordinate of the box.
        /// \param MinimumZ The minimum Z coordinate of the box.
        /// \param MaximumX The maximum X coordinate of the box.
        /// \param MaximumY The maximum Y coordinate of the box.
        /// \param MaximumZ The maximum Z coordinate of the box.
        ZYPHRYON_INLINE constexpr AnyBox(Type MinimumX, Type MinimumY, Type MinimumZ,
                                         Type MaximumX, Type MaximumY, Type MaximumZ)
            : mMinimum { MinimumX, MinimumY, MinimumZ },
              mMaximum { MaximumX, MaximumY, MaximumZ }
        {
        }

        /// \brief Constructor initializing the box from minimum and maximum points.
        ///
        /// \param Minimum The minimum point of the box.
        /// \param Maximum The maximum point of the box.
        ZYPHRYON_INLINE constexpr AnyBox(AnyVector3<Type> Minimum, AnyVector3<Type> Maximum)
            : mMinimum { Minimum },
              mMaximum { Maximum }
        {
        }

        /// \brief Conversion constructor from another box with a different base type.
        ///
        /// \param Other The source box to convert from.
        template<typename Base>
        ZYPHRYON_INLINE constexpr explicit AnyBox(AnyBox<Base> Other)
            : mMinimum { static_cast<Type>(Other.GetMinimumX()),
                         static_cast<Type>(Other.GetMinimumY()),
                         static_cast<Type>(Other.GetMinimumZ()) },
              mMaximum { static_cast<Type>(Other.GetMaximumX()),
                         static_cast<Type>(Other.GetMaximumY()),
                         static_cast<Type>(Other.GetMaximumZ()) }
        {
        }

        /// \brief Checks if the box is valid (min <= max on all axes).
        ///
        /// \return `true` if the box is valid, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsValid() const
        {
            return mMinimum.GetX() <= mMaximum.GetX() &&
                   mMinimum.GetY() <= mMaximum.GetY() &&
                   mMinimum.GetZ() <= mMaximum.GetZ();
        }

        /// \brief Checks if the box is approximately zero.
        ///
        /// \return `true` if all coordinates are approximately zero, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsAlmostZero() const
        {
            return mMinimum.IsAlmostZero() && mMaximum.IsAlmostZero();
        }

        /// \brief Sets the coordinates of the box.
        ///
        /// \param MinimumX The minimum X coordinate.
        /// \param MinimumY The minimum Y coordinate.
        /// \param MinimumZ The minimum Z coordinate.
        /// \param MaximumX The maximum X coordinate.
        /// \param MaximumY The maximum Y coordinate.
        /// \param MaximumZ The maximum Z coordinate.
        ZYPHRYON_INLINE constexpr void Set(Type MinimumX, Type MinimumY, Type MinimumZ,
                                           Type MaximumX, Type MaximumY, Type MaximumZ)
        {
            mMinimum = AnyVector3<Type>(MinimumX, MinimumY, MinimumZ);
            mMaximum = AnyVector3<Type>(MaximumX, MaximumY, MaximumZ);
        }

        /// \brief Sets the minimum point of the box.
        ///
        /// \param MinimumX The minimum X coordinate.
        /// \param MinimumY The minimum Y coordinate.
        /// \param MinimumZ The minimum Z coordinate.
        ZYPHRYON_INLINE constexpr void SetMinimum(Type MinimumX, Type MinimumY, Type MinimumZ)
        {
            mMinimum = AnyVector3<Type>(MinimumX, MinimumY, MinimumZ);
        }

        /// \brief Sets the minimum point of the box.
        ///
        /// \param Minimum The minimum point.
        ZYPHRYON_INLINE constexpr void SetMinimum(AnyVector3<Type> Minimum)
        {
            mMinimum = Minimum;
        }

        /// \brief Gets the minimum X coordinate of the box.
        ///
        /// \return The minimum X coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMinimumX() const
        {
            return mMinimum.GetX();
        }

        /// \brief Gets the minimum Y coordinate of the box.
        ///
        /// \return The minimum Y coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMinimumY() const
        {
            return mMinimum.GetY();
        }

        /// \brief Gets the minimum Z coordinate of the box.
        ///
        /// \return The minimum Z coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMinimumZ() const
        {
            return mMinimum.GetZ();
        }

        /// \brief Gets the minimum point of the box.
        ///
        /// \return The minimum point as a 3D vector.
        ZYPHRYON_INLINE constexpr AnyVector3<Type> GetMinimum() const
        {
            return mMinimum;
        }

        /// \brief Sets the maximum point of the box.
        ///
        /// \param MaximumX The maximum X coordinate.
        /// \param MaximumY The maximum Y coordinate.
        /// \param MaximumZ The maximum Z coordinate.
        ZYPHRYON_INLINE constexpr void SetMaximum(Type MaximumX, Type MaximumY, Type MaximumZ)
        {
            mMaximum = AnyVector3<Type>(MaximumX, MaximumY, MaximumZ);
        }

        /// \brief Sets the maximum point of the box.
        ///
        /// \param Maximum The maximum point.
        ZYPHRYON_INLINE constexpr void SetMaximum(AnyVector3<Type> Maximum)
        {
            mMaximum = Maximum;
        }

        /// \brief Gets the maximum X coordinate of the box.
        ///
        /// \return The maximum X coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMaximumX() const
        {
            return mMaximum.GetX();
        }

        /// \brief Gets the maximum Y coordinate of the box.
        ///
        /// \return The maximum Y coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMaximumY() const
        {
            return mMaximum.GetY();
        }

        /// \brief Gets the maximum Z coordinate of the box.
        ///
        /// \return The maximum Z coordinate value.
        ZYPHRYON_INLINE constexpr Type GetMaximumZ() const
        {
            return mMaximum.GetZ();
        }

        /// \brief Gets the maximum point of the box.
        ///
        /// \return The maximum point as a 3D vector.
        ZYPHRYON_INLINE constexpr AnyVector3<Type> GetMaximum() const
        {
            return mMaximum;
        }

        /// \brief Calculates the width (extent along the X axis) of the box.
        ///
        /// \return The width of the box.
        ZYPHRYON_INLINE constexpr Type GetWidth() const
        {
            return mMaximum.GetX() - mMinimum.GetX();
        }

        /// \brief Calculates the height (extent along the Y axis) of the box.
        ///
        /// \return The height of the box.
        ZYPHRYON_INLINE constexpr Type GetHeight() const
        {
            return mMaximum.GetY() - mMinimum.GetY();
        }

        /// \brief Calculates the depth (extent along the Z axis) of the box.
        ///
        /// \return The depth of the box.
        ZYPHRYON_INLINE constexpr Type GetDepth() const
        {
            return mMaximum.GetZ() - mMinimum.GetZ();
        }

        /// \brief Gets the position (minimum corner) of the box.
        ///
        /// \return The position of the box as a 3D vector.
        ZYPHRYON_INLINE constexpr AnyVector3<Type> GetPosition() const
        {
            return mMinimum;
        }

        /// \brief Gets the size (width, height, depth) of the box.
        ///
        /// \return The size of the box as a 3D vector.
        ZYPHRYON_INLINE constexpr AnyVector3<Type> GetSize() const
        {
            return mMaximum - mMinimum;
        }

        /// \brief Calculates the center point of the box.
        ///
        /// \return The center point of the box as a 3D vector.
        ZYPHRYON_INLINE constexpr AnyVector3<Type> GetCenter() const
        {
            if constexpr (IsReal<Type>)
            {
                return mMinimum + GetSize() * Type(0.5);
            }
            else
            {
                return mMinimum + GetSize() / Type(2);
            }
        }

        /// \brief Gets the half-extents (half-size on each axis) of the box.
        ///
        /// \return The half-extents of the box as a 3D vector.
        ZYPHRYON_INLINE constexpr AnyVector3<Type> GetExtents() const
            requires (IsReal<Type>)
        {
            return GetSize() * Type(0.5);
        }

        /// \brief Gets the volume of the box.
        ///
        /// \return The volume of the box.
        ZYPHRYON_INLINE constexpr Type GetVolume() const
        {
            return GetWidth() * GetHeight() * GetDepth();
        }

        /// \brief Gets the total surface area of the box.
        ///
        /// \return The surface area of the box.
        ZYPHRYON_INLINE constexpr Type GetArea() const
        {
            return Type(2) * (GetWidth() * GetHeight() + GetHeight() * GetDepth() + GetDepth() * GetWidth());
        }

        /// \brief Returns the nearest point on or inside the box to a given point.
        ///
        /// \param Point The query point in 3D space.
        /// \return The point clamped to the box's boundaries.
        ZYPHRYON_INLINE constexpr AnyVector3<Type> GetNearest(AnyVector3<Type> Point) const
        {
            return AnyVector3<Type>(
                Math::Clamp(Point.GetX(), mMinimum.GetX(), mMaximum.GetX()),
                Math::Clamp(Point.GetY(), mMinimum.GetY(), mMaximum.GetY()),
                Math::Clamp(Point.GetZ(), mMinimum.GetZ(), mMaximum.GetZ()));
        }

        /// \brief Expands the box by the given amount in all directions.
        ///
        /// \param Amount The amount to expand by.
        /// \return The expanded box.
        ZYPHRYON_INLINE constexpr AnyBox Expand(Type Amount) const
        {
            return AnyBox(mMinimum - AnyVector3<Type>(Amount), mMaximum + AnyVector3<Type>(Amount));
        }

        /// \brief Expands the box by the given amounts on each axis.
        ///
        /// \param AmountX The amount to expand on the X axis.
        /// \param AmountY The amount to expand on the Y axis.
        /// \param AmountZ The amount to expand on the Z axis.
        /// \return The expanded box.
        ZYPHRYON_INLINE constexpr AnyBox Expand(Type AmountX, Type AmountY, Type AmountZ) const
        {
            return AnyBox(
                mMinimum - AnyVector3<Type>(AmountX, AmountY, AmountZ),
                mMaximum + AnyVector3<Type>(AmountX, AmountY, AmountZ));
        }

        /// \brief Contracts the box by the given amount in all directions.
        ///
        /// \param Amount The amount to contract by.
        /// \return The contracted box.
        ZYPHRYON_INLINE constexpr AnyBox Contract(Type Amount) const
        {
            return AnyBox(mMinimum + AnyVector3<Type>(Amount), mMaximum - AnyVector3<Type>(Amount));
        }

        /// \brief Checks if this box completely contains another box.
        ///
        /// \param Other The other box to check.
        /// \return `true` if this box contains the other, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(ConstRef<AnyBox> Other) const
        {
            return mMinimum.GetX() <= Other.mMinimum.GetX() && mMaximum.GetX() >= Other.mMaximum.GetX() &&
                   mMinimum.GetY() <= Other.mMinimum.GetY() && mMaximum.GetY() >= Other.mMaximum.GetY() &&
                   mMinimum.GetZ() <= Other.mMinimum.GetZ() && mMaximum.GetZ() >= Other.mMaximum.GetZ();
        }

        /// \brief Checks if this box contains a point.
        ///
        /// \param X The X coordinate of the point.
        /// \param Y The Y coordinate of the point.
        /// \param Z The Z coordinate of the point.
        /// \return `true` if the point is inside the box, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(Type X, Type Y, Type Z) const
        {
            return X >= mMinimum.GetX() && X < mMaximum.GetX() &&
                   Y >= mMinimum.GetY() && Y < mMaximum.GetY() &&
                   Z >= mMinimum.GetZ() && Z < mMaximum.GetZ();
        }

        /// \brief Checks if this box contains a point.
        ///
        /// \param Point The point to check.
        /// \return `true` if the point is inside the box, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(AnyVector3<Type> Point) const
        {
            return Contains(Point.GetX(), Point.GetY(), Point.GetZ());
        }

        /// \brief Checks if this box intersects with another box.
        ///
        /// \param Other The other box to check.
        /// \return `true` if the two boxes intersect, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Test(AnyBox Other) const
        {
            return mMinimum.GetX() < Other.mMaximum.GetX() && mMaximum.GetX() > Other.mMinimum.GetX() &&
                   mMinimum.GetY() < Other.mMaximum.GetY() && mMaximum.GetY() > Other.mMinimum.GetY() &&
                   mMinimum.GetZ() < Other.mMaximum.GetZ() && mMaximum.GetZ() > Other.mMinimum.GetZ();
        }

        /// \brief Checks if this box is equal to another box.
        ///
        /// \param Other The box to compare to.
        /// \return `true` if all coordinates are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(AnyBox Other) const
        {
            return mMinimum == Other.mMinimum && mMaximum == Other.mMaximum;
        }

        /// \brief Checks if this box is not equal to another box.
        ///
        /// \param Other The box to compare to.
        /// \return `true` if the boxes are not equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(AnyBox Other) const
        {
            return !(* this == Other);
        }

        /// \brief Adds another box to this box (component-wise).
        ///
        /// \param Other The box to add.
        /// \return A new box that is the component-wise sum.
        ZYPHRYON_INLINE constexpr AnyBox operator+(AnyBox Other) const
        {
            return AnyBox(mMinimum + Other.mMinimum, mMaximum + Other.mMaximum);
        }

        /// \brief Adds a scalar to all coordinates of this box.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new box with the scalar added to all coordinates.
        ZYPHRYON_INLINE constexpr AnyBox operator+(Type Scalar) const
        {
            return AnyBox(mMinimum + AnyVector3<Type>(Scalar), mMaximum + AnyVector3<Type>(Scalar));
        }

        /// \brief Translates the box by a 3D vector.
        ///
        /// \param Vector The vector to add.
        /// \return A new box translated by the vector.
        ZYPHRYON_INLINE constexpr AnyBox operator+(AnyVector3<Type> Vector) const
        {
            return AnyBox(mMinimum + Vector, mMaximum + Vector);
        }

        /// \brief Subtracts another box from this box (component-wise).
        ///
        /// \param Other The box to subtract.
        /// \return A new box that is the component-wise difference.
        ZYPHRYON_INLINE constexpr AnyBox operator-(AnyBox Other) const
        {
            return AnyBox(mMinimum - Other.mMinimum, mMaximum - Other.mMaximum);
        }

        /// \brief Subtracts a scalar from all coordinates of this box.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new box with the scalar subtracted from all coordinates.
        ZYPHRYON_INLINE constexpr AnyBox operator-(Type Scalar) const
        {
            return AnyBox(mMinimum - AnyVector3<Type>(Scalar), mMaximum - AnyVector3<Type>(Scalar));
        }

        /// \brief Translates the box by the negation of a 3D vector.
        ///
        /// \param Vector The vector to subtract.
        /// \return A new box translated by the negation of the vector.
        ZYPHRYON_INLINE constexpr AnyBox operator-(AnyVector3<Type> Vector) const
        {
            return AnyBox(mMinimum - Vector, mMaximum - Vector);
        }

        /// \brief Multiplies all coordinates of this box by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new box with all coordinates scaled.
        ZYPHRYON_INLINE constexpr AnyBox operator*(Type Scalar) const
        {
            return AnyBox(mMinimum * AnyVector3<Type>(Scalar), mMaximum * AnyVector3<Type>(Scalar));
        }

        /// \brief Multiplies all coordinates of this box by a 3D vector.
        ///
        /// \param Vector The vector to multiply by.
        /// \return A new box with coordinates scaled per-axis.
        ZYPHRYON_INLINE constexpr AnyBox operator*(AnyVector3<Type> Vector) const
        {
            return AnyBox(mMinimum * Vector, mMaximum * Vector);
        }

        /// \brief Divides all coordinates of this box by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new box with all coordinates divided by the scalar.
        ZYPHRYON_INLINE constexpr AnyBox operator/(Type Scalar) const
        {
            LOG_ASSERT(!Math::IsAlmostZero(Scalar), "Division by zero");

            return AnyBox(mMinimum / AnyVector3<Type>(Scalar), mMaximum / AnyVector3<Type>(Scalar));
        }

        /// \brief Divides all coordinates of this box by a 3D vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A new box with coordinates divided per-axis.
        ZYPHRYON_INLINE constexpr AnyBox operator/(AnyVector3<Type> Vector) const
        {
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetX()), "Division by zero (X)");
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetY()), "Division by zero (Y)");
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetZ()), "Division by zero (Z)");

            return AnyBox(mMinimum / Vector, mMaximum / Vector);
        }

        /// \brief Adds another box to this box in place.
        ///
        /// \param Other The box to add.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator+=(AnyBox Other)
        {
            mMinimum += Other.mMinimum;
            mMaximum += Other.mMaximum;
            return (* this);
        }

        /// \brief Adds a scalar to all coordinates of this box in place.
        ///
        /// \param Scalar The scalar to add.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator+=(Type Scalar)
        {
            mMinimum += AnyVector3<Type>(Scalar);
            mMaximum += AnyVector3<Type>(Scalar);
            return (* this);
        }

        /// \brief Translates the box by a 3D vector in place.
        ///
        /// \param Vector The vector to add.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator+=(AnyVector3<Type> Vector)
        {
            mMinimum += Vector;
            mMaximum += Vector;
            return (* this);
        }

        /// \brief Subtracts another box from this box in place.
        ///
        /// \param Other The box to subtract.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator-=(AnyBox Other)
        {
            mMinimum -= Other.mMinimum;
            mMaximum -= Other.mMaximum;
            return (* this);
        }

        /// \brief Subtracts a scalar from all coordinates of this box in place.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator-=(Type Scalar)
        {
            mMinimum -= AnyVector3<Type>(Scalar);
            mMaximum -= AnyVector3<Type>(Scalar);
            return (* this);
        }

        /// \brief Translates the box by the negation of a 3D vector in place.
        ///
        /// \param Vector The vector to subtract.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator-=(AnyVector3<Type> Vector)
        {
            mMinimum -= Vector;
            mMaximum -= Vector;
            return (* this);
        }

        /// \brief Multiplies all coordinates of this box by a scalar in place.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator*=(Type Scalar)
        {
            mMinimum *= AnyVector3<Type>(Scalar);
            mMaximum *= AnyVector3<Type>(Scalar);
            return (* this);
        }

        /// \brief Multiplies all coordinates of this box by a 3D vector in place.
        ///
        /// \param Vector The vector to multiply by.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator*=(AnyVector3<Type> Vector)
        {
            mMinimum *= Vector;
            mMaximum *= Vector;
            return (* this);
        }

        /// \brief Divides all coordinates of this box by a scalar in place.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator/=(Type Scalar)
        {
            LOG_ASSERT(!Math::IsAlmostZero(Scalar), "Division by zero");

            mMinimum /= AnyVector3<Type>(Scalar);
            mMaximum /= AnyVector3<Type>(Scalar);
            return (* this);
        }

        /// \brief Divides all coordinates of this box by a 3D vector in place.
        ///
        /// \param Vector The vector to divide by.
        /// \return A reference to the updated box.
        ZYPHRYON_INLINE constexpr Ref<AnyBox> operator/=(AnyVector3<Type> Vector)
        {
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetX()), "Division by zero (X)");
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetY()), "Division by zero (Y)");
            LOG_ASSERT(!Math::IsAlmostZero(Vector.GetZ()), "Division by zero (Z)");

            mMinimum /= Vector;
            mMaximum /= Vector;
            return (* this);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mMinimum);
            Archive.SerializeObject(mMaximum);
        }

    public:

        /// \brief Returns an invalid box (min > max).
        ///
        /// \return An invalid box.
        ZYPHRYON_INLINE static constexpr AnyBox Invalid()
        {
            return AnyBox(Type(1), Type(1), Type(1), Type(0), Type(0), Type(0));
        }

        /// \brief Returns a zero box with all coordinates set to zero.
        ///
        /// \return A zero box.
        ZYPHRYON_INLINE static constexpr AnyBox Zero()
        {
            return AnyBox(Type(0), Type(0), Type(0), Type(0), Type(0), Type(0));
        }

        /// \brief Returns a unit box spanning from (0,0,0) to (1,1,1).
        ///
        /// \return A unit box.
        ZYPHRYON_INLINE static constexpr AnyBox One()
        {
            return AnyBox(Type(0), Type(0), Type(0), Type(1), Type(1), Type(1));
        }

        /// \brief Canonicalizes a box by ensuring min <= max on every axis.
        ///
        /// \param Box The box to canonicalize.
        /// \return A canonicalized box.
        ZYPHRYON_INLINE static constexpr AnyBox Canonicalize(AnyBox Box)
        {
            return AnyBox(
                AnyVector3<Type>(
                    Math::Min(Box.mMinimum.GetX(), Box.mMaximum.GetX()),
                    Math::Min(Box.mMinimum.GetY(), Box.mMaximum.GetY()),
                    Math::Min(Box.mMinimum.GetZ(), Box.mMaximum.GetZ())),
                AnyVector3<Type>(
                    Math::Max(Box.mMinimum.GetX(), Box.mMaximum.GetX()),
                    Math::Max(Box.mMinimum.GetY(), Box.mMaximum.GetY()),
                    Math::Max(Box.mMinimum.GetZ(), Box.mMaximum.GetZ())));
        }

        /// \brief Returns the component-wise minimum of two boxes.
        ///
        /// \param First  The first box.
        /// \param Second The second box.
        /// \return A box with the component-wise minimum values.
        ZYPHRYON_INLINE static constexpr AnyBox Min(AnyBox First, AnyBox Second)
        {
            return AnyBox(
                AnyVector3<Type>(
                    Math::Min(First.mMinimum.GetX(), Second.mMinimum.GetX()),
                    Math::Min(First.mMinimum.GetY(), Second.mMinimum.GetY()),
                    Math::Min(First.mMinimum.GetZ(), Second.mMinimum.GetZ())),
                AnyVector3<Type>(
                    Math::Min(First.mMaximum.GetX(), Second.mMaximum.GetX()),
                    Math::Min(First.mMaximum.GetY(), Second.mMaximum.GetY()),
                    Math::Min(First.mMaximum.GetZ(), Second.mMaximum.GetZ())));
        }

        /// \brief Returns the component-wise maximum of two boxes.
        ///
        /// \param First  The first box.
        /// \param Second The second box.
        /// \return A box with the component-wise maximum values.
        ZYPHRYON_INLINE static constexpr AnyBox Max(AnyBox First, AnyBox Second)
        {
            return AnyBox(
                AnyVector3<Type>(
                    Math::Max(First.mMinimum.GetX(), Second.mMinimum.GetX()),
                    Math::Max(First.mMinimum.GetY(), Second.mMinimum.GetY()),
                    Math::Max(First.mMinimum.GetZ(), Second.mMinimum.GetZ())),
                AnyVector3<Type>(
                    Math::Max(First.mMaximum.GetX(), Second.mMaximum.GetX()),
                    Math::Max(First.mMaximum.GetY(), Second.mMaximum.GetY()),
                    Math::Max(First.mMaximum.GetZ(), Second.mMaximum.GetZ())));
        }

        /// \brief Returns a box with all coordinates floored to the nearest integer.
        ///
        /// \param Box The source box with real-valued coordinates.
        /// \return A box with all coordinates rounded down.
        ZYPHRYON_INLINE static constexpr AnyBox Floor(AnyBox Box)
            requires (IsReal<Type>)
        {
            return AnyBox(
                AnyVector3<Type>(
                    Math::Floor(Box.mMinimum.GetX()),
                    Math::Floor(Box.mMinimum.GetY()),
                    Math::Floor(Box.mMinimum.GetZ())),
                AnyVector3<Type>(
                    Math::Floor(Box.mMaximum.GetX()),
                    Math::Floor(Box.mMaximum.GetY()),
                    Math::Floor(Box.mMaximum.GetZ())));
        }

        /// \brief Returns a box with all coordinates ceiled to the nearest integer.
        ///
        /// \param Box The source box with real-valued coordinates.
        /// \return A box with all coordinates rounded up.
        ZYPHRYON_INLINE static constexpr AnyBox Ceil(AnyBox Box)
            requires (IsReal<Type>)
        {
            return AnyBox(
                AnyVector3<Type>(
                    Math::Ceil(Box.mMinimum.GetX()),
                    Math::Ceil(Box.mMinimum.GetY()),
                    Math::Ceil(Box.mMinimum.GetZ())),
                AnyVector3<Type>(
                    Math::Ceil(Box.mMaximum.GetX()),
                    Math::Ceil(Box.mMaximum.GetY()),
                    Math::Ceil(Box.mMaximum.GetZ())));
        }

        /// \brief Returns the intersection of two boxes.
        ///
        /// If the boxes do not overlap, the result may be invalid or zero.
        ///
        /// \param First  The first box.
        /// \param Second The second box.
        /// \return A box representing the overlapping region.
        ZYPHRYON_INLINE static constexpr AnyBox Intersection(AnyBox First, AnyBox Second)
        {
            const Type MinimumX = Math::Max(First.mMinimum.GetX(), Second.mMinimum.GetX());
            const Type MinimumY = Math::Max(First.mMinimum.GetY(), Second.mMinimum.GetY());
            const Type MinimumZ = Math::Max(First.mMinimum.GetZ(), Second.mMinimum.GetZ());
            const Type MaximumX = Math::Min(First.mMaximum.GetX(), Second.mMaximum.GetX());
            const Type MaximumY = Math::Min(First.mMaximum.GetY(), Second.mMaximum.GetY());
            const Type MaximumZ = Math::Min(First.mMaximum.GetZ(), Second.mMaximum.GetZ());

            if (MaximumX > MinimumX && MaximumY > MinimumY && MaximumZ > MinimumZ)
            {
                return AnyBox(MinimumX, MinimumY, MinimumZ, MaximumX, MaximumY, MaximumZ);
            }
            return AnyBox::Zero();
        }

        /// \brief Returns the union (bounding box) of two boxes.
        ///
        /// \param First  The first box.
        /// \param Second The second box.
        /// \return A box that fully contains both inputs.
        ZYPHRYON_INLINE static constexpr AnyBox Union(AnyBox First, AnyBox Second)
        {
            return AnyBox(
                AnyVector3<Type>(
                    Math::Min(First.mMinimum.GetX(), Second.mMinimum.GetX()),
                    Math::Min(First.mMinimum.GetY(), Second.mMinimum.GetY()),
                    Math::Min(First.mMinimum.GetZ(), Second.mMinimum.GetZ())),
                AnyVector3<Type>(
                    Math::Max(First.mMaximum.GetX(), Second.mMaximum.GetX()),
                    Math::Max(First.mMaximum.GetY(), Second.mMaximum.GetY()),
                    Math::Max(First.mMaximum.GetZ(), Second.mMaximum.GetZ())));
        }

        /// \brief Returns the AABB that fully contains the given box after transformation.
        ///
        /// \param Box    The box to transform.
        /// \param Matrix The 4x4 affine transformation matrix to apply.
        /// \return The transformed AABB.
        ZYPHRYON_INLINE static AnyBox Transform(AnyBox Box, ConstRef<Matrix4x4> Matrix)
            requires (IsReal<Type>)
        {
            // Transform the center point.
            const Vector3 Center = Matrix4x4::Project<true>(Matrix, Box.GetCenter());

            // Extract the three rotation/scale columns of the matrix.
            const Vector4 C0 = Matrix.GetColumn(0);
            const Vector4 C1 = Matrix.GetColumn(1);
            const Vector4 C2 = Matrix.GetColumn(2);

            // New half-extents along each axis = |row_i| · old_half_extents
            const AnyVector3<Type> Extents = Box.GetExtents();
            const Type Ex = Extents.GetX();
            const Type Ey = Extents.GetY();
            const Type Ez = Extents.GetZ();

            const Type HalfX = Abs(C0.GetX()) * Ex + Abs(C1.GetX()) * Ey + Abs(C2.GetX()) * Ez;
            const Type HalfY = Abs(C0.GetY()) * Ex + Abs(C1.GetY()) * Ey + Abs(C2.GetY()) * Ez;
            const Type HalfZ = Abs(C0.GetZ()) * Ex + Abs(C1.GetZ()) * Ey + Abs(C2.GetZ()) * Ez;

            return AnyBox(
                AnyVector3<Type>(Center.GetX() - HalfX, Center.GetY() - HalfY, Center.GetZ() - HalfZ),
                AnyVector3<Type>(Center.GetX() + HalfX, Center.GetY() + HalfY, Center.GetZ() + HalfZ));
        }

        /// \brief Linearly interpolates between two boxes.
        ///
        /// \param Start      The starting box.
        /// \param End        The ending box.
        /// \param Percentage The interpolation factor in the range [0, 1].
        /// \return A box interpolated between the start and end boxes.
        ZYPHRYON_INLINE static constexpr AnyBox Lerp(AnyBox Start, AnyBox End, Type Percentage)
            requires (IsReal<Type>)
        {
            LOG_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

            return Start + (End - Start) * Percentage;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        AnyVector3<Type> mMinimum;
        AnyVector3<Type> mMaximum;
    };

    /// \brief Represents a 3D axis-aligned bounding box (AABB) with floating-point coordinates.
    using Box     = AnyBox<Real32>;

    /// \brief Represents a 3D axis-aligned bounding box (AABB) with signed integer coordinates.
    using IntBox  = AnyBox<SInt32>;

    /// \brief Represents a 3D axis-aligned bounding box (AABB) with unsigned integer coordinates.
    using UIntBox = AnyBox<UInt32>;
}

