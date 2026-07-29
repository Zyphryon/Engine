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

#include "Zyphryon.Math/Animation/Playback.hpp"
#include "Zyphryon.Render/Resource/Animation.hpp"
#include "Zyphryon.Render/Resource/Model.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief One playing instance of a model, holding its clip position and the palette posed from it.
    class Animator final
    {
    public:

        /// \brief The number of clips an animator holds at once: the one playing, and the one it fades out of.
        static constexpr UInt kSources = 2;

        /// \brief Scratch space for posing, shared by every animator advanced on one thread.
        ///
        /// The local poses are written, folded into the palette, and never read again, so they live here once
        /// per thread instead of once per instance.
        class Scratch final
        {
        public:

            /// \brief Gets a pose sized to the given rig, growing the buffers when one is wider than any before.
            ///
            /// \param Bones The number of bones the caller is about to pose.
            /// \param Slot  Which of the \ref kSources poses to hand out.
            /// \return A pose covering exactly \p Bones, whose contents are indeterminate.
            Skeleton::Pose Acquire(UInt Bones, UInt Slot = 0);

        private:

            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

            Sequence<Vector3>    mPosition;
            Sequence<Vector3>    mScale;
            Sequence<Quaternion> mRotation;
        };

    public:

        /// \brief Constructs an animator that is not yet wearing a model.
        Animator();

        /// \brief Attaches the model to be posed.
        ///
        /// \param Model The model to wear.
        void SetModel(ConstRetainer<Model> Model);

        /// \brief Gets the model being posed.
        ///
        /// \return The model, or an empty retainer when none was attached.
        ZY_INLINE ConstRetainer<Model> GetModel() const
        {
            return mModel;
        }

        /// \brief Starts a clip over the attached model's rig.
        ///
        /// \param Animation The clip to play.
        /// \param Mode      How the clip behaves once it reaches its end.
        /// \param Fade      How long to blend out of the clip already playing, in seconds.
        void Play(ConstRetainer<Animation> Animation, Repeat Mode = Repeat::Loop, Real64 Fade = 0.0);

        /// \brief Gets the cursor tracking where the playing clip has reached.
        ///
        /// \note A clip being faded out keeps its own cursor, which this does not reach.
        ///
        /// \return The playback cursor of the clip that is taking hold.
        ZY_INLINE Ref<Playback> GetPlayback()
        {
            return mSources[mActive].Cursor;
        }

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

    private:

        /// \brief One clip paired with a rig: where it has reached, and how its lanes reach the bones.
        struct Source final
        {
            /// The clip being played, or empty when the source holds none.
            Retainer<Animation> Clip;

            /// The cursor tracking where the clip has reached.
            Playback            Cursor;

            /// One bone index per lane, as resolved by \ref Animation::Resolve.
            Sequence<SInt32>    Binding;

            /// Whether the clip has been paired with the rig, so a frame need not rediscover it.
            Bool                Bound   = false;

            /// Whether the clip writes every component of every bone, so the rest pose may be skipped.
            Bool                Covered = false;
        };

        /// \brief Resolves what only changes when the clip does, so a frame need not rediscover it.
        ///
        /// \param Entry The source to pair.
        /// \param Rig   The skeleton the clip is playing over.
        /// \return `true` once the clip has been paired with the rig, `false` while it is still loading.
        Bool Rebind(Ref<Source> Entry, ConstRef<Skeleton> Rig);

        /// \brief Seeds a pose and samples one source's clip over it.
        ///
        /// \param Entry  The source to sample, which must be paired and carrying a clip.
        /// \param Rig    The skeleton the clip is playing over.
        /// \param Output Receives the local transform of every bone.
        void Evaluate(ConstRef<Source> Entry, ConstRef<Skeleton> Rig, ConstRef<Skeleton::Pose> Output) const;

        /// \brief Lets a source go, ending whatever fade it was taking part in.
        ///
        /// \param Entry The source to empty.
        void Retire(Ref<Source> Entry);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Model>      mModel;
        Source               mSources[kSources];
        Sequence<Matrix4x3>  mPalette;
        Real64               mClock;
        Real64               mEpoch;
        Real64               mFade;
        UInt8                mActive;
    };
}