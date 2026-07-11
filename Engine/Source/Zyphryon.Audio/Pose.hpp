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

#include "Zyphryon.Math/Quaternion.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Represents the spatial properties of a sound source or listener in 3D space.
    class Pose final
    {
    public:

        /// \brief Constructs a pose with default parameters (position at origin, zero velocity, and no rotation).
        ZY_INLINE Pose() = default;

        /// \brief Constructs a pose with the specified position, velocity, and orientation.
        ///
        /// \param Position    The position of the pose in 3D space.
        /// \param Velocity    The velocity of the pose in 3D space.
        /// \param Orientation The orientation of the pose as a unit quaternion.
        ZY_INLINE Pose(Vector3 Position, Vector3 Velocity, Quaternion Orientation)
            : mPosition    { Position },
              mVelocity    { Velocity },
              mOrientation { Orientation }
        {
            ZY_ASSERT(Orientation.IsNormalized(), "Orientation must be a unit quaternion");
        }

        /// \brief Sets the position of the pose in 3D space.
        ///
        /// \param Position The new position to set.
        ZY_INLINE void SetPosition(Vector3 Position)
        {
            mPosition = Position;
        }

        /// \brief Gets the position of the pose in 3D space.
        ///
        /// \return The current position of the pose.
        ZY_INLINE Vector3 GetPosition() const
        {
            return mPosition;
        }

        /// \brief Sets the velocity of the pose in 3D space.
        ///
        /// \param Velocity The new velocity to set.
        ZY_INLINE void SetVelocity(Vector3 Velocity)
        {
            mVelocity = Velocity;
        }

        /// \brief Gets the velocity of the pose in 3D space.
        ///
        /// \return The current velocity of the pose.
        ZY_INLINE Vector3 GetVelocity() const
        {
            return mVelocity;
        }

        /// \brief Sets the orientation of the pose as a unit quaternion.
        ///
        /// \param Orientation The new orientation to set, which must be a normalized quaternion.
        ZY_INLINE void SetOrientation(Quaternion Orientation)
        {
            ZY_ASSERT(Orientation.IsNormalized(), "Orientation must be a unit quaternion");

            mOrientation = Orientation;
        }

        /// \brief Gets the orientation of the pose as a unit quaternion.
        ///
        /// \return The current orientation of the pose.
        ZY_INLINE Quaternion GetOrientation() const
        {
            return mOrientation;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector3    mPosition;
        Vector3    mVelocity;
        Quaternion mOrientation;
    };
}