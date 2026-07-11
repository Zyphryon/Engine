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
    /// \brief Represents a pivot point in normalized coordinates for 2D transformations.
    class Pivot2D final
    {
    public:

        /// \brief Constructs a pivot point at the origin (0.0, 0.0).
        ///
        /// \remarks This represents the top-left corner in normalized coordinates.
        ZY_INLINE constexpr Pivot2D()
            : mX { 0.0f },
              mY { 0.0f }
        {
        }

        /// \brief Constructs a pivot point with specified normalized coordinates.
        ///
        /// \param X The normalized X coordinate (0.0 to 1.0).
        /// \param Y The normalized Y coordinate (0.0 to 1.0).
        ZY_INLINE constexpr Pivot2D(Real32 X, Real32 Y)
            : mX { X },
              mY { Y }
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

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<5> kPattern("({0}, {1})");
            Format::Processor<Output>::Format(Buffer, kPattern, mX, mY);
        }

    public:

        /// \brief Predefined pivot point at the top-left corner (0.0, 1.0).
        ///
        /// \return A pivot point representing the top-left corner.
        ZY_INLINE static constexpr Pivot2D LeftTop()
        {
            return Pivot2D(0.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the middle-left edge (0.0, 0.5).
        ///
        /// \return A pivot point representing the middle-left edge.
        ZY_INLINE static constexpr Pivot2D LeftMiddle()
        {
            return Pivot2D(0.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the bottom-left corner (0.0, 0.0).
        ///
        /// \return A pivot point representing the bottom-left corner.
        ZY_INLINE static constexpr Pivot2D LeftBottom()
        {
            return Pivot2D(0.0f, 0.0f);
        }

        /// \brief Predefined pivot point at the top-center edge (0.5, 1.0).
        ///
        /// \return A pivot point representing the top-center edge.
        ZY_INLINE static constexpr Pivot2D CenterTop()
        {
            return Pivot2D(0.5f, 1.0f);
        }

        /// \brief Predefined pivot point at the center (0.5, 0.5).
        ///
        /// \return A pivot point representing the center.
        ZY_INLINE static constexpr Pivot2D CenterMiddle()
        {
            return Pivot2D(0.5f, 0.5f);
        }

        /// \brief Predefined pivot point at the bottom-center edge (0.5, 0.0).
        ///
        /// \return A pivot point representing the bottom-center edge.
        ZY_INLINE static constexpr Pivot2D CenterBottom()
        {
            return Pivot2D(0.5f, 0.0f);
        }

        /// \brief Predefined pivot point at the top-right corner (1.0, 1.0).
        ///
        /// \return A pivot point representing the top-right corner.
        ZY_INLINE static constexpr Pivot2D RightTop()
        {
            return Pivot2D(1.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the middle-right edge (1.0, 0.5).
        ///
        /// \return A pivot point representing the middle-right edge.
        ZY_INLINE static constexpr Pivot2D RightMiddle()
        {
            return Pivot2D(1.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the bottom-right corner (1.0, 0.0).
        ///
        /// \return A pivot point representing the bottom-right corner.
        ZY_INLINE static constexpr Pivot2D RightBottom()
        {
            return Pivot2D(1.0f, 0.0f);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real32 mX;
        Real32 mY;
    };
}