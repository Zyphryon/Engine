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

inline namespace ZyMath
{
    /// \brief Specifies how a playback cursor behaves once it reaches the end of its duration.
    enum class Repeat : UInt8
    {
        Once,    ///< Stop at the end.
        Loop,    ///< Wrap back to the start.
        Mirror,  ///< Reverse direction at each boundary.
    };

    /// \brief Represents a deterministic time cursor over a fixed duration, with looping, speed, and pause control.
    class Playback final
    {
    public:

        /// \brief Constructs a stopped cursor with no duration.
        ZY_INLINE Playback()
            : Playback(0.0)
        {
        }

        /// \brief Constructs a stopped cursor over the given duration; call `Play` to start it.
        ///
        /// \param Duration The total duration, in seconds.
        /// \param Mode     The behavior once the end is reached.
        /// \param Laps     The number of laps a looping or mirrored cursor runs, where `0` runs forever.
        ZY_INLINE explicit Playback(Real64 Duration, Repeat Mode = Repeat::Once, UInt32 Laps = 0)
            : mDuration { Duration },
              mOffset   { 0 },
              mEpoch    { 0 },
              mClock    { 0 },
              mSpeed    { 1 },
              mLaps     { Laps },
              mRepeat   { Mode },
              mPlaying  { false },
              mFresh    { false }
        {
        }

        /// \brief Advances the cursor to an absolute point on the timeline (idempotent for a given time).
        ///
        /// \param Time The current simulation time, in seconds (must be deterministic for networked play).
        ZY_INLINE void Advance(Real64 Time)
        {
            mClock = Time;
            mFresh = false;

            if (mPlaying && !IsEndless(mRepeat, mLaps))
            {
                const Real64 Local   = GetElapsed();
                const Real64 Length  = GetLength(mDuration, mRepeat, mLaps);
                const Bool   Forward = mSpeed >= 0.0f;

                if (Forward ? Local >= Length : Local <= 0.0)
                {
                    mOffset  = Forward ? Length : 0.0;
                    mPlaying = false;
                }
            }
        }

        /// \brief Begins or resumes playback, anchoring the epoch to the current clock.
        ZY_INLINE void Play()
        {
            if (!mPlaying)
            {
                mEpoch   = mClock;
                mPlaying = true;
                mFresh   = true;
            }
        }

        /// \brief Suspends playback, banking the elapsed time so it resumes without a jump.
        ZY_INLINE void Pause()
        {
            if (mPlaying)
            {
                mOffset  = GetElapsed();
                mPlaying = false;
            }
        }

        /// \brief Stops playback and rewinds to the start.
        ZY_INLINE void Stop()
        {
            mOffset  = 0.0;
            mEpoch   = mClock;
            mPlaying = false;
            mFresh   = true;
        }

        /// \brief Moves the cursor to an absolute local time, preserving continuity.
        ///
        /// \param Local The target local time, in seconds.
        ZY_INLINE void Seek(Real64 Local)
        {
            mOffset = Local;
            mEpoch  = mClock;
            mFresh  = true;
        }

        /// \brief Sets the playback speed multiplier, rebasing so the change is continuous.
        ///
        /// \param Speed The speed multiplier (`1` is real-time, negative plays backward).
        ZY_INLINE void SetSpeed(Real32 Speed)
        {
            if (mPlaying)
            {
                mOffset = GetElapsed();
                mEpoch  = mClock;
            }
            mSpeed = Speed;
        }

        /// \brief Gets the playback speed multiplier.
        ///
        /// \return The speed multiplier.
        ZY_INLINE Real32 GetSpeed() const
        {
            return mSpeed;
        }

        /// \brief Sets the total duration.
        ///
        /// \param Duration The duration, in seconds.
        ZY_INLINE void SetDuration(Real64 Duration)
        {
            mDuration = Duration;
        }

        /// \brief Gets the total duration.
        ///
        /// \return The duration, in seconds.
        ZY_INLINE Real64 GetDuration() const
        {
            return mDuration;
        }

        /// \brief Sets the repeat behavior.
        ///
        /// \param Mode The behavior once the end is reached.
        ZY_INLINE void SetRepeat(Repeat Mode)
        {
            mRepeat = Mode;
        }

        /// \brief Gets the repeat behavior.
        ///
        /// \return The repeat mode.
        ZY_INLINE Repeat GetRepeat() const
        {
            return mRepeat;
        }

        /// \brief Sets how many laps a looping or mirrored cursor runs before it stops.
        ///
        /// \note A mirrored lap is one pass in one direction, so two laps run there and back.
        ///
        /// \param Laps The number of laps, where `0` runs forever.
        ZY_INLINE void SetLaps(UInt32 Laps)
        {
            mLaps = Laps;
        }

        /// \brief Gets how many laps a looping or mirrored cursor runs before it stops.
        ///
        /// \return The number of laps, where `0` runs forever.
        ZY_INLINE UInt32 GetLaps() const
        {
            return mLaps;
        }

        /// \brief Gets the current cursor time, wrapped by the repeat policy.
        ///
        /// \return The local time in [0, duration], in seconds.
        ZY_INLINE Real64 GetTime() const
        {
            return Map(GetElapsed(), mDuration, mRepeat, mLaps);
        }

        /// \brief Gets the normalized progress through the duration.
        ///
        /// \return The progress in [0, 1], or `0` when there is no duration.
        ZY_INLINE Real32 GetProgress() const
        {
            return mDuration > 0.0 ? static_cast<Real32>(GetTime() / mDuration) : 0.0f;
        }

        /// \brief Gets the local time before the repeat policy wraps, mirrors or clamps it.
        ///
        /// \return The unwrapped local time, in seconds, which counts every lap run so far.
        ZY_INLINE Real64 GetElapsed() const
        {
            return mPlaying ? mOffset + mSpeed * (mClock - mEpoch) : mOffset;
        }

        /// \brief Gets the current playback direction, accounting for mirror reflection.
        ///
        /// \return `true` when advancing forward, `false` when reversed.
        ZY_INLINE Bool IsForward() const
        {
            if (mRepeat == Repeat::Mirror && mDuration > 0.0)
            {
                return ::Wrap(GetElapsed(), mDuration * 2.0) <= mDuration;
            }
            return mSpeed >= 0.0f;
        }

        /// \brief Checks whether the cursor was stood somewhere, and not advanced since.
        ///
        /// \return `true` until the next advance, `false` after it.
        ZY_INLINE Bool IsFresh() const
        {
            return mFresh;
        }

        /// \brief Checks whether the cursor is currently advancing.
        ///
        /// \return `true` if playing, `false` otherwise.
        ZY_INLINE Bool IsPlaying() const
        {
            return mPlaying;
        }

        /// \brief Checks whether a cursor that does not run forever has reached the end of its last lap.
        ///
        /// \return `true` if the cursor is complete, `false` otherwise.
        ZY_INLINE Bool IsComplete() const
        {
            if (!IsEndless(mRepeat, mLaps))
            {
                const Real64 Elapsed = GetElapsed();
                return mSpeed >= 0.0f ? Elapsed >= GetLength(mDuration, mRepeat, mLaps) : Elapsed <= 0.0;
            }
            return false;
        }

    private:

        /// \brief Checks whether a repeat policy runs forever, so it never reaches an end.
        ///
        /// \param Mode The behavior once the end is reached.
        /// \param Laps The number of laps a looping or mirrored cursor runs, where `0` runs forever.
        /// \return `true` if a looping or mirrored policy has no lap count, `false` otherwise.
        ZY_INLINE static Bool IsEndless(Repeat Mode, UInt32 Laps)
        {
            return Mode != Repeat::Once && Laps == 0;
        }

        /// \brief Gets the local time the last lap of a repeat policy ends at.
        ///
        /// \param Duration The length of one lap, in seconds.
        /// \param Mode     The behavior once the end is reached.
        /// \param Laps     The number of laps a looping or mirrored cursor runs.
        /// \return The duration times the laps run, in seconds, meaningful only when the policy is not endless.
        ZY_INLINE static Real64 GetLength(Real64 Duration, Repeat Mode, UInt32 Laps)
        {
            return Mode == Repeat::Once ? Duration : Duration * static_cast<Real64>(Laps);
        }

    public:

        /// \brief Maps a local time onto the cursor time a repeat policy gives it, as a cursor holding it would.
        ///
        /// \param Elapsed  The local time before the repeat policy wraps, mirrors or clamps it, in seconds.
        /// \param Duration The length of one lap, in seconds.
        /// \param Mode     The behavior once the end is reached.
        /// \param Laps     The number of laps a looping or mirrored cursor runs, where `0` runs forever.
        /// \return The local time in [0, duration], in seconds, or `0` when there is no duration.
        ZY_INLINE static Real64 Map(Real64 Elapsed, Real64 Duration, Repeat Mode, UInt32 Laps = 0)
        {
            if (Duration <= 0.0)
            {
                return 0.0;
            }

            const Bool   Endless = IsEndless(Mode, Laps);
            const Real64 Length  = GetLength(Duration, Mode, Laps);
            const Real64 Local   = Endless ? Elapsed : Clamp(Elapsed, 0.0, Length);

            switch (Mode)
            {
            case Repeat::Once:
                return Local;
            case Repeat::Loop:
                return !Endless && Local >= Length ? Duration : ::Wrap(Local, Duration);
            case Repeat::Mirror:
            {
                const Real64 Cycle = Duration * 2.0;
                const Real64 Phase = ::Wrap(Local, Cycle);
                return Phase <= Duration ? Phase : Cycle - Phase;
            }
            }
            return 0.0;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real64 mDuration;
        Real64 mOffset;
        Real64 mEpoch;
        Real64 mClock;
        Real32 mSpeed;
        UInt32 mLaps;
        Repeat mRepeat;
        Bool   mPlaying;
        Bool   mFresh;
    };
}