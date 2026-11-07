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

#include "Zyphryon.Math/Vector3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    /// \brief Describes how two shapes meet, as the way out, the moment of a moving one and the depth of a sunk one.
    template<typename Vector>
    class AnyManifold final
    {
    public:

        /// \brief Constructs a contact that stands for nothing having been met, with no way out.
        ZY_INLINE constexpr AnyManifold()
            : mNormal { Vector::Zero() },
              mTime   { 0.0f },
              mDepth  { 0.0f }
        {
        }

        /// \brief Constructs a contact from its way out, the moment it happens and how deep it goes.
        ///
        /// \param Normal The way out, of unit length, pointing from the second shape at the first.
        /// \param Time   The fraction of the motion the two meet at, from 0 to 1, or 0 for two found sunk in.
        /// \param Depth  The distance one has sunk into the other along the way out, or 0 for a meeting ahead.
        ZY_INLINE constexpr AnyManifold(Vector Normal, Real32 Time, Real32 Depth)
            : mNormal { Normal },
              mTime   { Time },
              mDepth  { Depth }
        {
        }

        /// \brief Sets the way out of the contact.
        ///
        /// \param Normal The way out to set, of unit length, pointing from the second shape at the first.
        ZY_INLINE constexpr void SetNormal(Vector Normal)
        {
            mNormal = Normal;
        }

        /// \brief Gets the way out of the contact.
        ///
        /// \return The way out, of unit length, pointing from the second shape at the first, or zero for none.
        ZY_INLINE constexpr Vector GetNormal() const
        {
            return mNormal;
        }

        /// \brief Sets the fraction of the motion the two meet at.
        ///
        /// \param Time The fraction to set, from 0 to 1, or 0 for two found sunk in.
        ZY_INLINE constexpr void SetTime(Real32 Time)
        {
            mTime = Time;
        }

        /// \brief Gets the fraction of the motion the two meet at.
        ///
        /// \return The fraction, from 0 to 1, which is 0 for two found sunk in.
        ZY_INLINE constexpr Real32 GetTime() const
        {
            return mTime;
        }

        /// \brief Sets how far one shape has sunk into the other.
        ///
        /// \param Depth The distance to set along the way out, or 0 for a meeting ahead.
        ZY_INLINE constexpr void SetDepth(Real32 Depth)
        {
            mDepth = Depth;
        }

        /// \brief Gets how far one shape has sunk into the other.
        ///
        /// \return The distance along the way out, which is 0 for a meeting ahead.
        ZY_INLINE constexpr Real32 GetDepth() const
        {
            return mDepth;
        }

        /// \brief Gets the shortest move that takes the first shape clear of the second.
        ///
        /// \return The way out scaled by the depth, which is zero for a meeting ahead.
        ZY_INLINE constexpr Vector GetEscape() const
        {
            return mNormal * mDepth;
        }

        /// \brief Checks whether the contact has a way out, which every contact a test hands back does.
        ///
        /// \return `true` if a way out was found, `false` for a contact that stands for nothing.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return !mNormal.IsAlmostZero();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector mNormal;
        Real32 mTime;
        Real32 mDepth;
    };

    /// \brief Describes how two volumes meet, with the way out in three dimensions.
    using Manifold3D = AnyManifold<Vector3>;

    /// \brief Describes how two flat shapes meet, with the way out across the plane they lie on.
    using Manifold2D = AnyManifold<Vector2>;
}