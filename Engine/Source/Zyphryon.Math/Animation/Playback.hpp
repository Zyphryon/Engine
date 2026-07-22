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
    /// \brief Determines how a playback cursor behaves once it reaches the end of its duration.
    enum class Repeat : UInt8
    {
        Once,    ///< Stop at the end.
        Loop,    ///< Wrap back to the start.
        Mirror,  ///< Reverse direction at each boundary.
    };

    /// \brief A deterministic time cursor over a fixed duration, with looping, speed, and pause control.
    ///
    /// The cursor is an absolute function of an external clock rather than an accumulator: the local time is
    /// `Offset + Speed * (Clock - Epoch)`. Advancing to a given time is idempotent, so the cursor never drifts
    /// and is immune to divergent tick counts or sampling rates. This makes it safe for lockstep and rollback
    /// networking, where `Advance` is fed the deterministic simulation time and the whole object snapshots cleanly.
    class Playback final
    {
    public:

        /// \brief Constructs a stopped cursor with no duration.
        ZY_INLINE Playback()
            : mDuration { 0 },
              mOffset   { 0 },
              mEpoch    { 0 },
              mClock    { 0 },
              mSpeed    { 1 },
              mRepeat   { Repeat::Once },
              mPlaying  { false }
        {
        }

        /// \brief Constructs a stopped cursor over the given duration; call `Play` to start it.
        ///
        /// \param Duration The total duration, in seconds.
        /// \param Mode     The behavior once the end is reached.
        ZY_INLINE explicit Playback(Real64 Duration, Repeat Mode = Repeat::Once)
            : mDuration { Duration },
              mOffset   { 0 },
              mEpoch    { 0 },
              mClock    { 0 },
              mSpeed    { 1 },
              mRepeat   { Mode },
              mPlaying  { false }
        {
        }

        /// \brief Advances the cursor to an absolute point on the timeline (idempotent for a given time).
        ///
        /// \param Time The current simulation time, in seconds (must be deterministic for networked play).
        ZY_INLINE void Advance(Real64 Time)
        {
            mClock = Time;

            if (mPlaying && mRepeat == Repeat::Once)
            {
                if (const Real64 Local = GetElapsed(); Local >= mDuration)
                {
                    mOffset  = mDuration;
                    mPlaying = false;
                }
                else if (Local <= 0.0)
                {
                    mOffset  = 0.0;
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
        }

        /// \brief Moves the cursor to an absolute local time, preserving continuity.
        ///
        /// \param Local The target local time, in seconds.
        ZY_INLINE void Seek(Real64 Local)
        {
            mOffset = Local;
            mEpoch  = mClock;
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

        /// \brief Sets the total duration.
        ///
        /// \param Duration The duration, in seconds.
        ZY_INLINE void SetDuration(Real64 Duration)
        {
            mDuration = Duration;
        }

        /// \brief Sets the repeat behavior.
        ///
        /// \param Mode The behavior once the end is reached.
        ZY_INLINE void SetRepeat(Repeat Mode)
        {
            mRepeat = Mode;
        }

        /// \brief Gets the current cursor time, wrapped by the repeat policy.
        ///
        /// \return The local time in [0, duration], in seconds.
        ZY_INLINE Real64 GetTime() const
        {
            if (mDuration <= 0.0)
            {
                return 0.0;
            }

            const Real64 Local = GetElapsed();

            switch (mRepeat)
            {
            case Repeat::Once:
                return Clamp(Local, 0.0, mDuration);
            case Repeat::Loop:
            {
                const Real64 Wrapped = Mod(Local, mDuration);
                return Wrapped < 0.0 ? Wrapped + mDuration : Wrapped;
            }
            case Repeat::Mirror:
            {
                const Real64 Cycle = mDuration * 2.0;
                Real64       Phase = Mod(Local, Cycle);

                if (Phase < 0.0)
                {
                    Phase += Cycle;
                }
                return Phase <= mDuration ? Phase : Cycle - Phase;
            }
            }
            return 0.0;
        }

        /// \brief Gets the total duration.
        ///
        /// \return The duration, in seconds.
        ZY_INLINE Real64 GetDuration() const
        {
            return mDuration;
        }

        /// \brief Gets the playback speed multiplier.
        ///
        /// \return The speed multiplier.
        ZY_INLINE Real32 GetSpeed() const
        {
            return mSpeed;
        }

        /// \brief Gets the repeat behavior.
        ///
        /// \return The repeat mode.
        ZY_INLINE Repeat GetRepeat() const
        {
            return mRepeat;
        }

        /// \brief Gets the normalized progress through the duration.
        ///
        /// \return The progress in [0, 1], or `0` when there is no duration.
        ZY_INLINE Real32 GetProgress() const
        {
            return mDuration > 0.0 ? static_cast<Real32>(GetTime() / mDuration) : 0.0f;
        }

        /// \brief Gets the current playback direction, accounting for mirror reflection.
        ///
        /// \return `true` when advancing forward, `false` when reversed.
        ZY_INLINE Bool IsForward() const
        {
            if (mRepeat == Repeat::Mirror && mDuration > 0.0)
            {
                const Real64 Cycle = mDuration * 2.0;
                Real64       Phase = Mod(GetElapsed(), Cycle);

                if (Phase < 0.0)
                {
                    Phase += Cycle;
                }
                return Phase <= mDuration;
            }
            return mSpeed >= 0.0f;
        }

        /// \brief Checks whether the cursor is currently advancing.
        ///
        /// \return `true` if playing, `false` otherwise.
        ZY_INLINE Bool IsPlaying() const
        {
            return mPlaying;
        }

        /// \brief Checks whether a non-repeating cursor has reached the end.
        ///
        /// \return `true` if the cursor is complete, `false` otherwise.
        ZY_INLINE Bool IsComplete() const
        {
            return mRepeat == Repeat::Once && GetElapsed() >= mDuration;
        }

    private:

        /// \brief Computes the unwrapped local time from the absolute clock.
        ///
        /// \return The local time before applying the repeat policy, in seconds.
        ZY_INLINE Real64 GetElapsed() const
        {
            return mPlaying ? mOffset + mSpeed * (mClock - mEpoch) : mOffset;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real64 mDuration;
        Real64 mOffset;
        Real64 mEpoch;
        Real64 mClock;
        Real32 mSpeed;
        Repeat mRepeat;
        Bool   mPlaying;
    };
}
