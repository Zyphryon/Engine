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
    class Pivot final
    {
    public:

        /// \brief Constructs a pivot point at the origin (0.0, 0.0).
        ///
        /// \remarks This represents the top-left corner in normalized coordinates.
        ZYPHRYON_INLINE constexpr Pivot()
            : mX { 0.0f },
              mY { 0.0f }
        {
        }

        /// \brief Constructs a pivot point with specified normalized coordinates.
        ///
        /// \param X The normalized X coordinate (0.0 to 1.0).
        /// \param Y The normalized Y coordinate (0.0 to 1.0).
        ZYPHRYON_INLINE constexpr Pivot(Real32 X, Real32 Y)
            : mX { X },
              mY { Y }
        {
        }

        /// \brief Retrieves the normalized X coordinate of the pivot point.
        ///
        /// \return The X coordinate in the range [0.0, 1.0].
        ZYPHRYON_INLINE constexpr Real32 GetX() const
        {
            return mX;
        }

        /// \brief Retrieves the normalized Y coordinate of the pivot point.
        ///
        /// \return The Y coordinate in the range [0.0, 1.0].
        ZYPHRYON_INLINE constexpr Real32 GetY() const
        {
            return mY;
        }

        /// \brief Serializes the state of the pivot point to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mX);
            Archive.SerializeObject(mY);
        }

    public:

        /// \brief Predefined pivot point at the top-left corner (0.0, 0.0).
        ///
        /// \return A pivot point representing the top-left corner.
        ZYPHRYON_INLINE static constexpr Pivot LeftTop()
        {
            return Pivot(0.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the middle-left edge (0.0, 0.5).
        ///
        /// \return A pivot point representing the middle-left edge.
        ZYPHRYON_INLINE static constexpr Pivot LeftMiddle()
        {
            return Pivot(0.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the bottom-left corner (0.0, 1.0).
        ///
        /// \return A pivot point representing the bottom-left corner.
        ZYPHRYON_INLINE static constexpr Pivot LeftBottom()
        {
            return Pivot(0.0f, 0.0f);
        }

        /// \brief Predefined pivot point at the top-center edge (0.5, 0.0).
        ///
        /// \return A pivot point representing the top-center edge.
        ZYPHRYON_INLINE static constexpr Pivot CenterTop()
        {
            return Pivot(0.5f, 1.0f);
        }

        /// \brief Predefined pivot point at the center (0.5, 0.5).
        ///
        /// \return A pivot point representing the center.
        ZYPHRYON_INLINE static constexpr Pivot CenterMiddle()
        {
            return Pivot(0.5f, 0.5f);
        }

        /// \brief Predefined pivot point at the bottom-center edge (0.5, 1.0).
        ///
        /// \return A pivot point representing the bottom-center edge.
        ZYPHRYON_INLINE static constexpr Pivot CenterBottom()
        {
            return Pivot(0.5f, 0.0f);
        }

        /// \brief Predefined pivot point at the top-right corner (1.0, 0.0).
        ///
        /// \return A pivot point representing the top-right corner.
        ZYPHRYON_INLINE static constexpr Pivot RightTop()
        {
            return Pivot(1.0f, 1.0f);
        }

        /// \brief Predefined pivot point at the middle-right edge (1.0, 0.5).
        ///
        /// \return A pivot point representing the middle-right edge.
        ZYPHRYON_INLINE static constexpr Pivot RightMiddle()
        {
            return Pivot(1.0f, 0.5f);
        }

        /// \brief Predefined pivot point at the bottom-right corner (1.0, 1.0).
        ///
        /// \return A pivot point representing the bottom-right corner.
        ZYPHRYON_INLINE static constexpr Pivot RightBottom()
        {
            return Pivot(1.0f, 0.0f);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real32 mX;
        Real32 mY;
    };
}