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
#include "Zyphryon.Math/Transform.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a 3D camera with view and projection transforms.
    class Camera final
    {
    public:

        /// \brief Constructs a camera with identity view and projection.
        Camera();

        /// \brief Recalculates the camera's matrices if marked dirty.
        ///
        /// \return `true` if any matrix was updated, `false` otherwise.
        Bool Compute();

        /// \brief Sets the projection matrix of the camera.
        ///
        /// \param Matrix The new projection matrix.
        ZYPHRYON_INLINE void SetProjectionMatrix(ConstRef<Matrix4x4> Matrix)
        {
            mProjection = Matrix;
            mDirty      = SetBit(mDirty, kDirtyBitProjection);
        }

        /// \brief Returns the current projection of the camera.
        ///
        /// \return The projection matrix.
        ZYPHRYON_INLINE ConstRef<Matrix4x4> GetProjectionMatrix() const
        {
            return mProjection;
        }

        /// \brief Sets the view matrix of the camera.
        ///
        /// \param Matrix The new view matrix.
        ZYPHRYON_INLINE void SetView(ConstRef<Matrix4x4> Matrix)
        {
            mView  = Matrix;
            mDirty = SetBit(mDirty, kDirtyBitProjection);
        }

        /// \brief Returns the current view of the camera.
        ///
        /// \return The view matrix.
        ZYPHRYON_INLINE ConstRef<Matrix4x4> GetView() const
        {
            return mView;
        }

        /// \brief Returns the current view-projection of the camera.
        ///
        /// \return The view-projection matrix.
        ZYPHRYON_INLINE ConstRef<Matrix4x4> GetViewProjection() const
        {
            return mViewProjection;
        }

        /// \brief Returns the inverse of the view-projection matrix.
        ///
        /// \return The inverse view-projection matrix.
        ZYPHRYON_INLINE ConstRef<Matrix4x4> GetViewProjectionInverse() const
        {
            return mViewProjectionInverse;
        }

        /// \brief Gets the camera's right vector.
        ///
        /// \return The right direction vector in world space.
        ZYPHRYON_INLINE Vector3 GetRight() const
        {
            return mView.GetRight();
        }

        /// \brief Gets the camera's left vector.
        ///
        /// \return The left direction vector in world space.
        ZYPHRYON_INLINE Vector3 GetLeft() const
        {
            return mView.GetLeft();
        }

        /// \brief Gets the camera's up vector.
        ///
        /// \return The up direction vector in world space.
        ZYPHRYON_INLINE Vector3 GetUp() const
        {
            return mView.GetUp();
        }

        /// \brief Gets the camera's down vector.
        ///
        /// \return The down direction vector in world space.
        ZYPHRYON_INLINE Vector3 GetDown() const
        {
            return mView.GetDown();
        }

        /// \brief Gets the camera's forward vector.
        ///
        /// \return The forward direction vector in world space.
        ZYPHRYON_INLINE Vector3 GetForward() const
        {
            return mView.GetForward();
        }

        /// \brief Gets the camera's backward vector.
        ///
        /// \return The backward direction vector in world space.
        ZYPHRYON_INLINE Vector3 GetBackward() const
        {
            return mView.GetBackward();
        }

        /// \brief Sets the perspective projection matrix.
        ///
        /// \param Eyes   Field of view in radians.
        /// \param Aspect Aspect ratio (width / height).
        /// \param ZNear  Distance to the near clipping plane.
        /// \param ZFar   Distance to the far clipping plane.
        ZYPHRYON_INLINE void SetPerspective(Real32 Eyes, Real32 Aspect, Real32 ZNear, Real32 ZFar)
        {
            mProjection = Matrix4x4::CreatePerspective(Eyes, Aspect, ZNear, ZFar);
            mDirty = SetBit(mDirty, kDirtyBitProjection);
        }

        /// \brief Sets the orthographic projection matrix using bounds.
        ///
        /// \param Left   Left clipping plane.
        /// \param Right  Right clipping plane.
        /// \param Bottom Bottom clipping plane.
        /// \param Top    Top clipping plane.
        /// \param ZNear  Near clipping plane.
        /// \param ZFar   Far clipping plane.
        ZYPHRYON_INLINE void SetOrthographic(Real32 Left, Real32 Right, Real32 Bottom, Real32 Top, Real32 ZNear, Real32 ZFar)
        {
            mProjection = Matrix4x4::CreateOrthographic(Left, Right, Bottom, Top, ZNear, ZFar);
            mDirty = SetBit(mDirty, kDirtyBitProjection);
        }

        /// \brief Sets an orthographic projection with given dimensions.
        ///
        /// \param Width  Viewport width.
        /// \param Height Viewport height.
        /// \param ZNear  Distance to near clipping plane.
        /// \param ZFar   Distance to far clipping plane.
        ZYPHRYON_INLINE void SetOrthographic(Real32 Width, Real32 Height, Real32 ZNear, Real32 ZFar)
        {
            SetOrthographic(0, Width, Height, 0, ZNear, ZFar);
        }

        /// \brief Sets the view matrix using eye position, target focus, and up direction.
        ///
        /// \param Eye   Camera position.
        /// \param Focus Target position the camera looks at.
        /// \param Up    Upward direction for orientation.
        ZYPHRYON_INLINE void SetLook(ConstRef<Vector3> Eye, ConstRef<Vector3> Focus, ConstRef<Vector3> Up)
        {
            const Matrix4x4 Matrix = Matrix4x4::Inverse<true>(Matrix4x4::CreateLook(Eye, Focus, Up));

            mTransform.SetTranslation(Matrix.GetTranslation());
            mTransform.SetScale(Matrix.GetScale());
            mTransform.SetRotation(Matrix.GetRotation());

            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera translation.
        ///
        /// \param Translation Camera's 3D coordinate.
        ZYPHRYON_INLINE void SetTranslation(ConstRef<Vector3> Translation)
        {
            mTransform.SetTranslation(Translation);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera translation.
        ///
        /// \param Translation Camera's 2D coordinate.
        ZYPHRYON_INLINE void SetTranslation(ConstRef<Vector2> Translation)
        {
            mTransform.SetTranslation(Translation);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera translation.
        ///
        /// \param X Camera's X coordinate.
        /// \param Y Camera's Y coordinate.
        /// \param Z Camera's Z coordinate.
        ZYPHRYON_INLINE void SetTranslation(Real32 X, Real32 Y, Real32 Z)
        {
            mTransform.SetTranslation(Vector3(X, Y, Z));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera translation.
        ///
        /// \param X Camera's X coordinate.
        /// \param Y Camera's Y coordinate.
        ZYPHRYON_INLINE void SetTranslation(Real32 X, Real32 Y)
        {
            mTransform.SetTranslation(Vector2(X, Y));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Returns the current translation.
        ///
        /// \return The camera's 3D position.
        ZYPHRYON_INLINE ConstRef<Vector3> GetTranslation() const
        {
            return mTransform.GetTranslation();
        }

        /// \brief Sets the camera scale.
        ///
        /// \param Scale Camera's 3D scale factor.
        ZYPHRYON_INLINE void SetScale(ConstRef<Vector3> Scale)
        {
            mTransform.SetScale(Scale);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera scale.
        ///
        /// \param Scale Camera's 2D scale factor.
        ZYPHRYON_INLINE void SetScale(ConstRef<Vector2> Scale)
        {
            mTransform.SetScale(Scale);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera scale.
        ///
        /// \param X Camera's X scale factor.
        /// \param Y Camera's Y scale factor.
        /// \param Z Camera's Z scale factor.
        ZYPHRYON_INLINE void SetScale(Real32 X, Real32 Y, Real32 Z)
        {
            mTransform.SetScale(Vector3(X, Y, Z));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera scale.
        ///
        /// \param X Camera's X scale factor.
        /// \param Y Camera's Y scale factor.
        ZYPHRYON_INLINE void SetScale(Real32 X, Real32 Y)
        {
            mTransform.SetScale(Vector2(X, Y));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Returns the current scale factor.
        ///
        /// \return The camera's 3D scale.
        ZYPHRYON_INLINE ConstRef<Vector3> GetScale() const
        {
            return mTransform.GetScale();
        }

        /// \brief Sets the camera rotation using an angle and axis.
        ///
        /// \param Angle Rotation angle in radians.
        /// \param Axis  Rotation axis.
        ZYPHRYON_INLINE void SetRotation(Real32 Angle, ConstRef<Vector3> Axis)
        {
            mTransform.SetRotation(Quaternion::FromAngles(Angle, Axis));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera rotation using an angle and axis.
        ///
        /// \param Angle Rotation angle in radians.
        /// \param Axis  2D axis (X, Y).
        ZYPHRYON_INLINE void SetRotation(Real32 Angle, ConstRef<Vector2> Axis)
        {
            mTransform.SetRotation(Quaternion::FromAngles(Angle, Vector3(Axis.GetX(), Axis.GetY(), 0)));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera rotation using Euler angles.
        ///
        /// \param Angles Euler angles in radians (pitch, yaw, roll).
        ZYPHRYON_INLINE void SetRotation(ConstRef<Vector3> Angles)
        {
            mTransform.SetRotation(Quaternion::FromEulerAngles(Angles));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera rotation using Euler angles.
        ///
        /// \param Pitch The pitch angle in radians.
        /// \param Yaw   The yaw angle in radians.
        /// \param Roll  The roll angle in radians.
        ZYPHRYON_INLINE void SetRotation(Real32 Pitch, Real32 Yaw, Real32 Roll)
        {
            mTransform.SetRotation(Quaternion::FromEulerAngles(Vector3(Pitch, Yaw, Roll)));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera rotation from a direction vector and up vector.
        ///
        /// \param Direction Forward direction.
        /// \param Up        Upward vector.
        ZYPHRYON_INLINE void SetRotation(ConstRef<Vector3> Direction, ConstRef<Vector3> Up)
        {
            mTransform.SetRotation(Quaternion::FromDirection(Direction, Up));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera rotation from look-at vectors.
        ///
        /// \param Eye   Camera position.
        /// \param Focus Look-at target.
        /// \param Up    Upward vector.
        ZYPHRYON_INLINE void SetRotation(ConstRef<Vector3> Eye, ConstRef<Vector3> Focus, ConstRef<Vector3> Up)
        {
            mTransform.SetRotation(Quaternion::FromDirection(Focus - Eye, Up));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Sets the camera rotation from a quaternion rotation.
        ///
        /// \param Rotation A unit quaternion representing the desired orientation.
        ZYPHRYON_INLINE void SetRotation(ConstRef<Quaternion> Rotation)
        {
            mTransform.SetRotation(Rotation);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Returns the current rotation.
        ///
        /// \return The camera's orientation.
        ZYPHRYON_INLINE ConstRef<Quaternion> GetRotation() const
        {
            return mTransform.GetRotation();
        }

        /// \brief Applies a relative translation to the camera.
        ///
        /// \param Translation 3D vector to translate.
        ZYPHRYON_INLINE void Translate(ConstRef<Vector3> Translation)
        {
            mTransform.Translate(Translation);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies a relative translation to the camera.
        ///
        /// \param Translation 2D vector to translate.
        ZYPHRYON_INLINE void Translate(ConstRef<Vector2> Translation)
        {
            mTransform.Translate(Translation);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies a relative translation to the camera.
        ///
        /// \param X X offset.
        /// \param Y Y offset.
        /// \param Z Z offset.
        ZYPHRYON_INLINE void Translate(Real32 X, Real32 Y, Real32 Z)
        {
            mTransform.Translate(Vector3(X, Y, Z));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies a relative translation to the camera.
        ///
        /// \param X X offset.
        /// \param Y Y offset.
        ZYPHRYON_INLINE void Translate(Real32 X, Real32 Y)
        {
            mTransform.Translate(Vector2(X, Y));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies a relative scaling to the camera.
        ///
        /// \param Scale 3D scale factor.
        ZYPHRYON_INLINE void Scale(ConstRef<Vector3> Scale)
        {
            mTransform.Scale(Scale);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies a relative scaling to the camera.
        ///
        /// \param Scale 2D scale factor.
        ZYPHRYON_INLINE void Scale(ConstRef<Vector2> Scale)
        {
            mTransform.Scale(Scale);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies a relative scaling to the camera.
        ///
        /// \param X Scale on X-axis.
        /// \param Y Scale on Y-axis.
        /// \param Z Scale on Z-axis.
        ZYPHRYON_INLINE void Scale(Real32 X, Real32 Y, Real32 Z)
        {
            mTransform.Scale(Vector3(X, Y, Z));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies a relative scaling to the camera.
        ///
        /// \param X Scale on X-axis.
        /// \param Y Scale on Y-axis.
        ZYPHRYON_INLINE void Scale(Real32 X, Real32 Y)
        {
            mTransform.Scale(Vector2(X, Y));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies a relative rotation to the camera.
        ///
        /// \param Angle Rotation angle.
        /// \param Axis  3D axis of rotation.
        ZYPHRYON_INLINE void Rotate(Real32 Angle, ConstRef<Vector3> Axis)
        {
            mTransform.Rotate(Angle, Axis);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies a relative rotation to the camera.
        ///
        /// \param Angle Rotation angle.
        /// \param Axis  2D axis.
        ZYPHRYON_INLINE void Rotate(Real32 Angle, ConstRef<Vector2> Axis)
        {
            mTransform.Rotate(Angle, Vector3(Axis.GetX(), Axis.GetY(), 0));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies relative Euler angle rotation.
        ///
        /// \param Angles Pitch, Yaw, Roll angles.
        ZYPHRYON_INLINE void Rotate(ConstRef<Vector3> Angles)
        {
            mTransform.Rotate(Angles);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies relative Euler angle rotation.
        ///
        /// \param Pitch The pitch angle in radians.
        /// \param Yaw   The yaw angle in radians.
        /// \param Roll  The roll angle in radians.
        ZYPHRYON_INLINE void Rotate(Real32 Pitch, Real32 Yaw, Real32 Roll)
        {
            mTransform.Rotate(Vector3(Pitch, Yaw, Roll));
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Applies relative Euler angle rotation.
        ///
        /// \param Angles 2D rotation angles (X, Y).
        ZYPHRYON_INLINE void Rotate(ConstRef<Vector2> Angles)
        {
            mTransform.Rotate(Angles);
            mDirty = SetBit(mDirty, kDirtyBitTransformation);
        }

        /// \brief Transforms a screen-space coordinates into world-space position.
        ///
        /// \param Position The 3D point in screen space to transform.
        /// \param Viewport The viewport definition, including dimensions and depth range.
        /// \return The reconstructed world-space position.
        template<Coordinates Origin = Coordinates::Southwest>
        Vector3 GetWorldCoordinates(ConstRef<Vector3> Position, ConstRef<Viewport> Viewport) const
        {
            LOG_ASSERT(Viewport.Width > 0 && Viewport.Height > 0, "Invalid viewport size");
            LOG_ASSERT(!Base::IsAlmostZero(Viewport.MaxDepth - Viewport.MinDepth), "Invalid depth range");

            const Real32 X = (Position.GetX() - Viewport.X) / Viewport.Width * 2.0f - 1.0f;
            const Real32 Y = ScreenYToNormalizedDeviceY<Origin>(Position.GetY(), Viewport);
            const Real32 Z = (Position.GetZ() - Viewport.MinDepth) / (Viewport.MaxDepth - Viewport.MinDepth);

            return Matrix4x4::Project<false>(mViewProjectionInverse, Vector3(X, Y, Z));
        }

        /// \brief Transforms a screen-space coordinates into world-space position.
        ///
        /// \param Position The 2D point in screen space to transform.
        /// \param Viewport The viewport definition, including dimensions and depth range.
        /// \return The reconstructed world-space position.
        template<Coordinates Origin = Coordinates::Southwest>
        Vector2 GetWorldCoordinates(ConstRef<Vector2> Position, ConstRef<Viewport> Viewport) const
        {
            LOG_ASSERT(Viewport.Width > 0 && Viewport.Height > 0, "Invalid viewport size");
            LOG_ASSERT(!Base::IsAlmostZero(Viewport.MaxDepth - Viewport.MinDepth), "Invalid depth range");

            const Real32 X = (Position.GetX() - Viewport.X) / Viewport.Width * 2.0f - 1.0f;
            const Real32 Y = ScreenYToNormalizedDeviceY<Origin>(Position.GetY(), Viewport);

            return Matrix4x4::Project<false>(mViewProjectionInverse, Vector2(X, Y));
        }

        /// \brief Transforms a world-space position into screen-space coordinates.
        ///
        /// \param Position The 3D point in world space to transform.
        /// \param Viewport The viewport definition, including dimensions and depth range.
        /// \return The screen-space position, where X and Y are in pixel coordinates and Z is in depth range.
        template<Coordinates Origin = Coordinates::Southwest>
        Vector3 GetScreenCoordinates(ConstRef<Vector3> Position, ConstRef<Viewport> Viewport) const
        {
            LOG_ASSERT(Viewport.Width > 0 && Viewport.Height > 0, "Invalid viewport size");
            LOG_ASSERT(!Base::IsAlmostZero(Viewport.MaxDepth - Viewport.MinDepth), "Invalid depth range");

            const Vector3 Point = Matrix4x4::Project<false>(mViewProjection, Position);

            const Real32 X = Viewport.Width  * (Point.GetX() + 1.0f) * 0.5f + Viewport.X;
            const Real32 Y = NormalizedDeviceYToScreenY<Origin>(Position.GetY(), Viewport);
            const Real32 Z = Point.GetZ() * (Viewport.MaxDepth - Viewport.MinDepth) + Viewport.MinDepth;

            return Vector3(X, Y, Z);
        }

        /// \brief Transforms a world-space position into screen-space coordinates.
        ///
        /// \param Position The 2D point in world space to transform.
        /// \param Viewport The viewport definition, including dimensions and depth range.
        /// \return The screen-space position, where X and Y are in pixel coordinates.
        template<Coordinates Origin = Coordinates::Southwest>
        Vector2 GetScreenCoordinates(ConstRef<Vector2> Position, ConstRef<Viewport> Viewport) const
        {
            LOG_ASSERT(Viewport.Width > 0 && Viewport.Height > 0, "Invalid viewport size");
            LOG_ASSERT(!Base::IsAlmostZero(Viewport.MaxDepth - Viewport.MinDepth), "Invalid depth range");

            const Vector2 Point = Matrix4x4::Project<false>(mViewProjection, Position);

            const Real32 X = Viewport.Width  * (Point.GetX() + 1.0f) * 0.5f + Viewport.X;
            const Real32 Y = NormalizedDeviceYToScreenY<Origin>(Position.GetY(), Viewport);

            return Vector2(X, Y);
        }

    private:

        static constexpr UInt32 kDirtyBitTransformation = 1 << 0;
        static constexpr UInt32 kDirtyBitProjection     = 1 << 1;

        /// \brief Converts a screen-space Y coordinate to a normalized device Y coordinate.
        ///
        /// \param Y        The screen-space Y coordinate to transform.
        /// \param Viewport The viewport definition providing the origin offset and dimensions.
        /// \return The corresponding Y coordinate in normalized device space `[-1.0, 1.0]`.
        template<Coordinates Origin>
        ZYPHRYON_INLINE static constexpr Real32 ScreenYToNormalizedDeviceY(Real32 Y, ConstRef<Viewport> Viewport)
        {
            if constexpr(Origin == Coordinates::Northwest)
            {
                return (Viewport.Height - (Y - Viewport.Y)) / Viewport.Height * 2.0f - 1.0f;
            }
            else
            {
                return ((Y - Viewport.Y) / Viewport.Height ) * 2.0f - 1.0f;
            }
        }

        /// \brief Converts a normalized device Y coordinate to a screen-space Y coordinate.
        ///
        /// \param Y        The normalized device Y coordinate `[-1.0, 1.0]` to transform.
        /// \param Viewport The viewport definition providing the origin offset and dimensions.
        /// \return The corresponding Y coordinate in screen space.
        template<Coordinates Origin>
        ZYPHRYON_INLINE static constexpr Real32 NormalizedDeviceYToScreenY(Real32 Y, ConstRef<Viewport> Viewport)
        {
            if constexpr(Origin == Coordinates::Northwest)
            {
                return Viewport.Height - (Viewport.Height * (Y + 1.0f) * 0.5f + Viewport.Y);
            }
            else
            {
                return Viewport.Height * (Y + 1.0f) * 0.5f + Viewport.Y;
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32    mDirty;
        Matrix4x4 mProjection;
        Matrix4x4 mView;
        Matrix4x4 mViewProjection;
        Matrix4x4 mViewProjectionInverse;
        Transform mTransform;
    };
}