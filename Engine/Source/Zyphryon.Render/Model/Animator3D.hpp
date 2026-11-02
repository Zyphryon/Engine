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

#include "Model3D.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents one playing instance of a model, holding its clip position and the palette posed from it.
    class ZY_API Animator3D final
    {
    public:

        /// \brief The number of clips an animator holds at once, the one playing and the one it fades out of.
        static constexpr UInt kSources = 2;

        /// \brief The value a slot takes while the clip asked for has not been found in the model.
        static constexpr UInt16 kUnresolved = kMaximum<UInt16>;

        /// \brief Represents the scratch space for posing, shared by every animator advanced on one thread.
        ///
        /// \note Local poses are never read once folded into the palette, so one set per thread is enough.
        class Scratch final
        {
        public:

            /// \brief Gets a pose sized to the given rig, growing the buffers when one is wider than any before.
            ///
            /// \param Bones The number of bones the caller is about to pose.
            /// \param Slot  The pose to hand out, below \ref kSources.
            /// \return The pose covering exactly \p Bones, whose contents are indeterminate.
            Skeleton3D::Pose Acquire(UInt Bones, UInt Slot = 0);

        private:

            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

            Sequence<Vector3>    mPosition;
            Sequence<Vector3>    mScale;
            Sequence<Quaternion> mRotation;
        };

    public:

        /// \brief Constructs an animator that is not yet wearing a model.
        Animator3D();

        /// \brief Attaches the model to be posed.
        ///
        /// \param Model The model to wear.
        void SetModel(ConstRetainer<Model3D> Model);

        /// \brief Gets the model being posed.
        ///
        /// \return The model, or an empty retainer when none was attached.
        ZY_INLINE ConstRetainer<Model3D> GetModel() const
        {
            return mModel;
        }

        /// \brief Gets the clip being played.
        ///
        /// \return The clip, or `nullptr` until the model has loaded and the clip asked for has been found in it.
        ZY_INLINE ConstPtr<Model3D::Clip> GetClip() const
        {
            if (mModel && mModel->HasCompleted())
            {
                ConstRef<Source>               Active = mSources[mActive];
                const ConstSpan<Model3D::Clip> Clips  = mModel->GetClips();
                const Bool                     Found  = Active.Slot < Clips.GetSize() && Clips[Active.Slot].Name == Active.Name;
                return Found ? AddressOf(Clips[Active.Slot]) : nullptr;
            }
            return nullptr;
        }

        /// \brief Sets the playback rate relative to the speed every clip plays at by its own rate.
        ///
        /// \param Rate The rate, where `1` plays as authored, `0` holds the current pose and a negative one is `0`.
        ZY_INLINE void SetRate(Real32 Rate)
        {
            mRate = Max(Rate, 0.0f);

            for (Ref<Source> Entry : mSources)
            {
                Entry.Cursor.SetSpeed(mRate * Entry.Rate);
            }
        }

        /// \brief Gets the playback rate relative to the speed every clip plays at by its own rate.
        ///
        /// \return The rate, where `1` plays as authored.
        ZY_INLINE Real32 GetRate() const
        {
            return mRate;
        }

        /// \brief Gets how far through the clip the animator is, counted the way \ref Play takes it back.
        ///
        /// \return The share of the clip's duration elapsed from zero to one, or the share asked for until it starts.
        ZY_INLINE Real32 GetPhase() const
        {
            ConstRef<Source> Active = mSources[mActive];
            return Active.Bound ? Active.Cursor.GetProgress() : Active.Phase;
        }

        /// \brief Starts a clip of the model, playing it the way its \ref Model3D::Clip::Mode says.
        ///
        /// \param Clip      The hash of the clip's name.
        /// \param Timestamp The time playback starts, in seconds, on the clock \ref Sample is given.
        /// \param Phase     The share of the clip to start at, where zero is its first pose.
        /// \param Fade      The time to blend out of the clip already playing, in seconds, counted from the timestamp.
        /// \param Back      `true` to return to the loop it cuts once a play-once clip ends, `false` to hold its end.
        void Play(UInt64 Clip, Real64 Timestamp, Real32 Phase = 0.0f, Real64 Fade = 0.0, Bool Back = false);

        /// \brief Advances the clip and samples the local pose it puts the rig in, leaving the palette as it was.
        ///
        /// \param Time    The current simulation time, in seconds, on the same clock every call is given.
        /// \param Scratch The working room to pose in, which holds the returned pose until it is acquired again.
        /// \return The local transform of every bone, or an empty pose while the model rests or is still loading.
        Skeleton3D::Pose Sample(Real64 Time, Ref<Scratch> Scratch);

        /// \brief Folds a local pose into the palette the vertex shader deforms by.
        ///
        /// \param Pose The local transform of every bone as \ref Sample returned it, adjusted or not.
        void Compose(ConstRef<Skeleton3D::Pose> Pose);

        /// \brief Advances the clip and rebuilds the palette it puts the rig in.
        ///
        /// \param Time    The current simulation time, in seconds, on the same clock every call is given.
        /// \param Scratch The working room to pose in, which the call leaves in an indeterminate state.
        void Update(Real64 Time, Ref<Scratch> Scratch);

        /// \brief Gets the matrices the vertex shader deforms by, one per bone.
        ///
        /// \return The palette, or an empty span before the first \ref Update.
        ZY_INLINE ConstSpan<Matrix4x3> GetPalette() const
        {
            return mPalette;
        }

        /// \brief Checks whether a clip that plays once has reached its end.
        ///
        /// \return `true` when a play-once clip is holding on its final pose, otherwise `false`.
        ZY_INLINE Bool HasEnded() const
        {
            ConstRef<Source> Active = mSources[mActive];
            return Active.Bound && Active.Cursor.IsComplete();
        }

    private:

        /// \brief Represents one clip paired with a rig, where it has reached and how its lanes reach the bones.
        struct Source final
        {
            /// The hash of the clip's name, or zero when the source holds none.
            UInt64                Name      = 0;

            /// The motion of the clip last paired, empty when none was or the model lists no such clip.
            Retainer<Animation3D> Motion;

            /// The time the clip starts at, in seconds, on the clock \ref Sample is given.
            Real64                Timestamp = 0.0;

            /// The share of the clip to start at, where zero is its first pose.
            Real32                Phase     = 0.0f;

            /// The rate of the clip last paired, as its \ref Model3D::Clip::Rate gives it.
            Real32                Rate      = 1.0f;

            /// The place of the clip in the model's list, or \ref kUnresolved while the model lists no such clip.
            UInt16                Slot      = kUnresolved;

            /// The cursor tracking where the clip has reached.
            Playback              Cursor;

            /// The bone index of every lane, as resolved by \ref Animation3D::Resolve.
            Sequence<SInt32>      Binding;

            /// `true` once the clip is paired with the rig, so a frame need not rediscover it, `false` until then.
            Bool                  Bound     = false;

            /// `true` when the clip writes every component of every bone, so no rest pose is seeded, otherwise `false`.
            Bool                  Covered   = false;
        };

        /// \brief Finds the clip in the model and pairs it with the rig, starting it from its timestamp and phase.
        ///
        /// \param Entry The source to pair, left as it is once paired.
        /// \param Rig   The skeleton the clip is playing over.
        void Rebind(Ref<Source> Entry, ConstRef<Skeleton3D> Rig);

        /// \brief Seeds a pose and samples one source's clip over it.
        ///
        /// \param Entry  The source to sample, which must be paired and carrying a clip.
        /// \param Rig    The skeleton the clip is playing over.
        /// \param Output Receives the local transform of every bone.
        void Evaluate(ConstRef<Source> Entry, ConstRef<Skeleton3D> Rig, ConstRef<Skeleton3D::Pose> Output) const;

        /// \brief Lets a source go, ending whatever fade it was taking part in.
        ///
        /// \param Entry The source to empty.
        void Retire(Ref<Source> Entry);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Model3D>   mModel;
        Source              mSources[kSources];
        Sequence<Matrix4x3> mPalette;
        Real64              mEpoch;
        Real64              mFade;
        Real64              mBack;
        UInt64              mReturn;
        Real32              mResume;
        Real32              mRate;
        UInt8               mActive;
    };
}