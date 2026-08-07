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
    /// \brief Represents a 3D sphere defined by a center point and radius.
    class Sphere final
    {
    public:

        /// \brief Initializes the sphere to center (0.0f, 0.0f, 0.0f) with radius 0.0f.
        ZY_INLINE constexpr Sphere()
            : mCenter { 0.0f, 0.0f, 0.0f },
              mRadius { 0.0f }
        {
        }

        /// \brief Initializes the sphere with the given center and radius.
        ///
        /// \param Center The center point.
        /// \param Radius The radius.
        ZY_INLINE constexpr Sphere(Vector3 Center, Real32 Radius)
            : mCenter { Center },
              mRadius { Radius }
        {
        }

        /// \brief Initializes the sphere with the given components.
        ///
        /// \param X      The center's x-coordinate.
        /// \param Y      The center's y-coordinate.
        /// \param Z      The center's z-coordinate.
        /// \param Radius The radius.
        ZY_INLINE constexpr Sphere(Real32 X, Real32 Y, Real32 Z, Real32 Radius)
            : mCenter { X, Y, Z },
              mRadius { Radius }
        {
        }

        /// \brief Checks if the sphere is approximately zero.
        ///
        /// \return `true` if center and radius are approximately zero, `false` otherwise.
        ZY_INLINE constexpr Bool IsAlmostZero() const
        {
            return mCenter.IsAlmostZero() && ::IsAlmostZero(mRadius);
        }

        /// \brief Checks if the sphere is valid.
        ///
        /// \return `true` if the radius is non-negative, `false` otherwise.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mRadius >= 0.0f;
        }

        /// \brief Checks if the sphere encloses nothing.
        ///
        /// \return `true` if the radius is approximately zero, `false` otherwise.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return ::IsAlmostZero(mRadius);
        }

        /// \brief Sets the sphere's center and radius.
        ///
        /// \param Center The center point.
        /// \param Radius The radius.
        ZY_INLINE constexpr void Set(Vector3 Center, Real32 Radius)
        {
            mCenter = Center;
            mRadius = Radius;
        }

        /// \brief Sets the sphere's center.
        ///
        /// \param Center The center point.
        ZY_INLINE constexpr void SetCenter(Vector3 Center)
        {
            mCenter = Center;
        }

        /// \brief Gets the sphere's center.
        ///
        /// \return The center point.
        ZY_INLINE constexpr Vector3 GetCenter() const
        {
            return mCenter;
        }

        /// \brief Sets the sphere's radius.
        ///
        /// \param Radius The radius.
        ZY_INLINE constexpr void SetRadius(Real32 Radius)
        {
            ZY_ASSERT(Radius >= 0.0f, "Sphere radius cannot be negative");

            mRadius = Radius;
        }

        /// \brief Gets the sphere's radius.
        ///
        /// \return The radius.
        ZY_INLINE constexpr Real32 GetRadius() const
        {
            return mRadius;
        }

        /// \brief Gets the sphere flattened onto the XY plane.
        ///
        /// \return A new circle holding the (x, y) center and the sphere's radius.
        ZY_INLINE constexpr Circle GetXY() const
        {
            return Circle(mCenter.GetXY(), mRadius);
        }

        /// \brief Gets the sphere flattened onto the XZ plane.
        ///
        /// \return A new circle holding the (x, z) center and the sphere's radius.
        ZY_INLINE constexpr Circle GetXZ() const
        {
            return Circle(mCenter.GetXZ(), mRadius);
        }

        /// \brief Gets the sphere's diameter.
        ///
        /// \return The diameter.
        ZY_INLINE constexpr Real32 GetDiameter() const
        {
            return mRadius * 2.0f;
        }

        /// \brief Gets the sphere's surface area.
        ///
        /// \return The surface area.
        ZY_INLINE constexpr Real32 GetArea() const
        {
            return 4.0f * kPI<Real32> * mRadius * mRadius;
        }

        /// \brief Gets the sphere's volume.
        ///
        /// \return The volume.
        ZY_INLINE constexpr Real32 GetVolume() const
        {
            return (4.0f / 3.0f) * kPI<Real32> * mRadius * mRadius * mRadius;
        }

        /// \brief Gets the shortest distance from the sphere's surface to a point.
        ///
        /// \param Point The point to measure to.
        /// \return The distance, negative when the point lies inside.
        ZY_INLINE Real32 GetDistance(Vector3 Point) const
        {
            return mCenter.GetDistance(Point) - mRadius;
        }

        /// \brief Gets the shortest distance between two spheres' surfaces.
        ///
        /// \param Other The other sphere.
        /// \return The distance, negative when the two overlap.
        ZY_INLINE Real32 GetDistance(Sphere Other) const
        {
            return mCenter.GetDistance(Other.mCenter) - (mRadius + Other.mRadius);
        }

        /// \brief Grows the sphere by an amount.
        ///
        /// \param Amount The amount to grow by.
        /// \return The grown sphere.
        ZY_INLINE constexpr Sphere Expand(Real32 Amount) const
        {
            return Sphere(mCenter, mRadius + Amount);
        }

        /// \brief Shrinks the sphere by an amount, never past nothing.
        ///
        /// \param Amount The amount to shrink by.
        /// \return The shrunken sphere.
        ZY_INLINE constexpr Sphere Contract(Real32 Amount) const
        {
            return Sphere(mCenter, Max(0.0f, mRadius - Amount));
        }

        /// \brief Checks if this sphere completely contains another sphere.
        ///
        /// \param Other The other sphere to check.
        /// \return `true` if this sphere contains the other, `false` otherwise.
        ZY_INLINE constexpr Bool Contains(Sphere Other) const
        {
            if (mRadius >= Other.mRadius)
            {
                const Real32 DistanceSquared = mCenter.GetDistanceSquared(Other.mCenter);
                const Real32 DistanceRadius  = mRadius - Other.mRadius;
                return DistanceSquared <= (DistanceRadius * DistanceRadius);
            }
            return false;
        }

        /// \brief Checks if this sphere contains a point.
        ///
        /// \param Point The point to check.
        /// \return `true` if the point is inside the sphere, `false` otherwise.
        ZY_INLINE constexpr Bool Contains(Vector3 Point) const
        {
            return mCenter.GetDistanceSquared(Point) <= (mRadius * mRadius);
        }

        /// \brief Checks if this sphere intersects with another sphere.
        ///
        /// \param Other The other sphere to check.
        /// \return `true` if the spheres intersect, `false` otherwise.
        ZY_INLINE constexpr Bool Test(Sphere Other) const
        {
            const Real32 DistanceSquared = mCenter.GetDistanceSquared(Other.mCenter);
            const Real32 RadiusSum       = mRadius + Other.mRadius;
            return DistanceSquared <= (RadiusSum * RadiusSum);
        }

        /// \brief Checks if this sphere is equal to another sphere.
        ///
        /// \param Other The sphere to compare to.
        /// \return `true` if center and radius are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(Sphere Other) const
        {
            return mCenter == Other.mCenter && IsAlmostEqual(mRadius, Other.mRadius);
        }

        /// \brief Checks if this sphere is not equal to another sphere.
        ///
        /// \param Other The sphere to compare to.
        /// \return `true` if the spheres are not equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(Sphere Other) const
        {
            return !(* this == Other);
        }

    public:

        /// \brief Gets a sphere that contains nothing and is grown into.
        ///
        /// \return An invalid sphere.
        ZY_INLINE static constexpr Sphere Invalid()
        {
            return Sphere(Vector3(0.0f, 0.0f, 0.0f), -1.0f);
        }

        /// \brief Gets the unit sphere centered at origin with radius 1.0.
        ///
        /// \return A unit sphere.
        ZY_INLINE static constexpr Sphere Unit()
        {
            return Sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
        }

        /// \brief Creates a sphere from a circle that lies on the XY plane.
        ///
        /// \param XY The circle supplying the (x, y) center and the radius.
        /// \param Z  The depth to place the center at.
        /// \return A new sphere centered on the circle at the given depth.
        ZY_INLINE static constexpr Sphere FromXY(Circle XY, Real32 Z = 0.0f)
        {
            return Sphere(Vector3::FromXY(XY.GetCenter(), Z), XY.GetRadius());
        }

        /// \brief Creates a sphere from a circle that lies on the XZ plane.
        ///
        /// \param XZ The circle supplying the (x, z) center and the radius.
        /// \param Y  The height to place the center at.
        /// \return A new sphere centered on the circle at the given height.
        ZY_INLINE static constexpr Sphere FromXZ(Circle XZ, Real32 Y = 0.0f)
        {
            return Sphere(Vector3::FromXZ(XZ.GetCenter(), Y), XZ.GetRadius());
        }

        /// \brief Canonicalizes a sphere by ensuring its radius is non-negative.
        ///
        /// \param Source The sphere to canonicalize.
        /// \return A sphere with non-negative radius.
        ZY_INLINE static constexpr Sphere Canonicalize(Sphere Source)
        {
            return Sphere(Source.mCenter, Abs(Source.mRadius));
        }

        /// \brief Gets the smallest sphere containing both spheres.
        ///
        /// \param First  The first sphere.
        /// \param Second The second sphere.
        /// \return The bounding sphere that contains both.
        ZY_INLINE static Sphere Merge(Sphere First, Sphere Second)
        {
            if (!First.IsValid())
            {
                return Second;
            }

            if (!Second.IsValid())
            {
                return First;
            }

            const Vector3 Direction = Second.mCenter - First.mCenter;
            const Real32  Distance  = Direction.GetLength();

            if (Distance + Second.mRadius <= First.mRadius)
            {
                return First;
            }

            if (Distance + First.mRadius <= Second.mRadius)
            {
                return Second;
            }

            const Real32 Radius = (Distance + First.mRadius + Second.mRadius) * 0.5f;
            return Sphere(First.mCenter + Direction * ((Radius - First.mRadius) / Distance), Radius);
        }

        /// \brief Grows a sphere to contain a point.
        ///
        /// \param Source The sphere to grow.
        /// \param Point  The point to take in.
        /// \return The smallest sphere containing both.
        ZY_INLINE static Sphere Merge(Sphere Source, Vector3 Point)
        {
            return Merge(Source, Sphere(Point, 0.0f));
        }

        /// \brief Gets the smallest sphere containing a box.
        ///
        /// \param Source The box to enclose.
        /// \return The bounding sphere.
        ZY_INLINE static Sphere Enclose(ConstRef<Box> Source)
        {
            const Vector3 Center = (Source.GetMinimum() + Source.GetMaximum()) * 0.5f;

            return Sphere(Center, (Source.GetMaximum() - Center).GetLength());
        }

        /// \brief Carries a sphere through an affine transform.
        ///
        /// \param Source The sphere to transform.
        /// \param Matrix The transform to apply.
        /// \return The transformed sphere.
        ZY_INLINE static Sphere Transform(Sphere Source, ConstRef<Matrix4x3> Matrix)
        {
            const Vector3 Center = Matrix4x3::Project(Matrix, Source.mCenter);

            const Real32 Scale = Max(
                Matrix.GetBasisX().GetLength(),
                Matrix.GetBasisY().GetLength(),
                Matrix.GetBasisZ().GetLength());

            return Sphere(Center, Source.mRadius * Scale);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector3 mCenter;
        Real32  mRadius;
    };
}