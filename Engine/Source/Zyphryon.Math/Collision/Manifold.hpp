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

inline namespace Math
{
    /// \brief Describes where two volumes meet and which way one has to go to be clear of the other.
    class Manifold final
    {
    public:

        /// \brief Initializes a contact that stands for nothing having been met.
        ZY_INLINE constexpr Manifold()
            : mNormal { 0.0f, 0.0f, 0.0f },
              mTime   { 0.0f },
              mDepth  { 0.0f }
        {
        }

        /// \brief Initializes a contact with the given facing, time and depth.
        ///
        /// \param Normal The way out, pointing from the second volume at the first.
        /// \param Time   The fraction of the motion the two meet at.
        /// \param Depth  How far one has sunk into the other.
        ZY_INLINE constexpr Manifold(Vector3 Normal, Real32 Time, Real32 Depth)
            : mNormal { Normal },
              mTime   { Time },
              mDepth  { Depth }
        {
        }

        /// \brief Sets the way out of the contact.
        ///
        /// \param Normal The facing to set, pointing from the second volume at the first.
        ZY_INLINE constexpr void SetNormal(Vector3 Normal)
        {
            mNormal = Normal;
        }

        /// \brief Gets the way out of the contact.
        ///
        /// \return The facing, pointing from the second volume at the first.
        ZY_INLINE constexpr Vector3 GetNormal() const
        {
            return mNormal;
        }

        /// \brief Sets the fraction of the motion the two meet at.
        ///
        /// \param Time The fraction to set.
        ZY_INLINE constexpr void SetTime(Real32 Time)
        {
            mTime = Time;
        }

        /// \brief Gets the fraction of the motion the two meet at.
        ///
        /// \return The fraction, which is nothing when the two already meet where they stand.
        ZY_INLINE constexpr Real32 GetTime() const
        {
            return mTime;
        }

        /// \brief Sets how far one volume has sunk into the other.
        ///
        /// \param Depth The depth to set.
        ZY_INLINE constexpr void SetDepth(Real32 Depth)
        {
            mDepth = Depth;
        }

        /// \brief Gets how far one volume has sunk into the other.
        ///
        /// \return The depth, which is nothing for a contact that has yet to happen.
        ZY_INLINE constexpr Real32 GetDepth() const
        {
            return mDepth;
        }

        /// \brief Gets how far to move the first volume to get it clear of the second.
        ///
        /// \return The push, which is nothing for a contact that has yet to happen.
        ZY_INLINE constexpr Vector3 GetEscape() const
        {
            return mNormal * mDepth;
        }

        /// \brief Checks whether the contact stands for anything at all.
        ///
        /// \return `true` when a way out was found.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return !mNormal.IsAlmostZero();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector3 mNormal;
        Real32  mTime;
        Real32  mDepth;
    };
}