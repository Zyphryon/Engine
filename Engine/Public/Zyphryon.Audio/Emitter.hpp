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

#include "Common.hpp"
#include "Zyphryon.Math/Quaternion.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Represents a 3D sound emitter used in spatial audio.
    class Emitter final : public Trackable
    {
    public:

        /// \brief Constructs a new emitter at the origin with zero radius.
        ZYPHRYON_INLINE Emitter()
            : mVersion { 1 },
              mRadius  { 0.0f }
        {
        }

        /// \brief Constructs a new emitter with specified transform state.
        ///
        /// \param Position    Initial emitter position (world units).
        /// \param Velocity    Initial emitter velocity (world units per second).
        /// \param Orientation Initial emitter orientation.
        ZYPHRYON_INLINE Emitter(ConstRef<Vector3> Position, ConstRef<Vector3> Velocity, ConstRef<Quaternion> Orientation)
            : mVersion     { 1 },
              mPosition    { Position },
              mVelocity    { Velocity },
              mOrientation { Orientation },
              mRadius      { 0.0f }
        {
        }

        /// \brief Gets the current change version of the emitter.
        ///
        /// Incremented whenever position, velocity, orientation, or radius changes.
        ///
        /// \return Monotonic version counter.
        ZYPHRYON_INLINE UInt32 GetVersion() const
        {
            return mVersion;
        }

        /// \brief Sets the position of the emitter in 3D space.
        ///
        /// \param Position New emitter position.
        ZYPHRYON_INLINE void SetPosition(ConstRef<Vector3> Position)
        {
            if (mPosition != Position)
            {
                mPosition = Position;
                Touch();
            }
        }

        /// \brief Gets the emitter position.
        ///
        /// \return Constant reference to the position vector.
        ZYPHRYON_INLINE ConstRef<Vector3> GetPosition() const
        {
            return mPosition;
        }

        /// \brief Sets the emitter velocity.
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

        /// \brief Gets the emitter velocity.
        ///
        /// \return Constant reference to the velocity vector.
        ZYPHRYON_INLINE ConstRef<Vector3> GetVelocity() const
        {
            return mVelocity;
        }

        /// \brief Sets the emitter orientation.
        ///
        /// \param Orientation New orientation quaternion.
        ZYPHRYON_INLINE void SetOrientation(ConstRef<Quaternion> Orientation)
        {
            if (mOrientation != Orientation)
            {
                mOrientation = Orientation;
                Touch();
            }
        }

        /// \brief Gets the emitter orientation.
        ///
        /// \return Constant reference to the orientation quaternion.
        ZYPHRYON_INLINE ConstRef<Quaternion> GetOrientation() const
        {
            return mOrientation;
        }

        /// \brief Sets the attenuation radius of the emitter.
        ///
        /// \param Radius New radius value (world units).
        ZYPHRYON_INLINE void SetRadius(Real32 Radius)
        {
            if (!IsAlmostEqual(mRadius, Radius))
            {
                mRadius = Radius;
                Touch();
            }
        }

        /// \brief Gets the attenuation radius of the emitter.
        ///
        /// \return Current radius value.
        ZYPHRYON_INLINE Real32 GetRadius() const
        {
            return mRadius;
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
        Real32     mRadius;
    };
}
