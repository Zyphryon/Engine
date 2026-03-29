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
    /// \brief Represents the spatial properties of a 3D audio object.
    class Pose final
    {
    public:

        /// \brief Default constructor initializes to origin with no rotation or velocity.
        ZYPHRYON_INLINE Pose() = default;

        /// \brief Constructs a spatial object with the specified position, velocity, and orientation.
        ///
        /// \param Position    The position of the object in 3D space.
        /// \param Velocity    The velocity vector of the object.
        /// \param Orientation The orientation of the object.
        ZYPHRYON_INLINE Pose(Vector3 Position, Vector3 Velocity, Quaternion Orientation)
            : mPosition    { Position },
              mVelocity    { Velocity },
              mOrientation { Orientation }
        {
            LOG_ASSERT(Orientation.IsNormalized(), "Orientation must be a unit quaternion");
        }

        /// \brief Sets the position of the object in 3D space.
        ///
        /// \param Position The new position vector.
        ZYPHRYON_INLINE void SetPosition(Vector3 Position)
        {
            mPosition = Position;
        }

        /// \brief Gets the position of the object in 3D space.
        ///
        /// \return The position vector.
        ZYPHRYON_INLINE Vector3 GetPosition() const
        {
            return mPosition;
        }

        /// \brief Sets the velocity of the object.
        ///
        /// \param Velocity The new velocity vector.
        ZYPHRYON_INLINE void SetVelocity(Vector3 Velocity)
        {
            mVelocity = Velocity;
        }

        /// \brief Gets the velocity of the object.
        ///
        /// \return The velocity vector.
        ZYPHRYON_INLINE Vector3 GetVelocity() const
        {
            return mVelocity;
        }

        /// \brief Sets the orientation of the object.
        ///
        /// \param Orientation The new orientation quaternion.
        ZYPHRYON_INLINE void SetOrientation(Quaternion Orientation)
        {
            LOG_ASSERT(Orientation.IsNormalized(), "Orientation must be a unit quaternion");

            mOrientation = Orientation;
        }

        /// \brief Gets the orientation of the object.
        ///
        /// \return The orientation quaternion.
        ZYPHRYON_INLINE Quaternion GetOrientation() const
        {
            return mOrientation;
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mPosition);
            Archive.SerializeObject(mVelocity);
            Archive.SerializeObject(mOrientation);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector3    mPosition;
        Vector3    mVelocity;
        Quaternion mOrientation;
    };
}