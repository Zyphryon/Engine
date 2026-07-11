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
        ZY_INLINE Transform2D()
            : mScale { Vector2::One() }
        {
        }

        /// \brief Copy constructor for Transform2D.
        ZY_INLINE Transform2D(ConstRef<Transform2D> Other) = default;

        /// \brief Constructs a transform with the specified translation, scale, and rotation.
        ///
        /// \param Translation The translation vector to set (2D).
        /// \param Scale       The scale vector to set (2D).
        /// \param Rotation    The rotation to set, represented as an angle in radians.
        ZY_INLINE Transform2D(Vector2 Translation, Vector2 Scale, Angle Rotation)
        {
            SetTranslation(Translation);
            SetScale(Scale);
            SetRotation(Rotation);
        }

        /// \brief Constructs a transform with the specified translation and rotation.
        ///
        /// \param Translation The translation vector to set (2D).
        /// \param Rotation    The rotation to set, represented as an angle in radians.
        ZY_INLINE Transform2D(Vector2 Translation, Angle Rotation)
            : Transform2D()
        {
            SetTranslation(Translation);
            SetRotation(Rotation);
        }

        /// \brief Constructs a transform with the specified translation.
        ///
        /// \param Translation The translation vector to set (2D).
        ZY_INLINE explicit Transform2D(Vector2 Translation)
            : Transform2D()
        {
            SetTranslation(Translation);
        }

        /// \brief Computes the final transformation matrix with an origin point.
        ///
        /// \param Origin The center point for rotation, scale, and skew operations.
        /// \return A 3x2 transformation matrix that combines translation, scale, rotation and skew with the specified origin.
        ZY_INLINE Matrix3x2 Compute(Vector2 Origin = Vector2()) const
        {
            return Matrix3x2::FromTransform(Origin, mTranslation, mScale, mRotation);
        }

        /// \brief Sets the translation vector for the transform.
        ///
        /// \param Translation The 2D translation vector.
        /// \return A reference to this transform, modified by the new translation.
        ZY_INLINE Ref<Transform2D> SetTranslation(Vector2 Translation)
        {
            mTranslation = Translation;
            return (* this);
        }

        /// \brief Gets the current translation of the transform.
        ///
        /// \return The current 2D translation vector.
        ZY_INLINE Vector2 GetTranslation() const
        {
            return mTranslation;
        }

        /// \brief Sets the scale vector for the transform.
        ///
        /// \param Scale The 2D scale vector.
        /// \return A reference to this transform, modified by the new scale.
        ZY_INLINE Ref<Transform2D> SetScale(Vector2 Scale)
        {
            mScale = Scale;
            return (* this);
        }

        /// \brief Gets the current scale of the transform.
        ///
        /// \return The current 2D scale vector.
        ZY_INLINE Vector2 GetScale() const
        {
            return mScale;
        }

        /// \brief Sets the rotation for the transform.
        ///
        /// \param Rotation The rotation angle.
        /// \return A reference to this transform, modified by the new rotation.
        ZY_INLINE Ref<Transform2D> SetRotation(Angle Rotation)
        {
            mRotation = Rotation;
            return (* this);
        }

        /// \brief Gets the current rotation of the transform.
        ///
        /// \return The rotation of the transform as an angle.
        ZY_INLINE Angle GetRotation() const
        {
            return mRotation;
        }

        /// \brief Translates the transform by the given 2D vector.
        ///
        /// \param Translation The 2D translation vector to apply.
        /// \return A reference to this transform, modified by the translation.
        ZY_INLINE Ref<Transform2D> Translate(Vector2 Translation)
        {
            mTranslation += Translation;
            return (* this);
        }

        /// \brief Scales the transform uniformly by a scalar value.
        ///
        /// \param Scalar The scalar value to apply as a scale factor.
        /// \return A reference to this transform, modified by the scale.
        ZY_INLINE Ref<Transform2D> Scale(Real32 Scalar)
        {
            ZY_ASSERT(Scalar != 0.0f, "Scale factor must not be zero");

            mScale *= Scalar;
            return (* this);
        }

        /// \brief Scales the transform non-uniformly by a 2D vector.
        ///
        /// \param Vector The 2D scale factors to apply.
        /// \return A reference to this transform, modified by the scale.
        ZY_INLINE Ref<Transform2D> Scale(Vector2 Vector)
        {
            ZY_ASSERT(Vector.GetX() != 0.0f, "Scale X must not be zero");
            ZY_ASSERT(Vector.GetY() != 0.0f, "Scale Y must not be zero");

            mScale *= Vector;
            return (* this);
        }

        /// \brief Rotates the transform by the specified angle.
        ///
        /// \param Rotation The rotation angle to apply.
        /// \return A reference to this transform, modified by the rotation.
        ZY_INLINE Ref<Transform2D> Rotate(Angle Rotation)
        {
            mRotation += Rotation;
            return (* this);
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<7> kPattern("({0}, {1}, {2})");
            Format::Processor<Output>::Format(Buffer, kPattern, mTranslation, mScale, mRotation);
        }

    public:

        /// \brief Linearly interpolates between two transformations.
        ///
        /// \param Start      The starting transformation.
        /// \param End        The ending transformation.
        /// \param Percentage The interpolation percentage (range between 0 and 1).
        /// \return A transformation interpolated between the start and end transformations.
        ZY_INLINE static Transform2D Lerp(ConstRef<Transform2D> Start, ConstRef<Transform2D> End, Real32 Percentage)
        {
            const Vector2 Translation = ::Lerp(Start.GetTranslation(), End.GetTranslation(), Percentage);
            const Vector2 Scale       = ::Lerp(Start.GetScale(), End.GetScale(), Percentage);
            const Angle   Rotation    = ::Lerp(Start.GetRotation(), End.GetRotation(), Percentage);
            return Transform2D(Translation, Scale, Rotation);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector2 mTranslation;
        Vector2 mScale;
        Angle   mRotation;
    };
}