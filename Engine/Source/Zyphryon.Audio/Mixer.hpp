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

        /// \brief Drains the handles of voices that completed since the last call (game thread).
        ///
        /// \param Output The sequence that receives the completed playback handles.
        void Drain(Ref<Sequence<Object>> Output);

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
            Play, Stop, Pause, Resume, Loop, Volume, Move, Listener, Cone
        };

        /// \brief A single queued control command transferred from the game thread to the audio thread.
        ///
        /// \note Members are ordered by descending alignment to minimize padding.
        struct Command final
        {
            Matrix4x4    Transform;
            Ptr<Decoder> Decoder;
            Emitter      Emitter;
            Real32       Volume;
            UInt32       Stride;
            Angle        InnerAngle;
            Angle        OuterAngle;
            Real32       OuterGain;
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

        /// \brief Mixes a single voice into the master accumulators for the current block (audio thread).
        ///
        /// \param Voice  The voice to mix.
        /// \param Frames The number of frames to mix.
        void Mix(Ref<Voice> Voice, UInt32 Frames);

        /// \brief Reads a voice's decoded PCM into planar stereo scratch, handling looping (audio thread).
        ///
        /// \param Voice  The voice to read from.
        /// \param Left   Destination buffer for the left channel.
        /// \param Right  Destination buffer for the right channel.
        /// \param Frames The number of frames to read.
        /// \return The number of frames actually read; fewer than \p Frames indicates the stream ended.
        UInt32 Read(Ref<Voice> Voice, Ptr<Real32> Left, Ptr<Real32> Right, UInt32 Frames);

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

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Pool<Voice, kMaxInstances>                     mVoices;
        Spatializer                                    mSpatializer;
        Ring<Command, kMaxCommands>                    mCommands;
        Ring<Object, kMaxCommands>                     mCompletions;
        Atomic<Real32>                                 mMasterVolume;
        Array<Atomic<Real32>, Enum::Count<Category>()> mSubmixVolume;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZY_ALIGN(16) Array<Real32, kMixerBlock>        mMasterLeft;
        ZY_ALIGN(16) Array<Real32, kMixerBlock>        mMasterRight;
        ZY_ALIGN(16) Array<Real32, kMixerBlock>        mScratchLeft;
        ZY_ALIGN(16) Array<Real32, kMixerBlock>        mScratchRight;
        Array<Real32, kMixerBlock * kMixerStride>      mDecode;
    };
}
