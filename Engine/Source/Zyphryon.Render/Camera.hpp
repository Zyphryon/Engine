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

#include "Zyphryon.Graphic/Types.hpp"
#include "Zyphryon.Math/Geometry/Frustum.hpp"
#include "Zyphryon.Math/Transform.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents a camera: where it stands, how it projects, and the matrices both derive.
    class ZY_API Camera final
    {
    public:

        /// \brief Constructs a camera at the origin with an identity projection.
        ZY_INLINE Camera()
            : mDirty { true }
        {
        }

        /// \brief Derives the view, the view-projection, its inverse and the frustum, if anything changed.
        ///
        /// \return `true` if the matrices were derived again, `false` if nothing changed since the last call.
        Bool Compute();

        /// \brief Sets where the camera stands and how it is turned.
        ///
        /// \param Transform The camera's placement in world space, whose inverse is the view.
        ZY_INLINE void SetTransform(ConstRef<Transform> Transform)
        {
            mTransform = Transform;
            mDirty     = true;
        }

        /// \brief Gets where the camera stands and how it is turned.
        ///
        /// \return The camera's placement in world space.
        ZY_INLINE ConstRef<Transform> GetTransform() const
        {
            return mTransform;
        }

        /// \brief Sets the projection matrix of the camera.
        ///
        /// \param Matrix The projection, built with \ref Matrix4x4::CreatePerspective or \ref Matrix4x4::CreateOrthographic.
        ZY_INLINE void SetProjection(ConstRef<Matrix4x4> Matrix)
        {
            mProjection = Matrix;
            mDirty      = true;
        }

        /// \brief Gets the projection matrix of the camera.
        ///
        /// \return The projection matrix.
        ZY_INLINE ConstRef<Matrix4x4> GetProjection() const
        {
            return mProjection;
        }

        /// \brief Gets the view matrix, the inverse of the camera's transform.
        ///
        /// \return The view matrix as of the last \ref Compute.
        ZY_INLINE ConstRef<Matrix4x3> GetView() const
        {
            return mView;
        }

        /// \brief Gets the view-projection matrix of the camera.
        ///
        /// \return The view-projection matrix as of the last \ref Compute.
        ZY_INLINE ConstRef<Matrix4x4> GetViewProjection() const
        {
            return mViewProjection;
        }

        /// \brief Gets the inverse of the view-projection matrix.
        ///
        /// \return The inverse view-projection matrix as of the last \ref Compute.
        ZY_INLINE ConstRef<Matrix4x4> GetViewProjectionInverse() const
        {
            return mViewProjectionInverse;
        }

        /// \brief Gets the frustum the view-projection closes, for testing what the camera sees.
        ///
        /// \return The frustum as of the last \ref Compute.
        ZY_INLINE ConstRef<Frustum> GetFrustum() const
        {
            return mFrustum;
        }

        /// \brief Transforms a screen-space position into the world-space position it was projected from.
        ///
        /// \param Position The point on screen, in pixels from the viewport's top-left corner, with its depth.
        /// \param Viewport The viewport the point lies in, including its depth range.
        /// \return The world-space position.
        ZY_INLINE Vector3 GetWorldCoordinates(Vector3 Position, ConstRef<ZyGraphic::Viewport> Viewport) const
        {
            ZY_ASSERT(Viewport.Width > 0 && Viewport.Height > 0, "Invalid viewport size");
            ZY_ASSERT(!IsAlmostZero(Viewport.MaxDepth - Viewport.MinDepth), "Invalid depth range");

            const Real32 X = (Position.GetX() - Viewport.X) / Viewport.Width  * 2.0f - 1.0f;
            const Real32 Y = 1.0f - (Position.GetY() - Viewport.Y) / Viewport.Height * 2.0f;
            const Real32 Z = (Position.GetZ() - Viewport.MinDepth) / (Viewport.MaxDepth - Viewport.MinDepth);

            return Matrix4x4::Project(mViewProjectionInverse, Vector3(X, Y, Z));
        }

        /// \brief Transforms a world-space position into the screen-space position it projects to.
        ///
        /// \param Position The point in world space.
        /// \param Viewport The viewport to project into, including its depth range.
        /// \return The point on screen, in pixels from the viewport's top-left corner, with its depth.
        ZY_INLINE Vector3 GetScreenCoordinates(Vector3 Position, ConstRef<ZyGraphic::Viewport> Viewport) const
        {
            ZY_ASSERT(Viewport.Width > 0 && Viewport.Height > 0, "Invalid viewport size");
            ZY_ASSERT(!IsAlmostZero(Viewport.MaxDepth - Viewport.MinDepth), "Invalid depth range");

            const Vector3 Point = Matrix4x4::Project(mViewProjection, Position);

            const Real32 X = Viewport.X + Viewport.Width  * (Point.GetX() + 1.0f) * 0.5f;
            const Real32 Y = Viewport.Y + Viewport.Height * (1.0f - Point.GetY()) * 0.5f;
            const Real32 Z = Point.GetZ() * (Viewport.MaxDepth - Viewport.MinDepth) + Viewport.MinDepth;

            return Vector3(X, Y, Z);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bool      mDirty;
        Transform mTransform;
        Matrix4x4 mProjection;
        Matrix4x3 mView;
        Matrix4x4 mViewProjection;
        Matrix4x4 mViewProjectionInverse;
        Frustum   mFrustum;
    };
}
