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
#include "Zyphryon.Math/Matrix4x3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAudio
{
    /// \brief Represents the engine's software audio mixer: voices, submixes, spatialization, and effects.
    class ZY_API Mixer final
    {
    public:

        /// \brief Constructs the mixer with default volumes and an empty voice pool.
        Mixer();

        /// \brief Renders interleaved output frames (audio thread).
        ///
        /// \param Output Receives the interleaved frames, sized `Frames * kMixerStride`.
        /// \param Frames The number of frames to render.
        void Render(Span<Real32> Output, UInt32 Frames);

        /// \brief Drains the voices that ended since the last call, with the reason each one ended (game thread).
        ///
        /// \param Output Receives the ended playbacks.
        void Drain(Ref<Sequence<Completion>> Output);

        /// \brief Enqueues a playback command, spatial when an emitter and a transform are given.
        ///
        /// \param Handle    The pre-allocated playback handle.
        /// \param Category  The submix category the voice is routed through.
        /// \param Decoder   The decoder whose ownership transfers to the mixer.
        /// \param Stride    The decoder's interleaved sample count per frame.
        /// \param Volume    The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Emitter   The spatial configuration of the source, or `nullptr` for a non-spatial playback.
        /// \param Transform The source's world transform, or `nullptr` for a non-spatial playback.
        /// \return `true` if the command was enqueued, `false` if the queue was full.
        Bool Play(
            Object              Handle,
            Category            Category,
            Ptr<Decoder>        Decoder,
            UInt32              Stride,
            Real32              Volume,
            ConstPtr<Emitter>   Emitter   = nullptr,
            ConstPtr<Matrix4x3> Transform = nullptr);

        /// \brief Enqueues a command to set whether a voice loops.
        ///
        /// \param Handle  The playback to change.
        /// \param Looping `true` to loop, `false` to play once.
        void SetLooping(Object Handle, Bool Looping);

        /// \brief Enqueues a command to set a voice's volume.
        ///
        /// \param Handle The playback to change.
        /// \param Volume The playback volume (0.0 = silent, 1.0 = full volume).
        void SetVolume(Object Handle, Real32 Volume);

        /// \brief Enqueues a command to set a voice's low-pass cutoff.
        ///
        /// \param Handle The playback to change.
        /// \param Cutoff The cutoff frequency in hertz, or `0` to leave the voice unfiltered.
        void SetCutoff(Object Handle, Real32 Cutoff);

        /// \brief Enqueues a command to set a spatial voice's world transform.
        ///
        /// \param Handle    The playback to move.
        /// \param Transform The source's world transform.
        void SetTransform(Object Handle, ConstRef<Matrix4x3> Transform);

        /// \brief Enqueues a command to stop a voice.
        ///
        /// \param Handle The playback to stop.
        void Stop(Object Handle);

        /// \brief Enqueues a command to pause a voice.
        ///
        /// \param Handle The playback to pause.
        void Pause(Object Handle);

        /// \brief Enqueues a command to resume a voice.
        ///
        /// \param Handle The playback to resume.
        void Resume(Object Handle);

        /// \brief Enqueues a command to set the listener's world transform.
        ///
        /// \param Transform The listener's new world transform.
        void SetListener(ConstRef<Matrix4x3> Transform);

        /// \brief Enqueues a command to set the listener's directional cone.
        ///
        /// \param InnerAngle The width of the cone heard at full gain.
        /// \param OuterAngle The width of the cone past which only `OuterGain` is heard.
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
        /// \return The master volume (0.0 = silent, 1.0 = full volume).
        ZY_INLINE Real32 GetMasterVolume() const
        {
            return mMasterVolume.load(std::memory_order_relaxed);
        }

        /// \brief Sets the volume of a submix category.
        ///
        /// \param Category The submix category to adjust.
        /// \param Volume   The submix volume (0.0 = silent, 1.0 = full volume).
        ZY_INLINE void SetSubmixVolume(Category Category, Real32 Volume)
        {
            mSubmixVolume[ZyEnum::Cast(Category)].store(Volume, std::memory_order_relaxed);
        }

        /// \brief Gets the volume of a submix category.
        ///
        /// \param Category The submix category to query.
        /// \return The submix volume (0.0 = silent, 1.0 = full volume).
        ZY_INLINE Real32 GetSubmixVolume(Category Category) const
        {
            return mSubmixVolume[ZyEnum::Cast(Category)].load(std::memory_order_relaxed);
        }

        /// \brief Ranks a voice from its category and the gain it reaches the listener at.
        ///
        /// \note A category floor outranks any gain, so music is never pushed out and a footstep never takes dialogue.
        ///
        /// \param Category The submix category the voice is routed through.
        /// \param Gain     The post-attenuation linear gain (range [0, 1]).
        /// \return The rank, where a larger value names a voice worth keeping over a smaller one.
        ZY_INLINE static constexpr Real32 Rank(Category Category, Real32 Gain)
        {
            // Category floors in declaration order, each above zero so an unmixed slot stays distinguishable at zero.
            constexpr Array kFloor(6.0f, 2.0f, 4.0f, 1.0f, 5.0f);
            return kFloor[ZyEnum::Cast(Category)] + Min(Gain, 1.0f);
        }

    private:

        /// \brief Specifies the kind of a queued control command.
        enum class Op : UInt8
        {
            Play,           ///< Starts a voice in the slot its handle names.
            Stop,           ///< Ends a voice, reporting it stopped.
            Pause,          ///< Holds a voice where it is.
            Resume,         ///< Lets a paused voice advance again.
            Loop,           ///< Sets whether a voice loops.
            Volume,         ///< Sets a voice's volume.
            Cutoff,         ///< Sets the cutoff a voice's low-pass glides toward.
            Move,           ///< Sets a spatial voice's position and forward axis.
            Listener,       ///< Sets the listener's position and basis.
            Cone,           ///< Sets the listener's cone.
        };

        /// \brief Represents a single queued control command transferred from the game thread to the audio thread.
        struct Command final
        {
            /// The decoder a play command hands over to the voice.
            Ptr<Decoder> Decoder;

            /// The spatial configuration a spatial play command carries.
            Emitter      Emitter;

            /// The world-space position a play, move or listener command carries.
            Vector3      Position;

            /// The normalized forward axis a play, move or listener command carries.
            Vector3      Forward;

            /// The normalized right axis a listener command carries.
            Vector3      Right;

            /// The volume a play or volume command carries.
            Real32       Volume;

            /// The interleaved samples per frame of a play command's decoder.
            UInt32       Stride;

            /// The inner angle a cone command carries.
            Angle        InnerAngle;

            /// The outer angle a cone command carries.
            Angle        OuterAngle;

            /// The gain outside the outer angle a cone command carries.
            Real32       OuterGain;

            /// The cutoff frequency, in hertz, a cutoff command carries.
            Real32       Cutoff;

            /// The playback the command addresses.
            Object       Handle;

            /// The kind of command.
            Op           Kind;

            /// The submix category a play command routes through.
            Category     Category;

            /// `true` when a play command starts a spatial voice.
            Bool         Spatial;

            /// `true` when a loop command turns looping on.
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
        /// \param Left   Receives the left channel, or the whole image of a spatial voice.
        /// \param Right  Receives the right channel, left untouched by a spatial voice.
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
        /// \param Output Receives the interleaved frames of this sub-block.
        /// \param Frames The number of frames in this sub-block.
        void Block(Ptr<Real32> Output, UInt32 Frames);

        /// \brief Creates a zeroed command of the given kind for the given playback.
        ///
        /// \param Kind   The operation the command performs.
        /// \param Handle The playback the command addresses, or none for a listener command.
        /// \return The command, with every other field zeroed.
        static Command Make(Op Kind, Object Handle = Object());

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Pool<Voice, kMaxInstances>                       mVoices;
        Spatializer                                      mSpatializer;
        Ring<Command, kMaxCommands>                      mCommands;
        Ring<Completion, kMaxCommands>                   mCompletions;
        Atomic<Real32>                                   mMasterVolume;
        Array<Atomic<Real32>, ZyEnum::Count<Category>()> mSubmixVolume;
        Array<Atomic<Real32>, kMaxInstances>             mRanks;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZY_ALIGN(16) Array<Real32, kMixerBlock>          mMasterLeft;
        ZY_ALIGN(16) Array<Real32, kMixerBlock>          mMasterRight;
        ZY_ALIGN(16) Array<Real32, kMixerBlock>          mScratchLeft;
        ZY_ALIGN(16) Array<Real32, kMixerBlock>          mScratchRight;
        Array<Real32, kMixerBlock * kMixerStride>        mDecode;
    };
}