// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Camera.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Camera::Camera()
        : mDirty { false }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Camera::Compute()
    {
        const Bool Dirty = (mDirty > 0);

        if (Dirty)
        {
            // Recompute the view matrix if the transform was modified.
            if (HasBit(mDirty, kDirtyBitTransformation))
            {
                mView = Matrix4x4::Inverse<true>(mTransform.Compute());
            }

            // Build a new view-projection matrix from projection and view.
            mViewProjection        = mProjection * mView;

            // Derive the inverse view-projection for unprojection operations.
            mViewProjectionInverse = Matrix4x4::Inverse(mViewProjection);

            // Clear all dirty bits to mark matrices as up-to-date.
            mDirty = 0;
        }
        return Dirty;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector3 Camera::GetWorldCoordinates(ConstRef<Vector3> Position, ConstRef<Viewport> Viewport) const
    {
        const Real32 X = (Position.GetX() - Viewport.X) / Viewport.Width * 2.0f - 1.0f;
        const Real32 Y = (Viewport.Height - (Position.GetY() - Viewport.Y)) / Viewport.Height * 2.0f - 1.0f;
        const Real32 Z = (Position.GetZ() - Viewport.MinDepth) / (Viewport.MaxDepth - Viewport.MinDepth);

        return Matrix4x4::Project(mViewProjectionInverse, Vector3(X, Y, Z));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector2 Camera::GetWorldCoordinates(ConstRef<Vector2> Position, ConstRef<Viewport> Viewport) const
    {
        const Real32 X = (Position.GetX() - Viewport.X) / Viewport.Width * 2.0f - 1.0f;
        const Real32 Y = (Viewport.Height - (Position.GetY() - Viewport.Y)) / Viewport.Height * 2.0f - 1.0f;

        return Matrix4x4::Project(mViewProjectionInverse, Vector2(X, Y));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector3 Camera::GetScreenCoordinates(ConstRef<Vector3> Position, ConstRef<Viewport> Viewport) const
    {
        const Vector3 Point = Matrix4x4::Project(mViewProjection, Position);

        const Real32 X = Viewport.Width  * (Point.GetX() + 1.0f) * 0.5f + Viewport.X;
        const Real32 Y = Viewport.Height - (Viewport.Height * (Point.GetY() + 1.0f) * 0.5f + Viewport.Y);
        const Real32 Z = Point.GetZ() * (Viewport.MaxDepth - Viewport.MinDepth) + Viewport.MinDepth;

        return Vector3(X, Y, Z);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector2 Camera::GetScreenCoordinates(ConstRef<Vector2> Position, ConstRef<Viewport> Viewport) const
    {
        const Vector2 Point = Matrix4x4::Project(mViewProjection, Position);

        const Real32 X = Viewport.Width  * (Point.GetX() + 1.0f) * 0.5f + Viewport.X;
        const Real32 Y = Viewport.Height - (Viewport.Height * (Point.GetY() + 1.0f) * 0.5f + Viewport.Y);

        return Vector2(X, Y);
    }
}