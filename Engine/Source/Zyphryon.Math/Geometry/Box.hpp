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

#include "Rect.hpp"
#include "Zyphryon.Math/Matrix4x3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    /// \brief Represents a generic 3D axis-aligned bounding box (AABB) defined by a minimum and maximum point.
    template<typename Type>
    class AnyBox final
    {
    public:

        /// \brief Constructs a box with every coordinate at zero.
        ZY_INLINE constexpr AnyBox()
            : mMinimum { 0, 0, 0 },
              mMaximum { 0, 0, 0 }
        {
        }

        /// \brief Constructs a box from its minimum and maximum coordinates.
        ///
        /// \param MinimumX The minimum X coordinate of the box.
        /// \param MinimumY The minimum Y coordinate of the box.
        /// \param MinimumZ The minimum Z coordinate of the box.
        /// \param MaximumX The maximum X coordinate of the box.
        /// \param MaximumY The maximum Y coordinate of the box.
        /// \param MaximumZ The maximum Z coordinate of the box.
        ZY_INLINE constexpr AnyBox(Type MinimumX, Type MinimumY, Type MinimumZ,
                                   Type MaximumX, Type MaximumY, Type MaximumZ)
            : mMinimum { MinimumX, MinimumY, MinimumZ },
              mMaximum { MaximumX, MaximumY, MaximumZ }
        {
        }

        /// \brief Constructs a box from its minimum and maximum points.
        ///
        /// \param Minimum The minimum point of the box.
        /// \param Maximum The maximum point of the box.
        ZY_INLINE constexpr AnyBox(AnyVector3<Type> Minimum, AnyVector3<Type> Maximum)
            : mMinimum { Minimum },
              mMaximum { Maximum }
        {
        }

        /// \brief Constructs a box by converting one with a different coordinate type.
        ///
        /// \param Other The source box to convert from.
        template<typename Base>
        ZY_INLINE constexpr explicit AnyBox(AnyBox<Base> Other)
            : mMinimum { AnyVector3<Type>(Other.GetMinimum()) },
              mMaximum { AnyVector3<Type>(Other.GetMaximum()) }
        {
        }

        /// \brief Checks if the box is valid (min <= max on all axes).
        ///
        /// \return `true` if the minimum is at or below the maximum on every axis, `false` otherwise.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mMinimum.IsComponentWiseLessOrEqual(mMaximum);
        }

        /// \brief Checks if the box is approximately zero.
        ///
        /// \return `true` if all coordinates are approximately zero, `false` otherwise.
        ZY_INLINE constexpr Bool IsAlmostZero() const
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
        ZY_INLINE constexpr void Set(Type MinimumX, Type MinimumY, Type MinimumZ,
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
        ZY_INLINE constexpr void SetMinimum(Type MinimumX, Type MinimumY, Type MinimumZ)
        {
            mMinimum = AnyVector3<Type>(MinimumX, MinimumY, MinimumZ);
        }

        /// \brief Sets the minimum point of the box.
        ///
        /// \param Minimum The corner with the lowest coordinates.
        ZY_INLINE constexpr void SetMinimum(AnyVector3<Type> Minimum)
        {
            mMinimum = Minimum;
        }

        /// \brief Gets the minimum X coordinate of the box.
        ///
        /// \return The minimum X coordinate value.
        ZY_INLINE constexpr Type GetMinimumX() const
        {
            return mMinimum.GetX();
        }

        /// \brief Gets the minimum Y coordinate of the box.
        ///
        /// \return The minimum Y coordinate value.
        ZY_INLINE constexpr Type GetMinimumY() const
        {
            return mMinimum.GetY();
        }

        /// \brief Gets the minimum Z coordinate of the box.
        ///
        /// \return The minimum Z coordinate value.
        ZY_INLINE constexpr Type GetMinimumZ() const
        {
            return mMinimum.GetZ();
        }

        /// \brief Gets the minimum point of the box.
        ///
        /// \return The minimum point as a 3D vector.
        ZY_INLINE constexpr AnyVector3<Type> GetMinimum() const
        {
            return mMinimum;
        }

        /// \brief Sets the maximum point of the box.
        ///
        /// \param MaximumX The maximum X coordinate.
        /// \param MaximumY The maximum Y coordinate.
        /// \param MaximumZ The maximum Z coordinate.
        ZY_INLINE constexpr void SetMaximum(Type MaximumX, Type MaximumY, Type MaximumZ)
        {
            mMaximum = AnyVector3<Type>(MaximumX, MaximumY, MaximumZ);
        }

        /// \brief Sets the maximum point of the box.
        ///
        /// \param Maximum The corner with the highest coordinates.
        ZY_INLINE constexpr void SetMaximum(AnyVector3<Type> Maximum)
        {
            mMaximum = Maximum;
        }

        /// \brief Gets the maximum X coordinate of the box.
        ///
        /// \return The maximum X coordinate value.
        ZY_INLINE constexpr Type GetMaximumX() const
        {
            return mMaximum.GetX();
        }

        /// \brief Gets the maximum Y coordinate of the box.
        ///
        /// \return The maximum Y coordinate value.
        ZY_INLINE constexpr Type GetMaximumY() const
        {
            return mMaximum.GetY();
        }

        /// \brief Gets the maximum Z coordinate of the box.
        ///
        /// \return The maximum Z coordinate value.
        ZY_INLINE constexpr Type GetMaximumZ() const
        {
            return mMaximum.GetZ();
        }

        /// \brief Gets the maximum point of the box.
        ///
        /// \return The maximum point as a 3D vector.
        ZY_INLINE constexpr AnyVector3<Type> GetMaximum() const
        {
            return mMaximum;
        }

        /// \brief Gets the box flattened onto the XY plane.
        ///
        /// \return A new rect spanning the (x, y) extents of the box.
        ZY_INLINE constexpr AnyRect<Type> GetXY() const
        {
            return AnyRect<Type>(mMinimum.GetXY(), mMaximum.GetXY());
        }

        /// \brief Gets the box flattened onto the XZ plane.
        ///
        /// \note Drops the height extent, so a y-up box yields its ground footprint with z on the rect's y axis.
        ///
        /// \return A new rect spanning the (x, z) extents of the box.
        ZY_INLINE constexpr AnyRect<Type> GetXZ() const
        {
            return AnyRect<Type>(mMinimum.GetXZ(), mMaximum.GetXZ());
        }

        /// \brief Calculates the width (extent along the X axis) of the box.
        ///
        /// \return The width of the box.
        ZY_INLINE constexpr Type GetWidth() const
        {
            return mMaximum.GetX() - mMinimum.GetX();
        }

        /// \brief Calculates the height (extent along the Y axis) of the box.
        ///
        /// \return The height of the box.
        ZY_INLINE constexpr Type GetHeight() const
        {
            return mMaximum.GetY() - mMinimum.GetY();
        }

        /// \brief Calculates the depth (extent along the Z axis) of the box.
        ///
        /// \return The depth of the box.
        ZY_INLINE constexpr Type GetDepth() const
        {
            return mMaximum.GetZ() - mMinimum.GetZ();
        }

        /// \brief Gets the position (minimum corner) of the box.
        ///
        /// \return The position of the box as a 3D vector.
        ZY_INLINE constexpr AnyVector3<Type> GetPosition() const
        {
            return mMinimum;
        }

        /// \brief Gets the size (width, height, depth) of the box.
        ///
        /// \return The size of the box as a 3D vector.
        ZY_INLINE constexpr AnyVector3<Type> GetSize() const
        {
            return mMaximum - mMinimum;
        }

        /// \brief Calculates the center point of the box.
        ///
        /// \return The center point of the box as a 3D vector.
        ZY_INLINE constexpr AnyVector3<Type> GetCenter() const
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
        ZY_INLINE constexpr AnyVector3<Type> GetExtents() const
            requires (IsReal<Type>)
        {
            return GetSize() * Type(0.5);
        }

        /// \brief Measures how far the box reaches from its middle along an axis.
        ///
        /// \param Axis The axis to measure along, normalized.
        /// \return The half-width of the box along the axis, which is never negative.
        ZY_INLINE constexpr Type Reach(AnyVector3<Type> Axis) const
            requires (IsReal<Type>)
        {
            return AnyVector3<Type>::Dot(AnyVector3<Type>::Abs(Axis), GetExtents());
        }

        /// \brief Gets the volume of the box.
        ///
        /// \return The volume of the box.
        ZY_INLINE constexpr Type GetVolume() const
        {
            return GetWidth() * GetHeight() * GetDepth();
        }

        /// \brief Gets the total surface area of the box.
        ///
        /// \return The surface area of the box.
        ZY_INLINE constexpr Type GetArea() const
        {
            return Type(2) * (GetWidth() * GetHeight() + GetHeight() * GetDepth() + GetDepth() * GetWidth());
        }

        /// \brief Gets the nearest point on or inside the box to a given point.
        ///
        /// \param Point The query point in 3D space.
        /// \return The point clamped to the box's boundaries.
        ZY_INLINE constexpr AnyVector3<Type> GetNearest(AnyVector3<Type> Point) const
        {
            return AnyVector3<Type>::Clamp(Point, mMinimum, mMaximum);
        }

        /// \brief Expands the box by the given amount in all directions.
        ///
        /// \param Amount The distance to grow by on every side.
        /// \return The box with every face moved out by \p Amount.
        ZY_INLINE constexpr AnyBox Expand(Type Amount) const
        {
            return Expand(Amount, Amount, Amount);
        }

        /// \brief Expands the box by the given amounts on each axis.
        ///
        /// \param AmountX The distance to grow by on each side along X.
        /// \param AmountY The distance to grow by on each side along Y.
        /// \param AmountZ The distance to grow by on each side along Z.
        /// \return The box with its faces moved out by those distances.
        ZY_INLINE constexpr AnyBox Expand(Type AmountX, Type AmountY, Type AmountZ) const
        {
            const AnyVector3<Type> Amount(AmountX, AmountY, AmountZ);

            return AnyBox(mMinimum - Amount, mMaximum + Amount);
        }

        /// \brief Contracts the box by the given amount in all directions.
        ///
        /// \param Amount The distance to shrink by on every side.
        /// \return The box with every face moved in by \p Amount.
        ZY_INLINE constexpr AnyBox Contract(Type Amount) const
        {
            return AnyBox(mMinimum + Amount, mMaximum - Amount);
        }

        /// \brief Checks if this box completely contains another box.
        ///
        /// \param Other The other box to check.
        /// \return `true` if \p Other lies inside this box, even touching its faces, `false` otherwise.
        ZY_INLINE constexpr Bool Contains(ConstRef<AnyBox> Other) const
        {
            return mMinimum.IsComponentWiseLessOrEqual(Other.mMinimum)
                && mMaximum.IsComponentWiseGreaterOrEqual(Other.mMaximum);
        }

        /// \brief Checks if this box contains a point.
        ///
        /// \param X The X coordinate of the point.
        /// \param Y The Y coordinate of the point.
        /// \param Z The Z coordinate of the point.
        /// \return `true` if the point lies at or past the minimum and below the maximum, `false` otherwise.
        ZY_INLINE constexpr Bool Contains(Type X, Type Y, Type Z) const
        {
            return Contains(AnyVector3<Type>(X, Y, Z));
        }

        /// \brief Checks if this box contains a point.
        ///
        /// \param Point The point to check.
        /// \return `true` if the point lies at or past the minimum and below the maximum, `false` otherwise.
        ZY_INLINE constexpr Bool Contains(AnyVector3<Type> Point) const
        {
            return Point.IsComponentWiseGreaterOrEqual(mMinimum) && Point.IsComponentWiseLess(mMaximum);
        }

        /// \brief Checks if this box intersects with another box.
        ///
        /// \param Other The other box to check.
        /// \return `true` if the boxes overlap by more than a touch, `false` otherwise.
        ZY_INLINE constexpr Bool Test(AnyBox Other) const
        {
            return mMinimum.IsComponentWiseLess(Other.mMaximum) && mMaximum.IsComponentWiseGreater(Other.mMinimum);
        }

        /// \brief Checks if this box is equal to another box.
        ///
        /// \param Other The box to compare to.
        /// \return `true` if all coordinates are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(AnyBox Other) const
        {
            return mMinimum == Other.mMinimum && mMaximum == Other.mMaximum;
        }

        /// \brief Checks if this box is not equal to another box.
        ///
        /// \param Other The box to compare to.
        /// \return `true` if the boxes are not equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(AnyBox Other) const
        {
            return !(* this == Other);
        }

        /// \brief Adds another box to this box (component-wise).
        ///
        /// \param Other The box to add.
        /// \return A new box that is the component-wise sum.
        ZY_INLINE constexpr AnyBox operator+(AnyBox Other) const
        {
            return AnyBox(mMinimum + Other.mMinimum, mMaximum + Other.mMaximum);
        }

        /// \brief Adds a scalar to all coordinates of this box.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new box with the scalar added to all coordinates.
        ZY_INLINE constexpr AnyBox operator+(Type Scalar) const
        {
            return AnyBox(mMinimum + Scalar, mMaximum + Scalar);
        }

        /// \brief Translates the box by a 3D vector.
        ///
        /// \param Vector The vector to add.
        /// \return A new box translated by the vector.
        ZY_INLINE constexpr AnyBox operator+(AnyVector3<Type> Vector) const
        {
            return AnyBox(mMinimum + Vector, mMaximum + Vector);
        }

        /// \brief Subtracts another box from this box (component-wise).
        ///
        /// \param Other The box to subtract.
        /// \return A new box that is the component-wise difference.
        ZY_INLINE constexpr AnyBox operator-(AnyBox Other) const
        {
            return AnyBox(mMinimum - Other.mMinimum, mMaximum - Other.mMaximum);
        }

        /// \brief Subtracts a scalar from all coordinates of this box.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new box with the scalar subtracted from all coordinates.
        ZY_INLINE constexpr AnyBox operator-(Type Scalar) const
        {
            return AnyBox(mMinimum - Scalar, mMaximum - Scalar);
        }

        /// \brief Translates the box by the negation of a 3D vector.
        ///
        /// \param Vector The vector to subtract.
        /// \return A new box translated by the negation of the vector.
        ZY_INLINE constexpr AnyBox operator-(AnyVector3<Type> Vector) const
        {
            return AnyBox(mMinimum - Vector, mMaximum - Vector);
        }

        /// \brief Multiplies all coordinates of this box by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new box with all coordinates scaled.
        ZY_INLINE constexpr AnyBox operator*(Type Scalar) const
        {
            return AnyBox(mMinimum * Scalar, mMaximum * Scalar);
        }

        /// \brief Multiplies all coordinates of this box by a 3D vector.
        ///
        /// \param Vector The vector to multiply by.
        /// \return A new box with coordinates scaled per-axis.
        ZY_INLINE constexpr AnyBox operator*(AnyVector3<Type> Vector) const
        {
            return AnyBox(mMinimum * Vector, mMaximum * Vector);
        }

        /// \brief Divides all coordinates of this box by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new box with all coordinates divided by the scalar.
        ZY_INLINE constexpr AnyBox operator/(Type Scalar) const
        {
            return AnyBox(mMinimum / Scalar, mMaximum / Scalar);
        }

        /// \brief Divides all coordinates of this box by a 3D vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A new box with coordinates divided per-axis.
        ZY_INLINE constexpr AnyBox operator/(AnyVector3<Type> Vector) const
        {
            return AnyBox(mMinimum / Vector, mMaximum / Vector);
        }

        /// \brief Adds another box to this box in place.
        ///
        /// \param Other The box to add.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator+=(AnyBox Other)
        {
            return (* this) = (* this) + Other;
        }

        /// \brief Adds a scalar to all coordinates of this box in place.
        ///
        /// \param Scalar The scalar to add.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator+=(Type Scalar)
        {
            return (* this) = (* this) + Scalar;
        }

        /// \brief Translates the box by a 3D vector in place.
        ///
        /// \param Vector The vector to add.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator+=(AnyVector3<Type> Vector)
        {
            return (* this) = (* this) + Vector;
        }

        /// \brief Subtracts another box from this box in place.
        ///
        /// \param Other The box to subtract.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator-=(AnyBox Other)
        {
            return (* this) = (* this) - Other;
        }

        /// \brief Subtracts a scalar from all coordinates of this box in place.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator-=(Type Scalar)
        {
            return (* this) = (* this) - Scalar;
        }

        /// \brief Translates the box by the negation of a 3D vector in place.
        ///
        /// \param Vector The vector to subtract.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator-=(AnyVector3<Type> Vector)
        {
            return (* this) = (* this) - Vector;
        }

        /// \brief Multiplies all coordinates of this box by a scalar in place.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator*=(Type Scalar)
        {
            return (* this) = (* this) * Scalar;
        }

        /// \brief Multiplies all coordinates of this box by a 3D vector in place.
        ///
        /// \param Vector The vector to multiply by.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator*=(AnyVector3<Type> Vector)
        {
            return (* this) = (* this) * Vector;
        }

        /// \brief Divides all coordinates of this box by a scalar in place.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator/=(Type Scalar)
        {
            return (* this) = (* this) / Scalar;
        }

        /// \brief Divides all coordinates of this box by a 3D vector in place.
        ///
        /// \param Vector The vector to divide by.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator/=(AnyVector3<Type> Vector)
        {
            return (* this) = (* this) / Vector;
        }

        /// \brief Shifts left all coordinates of the box by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator<<=(Type Scalar)
            requires(IsIntegral<Type>)
        {
            return (* this) = AnyBox(mMinimum << Scalar, mMaximum << Scalar);
        }

        /// \brief Shifts right all coordinates of the box by a scalar value.
        ///
        /// \param Scalar The scalar to shift by.
        /// \return A reference to the updated box.
        ZY_INLINE constexpr Ref<AnyBox> operator>>=(Type Scalar)
            requires(IsIntegral<Type>)
        {
            return (* this) = AnyBox(mMinimum >> Scalar, mMaximum >> Scalar);
        }

    public:

        /// \brief Gets a box that fails every validity check and is the identity of \ref Union.
        ///
        /// \return A box whose minimum sits above its maximum on every axis.
        ZY_INLINE static constexpr AnyBox Invalid()
        {
            return AnyBox(kMaximum<Type>, kMaximum<Type>, kMaximum<Type>, Lowest(), Lowest(), Lowest());
        }

        /// \brief Gets a zero box with all coordinates set to zero.
        ///
        /// \return A zero box.
        ZY_INLINE static constexpr AnyBox Zero()
        {
            return AnyBox(Type(0), Type(0), Type(0), Type(0), Type(0), Type(0));
        }

        /// \brief Gets a unit box spanning from (0,0,0) to (1,1,1).
        ///
        /// \return A unit box.
        ZY_INLINE static constexpr AnyBox One()
        {
            return AnyBox(Type(0), Type(0), Type(0), Type(1), Type(1), Type(1));
        }

        /// \brief Creates a box by extruding a rect that lies on the XY plane along the depth axis.
        ///
        /// \param XY       The rect supplying the x and y extents.
        /// \param MinimumZ The minimum depth of the box.
        /// \param MaximumZ The maximum depth of the box.
        /// \return A new box spanning the rect over the given depth range.
        ZY_INLINE static constexpr AnyBox FromXY(AnyRect<Type> XY, Type MinimumZ = Type(0), Type MaximumZ = Type(0))
        {
            return AnyBox(XY.GetMinimumX(), XY.GetMinimumY(), MinimumZ, XY.GetMaximumX(), XY.GetMaximumY(), MaximumZ);
        }

        /// \brief Creates a box by extruding a rect that lies on the XZ plane along the height axis.
        ///
        /// \param XZ       The rect supplying the x and z extents, with z on its y axis.
        /// \param MinimumY The minimum height of the box.
        /// \param MaximumY The maximum height of the box.
        /// \return A new box spanning the rect over the given height range.
        ZY_INLINE static constexpr AnyBox FromXZ(AnyRect<Type> XZ, Type MinimumY = Type(0), Type MaximumY = Type(0))
        {
            return AnyBox(XZ.GetMinimumX(), MinimumY, XZ.GetMinimumY(), XZ.GetMaximumX(), MaximumY, XZ.GetMaximumY());
        }

        /// \brief Canonicalizes a box by ensuring min <= max on every axis.
        ///
        /// \param Source The box whose minimum and maximum may be swapped on some axis.
        /// \return The box with each axis's minimum and maximum in order.
        ZY_INLINE static constexpr AnyBox Canonicalize(AnyBox Source)
        {
            return AnyBox(AnyVector3<Type>::Min(Source.mMinimum, Source.mMaximum),
                          AnyVector3<Type>::Max(Source.mMinimum, Source.mMaximum));
        }

        /// \brief Gets the component-wise minimum of two boxes.
        ///
        /// \param First  The first box.
        /// \param Second The second box.
        /// \return A box with the component-wise minimum values.
        ZY_INLINE static constexpr AnyBox Min(AnyBox First, AnyBox Second)
        {
            return AnyBox(AnyVector3<Type>::Min(First.mMinimum, Second.mMinimum),
                          AnyVector3<Type>::Min(First.mMaximum, Second.mMaximum));
        }

        /// \brief Gets the component-wise maximum of two boxes.
        ///
        /// \param First  The first box.
        /// \param Second The second box.
        /// \return A box with the component-wise maximum values.
        ZY_INLINE static constexpr AnyBox Max(AnyBox First, AnyBox Second)
        {
            return AnyBox(AnyVector3<Type>::Max(First.mMinimum, Second.mMinimum),
                          AnyVector3<Type>::Max(First.mMaximum, Second.mMaximum));
        }

        /// \brief Gets a box with all coordinates floored to the nearest integer.
        ///
        /// \param Source The source box with real-valued coordinates.
        /// \return A box with all coordinates rounded down.
        ZY_INLINE static constexpr AnyBox Floor(AnyBox Source)
            requires (IsReal<Type>)
        {
            return AnyBox(AnyVector3<Type>::Floor(Source.mMinimum), AnyVector3<Type>::Floor(Source.mMaximum));
        }

        /// \brief Gets a box with all coordinates ceiled to the nearest integer.
        ///
        /// \param Source The source box with real-valued coordinates.
        /// \return A box with all coordinates rounded up.
        ZY_INLINE static constexpr AnyBox Ceil(AnyBox Source)
            requires (IsReal<Type>)
        {
            return AnyBox(AnyVector3<Type>::Ceil(Source.mMinimum), AnyVector3<Type>::Ceil(Source.mMaximum));
        }

        /// \brief Checks whether two boxes overlap, treating contact as an overlap.
        ///
        /// \param First  The first box.
        /// \param Second The second box.
        /// \return `true` if the boxes overlap or touch, `false` otherwise.
        ZY_INLINE static constexpr Bool Overlaps(AnyBox First, AnyBox Second)
        {
            return First.mMinimum.IsComponentWiseLessOrEqual(Second.mMaximum)
                && First.mMaximum.IsComponentWiseGreaterOrEqual(Second.mMinimum);
        }

        /// \brief Gets the intersection of two boxes.
        ///
        /// \param First  The first box.
        /// \param Second The second box.
        /// \return A box representing the overlapping region. If they don't overlap, the box may be invalid.
        ZY_INLINE static constexpr AnyBox Intersection(AnyBox First, AnyBox Second)
        {
            return AnyBox(AnyVector3<Type>::Max(First.mMinimum, Second.mMinimum),
                          AnyVector3<Type>::Min(First.mMaximum, Second.mMaximum));
        }

        /// \brief Gets the union (bounding box) of two boxes.
        ///
        /// \param First  The first box.
        /// \param Second The second box.
        /// \return A box that fully contains both inputs.
        ZY_INLINE static constexpr AnyBox Union(AnyBox First, AnyBox Second)
        {
            return AnyBox(AnyVector3<Type>::Min(First.mMinimum, Second.mMinimum),
                          AnyVector3<Type>::Max(First.mMaximum, Second.mMaximum));
        }

        /// \brief Computes an integer-aligned box that fully encloses the given box.
        ///
        /// \param Source The source box with real-valued coordinates.
        /// \return A box with integer-aligned bounds that fully contains the input.
        template<typename Target>
        ZY_INLINE static constexpr AnyBox<Target> Enclose(AnyBox Source)
            requires(IsReal<Type> && IsIntegral<Target>)
        {
            return AnyBox<Target>(AnyVector3<Target>(AnyVector3<Type>::Floor(Source.mMinimum)),
                                  AnyVector3<Target>(AnyVector3<Type>::Ceil(Source.mMaximum)));
        }

        /// \brief Gets the cells of a grid a box reaches into, both boxes inclusive of their maximum.
        ///
        /// \param Source The box, whose maximum is the last coordinate inside it.
        /// \param Cell   The size of one cell along all three axes.
        /// \return The range of cells, whose maximum is the last cell inside it.
        template<typename Target>
        ZY_INLINE static constexpr AnyBox<Target> Quantize(AnyBox Source, Type Cell)
            requires(IsIntegral<Type> && IsIntegral<Target>)
        {
            return AnyBox<Target>(
                AnyVector3<Target>(
                    AnyRect<Type>::template Divide<Target>(Source.mMinimum.GetX(), Cell),
                    AnyRect<Type>::template Divide<Target>(Source.mMinimum.GetY(), Cell),
                    AnyRect<Type>::template Divide<Target>(Source.mMinimum.GetZ(), Cell)),
                AnyVector3<Target>(
                    AnyRect<Type>::template Divide<Target>(Source.mMaximum.GetX(), Cell),
                    AnyRect<Type>::template Divide<Target>(Source.mMaximum.GetY(), Cell),
                    AnyRect<Type>::template Divide<Target>(Source.mMaximum.GetZ(), Cell)));
        }

        /// \brief Gets the AABB that fully contains the given box after transformation.
        ///
        /// \param Source The box to transform.
        /// \param Matrix The affine transformation matrix to apply.
        /// \return The smallest box holding the eight transformed corners.
        ZY_INLINE static AnyBox Transform(AnyBox Source, ConstRef<Matrix4x3> Matrix)
            requires (IsReal<Type>)
        {
            const Vector3 Center = Matrix4x3::Project(Matrix, Source.GetCenter());

            // A Matrix4x3 holds rows, so each output axis reads one row.
            const Vector3 RowX = Matrix.GetColumn(0).GetXYZ();
            const Vector3 RowY = Matrix.GetColumn(1).GetXYZ();
            const Vector3 RowZ = Matrix.GetColumn(2).GetXYZ();

            const AnyVector3<Type> Extents = Source.GetExtents();
            const Type ExtentX = Extents.GetX();
            const Type ExtentY = Extents.GetY();
            const Type ExtentZ = Extents.GetZ();

            const Type HalfX = Abs(RowX.GetX()) * ExtentX + Abs(RowX.GetY()) * ExtentY + Abs(RowX.GetZ()) * ExtentZ;
            const Type HalfY = Abs(RowY.GetX()) * ExtentX + Abs(RowY.GetY()) * ExtentY + Abs(RowY.GetZ()) * ExtentZ;
            const Type HalfZ = Abs(RowZ.GetX()) * ExtentX + Abs(RowZ.GetY()) * ExtentY + Abs(RowZ.GetZ()) * ExtentZ;

            return AnyBox(
                AnyVector3<Type>(Center.GetX() - HalfX, Center.GetY() - HalfY, Center.GetZ() - HalfZ),
                AnyVector3<Type>(Center.GetX() + HalfX, Center.GetY() + HalfY, Center.GetZ() + HalfZ));
        }

        /// \brief Provides the name this type is registered under in the reflection system.
        ///
        /// \return The fully qualified reflection name of the type, and how it is shown.
        ZY_INLINE static constexpr auto OnClassify()
        {
            ZyReflection::Presentation Presentation;

            if      constexpr (IsAnyOf<Type, Real32>)
            {
                Presentation.Name = "Math.Box";
            }
            else if constexpr (IsAnyOf<Type, SInt32>)
            {
                Presentation.Name = "Math.IntBox";
            }
            else if constexpr (IsAnyOf<Type, UInt32>)
            {
                Presentation.Name = "Math.UIntBox";
            }
            else
            {
                static_assert(false, "The box has no name for the type it holds");
            }
            return Presentation;
        }

        /// \brief Provides the reflected members of this type.
        ///
        /// \return The set of reflected fields.
        ZY_INLINE static constexpr auto OnDescribe()
        {
            return Array(
                ZyReflection::Field::Property<&AnyBox::mMinimum>("Minimum"),
                ZyReflection::Field::Property<&AnyBox::mMaximum>("Maximum"));
        }

    private:

        /// \brief Gets the lowest value the coordinate type can hold.
        ///
        /// \return The most negative finite value for reals, the minimum for integers.
        ZY_INLINE static constexpr Type Lowest()
        {
            if constexpr (IsReal<Type>)
            {
                return -kMaximum<Type>;
            }
            else
            {
                return kMinimum<Type>;
            }
        }

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