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

#include "Easing.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a run of frames, each held for as long as it says and each optionally showing a value.
    template<typename Type = void, UInt Count = 0>
    class Flipbook final
    {
    public:

        /// \brief Represents one frame of the run.
        struct Frame final
        {
            /// The time the frame is held for, in seconds.
            Real32             Duration;

            /// The value the frame shows, taking no room of its own when the run shows none.
            ZY_COMPRESSED Type Data;
        };

    public:

        /// \brief Checks whether the run holds no frame at all.
        ///
        /// \return `true` when the run is empty, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mFrames.IsEmpty();
        }

        /// \brief Checks whether the run holds as many frames as it can.
        ///
        /// \return `true` when no further frame fits, otherwise `false`.
        ZY_INLINE Bool IsFull() const
        {
            if constexpr (Count > 0)
            {
                return mFrames.IsFull();
            }
            else
            {
                return false;
            }
        }

        /// \brief Appends a frame that shows nothing for the given time.
        ///
        /// \param Duration The time the frame is held for, in seconds.
        ZY_INLINE void Insert(Real32 Duration)
        {
            mFrames.Append(Duration);
        }

        /// \brief Appends a frame that shows a value for the given time.
        ///
        /// \param Duration The time the frame is held for, in seconds.
        /// \param Data     The value the frame shows.
        ZY_INLINE void Insert(Real32 Duration, AnyRef<Type> Data)
        {
            mFrames.Append(Duration, Forward<Type>(Data));
        }

        /// \brief Drops a frame from the run.
        ///
        /// \param Keyframe The index of the frame to drop.
        ZY_INLINE void Remove(UInt32 Keyframe)
        {
            mFrames.Remove(Keyframe);
        }

        /// \brief Grows or trims the run to hold exactly the given number of frames.
        ///
        /// \param Length The frame count the run should hold.
        ZY_INLINE void Resize(UInt32 Length)
        {
            mFrames.Resize(Length);
        }

        /// \brief Drops every frame, leaving the run empty.
        ZY_INLINE void Clear()
        {
            mFrames.Clear();
        }

        /// \brief Gets how many frames the run holds.
        ///
        /// \return The frame count.
        ZY_INLINE UInt32 GetCount() const
        {
            return mFrames.GetSize();
        }

        /// \brief Gets how long one pass over the run lasts.
        ///
        /// \return The sum of every frame's time, in seconds.
        ZY_INLINE Real32 GetDuration() const
        {
            Real32 Total = 0.0f;

            for (ConstRef<Frame> Frame : mFrames)
            {
                Total += Frame.Duration;
            }
            return Total;
        }

        /// \brief Gets the run's frames.
        ///
        /// \return A read-only view over the frames, in the order they are shown.
        ZY_INLINE ConstSpan<Frame> GetFrames() const
        {
            return mFrames;
        }

        /// \brief Sets how long a frame is held for.
        ///
        /// \param Keyframe The index of the frame to change.
        /// \param Duration The time the frame is held for, in seconds.
        ZY_INLINE void SetDuration(UInt32 Keyframe, Real32 Duration)
        {
            mFrames[Keyframe].Duration = Duration;
        }

        /// \brief Gets how long a frame is held for.
        ///
        /// \param Keyframe The index of the frame to read.
        /// \return The time the frame is held for, in seconds.
        ZY_INLINE Real32 GetDuration(UInt32 Keyframe) const
        {
            return mFrames[Keyframe].Duration;
        }

        /// \brief Sets the value a frame shows.
        ///
        /// \param Keyframe The index of the frame to change.
        /// \param Data     The value the frame shows.
        ZY_INLINE void SetData(UInt32 Keyframe, AnyRef<Type> Data)
            requires (!::IsEmpty<Type>)
        {
            mFrames[Keyframe].Data = Forward<Type>(Data);
        }

        /// \brief Gets the value a frame shows.
        ///
        /// \param Keyframe The index of the frame to read.
        /// \return The value the frame shows.
        ZY_INLINE ConstRef<Type> GetData(UInt32 Keyframe) const
            requires (!::IsEmpty<Type>)
        {
            return mFrames[Keyframe].Data;
        }

        /// \brief Finds the frame showing at a moment in time.
        ///
        /// \param Time     The time to look up, in seconds.
        /// \param Function The easing to warp the pass with, which leaves the frames evenly spaced when linear.
        /// \return The index of the frame showing then, or zero when the run holds nothing to show.
        ZY_INLINE UInt32 Locate(Real64 Time, Easing Function = Easing::Linear) const
        {
            const Real64 Cycle = GetDuration();

            if (mFrames.GetSize() < 2 || Cycle <= 0.0)
            {
                return 0;
            }

            Real64 Absolute = Mod(Time, Cycle);

            if (Absolute < 0.0)
            {
                Absolute += Cycle;
            }

            const Real64 Elapsed = Clamp(Ease(Function, Absolute / Cycle), 0.0, 1.0) * Cycle;

            Real64 Consumed = 0.0;

            for (UInt32 Keyframe = 0, Limit = GetCount(); Keyframe < Limit; ++Keyframe)
            {
                Consumed += mFrames[Keyframe].Duration;

                if (Elapsed < Consumed)
                {
                    return Keyframe;
                }
            }
            return GetCount() - 1;
        }

        /// \brief Gets the value the run shows at a moment in time.
        ///
        /// \param Time     The time to sample at, in seconds.
        /// \param Function The easing to warp the pass with, which leaves the frames evenly spaced when linear.
        /// \return The value of the frame showing then.
        ZY_INLINE ConstRef<Type> Sample(Real64 Time, Easing Function = Easing::Linear)
            requires (!::IsEmpty<Type>)
        {
            return mFrames[Locate(Time, Function)].Data;
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mFrames);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Frame, Count> mFrames;
    };
}