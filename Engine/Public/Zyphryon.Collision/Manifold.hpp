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

#include "Zyphryon.Math/Vector2.hpp"
#include "Zyphryon.Math/Vector3.hpp"
#include "Zyphryon.Base/Memory/Pool.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Collision
{
    /// \brief Represents a contact manifold for collision resolution between two bodies.
    template<typename Type>
    class AnyManifold final
    {
    public:

        /// \brief The maximum number of contact points stored in a single manifold.
        static constexpr UInt32 kMaxCollisionPoints = 4;

    public:

        /// \brief Constructor initializing the manifold to an empty state.
        ZYPHRYON_INLINE AnyManifold()
            : mPenetration { Type(0) }
        {
        }

        /// \brief Checks if the manifold represents a valid collision.
        ///
        /// \return `true` if the manifold represents a collision, `false` otherwise.
        ZYPHRYON_INLINE Bool IsValid() const
        {
            return mPoints.GetSize() > 0 && mPenetration > Type(0);
        }

        /// \brief Sets the collision normal from a 2D vector.
        ///
        /// \param Normal The 2D normal vector of the collision.
        ZYPHRYON_INLINE void SetNormal(ConstRef<AnyVector2<Type>> Normal)
        {
            SetNormal(AnyVector3(Normal.GetX(), Normal.GetY(), Type(0)));
        }

        /// \brief Sets the collision normal from a 3D vector.
        ///
        /// \param Normal The 3D normal vector of the collision.
        ZYPHRYON_INLINE void SetNormal(ConstRef<AnyVector3<Type>> Normal)
        {
            mNormal = Normal;
        }

        /// \brief Gets the collision normal.
        ///
        /// \return The 3D normal vector of the collision.
        ZYPHRYON_INLINE ConstRef<AnyVector3<Type>> GetNormal() const
        {
            return mNormal;
        }

        /// \brief Sets the penetration depth of the collision.
        ///
        /// \param Penetration The depth by which the two bodies are interpenetrating.
        ZYPHRYON_INLINE void SetPenetration(Type Penetration)
        {
            mPenetration = Penetration;
        }

        /// \brief Gets the penetration depth of the collision.
        ///
        /// \return The depth by which the two bodies are interpenetrating.
        ZYPHRYON_INLINE Type GetPenetration() const
        {
            return mPenetration;
        }

        /// \brief Adds a 2D contact point to the manifold.
        ///
        /// \param Point The 2D contact point to add.
        ZYPHRYON_INLINE void AddPoint(ConstRef<AnyVector2<Type>> Point)
        {
            AddPoint(AnyVector3<Type>(Point.GetX(), Point.GetY(), Type(0)));
        }

        /// \brief Adds a 3D contact point to the manifold.
        ///
        /// \param Point The 3D contact point to add.
        ZYPHRYON_INLINE void AddPoint(ConstRef<AnyVector3<Type>> Point)
        {
            mPoints.Allocate(Point);
        }

        /// \brief Gets the list of contact points in the manifold.
        ///
        /// \return A constant span containing the contact points.
        ZYPHRYON_INLINE ConstSpan<AnyVector3<Type>> GetPoints() const
        {
            return mPoints.GetSpan();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        AnyVector3<Type>                            mNormal;
        Type                                        mPenetration;
        Pool<AnyVector3<Type>, kMaxCollisionPoints> mPoints;
    };

    /// \brief Represents a contact manifold with single-precision floating-point components.
    using Manifold = AnyManifold<Real32>;
}