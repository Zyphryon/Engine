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

#include "Matrix4x4.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents position, rotation, and scale of an object in 3D space.
    class Transform3D final
    {
    public:

        /// \brief Default constructor initializing the transform to an identity state.
        ZYPHRYON_INLINE Transform3D()
            : mScale { Vector3::One() }
        {
        }

        /// \brief Copy constructor for Transform3D.
        ZYPHRYON_INLINE Transform3D(ConstRef<Transform3D> Other) = default;

        /// \brief Constructs a Transform3D with the specified 3D translation, scale, and rotation.
        ///
        /// \param Translation The 3D translation vector to set.
        /// \param Scale       The 3D scale vector to set.
        /// \param Rotation    The quaternion representing the rotation to set.
        ZYPHRYON_INLINE Transform3D(Vector3 Translation, Vector3 Scale, Quaternion Rotation)
        {
            SetTranslation(Translation);
            SetScale(Scale);
            SetRotation(Rotation);
        }

        /// \brief Constructs a Transform3D with the specified 2D translation (Z = 0), scale (Z = 1), and rotation.
        ///
        /// \param Translation The 2D translation vector to set (Z component will be set to 0).
        /// \param Scale       The 2D scale vector to set (Z component will be set to 1).
        /// \param Rotation    The quaternion representing the rotation to set.
        ZYPHRYON_INLINE Transform3D(Vector2 Translation, Vector2 Scale, Quaternion Rotation)
        {
            SetTranslation(Translation);
            SetScale(Scale);
            SetRotation(Rotation);
        }

        /// \brief Constructs a Transform3D with the specified 3D translation and rotation.
        ///
        /// \param Translation The 3D translation vector to set.
        /// \param Rotation    The quaternion representing the rotation to set.
        ZYPHRYON_INLINE Transform3D(Vector3 Translation, Quaternion Rotation)
            : Transform3D()
        {
            SetTranslation(Translation);
            SetRotation(Rotation);
        }

        /// \brief Constructs a Transform3D with the specified 2D translation (Z = 0) and rotation.
        ///
        /// \param Translation The 2D translation vector to set (Z component will be set to 0).
        /// \param Rotation    The quaternion representing the rotation.
        ZYPHRYON_INLINE Transform3D(Vector2 Translation, Quaternion Rotation)
            : Transform3D()
        {
            SetTranslation(Translation);
            SetRotation(Rotation);
        }

        /// \brief Constructs a Transform3D with the specified 3D translation.
        ///
        /// \param Translation The 3D translation vector to set.
        ZYPHRYON_INLINE explicit Transform3D(Vector3 Translation)
            : Transform3D()
        {
            SetTranslation(Translation);
        }

        /// \brief Constructs a Transform3D with the specified 2D translation (Z = 0).
        ///
        /// \param Translation The 2D translation vector to set (Z component will be set to 0).
        ZYPHRYON_INLINE explicit Transform3D(Vector2 Translation)
            : Transform3D()
        {
            SetTranslation(Translation);
        }

        /// \brief Computes the final transformation matrix.
        /// 
        /// \return A 4x4 transformation matrix that combines translation, scale, and rotation.
        ZYPHRYON_INLINE Matrix4x4 Compute() const
        {
            return Matrix4x4::FromTransform(mTranslation, mScale, mRotation);
        }

        /// \brief Sets the translation vector for the transform.
        /// 
        /// \param Translation The translation vector to set (2D).
        /// \return A reference to this transform, modified by the new translation.
        ZYPHRYON_INLINE Ref<Transform3D> SetTranslation(Vector2 Translation)
        {
            mTranslation = Vector3(Translation.GetX(), Translation.GetY(), 0.0f);
            return (* this);
        }

        /// \brief Sets the translation vector for the transform in 3D space.
        /// 
        /// \param Translation The translation vector to set (3D).
        /// \return A reference to this transform, modified by the new translation.
        ZYPHRYON_INLINE Ref<Transform3D> SetTranslation(Vector3 Translation)
        {
            mTranslation = Translation;
            return (* this);
        }

        /// \brief Gets the current translation of the transform.
        /// 
        /// \return A reference to the translation vector (4D).
        ZYPHRYON_INLINE Vector3 GetTranslation() const
        {
            return mTranslation;
        }

        /// \brief Sets the scale vector for the transform in 2D space (Z scale = 1.0f).
        /// 
        /// \param Scale The scale vector to set (2D).
        /// \return A reference to this transform, modified by the new scale.
        ZYPHRYON_INLINE Ref<Transform3D> SetScale(Vector2 Scale)
        {
            return SetScale(Vector3(Scale.GetX(), Scale.GetY(), 1.0f));
        }

        /// \brief Sets the scale vector for the transform.
        /// 
        /// \param Scale The scale vector to set (3D).
        /// \return A reference to this transform, modified by the new scale.
        ZYPHRYON_INLINE Ref<Transform3D> SetScale(Vector3 Scale)
        {
            mScale = Scale;
            return (* this);
        }

        /// \brief Gets the current scale of the transform.
        /// 
        /// \return A reference to the scale vector.
        ZYPHRYON_INLINE Vector3 GetScale() const
        {
            return mScale;
        }

        /// \brief Sets the rotation for the transform.
        /// 
        /// \param Rotation The quaternion representing the rotation.
        /// \return A reference to this transform, modified by the new rotation.
        ZYPHRYON_INLINE Ref<Transform3D> SetRotation(Quaternion Rotation)
        {
            mRotation = Rotation;
            return (* this);
        }

        /// \brief Gets the current rotation of the transform.
        /// 
        /// \return A reference to the quaternion representing the rotation.
        ZYPHRYON_INLINE Quaternion GetRotation() const
        {
            return mRotation;
        }

        /// \brief Translates the transform by the given 2D vector.
        /// 
        /// \param Translation The translation vector to apply (2D).
        /// \return A reference to this transform, modified by the translation.
        ZYPHRYON_INLINE Ref<Transform3D> Translate(Vector2 Translation)
        {
            return Translate(Vector3(Translation.GetX(), Translation.GetY(), 0));
        }

        /// \brief Translates the transform by the given 3D vector.
        /// 
        /// \param Translation The translation vector to apply (3D).
        /// \return A reference to this transform, modified by the translation.
        ZYPHRYON_INLINE Ref<Transform3D> Translate(Vector3 Translation)
        {
            mTranslation += Translation;
            return (* this);
        }

        /// \brief Scales the transform by a scalar value.
        /// 
        /// \param Scalar The scalar value to apply as a scale factor.
        /// \return A reference to this transform, modified by the scale.
        ZYPHRYON_INLINE Ref<Transform3D> Scale(Real32 Scalar)
        {
            LOG_ASSERT(Scalar != 0.0f, "Scale factor must not be zero");

            mScale *= Scalar;
            return (* this);
        }

        /// \brief Scales the transform by a 2D vector (Z scale = 1.0f).
        /// 
        /// \param Vector The 2D vector to scale the transform by.
        /// \return A reference to this transform, modified by the scale.
        ZYPHRYON_INLINE Ref<Transform3D> Scale(Vector2 Vector)
        {
            return Scale(Vector3(Vector.GetX(), Vector.GetY(), 1.0f));
        }

        /// \brief Scales the transform by a 3D vector.
        /// 
        /// \param Vector The 3D vector to scale the transform by.
        /// \return A reference to this transform, modified by the scale.
        ZYPHRYON_INLINE Ref<Transform3D> Scale(Vector3 Vector)
        {
            LOG_ASSERT(Vector.GetX() != 0.0f, "Scale X must not be zero");
            LOG_ASSERT(Vector.GetY() != 0.0f, "Scale Y must not be zero");
            LOG_ASSERT(Vector.GetZ() != 0.0f, "Scale Z must not be zero");

            mScale *= Vector;
            return (* this);
        }

        /// \brief Rotates the transform in world space by the specified quaternion.
        ///
        /// \param Rotation The quaternion representing the rotation to apply.
        /// \return A reference to this transform, modified by the rotation.
        ZYPHRYON_INLINE Ref<Transform3D> Rotate(Quaternion Rotation)
        {
            mRotation = Rotation * mRotation;
            return (* this);
        }

        /// \brief Rotates the transform in world space  by pitch and yaw (using Euler angles).
        /// 
        /// \param Angles The 2D vector containing pitch and yaw (X = pitch, Y = yaw).
        /// \return A reference to this transform, modified by the rotation.
        ZYPHRYON_INLINE Ref<Transform3D> Rotate(Vector2 Angles)
        {
            return Rotate(Quaternion::FromEulerAngles(Vector3(Angles, 0.0f)));
        }

        /// \brief Rotates the transform in world space by using Euler angles.
        /// 
        /// \param Angles The 3D vector containing Euler angles for rotation.
        /// \return A reference to this transform, modified by the rotation.
        ZYPHRYON_INLINE Ref<Transform3D> Rotate(Vector3 Angles)
        {
            return Rotate(Quaternion::FromEulerAngles(Angles));
        }

        /// \brief Rotates the transform in world space around a specified axis by a given angle.
        /// 
        /// \param Rotation The rotation angle in radians.
        /// \param Axis     The axis around which to rotate (representing the direction of the axis).
        /// 
        /// \return A reference to this transform, modified by the rotation.
        ZYPHRYON_INLINE Ref<Transform3D> Rotate(Angle Rotation, Vector3 Axis)
        {
            return Rotate(Quaternion::FromAngles(Rotation, Axis));
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

        Vector3    mTranslation;
        Vector3    mScale;
        Quaternion mRotation;
    };
}