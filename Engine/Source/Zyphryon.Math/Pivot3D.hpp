// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a pivot point in normalized coordinates for 3D transformations.
    class Pivot3D final
    {
    public:

        /// \brief Constructs a pivot point at the origin (0.0, 0.0, 0.0).
        ///
        /// \remarks This represents the bottom-left-back corner in normalized coordinates.
        ZY_INLINE constexpr Pivot3D()
            : mX { 0.0f },
              mY { 0.0f },
              mZ { 0.0f }
        {
        }

        /// \brief Constructs a pivot point with specified normalized coordinates.
        ///
        /// \param X The normalized X coordinate (0.0 to 1.0).
        /// \param Y The normalized Y coordinate (0.0 to 1.0).
        /// \param Z The normalized Z coordinate (0.0 to 1.0).
        ZY_INLINE constexpr Pivot3D(Real32 X, Real32 Y, Real32 Z)
            : mX { X },
              mY { Y },
              mZ { Z }
        {
        }

        /// \brief Gets the normalized X coordinate of the pivot point.
        ///
        /// \return The X coordinate in the range [0.0, 1.0].
        ZY_INLINE constexpr Real32 GetX() const
        {
            return mX;
        }

        /// \brief Gets the normalized Y coordinate of the pivot point.
        ///
        /// \return The Y coordinate in the range [0.0, 1.0].
        ZY_INLINE constexpr Real32 GetY() const
        {
            return mY;
        }

        /// \brief Gets the normalized Z coordinate of the pivot point.
        ///
        /// \return The Z coordinate in the range [0.0, 1.0].
        ZY_INLINE constexpr Real32 GetZ() const
        {
            return mZ;
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<7> kPattern("({0}, {1}, {2})");
            Format::Processor<Output>::Format(Buffer, kPattern, mX, mY, mZ);
        }

    public:

        /// \brief Predefined pivot point at the left-top-front corner (0.0, 1.0, 1.0).
        ///
        /// \return A pivot point representing the left-top-front corner.
        ZY_INLINE static constexpr Pivot3D LeftTopFront()
        {
            return Pivot3D(0.0f, 1.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the left-top-middle edge (0.0, 1.0, 0.5).
        ///
        /// \return A pivot point representing the left-top-middle edge.
        ZY_INLINE static constexpr Pivot3D LeftTopMiddle()
        {
            return Pivot3D(0.0f, 1.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the left-top-back corner (0.0, 1.0, 0.0).
        ///
        /// \return A pivot point representing the left-top-back corner.
        ZY_INLINE static constexpr Pivot3D LeftTopBack()
        {
            return Pivot3D(0.0f, 1.0f, 0.0f);
        }

        /// \brief Predefined pivot point at the left-middle-front edge (0.0, 0.5, 1.0).
        ///
        /// \return A pivot point representing the left-middle-front edge.
        ZY_INLINE static constexpr Pivot3D LeftMiddleFront()
        {
            return Pivot3D(0.0f, 0.5f, 1.0f);
        }

        /// \brief Predefined pivot point at the left-middle-middle center (0.0, 0.5, 0.5).
        ///
        /// \return A pivot point representing the left-middle-middle center.
        ZY_INLINE static constexpr Pivot3D LeftMiddleMiddle()
        {
            return Pivot3D(0.0f, 0.5f, 0.5f);
        }

        /// \brief Predefined pivot point at the left-middle-back edge (0.0, 0.5, 0.0).
        ///
        /// \return A pivot point representing the left-middle-back edge.
        ZY_INLINE static constexpr Pivot3D LeftMiddleBack()
        {
            return Pivot3D(0.0f, 0.5f, 0.0f);
        }

        /// \brief Predefined pivot point at the left-bottom-front corner (0.0, 0.0, 1.0).
        ///
        /// \return A pivot point representing the left-bottom-front corner.
        ZY_INLINE static constexpr Pivot3D LeftBottomFront()
        {
            return Pivot3D(0.0f, 0.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the left-bottom-middle edge (0.0, 0.0, 0.5).
        ///
        /// \return A pivot point representing the left-bottom-middle edge.
        ZY_INLINE static constexpr Pivot3D LeftBottomMiddle()
        {
            return Pivot3D(0.0f, 0.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the left-bottom-back corner (0.0, 0.0, 0.0).
        ///
        /// \return A pivot point representing the left-bottom-back corner.
        ZY_INLINE static constexpr Pivot3D LeftBottomBack()
        {
            return Pivot3D(0.0f, 0.0f, 0.0f);
        }

        /// \brief Predefined pivot point at the center-top-front edge (0.5, 1.0, 1.0).
        ///
        /// \return A pivot point representing the center-top-front edge.
        ZY_INLINE static constexpr Pivot3D CenterTopFront()
        {
            return Pivot3D(0.5f, 1.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the center-top-middle face (0.5, 1.0, 0.5).
        ///
        /// \return A pivot point representing the center-top-middle face.
        ZY_INLINE static constexpr Pivot3D CenterTopMiddle()
        {
            return Pivot3D(0.5f, 1.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the center-top-back edge (0.5, 1.0, 0.0).
        ///
        /// \return A pivot point representing the center-top-back edge.
        ZY_INLINE static constexpr Pivot3D CenterTopBack()
        {
            return Pivot3D(0.5f, 1.0f, 0.0f);
        }

        /// \brief Predefined pivot point at the center-middle-front face (0.5, 0.5, 1.0).
        ///
        /// \return A pivot point representing the center-middle-front face.
        ZY_INLINE static constexpr Pivot3D CenterMiddleFront()
        {
            return Pivot3D(0.5f, 0.5f, 1.0f);
        }

        /// \brief Predefined pivot point at the center (0.5, 0.5, 0.5).
        ///
        /// \return A pivot point representing the center.
        ZY_INLINE static constexpr Pivot3D CenterMiddleMiddle()
        {
            return Pivot3D(0.5f, 0.5f, 0.5f);
        }

        /// \brief Predefined pivot point at the center-middle-back face (0.5, 0.5, 0.0).
        ///
        /// \return A pivot point representing the center-middle-back face.
        ZY_INLINE static constexpr Pivot3D CenterMiddleBack()
        {
            return Pivot3D(0.5f, 0.5f, 0.0f);
        }

        /// \brief Predefined pivot point at the center-bottom-front edge (0.5, 0.0, 1.0).
        ///
        /// \return A pivot point representing the center-bottom-front edge.
        ZY_INLINE static constexpr Pivot3D CenterBottomFront()
        {
            return Pivot3D(0.5f, 0.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the center-bottom-middle face (0.5, 0.0, 0.5).
        ///
        /// \return A pivot point representing the center-bottom-middle face.
        ZY_INLINE static constexpr Pivot3D CenterBottomMiddle()
        {
            return Pivot3D(0.5f, 0.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the center-bottom-back edge (0.5, 0.0, 0.0).
        ///
        /// \return A pivot point representing the center-bottom-back edge.
        ZY_INLINE static constexpr Pivot3D CenterBottomBack()
        {
            return Pivot3D(0.5f, 0.0f, 0.0f);
        }

        /// \brief Predefined pivot point at the right-top-front corner (1.0, 1.0, 1.0).
        ///
        /// \return A pivot point representing the right-top-front corner.
        ZY_INLINE static constexpr Pivot3D RightTopFront()
        {
            return Pivot3D(1.0f, 1.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the right-top-middle edge (1.0, 1.0, 0.5).
        ///
        /// \return A pivot point representing the right-top-middle edge.
        ZY_INLINE static constexpr Pivot3D RightTopMiddle()
        {
            return Pivot3D(1.0f, 1.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the right-top-back corner (1.0, 1.0, 0.0).
        ///
        /// \return A pivot point representing the right-top-back corner.
        ZY_INLINE static constexpr Pivot3D RightTopBack()
        {
            return Pivot3D(1.0f, 1.0f, 0.0f);
        }

        /// \brief Predefined pivot point at the right-middle-front edge (1.0, 0.5, 1.0).
        ///
        /// \return A pivot point representing the right-middle-front edge.
        ZY_INLINE static constexpr Pivot3D RightMiddleFront()
        {
            return Pivot3D(1.0f, 0.5f, 1.0f);
        }

        /// \brief Predefined pivot point at the right-middle-middle center (1.0, 0.5, 0.5).
        ///
        /// \return A pivot point representing the right-middle-middle center.
        ZY_INLINE static constexpr Pivot3D RightMiddleMiddle()
        {
            return Pivot3D(1.0f, 0.5f, 0.5f);
        }

        /// \brief Predefined pivot point at the right-middle-back edge (1.0, 0.5, 0.0).
        ///
        /// \return A pivot point representing the right-middle-back edge.
        ZY_INLINE static constexpr Pivot3D RightMiddleBack()
        {
            return Pivot3D(1.0f, 0.5f, 0.0f);
        }

        /// \brief Predefined pivot point at the right-bottom-front corner (1.0, 0.0, 1.0).
        ///
        /// \return A pivot point representing the right-bottom-front corner.
        ZY_INLINE static constexpr Pivot3D RightBottomFront()
        {
            return Pivot3D(1.0f, 0.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the right-bottom-middle edge (1.0, 0.0, 0.5).
        ///
        /// \return A pivot point representing the right-bottom-middle edge.
        ZY_INLINE static constexpr Pivot3D RightBottomMiddle()
        {
            return Pivot3D(1.0f, 0.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the right-bottom-back corner (1.0, 0.0, 0.0).
        ///
        /// \return A pivot point representing the right-bottom-back corner.
        ZY_INLINE static constexpr Pivot3D RightBottomBack()
        {
            return Pivot3D(1.0f, 0.0f, 0.0f);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real32 mX;
        Real32 mY;
        Real32 mZ;
    };
}