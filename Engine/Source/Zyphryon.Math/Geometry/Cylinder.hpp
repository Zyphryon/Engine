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

#include "Box.hpp"
#include "Circle.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents an upright 3D cylinder defined by a center point, a radius and a half height.
    class Cylinder final
    {
    public:

        /// \brief Initializes the cylinder to center (0.0f, 0.0f, 0.0f) with radius and half height 0.0f.
        ZY_INLINE constexpr Cylinder()
            : mCenter { 0.0f, 0.0f, 0.0f },
              mRadius { 0.0f },
              mExtent { 0.0f }
        {
        }

        /// \brief Initializes the cylinder with the given center, radius and half height.
        ///
        /// \param Center The center point.
        /// \param Radius The radius.
        /// \param Extent The half height.
        ZY_INLINE constexpr Cylinder(Vector3 Center, Real32 Radius, Real32 Extent)
            : mCenter { Center },
              mRadius { Radius },
              mExtent { Extent }
        {
        }

        /// \brief Initializes the cylinder with the given components.
        ///
        /// \param X      The center's x-coordinate.
        /// \param Y      The center's y-coordinate.
        /// \param Z      The center's z-coordinate.
        /// \param Radius The radius.
        /// \param Extent The half height.
        ZY_INLINE constexpr Cylinder(Real32 X, Real32 Y, Real32 Z, Real32 Radius, Real32 Extent)
            : mCenter { X, Y, Z },
              mRadius { Radius },
              mExtent { Extent }
        {
        }

        /// \brief Checks if the cylinder is approximately zero.
        ///
        /// \return `true` if center, radius and half height are approximately zero, `false` otherwise.
        ZY_INLINE constexpr Bool IsAlmostZero() const
        {
            return mCenter.IsAlmostZero() && ::IsAlmostZero(mRadius) && ::IsAlmostZero(mExtent);
        }

        /// \brief Checks if the cylinder is valid.
        ///
        /// \return `true` if the radius and half height are non-negative, `false` otherwise.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mRadius >= 0.0f && mExtent >= 0.0f;
        }

        /// \brief Checks if the cylinder encloses nothing.
        ///
        /// \return `true` if the radius is approximately zero, `false` otherwise.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return ::IsAlmostZero(mRadius);
        }

        /// \brief Sets the cylinder's center, radius and half height.
        ///
        /// \param Center The center point.
        /// \param Radius The radius.
        /// \param Extent The half height.
        ZY_INLINE constexpr void Set(Vector3 Center, Real32 Radius, Real32 Extent)
        {
            mCenter = Center;
            mRadius = Radius;
            mExtent = Extent;
        }

        /// \brief Sets the cylinder's center.
        ///
        /// \param Center The center point.
        ZY_INLINE constexpr void SetCenter(Vector3 Center)
        {
            mCenter = Center;
        }

        /// \brief Gets the cylinder's center.
        ///
        /// \return The center point.
        ZY_INLINE constexpr Vector3 GetCenter() const
        {
            return mCenter;
        }

        /// \brief Sets the cylinder's radius.
        ///
        /// \param Radius The radius.
        ZY_INLINE constexpr void SetRadius(Real32 Radius)
        {
            ZY_ASSERT(Radius >= 0.0f, "Cylinder radius cannot be negative");

            mRadius = Radius;
        }

        /// \brief Gets the cylinder's radius.
        ///
        /// \return The radius.
        ZY_INLINE constexpr Real32 GetRadius() const
        {
            return mRadius;
        }

        /// \brief Sets the cylinder's half height.
        ///
        /// \param Extent The half height.
        ZY_INLINE constexpr void SetExtent(Real32 Extent)
        {
            ZY_ASSERT(Extent >= 0.0f, "Cylinder half height cannot be negative");

            mExtent = Extent;
        }

        /// \brief Gets the cylinder's half height.
        ///
        /// \return The half height.
        ZY_INLINE constexpr Real32 GetExtent() const
        {
            return mExtent;
        }

        /// \brief Gets the elevation the cylinder starts at.
        ///
        /// \return The elevation of the bottom cap.
        ZY_INLINE constexpr Real32 GetBottom() const
        {
            return mCenter.GetY() - mExtent;
        }

        /// \brief Gets the elevation the cylinder reaches.
        ///
        /// \return The elevation of the top cap.
        ZY_INLINE constexpr Real32 GetTop() const
        {
            return mCenter.GetY() + mExtent;
        }

        /// \brief Gets the cylinder's height.
        ///
        /// \return The height.
        ZY_INLINE constexpr Real32 GetHeight() const
        {
            return mExtent * 2.0f;
        }

        /// \brief Gets the cylinder's diameter.
        ///
        /// \return The diameter.
        ZY_INLINE constexpr Real32 GetDiameter() const
        {
            return mRadius * 2.0f;
        }

        /// \brief Gets the cylinder flattened onto the XZ plane.
        ///
        /// \return A new circle holding the (x, z) center and the cylinder's radius.
        ZY_INLINE constexpr Circle GetXZ() const
        {
            return Circle(mCenter.GetXZ(), mRadius);
        }

        /// \brief Gets the cylinder's volume.
        ///
        /// \return The volume.
        ZY_INLINE constexpr Real32 GetVolume() const
        {
            return kPI<Real32> * mRadius * mRadius * GetHeight();
        }

        /// \brief Gets the smallest axis-aligned volume that contains the cylinder.
        ///
        /// \return The bounding volume.
        ZY_INLINE constexpr Box GetBoundaries() const
        {
            return Box(
                Vector3(mCenter.GetX() - mRadius, GetBottom(), mCenter.GetZ() - mRadius),
                Vector3(mCenter.GetX() + mRadius, GetTop(),    mCenter.GetZ() + mRadius));
        }

        /// \brief Grows the cylinder by an amount, on every side alike.
        ///
        /// \param Amount The amount to grow by.
        /// \return The grown cylinder.
        ZY_INLINE constexpr Cylinder Expand(Real32 Amount) const
        {
            return Cylinder(mCenter, mRadius + Amount, mExtent + Amount);
        }

        /// \brief Shrinks the cylinder by an amount, never past nothing.
        ///
        /// \param Amount The amount to shrink by.
        /// \return The shrunken cylinder.
        ZY_INLINE constexpr Cylinder Contract(Real32 Amount) const
        {
            return Cylinder(mCenter, Max(0.0f, mRadius - Amount), Max(0.0f, mExtent - Amount));
        }

        /// \brief Checks if this cylinder contains a point.
        ///
        /// \param Point The point to check.
        /// \return `true` if the point is inside the cylinder, `false` otherwise.
        ZY_INLINE constexpr Bool Contains(Vector3 Point) const
        {
            if (Point.GetY() < GetBottom() || Point.GetY() > GetTop())
            {
                return false;
            }
            return mCenter.GetXZ().GetDistanceSquared(Point.GetXZ()) <= (mRadius * mRadius);
        }

        /// \brief Checks if this cylinder intersects with another cylinder.
        ///
        /// \param Other The other cylinder to check.
        /// \return `true` if the cylinders intersect, `false` otherwise.
        ZY_INLINE constexpr Bool Test(Cylinder Other) const
        {
            if (GetBottom() > Other.GetTop() || GetTop() < Other.GetBottom())
            {
                return false;
            }

            const Real32 Reach = mRadius + Other.mRadius;
            return mCenter.GetXZ().GetDistanceSquared(Other.mCenter.GetXZ()) <= (Reach * Reach);
        }

        /// \brief Checks if this cylinder is equal to another cylinder.
        ///
        /// \param Other The cylinder to compare to.
        /// \return `true` if center, radius and half height are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(Cylinder Other) const
        {
            return mCenter == Other.mCenter
                && IsAlmostEqual(mRadius, Other.mRadius)
                && IsAlmostEqual(mExtent, Other.mExtent);
        }

        /// \brief Checks if this cylinder is not equal to another cylinder.
        ///
        /// \param Other The cylinder to compare to.
        /// \return `true` if the cylinders are not equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(Cylinder Other) const
        {
            return !(* this == Other);
        }

    public:

        /// \brief Gets a cylinder that contains nothing and is grown into.
        ///
        /// \return An invalid cylinder.
        ZY_INLINE static constexpr Cylinder Invalid()
        {
            return Cylinder(Vector3(0.0f, 0.0f, 0.0f), -1.0f, -1.0f);
        }

        /// \brief Gets the unit cylinder centered at origin with radius and half height 1.0.
        ///
        /// \return A unit cylinder.
        ZY_INLINE static constexpr Cylinder Unit()
        {
            return Cylinder(Vector3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f);
        }

        /// \brief Creates a cylinder standing on a point rather than centered on one.
        ///
        /// \param Base   The point the cylinder stands on.
        /// \param Radius The radius.
        /// \param Height The height it rises to.
        /// \return A new cylinder resting on the given point.
        ZY_INLINE static constexpr Cylinder FromBase(Vector3 Base, Real32 Radius, Real32 Height)
        {
            const Real32 Extent = Height * 0.5f;
            return Cylinder(Vector3(Base.GetX(), Base.GetY() + Extent, Base.GetZ()), Radius, Extent);
        }

        /// \brief Creates a cylinder from a circle that lies on the XZ plane.
        ///
        /// \param XZ     The circle supplying the (x, z) center and the radius.
        /// \param Y      The height to place the center at.
        /// \param Extent The half height.
        /// \return A new cylinder centered on the circle at the given height.
        ZY_INLINE static constexpr Cylinder FromXZ(Circle XZ, Real32 Y = 0.0f, Real32 Extent = 0.0f)
        {
            return Cylinder(Vector3::FromXZ(XZ.GetCenter(), Y), XZ.GetRadius(), Extent);
        }

        /// \brief Canonicalizes a cylinder by ensuring its radius and half height are non-negative.
        ///
        /// \param Source The cylinder to canonicalize.
        /// \return A cylinder with non-negative radius and half height.
        ZY_INLINE static constexpr Cylinder Canonicalize(Cylinder Source)
        {
            return Cylinder(Source.mCenter, Abs(Source.mRadius), Abs(Source.mExtent));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector3 mCenter;
        Real32  mRadius;
        Real32  mExtent;
    };
}