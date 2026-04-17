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

#include "Matrix3x2.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents position, rotation, and scale of an object in 2D space.
    class Transform2D final
    {
    public:

        /// \brief Default constructor initializing the transform to an identity state.
        ZYPHRYON_INLINE Transform2D()
            : mScale { Vector2::One() }
        {
        }

        /// \brief Copy constructor for Transform2D.
        ZYPHRYON_INLINE Transform2D(ConstRef<Transform2D> Other) = default;

        /// \brief Constructs a transform with the specified translation, scale, and rotation.
        ///
        /// \param Translation The translation vector to set (2D).
        /// \param Scale       The scale vector to set (2D).
        /// \param Rotation    The rotation to set, represented as an angle in radians.
        ZYPHRYON_INLINE Transform2D(Vector2 Translation, Vector2 Scale, Angle Rotation)
        {
            SetTranslation(Translation);
            SetScale(Scale);
            SetRotation(Rotation);
        }

        /// \brief Constructs a transform with the specified translation and rotation.
        ///
        /// \param Translation The translation vector to set (2D).
        /// \param Rotation    The rotation to set, represented as an angle in radians.
        ZYPHRYON_INLINE Transform2D(Vector2 Translation, Angle Rotation)
            : Transform2D()
        {
            SetTranslation(Translation);
            SetRotation(Rotation);
        }

        /// \brief Constructs a transform with the specified translation.
        ///
        /// \param Translation The translation vector to set (2D).
        ZYPHRYON_INLINE explicit Transform2D(Vector2 Translation)
            : Transform2D()
        {
            SetTranslation(Translation);
        }

        /// \brief Computes the final transformation matrix with an origin point.
        ///
        /// \param Origin The center point for rotation, scale, and skew operations.
        /// \return A 3x2 transformation matrix that combines translation, scale, rotation and skew with the specified origin.
        ZYPHRYON_INLINE Matrix3x2 Compute(Vector2 Origin = Vector2()) const
        {
            return Matrix3x2::FromTransform(Origin, mTranslation, mScale, mRotation);
        }

        /// \brief Sets the translation vector for the transform in 2D space.
        /// 
        /// \param Translation The translation vector to set (2D).
        /// \return A reference to this transform, modified by the new translation.
        ZYPHRYON_INLINE Ref<Transform2D> SetTranslation(Vector2 Translation)
        {
            mTranslation = Translation;
            return (* this);
        }

        /// \brief Gets the current translation of the transform.
        /// 
        /// \return A reference to the translation vector.
        ZYPHRYON_INLINE Vector2 GetTranslation() const
        {
            return mTranslation;
        }

        /// \brief Sets the scale vector for the transform.
        /// 
        /// \param Scale The scale vector to set (2D).
        /// \return A reference to this transform, modified by the new scale.
        ZYPHRYON_INLINE Ref<Transform2D> SetScale(Vector2 Scale)
        {
            mScale = Scale;
            return (* this);
        }

        /// \brief Gets the current scale of the transform.
        /// 
        /// \return A reference to the scale vector.
        ZYPHRYON_INLINE Vector2 GetScale() const
        {
            return mScale;
        }

        /// \brief Sets the rotation for the transform.
        /// 
        /// \param Rotation The rotation to set, represented as an angle in radians.
        /// \return A reference to this transform, modified by the new rotation.
        ZYPHRYON_INLINE Ref<Transform2D> SetRotation(Angle Rotation)
        {
            mRotation = Rotation;
            return (* this);
        }

        /// \brief Gets the current rotation of the transform.
        /// 
        /// \return The rotation of the transform, represented as an angle in radians.
        ZYPHRYON_INLINE Angle GetRotation() const
        {
            return mRotation;
        }

        /// \brief Translates the transform by the given 2D vector.
        /// 
        /// \param Translation The translation vector to apply (2D).
        /// \return A reference to this transform, modified by the translation.
        ZYPHRYON_INLINE Ref<Transform2D> Translate(Vector2 Translation)
        {
            mTranslation += Translation;
            return (* this);
        }

        /// \brief Scales the transform by a scalar value.
        /// 
        /// \param Scalar The scalar value to apply as a scale factor.
        /// \return A reference to this transform, modified by the scale.
        ZYPHRYON_INLINE Ref<Transform2D> Scale(Real32 Scalar)
        {
            LOG_ASSERT(Scalar != 0.0f, "Scale factor must not be zero");

            mScale *= Scalar;
            return (* this);
        }

        /// \brief Scales the transform by a 2D vector.
        /// 
        /// \param Vector The 2D vector to scale the transform by.
        /// \return A reference to this transform, modified by the scale.
        ZYPHRYON_INLINE Ref<Transform2D> Scale(Vector2 Vector)
        {
            LOG_ASSERT(Vector.GetX() != 0.0f, "Scale X must not be zero");
            LOG_ASSERT(Vector.GetY() != 0.0f, "Scale Y must not be zero");

            mScale *= Vector;
            return (* this);
        }

        /// \brief Rotates the transform in world space by the specified quaternion.
        ///
        /// \param Rotation The rotation to apply, represented as radians.
        /// \return A reference to this transform, modified by the rotation.
        ZYPHRYON_INLINE Ref<Transform2D> Rotate(Angle Rotation)
        {
            mRotation += Rotation;
            return (* this);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        /// 
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mTranslation);
            Archive.SerializeObject(mScale);
            Archive.SerializeObject(mRotation);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector2 mTranslation;
        Vector2 mScale;
        Angle   mRotation;
    };
}