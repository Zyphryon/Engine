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

#include "Spatializer.hpp"
#include "Voice.hpp"
#include "Zyphryon.Math/Matrix4x4.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief The engine's software audio mixer: voices, submixes, spatialization, and effects.
    class Mixer final
    {
    public:

        /// \brief Constructs the mixer with default volumes and an empty voice pool.
        Mixer();

        /// \brief Renders interleaved output frames (audio thread).
        ///
        /// \param Output The interleaved destination buffer, sized `Frames * channels`.
        /// \param Frames The number of frames to render.
        void Render(Span<Real32> Output, UInt32 Frames);

        /// \brief Drains the voices that ended since the last call, with the reason each one ended (game thread).
        ///
        /// \param Output The sequence that receives the ended playbacks.
        void Drain(Ref<Sequence<Completion>> Output);

        /// \brief Enqueues a non-spatial playback command.
        ///
        /// \param Handle    The pre-allocated playback handle.
        /// \param Category  The submix category.
        /// \param Decoder   The decoder whose ownership transfers to the mixer.
        /// \param Stride    The decoder's interleaved sample count per frame (stride).
        /// \param Volume    The playback volume (0.0 = silent, 1.0 = full volume).
        /// \return `true` if the command was enqueued, `false` if the queue was full.
        Bool Play(Object Handle, Category Category, Ptr<Decoder> Decoder, UInt32 Stride, Real32 Volume);

        /// \brief Enqueues a spatial playback command.
        ///
        /// \param Handle    The pre-allocated playback handle.
        /// \param Category  The submix category.
        /// \param Decoder   The decoder whose ownership transfers to the mixer.
        /// \param Stride    The decoder's interleaved sample count per frame (stride).
        /// \param Volume    The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Emitter   The spatial configuration of the source.
        /// \param Transform The source's world transform.
        /// \return `true` if the command was enqueued, `false` if the queue was full.
        Bool Play(Object Handle, Category Category, Ptr<Decoder> Decoder, UInt32 Stride, Real32 Volume, ConstRef<Emitter> Emitter, ConstRef<Matrix4x4> Transform);

        /// \brief Enqueues a command to set whether a voice loops.
        ///
        /// \param Handle  The playback handle.
        /// \param Looping `true` to loop, `false` to play once.
        void SetLooping(Object Handle, Bool Looping);

        /// \brief Enqueues a command to set a voice's volume.
        ///
        /// \param Handle The playback handle.
        /// \param Volume The new volume.
        void SetVolume(Object Handle, Real32 Volume);

        /// \brief Enqueues a command to set a voice's low-pass cutoff.
        ///
        /// \param Handle The playback handle.
        /// \param Cutoff The cutoff frequency in hertz, or `0` to leave the voice unfiltered.
        void SetCutoff(Object Handle, Real32 Cutoff);

        /// \brief Enqueues a command to set a spatial voice's world transform.
        ///
        /// \param Handle    The playback handle.
        /// \param Transform The new world transform.
        void SetTransform(Object Handle, ConstRef<Matrix4x4> Transform);

        /// \brief Enqueues a command to stop a voice.
        ///
        /// \param Handle The playback handle.
        void Stop(Object Handle);

        /// \brief Enqueues a command to pause a voice.
        ///
        /// \param Handle The playback handle.
        void Pause(Object Handle);

        /// \brief Enqueues a command to resume a voice.
        ///
        /// \param Handle The playback handle.
        void Resume(Object Handle);

        /// \brief Enqueues a command to set the listener's world transform.
        ///
        /// \param Transform The listener's new world transform.
        void SetListener(ConstRef<Matrix4x4> Transform);

        /// \brief Enqueues a command to set the listener's directional cone.
        ///
        /// \param InnerAngle The inner cone angle.
        /// \param OuterAngle The outer cone angle.
        /// \param OuterGain  The gain applied outside the outer cone.
        void SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain);

        /// \brief Gets the rank a slot's voice mixed at during the last block (game thread).
        ///
        /// \note A slot that has not mixed since it was filled reads `0`, naming a voice not yet heard.
        ///
        /// \param Slot The slot to read, counted from one.
        /// \return The voice's rank as of the last block.
        ZY_INLINE Real32 GetRank(Object::Slot Slot) const
        {
            ZY_ASSERT(Slot > 0 && Slot <= kMaxInstances, "Slot must name a voice");

            return mRanks[Slot - 1].load(std::memory_order_relaxed);
        }

        /// \brief Sets the master volume applied to the final mix.
        ///
        /// \param Volume The master volume (0.0 = silent, 1.0 = full volume).
        ZY_INLINE void SetMasterVolume(Real32 Volume)
        {
            mMasterVolume.store(Volume, std::memory_order_relaxed);
        }

        /// \brief Gets the master volume.
        ///
        /// \return The master volume.
        ZY_INLINE Real32 GetMasterVolume() const
        {
            return mMasterVolume.load(std::memory_order_relaxed);
        }

        /// \brief Sets the volume of a submix category.
        ///
        /// \param Category The submix category.
        /// \param Volume   The new volume.
        ZY_INLINE void SetSubmixVolume(Category Category, Real32 Volume)
        {
            mSubmixVolume[Enum::Cast(Category)].store(Volume, std::memory_order_relaxed);
        }

        /// \brief Gets the volume of a submix category.
        ///
        /// \param Category The submix category.
        /// \return The submix volume.
        ZY_INLINE Real32 GetSubmixVolume(Category Category) const
        {
            return mSubmixVolume[Enum::Cast(Category)].load(std::memory_order_relaxed);
        }

    private:

        /// \brief The kind of a queued control command.
        enum class Op : UInt8
        {
            Play, Stop, Pause, Resume, Loop, Volume, Cutoff, Move, Listener, Cone
        };

        /// \brief A single queued control command transferred from the game thread to the audio thread.
        struct Command final
        {
            Ptr<Decoder> Decoder;
            Emitter      Emitter;
            Vector3      Position;
            Vector3      Forward;
            Vector3      Right;
            Real32       Volume;
            UInt32       Stride;
            Angle        InnerAngle;
            Angle        OuterAngle;
            Real32       OuterGain;
            Real32       Cutoff;
            Object       Handle;
            Op           Kind;
            Category     Category;
            Bool         Spatial;
            Bool         Flag;
        };

    private:

        /// \brief Applies a single command (audio thread).
        ///
        /// \param Command The command to apply.
        void Apply(ConstRef<Command> Command);

        /// \brief Advances a voice's cutoff one block toward its target (audio thread).
        ///
        /// \param Voice  The voice whose cutoff to advance.
        /// \param Frames The number of frames in the current block.
        void Glide(Ref<Voice> Voice, UInt32 Frames);

        /// \brief Mixes a single voice into the master accumulators for the current block (audio thread).
        ///
        /// \param Voice  The voice to mix.
        /// \param Frames The number of frames to mix.
        void Mix(Ref<Voice> Voice, UInt32 Frames);

        /// \brief Reads a voice's decoded PCM into planar scratch, handling looping (audio thread).
        ///
        /// \param Voice  The voice to read from.
        /// \param Left   Destination buffer for the left channel, or for the image of a spatial voice.
        /// \param Right  Destination buffer for the right channel, left untouched by a spatial voice.
        /// \param Frames The number of frames to read.
        /// \return The number of frames actually read; fewer than \p Frames indicates the stream ended.
        UInt32 Read(Ref<Voice> Voice, Ptr<Real32> Left, Ptr<Real32> Right, UInt32 Frames);

        /// \brief Advances a voice past a block it contributes nothing to, so a silent voice still ends on time.
        ///
        /// \param Voice  The voice to advance.
        /// \param Frames The number of frames to advance past.
        /// \return The number of frames actually advanced; fewer than \p Frames indicates the stream ended.
        UInt32 Silence(Ref<Voice> Voice, UInt32 Frames);

        /// \brief Renders one sub-block of up to \ref kMixerBlock frames (audio thread).
        ///
        /// \param Output The interleaved destination for this sub-block.
        /// \param Frames The number of frames in this sub-block.
        void Block(Ptr<Real32> Output, UInt32 Frames);

        /// \brief Enqueues a command, dropping it if the queue is full.
        ///
        /// \param Command The command to enqueue.
        /// \return `true` if the command was enqueued, `false` otherwise.
        ZY_INLINE Bool Submit(ConstRef<Command> Command)
        {
            return mCommands.Push(Command);
        }

    public:

        /// \brief Ranks a voice from its category and the gain it reaches the listener at.
        ///
        /// \note A category floor outranks any gain, so a footstep never takes the slot music or dialogue holds.
        ///
        /// \param Category The submix category.
        /// \param Gain     The post-attenuation linear gain (range [0, 1]).
        /// \return The rank, where a larger value names a voice worth keeping over a smaller one.
        ZY_INLINE static constexpr Real32 Rank(Category Category, Real32 Gain)
        {
            // Category floors in declaration order, each above zero so an unmixed slot stays distinguishable at zero.
            constexpr Array kFloor(3.0f, 2.0f, 4.0f, 1.0f, 5.0f);
            return kFloor[Enum::Cast(Category)] + Min(Gain, 1.0f);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Pool<Voice, kMaxInstances>                     mVoices;
        Spatializer                                    mSpatializer;
        Ring<Command, kMaxCommands>                    mCommands;
        Ring<Completion, kMaxCommands>                 mCompletions;
        Atomic<Real32>                                 mMasterVolume;
        Array<Atomic<Real32>, Enum::Count<Category>()> mSubmixVolume;
        Array<Atomic<Real32>, kMaxInstances>           mRanks;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZY_ALIGN(16) Array<Real32, kMixerBlock>        mMasterLeft;
        ZY_ALIGN(16) Array<Real32, kMixerBlock>        mMasterRight;
        ZY_ALIGN(16) Array<Real32, kMixerBlock>        mScratchLeft;
        ZY_ALIGN(16) Array<Real32, kMixerBlock>        mScratchRight;
        Array<Real32, kMixerBlock * kMixerStride>      mDecode;
    };
}