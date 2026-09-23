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

#include "Zyphryon.Math/Geometry/Rect.hpp"
#include "Zyphryon.Math/Motion/Flipbook.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents a flipbook clip cut from a sprite sheet, drawn in one or more facing directions.
    class ZY_API Animation2D final
    {
    public:

        /// \brief Specifies how a clip plays once started.
        enum class Status : UInt8
        {
            Forward,   ///< Plays once from the first frame to the last, then holds.
            Backward,  ///< Plays once from the last frame to the first, then holds.
            Repeat,    ///< Loops from the first frame to the last.
            Mirror,    ///< Plays forward and backward in turn.
        };

    public:

        /// \brief The highest index a frame may take, since every frame index is stored as a `UInt8`.
        static constexpr UInt32 kMaxFrames  = kMaximum<UInt8>;

        /// \brief The default share of a direction's step a facing may pass its boundary before the direction changes.
        static constexpr Real32 kHysteresis = 0.15f;

    public:

        /// \brief Constructs an empty, single-direction clip that loops at a linear pace.
        ZY_INLINE Animation2D()
            : mStatus     { Status::Repeat },
              mEasing     { Easing::Linear },
              mDirections { 1 },
              mMirrored   { false },
              mHeading    { Angle::FromDegrees(270.0f) },
              mHysteresis { kHysteresis }
        {
        }

        /// \brief Appends a frame to the clip.
        ///
        /// \param Crop     The region of the sheet the frame shows, in normalized sheet coordinates.
        /// \param Duration The time the frame is held for, in seconds.
        /// \return `true` when the frame was added, `false` when the clip is already full.
        ZY_INLINE Bool AddFrame(Rect Crop, Real32 Duration)
        {
            if (mFlipbook.GetCount() <= kMaxFrames)
            {
                mFlipbook.Insert(Duration, Move(Crop));
                return true;
            }
            return false;
        }

        /// \brief Removes a frame, shifting the ones after it down by one.
        ///
        /// \param Keyframe The index of the frame to remove.
        ZY_INLINE void RemoveFrame(UInt32 Keyframe)
        {
            mFlipbook.Remove(Keyframe);
        }

        /// \brief Gets the number of frames in the clip.
        ///
        /// \return The frame count.
        ZY_INLINE UInt32 GetCount() const
        {
            return mFlipbook.GetCount();
        }

        /// \brief Gets how long one pass over the clip lasts.
        ///
        /// \return The sum of every frame's duration, in seconds.
        ZY_INLINE Real32 GetDuration() const
        {
            return mFlipbook.GetDuration();
        }

        /// \brief Sets the region of the sheet a frame shows.
        ///
        /// \param Keyframe The index of the frame.
        /// \param Crop     The region, in normalized sheet coordinates.
        ZY_INLINE void SetCrop(UInt32 Keyframe, Rect Crop)
        {
            mFlipbook.SetData(Keyframe, Move(Crop));
        }

        /// \brief Gets the region of the sheet a frame shows.
        ///
        /// \param Keyframe The index of the frame.
        /// \return The region, in normalized sheet coordinates.
        ZY_INLINE Rect GetCrop(UInt32 Keyframe) const
        {
            return mFlipbook.GetData(Keyframe);
        }

        /// \brief Sets how long a frame is held.
        ///
        /// \param Keyframe The index of the frame.
        /// \param Duration The time to hold it for, in seconds.
        ZY_INLINE void SetDuration(UInt32 Keyframe, Real32 Duration)
        {
            mFlipbook.SetDuration(Keyframe, Duration);
        }

        /// \brief Gets how long a frame is held.
        ///
        /// \param Keyframe The index of the frame.
        /// \return The hold time, in seconds.
        ZY_INLINE Real32 GetDuration(UInt32 Keyframe) const
        {
            return mFlipbook.GetDuration(Keyframe);
        }

        /// \brief Finds the frame showing at a time into the clip, warped by the clip's easing.
        ///
        /// \param Time The time into the clip, in seconds, wrapped past its end.
        /// \return The index of the frame, or zero when the clip holds fewer than two.
        ZY_INLINE UInt32 Locate(Real64 Time) const
        {
            return mFlipbook.Locate(Time, mEasing);
        }

        /// \brief Sets how the clip plays once started.
        ///
        /// \param Status The playback mode.
        ZY_INLINE void SetStatus(Status Status)
        {
            mStatus = Status;
        }

        /// \brief Gets how the clip plays once started.
        ///
        /// \return The playback mode.
        ZY_INLINE Status GetStatus() const
        {
            return mStatus;
        }

        /// \brief Checks whether the clip plays through once and holds on its final frame.
        ///
        /// \return `true` for \ref Status::Forward and \ref Status::Backward, otherwise `false`.
        ZY_INLINE Bool IsOnce() const
        {
            return mStatus == Status::Forward || mStatus == Status::Backward;
        }

        /// \brief Sets the easing applied to the clip's timeline.
        ///
        /// \note Elastic and bounce easings are refused, since they step back through frames already shown.
        ///
        /// \param Easing The easing function.
        ZY_INLINE void SetEasing(Easing Easing)
        {
            ZY_ASSERT(IsMonotonic(Easing), "Frames must be walked in one direction");

            mEasing = Easing;
        }

        /// \brief Gets the easing applied to the clip's timeline.
        ///
        /// \return The easing function.
        ZY_INLINE Easing GetEasing() const
        {
            return mEasing;
        }

        /// \brief Sets how many facing directions the clip is drawn in, each on its own row of the sheet.
        ///
        /// \param Directions The number of directions, where `1` draws one row for every facing and `0` is raised to `1`.
        ZY_INLINE void SetDirections(UInt8 Directions)
        {
            mDirections = Max<UInt8>(Directions, 1);
        }

        /// \brief Gets how many facing directions the clip is drawn in.
        ///
        /// \return The number of directions.
        ZY_INLINE UInt8 GetDirections() const
        {
            return mDirections;
        }

        /// \brief Sets the offset from one direction's row of the sheet to the next.
        ///
        /// \param Stride The offset, in normalized sheet coordinates.
        ZY_INLINE void SetStride(Vector2 Stride)
        {
            mStride = Stride;
        }

        /// \brief Gets the offset from one direction's row of the sheet to the next.
        ///
        /// \return The offset, in normalized sheet coordinates.
        ZY_INLINE Vector2 GetStride() const
        {
            return mStride;
        }

        /// \brief Sets the facing the first row is drawn at, the other rows stepping evenly on from it.
        ///
        /// \param Heading The facing of the first row, measured from the x-axis toward the z-axis.
        ZY_INLINE void SetHeading(Angle Heading)
        {
            mHeading = Heading;
        }

        /// \brief Gets the facing the first row is drawn at.
        ///
        /// \return The facing of the first row, measured from the x-axis toward the z-axis.
        ZY_INLINE Angle GetHeading() const
        {
            return mHeading;
        }

        /// \brief Sets whether the sheet holds only half the directions, the other half drawn as their mirror.
        ///
        /// \param Mirrored The flag, `true` to mirror the far half from the near half, `false` to give every direction its own row.
        ZY_INLINE void SetMirrored(Bool Mirrored)
        {
            mMirrored = Mirrored;
        }

        /// \brief Checks whether the far half of the directions is drawn mirrored from the near half.
        ///
        /// \return `true` when the sheet holds only the near half, otherwise `false`.
        ZY_INLINE Bool IsMirrored() const
        {
            return mMirrored;
        }

        /// \brief Sets how far past a direction's boundary a facing may go before the direction changes.
        ///
        /// \param Hysteresis The share of one direction's step, where `0` switches exactly on the boundary.
        ZY_INLINE void SetHysteresis(Real32 Hysteresis)
        {
            mHysteresis = Max(Hysteresis, 0.0f);
        }

        /// \brief Gets how far past a direction's boundary a facing may go before the direction changes.
        ///
        /// \return The share of one direction's step.
        ZY_INLINE Real32 GetHysteresis() const
        {
            return mHysteresis;
        }

        /// \brief Picks the direction nearest a facing, keeping the current one until the facing is clearly past it.
        ///
        /// \param Facing  The facing to draw, measured from the x-axis toward the z-axis.
        /// \param Current The direction drawn last.
        /// \return The index of the direction, counted from the one at the heading.
        UInt8 Turn(Angle Facing, UInt8 Current) const;

        /// \brief Finds the sheet row a direction is read from, and whether it is drawn mirrored.
        ///
        /// \note A mirrored row shares its \ref Motion2D keys with its source, and the caller must flip those too.
        ///
        /// \param Direction The index of the direction, counted from the one at the heading.
        /// \param Mirrored  Receives `true` when the row must be flipped horizontally, otherwise `false`.
        /// \return The index of the row.
        ZY_INLINE UInt8 Resolve(UInt8 Direction, Ref<Bool> Mirrored) const
        {
            Mirrored = mMirrored && Direction > mDirections / 2;
            return Mirrored ? static_cast<UInt8>(mDirections - Direction) : Direction;
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mFlipbook);
            Archive.Serialize(mStatus);
            Archive.Serialize(mEasing);
            Archive.Serialize(mDirections);
            Archive.Serialize(mStride);
            Archive.Serialize(mHeading);
            Archive.Serialize(mMirrored);
            Archive.Serialize(mHysteresis);
        }

    public:

        /// \brief Checks whether an easing only ever moves forward, which a clip needs to report the frames it enters.
        ///
        /// \param Easing The easing function to check.
        /// \return `true` for every easing but the elastic and bounce ones, otherwise `false`.
        ZY_INLINE static constexpr Bool IsMonotonic(Easing Easing)
        {
            return Easing < Easing::InElastic || Easing > Easing::InOutBounce;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Flipbook<Rect> mFlipbook;
        Status         mStatus;
        Easing         mEasing;
        UInt8          mDirections;
        Bool           mMirrored;
        Vector2        mStride;
        Angle          mHeading;
        Real32         mHysteresis;
    };
}