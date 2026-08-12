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
#include "Sphere.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a half-line defined by an origin and a normalized direction.
    class Ray final
    {
    public:

        /// \brief Initializes the ray at the origin, pointing along the elevation axis.
        ZY_INLINE constexpr Ray()
            : mOrigin    { 0.0f, 0.0f, 0.0f },
              mDirection { Vector3::UnitY() },
              mInverse   { Reciprocal(Vector3::UnitY()) }
        {
        }

        /// \brief Initializes the ray with the given origin and direction.
        ///
        /// \param Origin    The point the ray starts from.
        /// \param Direction The direction the ray travels, which must already be normalized.
        ZY_INLINE constexpr Ray(Vector3 Origin, Vector3 Direction)
            : mOrigin    { Origin },
              mDirection { Direction },
              mInverse   { Reciprocal(Direction) }
        {
        }

        /// \brief Checks if the ray is valid.
        ///
        /// \return `true` if the direction is normalized, `false` otherwise.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mDirection.IsNormalized();
        }

        /// \brief Sets the ray's origin and direction.
        ///
        /// \param Origin    The point the ray starts from.
        /// \param Direction The direction the ray travels, which must already be normalized.
        ZY_INLINE constexpr void Set(Vector3 Origin, Vector3 Direction)
        {
            mOrigin = Origin;

            SetDirection(Direction);
        }

        /// \brief Sets the point the ray starts from.
        ///
        /// \param Origin The point the ray starts from.
        ZY_INLINE constexpr void SetOrigin(Vector3 Origin)
        {
            mOrigin = Origin;
        }

        /// \brief Gets the point the ray starts from.
        ///
        /// \return The origin of the ray.
        ZY_INLINE constexpr Vector3 GetOrigin() const
        {
            return mOrigin;
        }

        /// \brief Sets the direction the ray travels.
        ///
        /// \param Direction The direction the ray travels, which must already be normalized.
        ZY_INLINE constexpr void SetDirection(Vector3 Direction)
        {
            mDirection = Direction;
            mInverse   = Reciprocal(Direction);
        }

        /// \brief Gets the direction the ray travels.
        ///
        /// \return The normalized direction of the ray.
        ZY_INLINE constexpr Vector3 GetDirection() const
        {
            return mDirection;
        }

        /// \brief Gets the component-wise reciprocal of the direction, cached for the slab tests.
        ///
        /// \return The reciprocal of the direction.
        ZY_INLINE constexpr Vector3 GetReciprocal() const
        {
            return mInverse;
        }

        /// \brief Gets the point at the given distance along the ray.
        ///
        /// \param Distance The distance travelled from the origin.
        /// \return The point the ray reaches after travelling that distance.
        ZY_INLINE constexpr Vector3 GetPoint(Real32 Distance) const
        {
            return mOrigin + mDirection * Distance;
        }

        /// \brief Gets the point on the ray closest to a given point.
        ///
        /// \param Point The query point in 3D space.
        /// \return The closest point on the ray, never behind the origin.
        ZY_INLINE constexpr Vector3 GetNearest(Vector3 Point) const
        {
            return GetPoint(::Max(0.0f, Vector3::Dot(Point - mOrigin, mDirection)));
        }

        /// \brief Gets the shortest distance from the ray to a point.
        ///
        /// \param Point The point to measure to.
        /// \return The distance between the point and the ray.
        ZY_INLINE Real32 GetDistance(Vector3 Point) const
        {
            return (Point - GetNearest(Point)).GetLength();
        }

        /// \brief Checks if this ray is equal to another ray.
        ///
        /// \param Other The ray to compare to.
        /// \return `true` if origin and direction are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(ConstRef<Ray> Other) const
        {
            return mOrigin == Other.mOrigin && mDirection == Other.mDirection;
        }

        /// \brief Checks if this ray is not equal to another ray.
        ///
        /// \param Other The ray to compare to.
        /// \return `true` if the rays are not equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(ConstRef<Ray> Other) const
        {
            return !(* this == Other);
        }

    public:

        /// \brief Gets a ray that travels nowhere and hits nothing.
        ///
        /// \return An invalid ray.
        ZY_INLINE static constexpr Ray Invalid()
        {
            return Ray(Vector3::Zero(), Vector3::Zero());
        }

        /// \brief Gets the ray that runs from one point towards another.
        ///
        /// \param From The point the ray starts from.
        /// \param To   The point the ray runs towards.
        /// \return The ray, or an invalid ray when the two points coincide.
        ZY_INLINE static Ray Between(Vector3 From, Vector3 To)
        {
            return Create(From, To - From);
        }

        /// \brief Carries a ray through an affine transform.
        ///
        /// \param Source The ray to transform.
        /// \param Matrix The transform to apply.
        /// \return The transformed ray, or an invalid ray when the matrix collapses the direction.
        ZY_INLINE static Ray Transform(ConstRef<Ray> Source, ConstRef<Matrix4x3> Matrix)
        {
            return Create(
                Matrix4x3::Project(Matrix, Source.mOrigin),
                Matrix4x3::ProjectDirection(Matrix, Source.mDirection));
        }

        /// \brief Checks if a ray meets an axis-aligned volume.
        ///
        /// \param Source The ray to cast.
        /// \param Volume The axis-aligned volume to test against.
        /// \param Limit  The furthest distance along the ray to consider.
        /// \return `true` if the ray meets the volume within the limit, `false` otherwise.
        ZY_INLINE static Bool Test(ConstRef<Ray> Source, ConstRef<Box> Volume, Real32 Limit = kInfinity<Real32>)
        {
            Real32 Entry = 0.0f;
            Real32 Exit  = 0.0f;

            return Traverse(Source, Volume, Entry, Exit, Limit);
        }

        /// \brief Finds the nearest intersection between a ray and an axis-aligned volume.
        ///
        /// \param Source   The ray to cast.
        /// \param Volume   The axis-aligned volume to test against.
        /// \param Distance Receives the distance to the entry point, unchanged when the ray misses.
        /// \param Limit    The furthest distance along the ray to consider.
        /// \return `true` if the ray meets the volume within the limit, `false` otherwise.
        ZY_INLINE static Bool Intersects(ConstRef<Ray> Source, ConstRef<Box> Volume, Ref<Real32> Distance, Real32 Limit = kInfinity<Real32>)
        {
            Real32 Entry = 0.0f;
            Real32 Exit  = 0.0f;

            if (Traverse(Source, Volume, Entry, Exit, Limit))
            {
                Distance = Entry;
                return true;
            }
            return false;
        }

        /// \brief Finds the interval of a ray that lies inside an axis-aligned volume.
        ///
        /// \param Source The ray to cast.
        /// \param Volume The axis-aligned volume to test against.
        /// \param Entry  Receives the distance the ray goes in at, unchanged when the ray misses.
        /// \param Exit   Receives the distance the ray comes out at, unchanged when the ray misses.
        /// \param Limit  The furthest distance along the ray to consider.
        /// \return `true` if the ray meets the volume within the limit, `false` otherwise.
        ZY_INLINE static Bool Traverse(ConstRef<Ray> Source, ConstRef<Box> Volume, Ref<Real32> Entry, Ref<Real32> Exit, Real32 Limit = kInfinity<Real32>)
        {
            const Vector3 Origin  = Source.mOrigin;
            const Vector3 Inverse = Source.mInverse;
            const Vector3 Minimum = Volume.GetMinimum();
            const Vector3 Maximum = Volume.GetMaximum();

            Real32 Near = 0.0f;
            Real32 Far  = Limit;

            if (!Clip(Origin.GetX(), Inverse.GetX(), Minimum.GetX(), Maximum.GetX(), Near, Far)
             || !Clip(Origin.GetY(), Inverse.GetY(), Minimum.GetY(), Maximum.GetY(), Near, Far)
             || !Clip(Origin.GetZ(), Inverse.GetZ(), Minimum.GetZ(), Maximum.GetZ(), Near, Far))
            {
                return false;
            }

            if (Near <= Far)
            {
                Entry = Near;
                Exit  = Far;
                return true;
            }
            return false;
        }

        /// \brief Checks if a ray meets a sphere.
        ///
        /// \param Source The ray to cast.
        /// \param Volume The sphere to test against.
        /// \param Limit  The furthest distance along the ray to consider.
        /// \return `true` if the ray meets the sphere within the limit, `false` otherwise.
        ZY_INLINE static Bool Test(ConstRef<Ray> Source, ConstRef<Sphere> Volume, Real32 Limit = kInfinity<Real32>)
        {
            Real32 Entry = 0.0f;
            Real32 Exit  = 0.0f;

            return Traverse(Source, Volume, Entry, Exit, Limit);
        }

        /// \brief Finds the nearest intersection between a ray and a sphere.
        ///
        /// \note An origin inside the sphere yields a distance of zero rather than the exit distance.
        ///
        /// \param Source   The ray to cast.
        /// \param Volume   The sphere to test against.
        /// \param Distance Receives the distance to the entry point, unchanged when the ray misses.
        /// \param Limit    The furthest distance along the ray to consider.
        /// \return `true` if the ray meets the sphere within the limit, `false` otherwise.
        ZY_INLINE static Bool Intersects(ConstRef<Ray> Source, ConstRef<Sphere> Volume, Ref<Real32> Distance, Real32 Limit = kInfinity<Real32>)
        {
            Real32 Entry = 0.0f;
            Real32 Exit  = 0.0f;

            if (Traverse(Source, Volume, Entry, Exit, Limit))
            {
                Distance = Entry;
                return true;
            }
            return false;
        }

        /// \brief Finds the interval of a ray that lies inside a sphere.
        ///
        /// \param Source The ray to cast.
        /// \param Volume The sphere to test against.
        /// \param Entry  Receives the distance the ray goes in at, unchanged when the ray misses.
        /// \param Exit   Receives the distance the ray comes out at, unchanged when the ray misses.
        /// \param Limit  The furthest distance along the ray to consider.
        /// \return `true` if the ray meets the sphere within the limit, `false` otherwise.
        ZY_INLINE static Bool Traverse(ConstRef<Ray> Source, ConstRef<Sphere> Volume, Ref<Real32> Entry, Ref<Real32> Exit, Real32 Limit = kInfinity<Real32>)
        {
            const Vector3 Offset  = Volume.GetCenter() - Source.mOrigin;
            const Real32  Closest = Vector3::Dot(Offset, Source.mDirection);
            const Real32  Radius  = Volume.GetRadius();

            const Real32 Reach = Radius * Radius - (Offset.GetLengthSquared() - Closest * Closest);

            if (Reach < 0.0f)
            {
                return false;
            }

            const Real32 Extent = Sqrt(Reach);
            const Real32 Near   = ::Max(0.0f,  Closest - Extent);
            const Real32 Far    = ::Min(Limit, Closest + Extent);

            if (Near <= Far)
            {
                Entry = Near;
                Exit  = Far;
                return true;
            }
            return false;
        }

        /// \brief Serializes the state of the ray to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the ray with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mOrigin);
            Archive.Serialize(mDirection);

            if constexpr (Serializer::IsReader)
            {
                mInverse = Reciprocal(mDirection);
            }
        }

    private:

        /// \brief The reciprocal standing in for an axis the ray does not travel along.
        static constexpr Real32 kUnbounded = 1.0e+30f;

        /// \brief Gets a ray from a direction that has not been checked for length.
        ///
        /// \param Origin    The point the ray starts from.
        /// \param Direction The direction the ray travels.
        /// \return The ray, or an invalid ray when the direction has no length.
        ZY_INLINE static Ray Create(Vector3 Origin, Vector3 Direction)
        {
            const Real32 Length = Direction.GetLength();

            if (::IsAlmostZero(Length))
            {
                return Ray(Origin, Vector3::Zero());
            }
            return Ray(Origin, Direction * (1.0f / Length));
        }

        /// \brief Gets the component-wise reciprocal of a direction, keeping every component finite.
        ///
        /// \param Direction The direction to invert.
        /// \return The reciprocal of the direction.
        ZY_INLINE static constexpr Vector3 Reciprocal(Vector3 Direction)
        {
            return Vector3(
                ::IsAlmostZero(Direction.GetX()) ? kUnbounded : 1.0f / Direction.GetX(),
                ::IsAlmostZero(Direction.GetY()) ? kUnbounded : 1.0f / Direction.GetY(),
                ::IsAlmostZero(Direction.GetZ()) ? kUnbounded : 1.0f / Direction.GetZ());
        }

        /// \brief Clips the ray's travel interval against one pair of slab planes.
        ///
        /// \param Origin  The ray's origin along the slab's axis.
        /// \param Inverse The reciprocal of the ray's direction along the slab's axis.
        /// \param Minimum The near plane of the slab.
        /// \param Maximum The far plane of the slab.
        /// \param Near    The entry distance, pushed forward in place.
        /// \param Far     The exit distance, pulled back in place.
        /// \return `false` once the interval collapses, meaning the ray misses the volume.
        ZY_INLINE static Bool Clip(Real32 Origin, Real32 Inverse, Real32 Minimum, Real32 Maximum, Ref<Real32> Near, Ref<Real32> Far)
        {
            // A ray that does not travel along this axis clips nothing, since its interval is unbounded.
            if (Abs(Inverse) >= kUnbounded)
            {
                return Origin >= Minimum && Origin <= Maximum;
            }

            const Real32 First  = (Minimum - Origin) * Inverse;
            const Real32 Second = (Maximum - Origin) * Inverse;

            Near = ::Max(Near, ::Min(First, Second));
            Far  = ::Min(Far,  ::Max(First, Second));
            return Near <= Far;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector3 mOrigin;
        Vector3 mDirection;
        Vector3 mInverse;
    };
}
