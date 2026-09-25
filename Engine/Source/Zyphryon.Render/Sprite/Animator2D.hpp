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

#include "Sheet2D.hpp"
#include "Zyphryon.Math/Motion/Playback.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents one instance playing a clip of a \ref Sheet2D.
    class ZY_API Animator2D final
    {
    public:

        /// \brief The value a slot takes while the clip asked for has not been started on the sheet.
        static constexpr UInt16 kUnresolved = kMaximum<UInt16>;

        /// \brief The frames entered during one advance, in the order they were entered.
        using Visited = Spool<UInt8>;

    public:

        /// \brief Constructs an animator holding no sheet, on the first frame and direction, at the authored rate.
        ZY_INLINE Animator2D()
            : mClip      { 0 },
              mPhase     { 0.0f },
              mSlot      { kUnresolved },
              mKeyframe  { 0 },
              mDirection { 0 }
        {
        }

        /// \brief Attaches the sheet whose clips are played, stopping whatever was playing.
        ///
        /// \param Sheet The sheet to play from.
        ZY_INLINE void SetSheet(ConstRetainer<Sheet2D> Sheet)
        {
            mSheet = Sheet;
            mClip  = 0;
            mSlot  = kUnresolved;
        }

        /// \brief Gets the sheet whose clips are played.
        ///
        /// \return The sheet, or an empty retainer when none was attached.
        ZY_INLINE ConstRetainer<Sheet2D> GetSheet() const
        {
            return mSheet;
        }

        /// \brief Gets the clip being played.
        ///
        /// \return The clip, or `nullptr` until the sheet has loaded and the clip asked for has been found in it.
        ZY_INLINE ConstPtr<Sheet2D::Clip> GetClip() const
        {
            if (mSheet && mSheet->HasCompleted())
            {
                const ConstSpan<Sheet2D::Clip> Clips = mSheet->GetClips();
                return (mSlot < Clips.GetSize() && Clips[mSlot].Name == mClip) ? AddressOf(Clips[mSlot]) : nullptr;
            }
            return nullptr;
        }

        /// \brief Sets the playback rate relative to the authored frame durations.
        ///
        /// \param Rate The rate, where `1` plays as authored and `0` holds the current frame; negative is clamped to `0`.
        ZY_INLINE void SetRate(Real32 Rate)
        {
            mPlayback.SetSpeed(mPlayback.GetSpeed() < 0.0f ? -Max(Rate, 0.0f) : Max(Rate, 0.0f));
        }

        /// \brief Gets the playback rate relative to the authored frame durations.
        ///
        /// \return The rate, where `1` plays as authored.
        ZY_INLINE Real32 GetRate() const
        {
            return Abs(mPlayback.GetSpeed());
        }

        /// \brief Gets how far through the clip the animator is, counted the way \ref Play takes it back.
        ///
        /// \return The share of the clip's duration elapsed in the direction it plays, from zero to one.
        ZY_INLINE Real64 GetPhase() const
        {
            const Real64 Duration = mPlayback.GetDuration();

            if (Duration > 0.0)
            {
                const Real64 Share = Clamp(mPlayback.GetTime() / Duration, 0.0, 1.0);
                return mPlayback.GetSpeed() < 0.0f ? 1.0 - Share : Share;
            }
            return 0.0;
        }

        /// \brief Gets the frame the animator is on.
        ///
        /// \return The index of the frame within the clip.
        ZY_INLINE UInt8 GetKeyframe() const
        {
            return mKeyframe;
        }

        /// \brief Gets the direction the animator is drawn in.
        ///
        /// \return The index of the direction, counted from the one at the clip's heading.
        ZY_INLINE UInt8 GetDirection() const
        {
            return mDirection;
        }

        /// \brief Starts a clip of the sheet, playing it the way its \ref Animation2D::Status says.
        ///
        /// \param Clip      The hash of the clip's name.
        /// \param Timestamp The time playback starts, in seconds, on the clock \ref Advance is given.
        /// \param Phase     The share of the clip to start at, counted the way it plays, where zero is the first frame shown.
        ZY_INLINE void Play(UInt64 Clip, Real64 Timestamp, Real32 Phase = 0.0f)
        {
            mClip  = Clip;
            mPhase = Phase;
            mSlot  = kUnresolved;

            Resolve(Timestamp);
        }

        /// \brief Advances playback to a time and records every frame entered on the way.
        ///
        /// \note A step spanning many laps reports one whole lap between its ends, or a pair for a mirror, not every lap.
        ///
        /// \param Time    The current time, in seconds, on the same clock every call is given.
        /// \param Visited Receives the frames entered, in the order they were entered.
        ZY_INLINE void Advance(Real64 Time, Ref<Visited> Visited)
        {
            if (const ConstPtr<Sheet2D::Clip> Entry = Resolve(Time))
            {
                Step(Time, Entry->Frames, Visited);
            }
        }

        /// \brief Updates the direction drawn for a facing, keeping the current one near a boundary.
        ///
        /// \param Facing The facing, measured from the x-axis toward the z-axis.
        ZY_INLINE void Turn(Angle Facing)
        {
            if (const ConstPtr<Sheet2D::Clip> Entry = GetClip())
            {
                mDirection = Entry->Frames.Turn(Facing, mDirection);
            }
        }

        /// \brief Checks whether a clip that plays once has reached its end.
        ///
        /// \return `true` when a play-once clip is holding on its final frame, otherwise `false`.
        ZY_INLINE Bool HasEnded() const
        {
            if (mPlayback.GetRepeat() == Repeat::Once)
            {
                return mPlayback.GetSpeed() < 0.0f
                    ? (mPlayback.GetTime() <= 0.0)
                    : (mPlayback.GetTime() >= mPlayback.GetDuration());
            }
            return false;
        }

    private:

        /// \brief Finds the clip asked for in the sheet, starting it from the phase asked for the first time it is found.
        ///
        /// \param Timestamp The time the clip starts at, should it be found now.
        /// \return The clip, or `nullptr` while the sheet is loading or carries no clip by that name.
        ConstPtr<Sheet2D::Clip> Resolve(Real64 Timestamp);

        /// \brief Sets the playback cursor running over a clip's frames.
        ///
        /// \param Sequence  The frames of the clip to play.
        /// \param Timestamp The time playback starts, in seconds.
        /// \param Phase     The share of the clip to start at, counted the way it plays.
        void Restart(ConstRef<Animation2D> Sequence, Real64 Timestamp, Real64 Phase);

        /// \brief Moves the playback cursor over a clip's frames and records every frame entered on the way.
        ///
        /// \param Time     The current time, in seconds.
        /// \param Sequence The frames of the clip being played.
        /// \param Visited  Receives the frames entered, in the order they were entered.
        void Step(Real64 Time, ConstRef<Animation2D> Sequence, Ref<Visited> Visited);

        /// \brief Finds the frame the playback cursor is on, holding the final frame once a pass reaches its end.
        ///
        /// \param Sequence The clip being played.
        /// \return The index of the frame.
        ZY_INLINE UInt8 Locate(ConstRef<Animation2D> Sequence) const
        {
            const UInt32 Count = Sequence.GetCount();
            const Real64 Local = mPlayback.GetTime();

            if (Count == 0)
            {
                return 0;
            }

            // The far end of a pass shows the last frame, where the run would otherwise wrap to the first.
            return static_cast<UInt8>(Local >= mPlayback.GetDuration() ? Count - 1 : Sequence.Locate(Local));
        }

        /// \brief Gets which pass over the clip an unwrapped local time falls in.
        ///
        /// \param Elapsed The unwrapped local time, in seconds.
        /// \return The index of the pass, which is always zero for a clip that plays once.
        ZY_INLINE SInt64 GetPass(Real64 Elapsed) const
        {
            const Real64 Duration = mPlayback.GetDuration();

            // A clip played once never wraps, and one with no length has nothing to wrap over.
            if (mPlayback.GetRepeat() == Repeat::Once || Duration <= 0.0)
            {
                return 0;
            }
            return static_cast<SInt64>(Floor(Elapsed / Duration));
        }

        /// \brief Checks whether frames are entered in rising order during a pass.
        ///
        /// \param Pass   The index of the pass.
        /// \param Rising The direction local time is moving, `true` when it grows.
        /// \return `true` when frames are entered from first to last, otherwise `false`.
        ZY_INLINE Bool IsAscending(SInt64 Pass, Bool Rising) const
        {
            return Rising != (mPlayback.GetRepeat() == Repeat::Mirror && (Pass & 1) != 0);
        }

        /// \brief Appends every frame between two frames, excluding the first and including the last.
        ///
        /// \param Visited   Receives the frames walked.
        /// \param From      The frame walked from.
        /// \param Into      The frame walked to.
        /// \param Ascending The direction, `true` to count up, `false` to count down.
        ZY_INLINE static void Walk(Ref<Visited> Visited, UInt8 From, UInt8 Into, Bool Ascending)
        {
            const SInt32 Step = Ascending ? 1 : -1;

            for (SInt32 Keyframe = From + Step; Ascending ? Keyframe <= Into : Keyframe >= Into; Keyframe += Step)
            {
                Visited.Append(static_cast<UInt8>(Keyframe));
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Sheet2D> mSheet;
        UInt64            mClip;
        Playback          mPlayback;
        Real32            mPhase;
        UInt16            mSlot;
        UInt8             mKeyframe;
        UInt8             mDirection;
    };
}