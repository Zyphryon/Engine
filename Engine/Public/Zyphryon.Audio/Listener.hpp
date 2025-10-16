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

#include "Zyphryon.Math/Quaternion.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Represents the listener in a 3D spatial audio environment.
    class Listener final : public Trackable<Listener>
    {
    public:

        /// \brief Constructs a listener at the origin with no velocity and identity orientation.
        ZYPHRYON_INLINE Listener()
            : mVersion { 1 }
        {
        }

        /// \brief Constructs a listener with specified transform state.
        ///
        /// \param Position    Initial listener position (world units).
        /// \param Velocity    Initial listener velocity (world units per second).
        /// \param Orientation Initial listener orientation.
        ZYPHRYON_INLINE Listener(ConstRef<Vector3> Position, ConstRef<Vector3> Velocity, ConstRef<Quaternion> Orientation)
            : mVersion     { 1 },
              mPosition    { Position },
              mVelocity    { Velocity },
              mOrientation { Orientation }
        {
        }

        /// \brief Gets the current change version of the listener.
        ///
        /// Incremented whenever position, velocity, or orientation changes.
        ///
        /// \return Monotonic version counter.
        ZYPHRYON_INLINE UInt32 GetVersion() const
        {
            return mVersion;
        }

        /// \brief Sets the position of the listener in 3D space.
        ///
        /// \param Position New listener position.
        ZYPHRYON_INLINE void SetPosition(ConstRef<Vector3> Position)
        {
            if (mPosition != Position)
            {
                mPosition = Position;
                Touch();
            }
        }

        /// \brief Gets the listener position.
        ///
        /// \return Constant reference to the position vector.
        ZYPHRYON_INLINE ConstRef<Vector3> GetPosition() const
        {
            return mPosition;
        }

        /// \brief Sets the listener velocity.
        ///
        /// \param Velocity New velocity vector.
        ZYPHRYON_INLINE void SetVelocity(ConstRef<Vector3> Velocity)
        {
            if (mVelocity != Velocity)
            {
                mVelocity = Velocity;
                Touch();
            }
        }

        /// \brief Gets the listener velocity.
        ///
        /// \return Constant reference to the velocity vector.
        ZYPHRYON_INLINE ConstRef<Vector3> GetVelocity() const
        {
            return mVelocity;
        }

        /// \brief Sets the listener orientation.
        ///
        /// \param Orientation New orientation quaternion.
        ZYPHRYON_INLINE void SetOrientation(ConstRef<Quaternion> Orientation)
        {
            LOG_ASSERT(Orientation.IsNormalized(), "Orientation must be a unit quaternion");

            if (mOrientation != Orientation)
            {
                mOrientation = Orientation;
                Touch();
            }
        }

        /// \brief Gets the listener orientation.
        ///
        /// \return Constant reference to the orientation quaternion.
        ZYPHRYON_INLINE ConstRef<Quaternion> GetOrientation() const
        {
            return mOrientation;
        }

    private:

        /// \brief Increments the version to mark a state change.
        ZYPHRYON_INLINE void Touch()
        {
            ++mVersion;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32     mVersion;
        Vector3    mPosition;
        Vector3    mVelocity;
        Quaternion mOrientation;
    };
}