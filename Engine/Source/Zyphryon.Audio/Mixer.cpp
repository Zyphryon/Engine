// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Mixer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Real32 ResolveCutoff(Real32 Cutoff)
    {
        return (Cutoff > 0.0f ? Cutoff : static_cast<Real32>(kMixerCutoff));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void MixAccumulate(Ptr<Real32> Dst, ConstPtr<Real32> Src, UInt32 Count, Real32 Start, Real32 End)
    {
        if (Count == 0)
        {
            return;
        }

        const Real32 Step       = (End - Start) / static_cast<Real32>(Count);
        const UInt32 Vectorized = Count & ~3u;

        Vector4       Gain    = Vector4(Start, Start + Step, Start + 2.0f * Step, Start + 3.0f * Step);
        const Vector4 Advance = Vector4(Step * 4.0f);

        UInt32 Index = 0;
        for (; Index < Vectorized; Index += 4)
        {
            const Vector4 Samples     = Vector4(Src + Index);
            const Vector4 Accumulated = Vector4(Dst + Index) + Samples * Gain;
            Accumulated.Store(Dst + Index);

            Gain = Gain + Advance;
        }

        Real32 Scalar = Start + Step * Index;
        for (; Index < Count; ++Index)
        {
            Dst[Index] += Src[Index] * Scalar;
            Scalar     += Step;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Mixer::Mixer()
        : mMasterVolume { 1.0f }
    {
        for (UInt32 Index = 0; Index < Enum::Count<Category>(); ++Index)
        {
            mSubmixVolume[Index].store(1.0f, std::memory_order_relaxed);
        }

        for (UInt32 Index = 0; Index < kMaxInstances; ++Index)
        {
            mRanks[Index].store(0.0f, std::memory_order_relaxed);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Render(Span<Real32> Output, UInt32 Frames)
    {
        for (Command Command; mCommands.Pop(Command); )
        {
            Apply(Command);
        }

        for (UInt32 Offset = 0; Offset < Frames; )
        {
            const UInt32 Count = Min(Frames - Offset, static_cast<UInt32>(kMixerBlock));
            Block(Output.GetData() + Offset * kMixerStride, Count);
            Offset += Count;
        }

        /// Frees voices that finished during the current render and reports why each one ended.
        mVoices.ForEach([this](Ref<Voice> Voice)
        {
            if (Voice.Finished && mCompletions.Push(Completion(Voice.Handle, Voice.Reason)))
            {
                mVoices.Free(Voice.Handle);
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Drain(Ref<Sequence<Completion>> Output)
    {
        for (Completion Completion; mCompletions.Pop(Completion); )
        {
            Output.Append(Completion);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Mixer::Play(Object Handle, Category Category, Ptr<Decoder> Decoder, UInt32 Stride, Real32 Volume)
    {
        Command Command { };
        Command.Kind     = Op::Play;
        Command.Handle   = Handle;
        Command.Category = Category;
        Command.Spatial  = false;
        Command.Volume   = Volume;
        Command.Decoder  = Decoder;
        Command.Stride   = Stride;
        return Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Mixer::Play(Object Handle, Category Category, Ptr<Decoder> Decoder, UInt32 Stride, Real32 Volume, ConstRef<Emitter> Emitter, ConstRef<Matrix4x3> Transform)
    {
        Command Command { };
        Command.Kind      = Op::Play;
        Command.Handle    = Handle;
        Command.Category  = Category;
        Command.Spatial   = true;
        Command.Volume    = Volume;
        Command.Decoder   = Decoder;
        Command.Stride    = Stride;
        Command.Emitter   = Emitter;
        Command.Position  = Transform.GetTranslation();
        Command.Forward   = Vector3::Normalize(Transform.GetForward());
        return Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::SetLooping(Object Handle, Bool Looping)
    {
        Command Command { };
        Command.Kind   = Op::Loop;
        Command.Handle = Handle;
        Command.Flag   = Looping;
        Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::SetVolume(Object Handle, Real32 Volume)
    {
        Command Command { };
        Command.Kind   = Op::Volume;
        Command.Handle = Handle;
        Command.Volume = Volume;
        Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::SetCutoff(Object Handle, Real32 Cutoff)
    {
        Command Command { };
        Command.Kind   = Op::Cutoff;
        Command.Handle = Handle;
        Command.Cutoff = Cutoff;
        Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::SetTransform(Object Handle, ConstRef<Matrix4x3> Transform)
    {
        Command Command { };
        Command.Kind     = Op::Move;
        Command.Handle   = Handle;
        Command.Position = Transform.GetTranslation();
        Command.Forward  = Vector3::Normalize(Transform.GetForward());
        Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Stop(Object Handle)
    {
        Command Command { };
        Command.Kind   = Op::Stop;
        Command.Handle = Handle;
        Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Pause(Object Handle)
    {
        Command Command { };
        Command.Kind   = Op::Pause;
        Command.Handle = Handle;
        Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Resume(Object Handle)
    {
        Command Command { };
        Command.Kind   = Op::Resume;
        Command.Handle = Handle;
        Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::SetListener(ConstRef<Matrix4x3> Transform)
    {
        Command Command { };
        Command.Kind     = Op::Listener;
        Command.Position = Transform.GetTranslation();
        Command.Forward  = Vector3::Normalize(Transform.GetForward());
        Command.Right    = Vector3::Normalize(Transform.GetRight());
        Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain)
    {
        Command Command { };
        Command.Kind       = Op::Cone;
        Command.InnerAngle = InnerAngle;
        Command.OuterAngle = OuterAngle;
        Command.OuterGain  = OuterGain;
        Submit(Command);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Apply(ConstRef<Command> Command)
    {
        switch (Command.Kind)
        {
        case Op::Play:
        {
            // A slot a superseded voice still holds is taken over in silence.
            if (mVoices.IsOccupied(Command.Handle.GetSlot()))
            {
                mVoices.Release(Command.Handle.GetSlot());
            }

            mVoices.Acquire(Command.Handle);

            Ref<Voice> Voice   = mVoices[Command.Handle];
            Voice.Handle       = Command.Handle;
            Voice.Decoder      = Unique(Command.Decoder);
            Voice.Category     = Command.Category;
            Voice.Stride       = static_cast<UInt16>(Command.Stride);
            Voice.Spatial      = Command.Spatial;
            Voice.Volume       = Command.Volume;
            Voice.Looping      = false;
            Voice.Paused       = false;
            Voice.Finished     = false;
            Voice.Primed       = false;
            Voice.Reason       = Reason::Completed;

            // A voice ranks at nothing until it has mixed once, so a sound is never stolen before it is heard.
            mRanks[Command.Handle.GetSlot() - 1].store(0.0f, std::memory_order_relaxed);
            Voice.Gain.Left    = 1.0f;
            Voice.Gain.Right   = 1.0f;

            if (Command.Spatial)
            {
                Voice.Emitter      = Command.Emitter;
                Voice.Position     = Command.Position;
                Voice.Forward      = Command.Forward;
                Voice.Cutoff       = ResolveCutoff(Command.Emitter.GetCutoff());
                Voice.CutoffTarget = Voice.Cutoff;
            }
            break;
        }
        case Op::Stop:
            if (const Ptr<Voice> Voice = mVoices.TryGet(Command.Handle))
            {
                Voice->Reason   = Reason::Stopped;
                Voice->Finished = true;
            }
            break;
        case Op::Pause:
            if (const Ptr<Voice> Voice = mVoices.TryGet(Command.Handle))
            {
                Voice->Paused = true;
            }
            break;
        case Op::Resume:
            if (const Ptr<Voice> Voice = mVoices.TryGet(Command.Handle))
            {
                Voice->Paused = false;
            }
            break;
        case Op::Loop:
            if (const Ptr<Voice> Voice = mVoices.TryGet(Command.Handle))
            {
                Voice->Looping = Command.Flag;
            }
            break;
        case Op::Volume:
            if (const Ptr<Voice> Voice = mVoices.TryGet(Command.Handle))
            {
                Voice->Volume = Command.Volume;
            }
            break;
        case Op::Cutoff:
            if (const Ptr<Voice> Voice = mVoices.TryGet(Command.Handle))
            {
                Voice->CutoffTarget = ResolveCutoff(Command.Cutoff);
            }
            break;
        case Op::Move:
            if (const Ptr<Voice> Voice = mVoices.TryGet(Command.Handle))
            {
                Voice->Position = Command.Position;
                Voice->Forward  = Command.Forward;
            }
            break;
        case Op::Listener:
            mSpatializer.SetListener(Command.Position, Command.Forward, Command.Right);
            break;
        case Op::Cone:
            mSpatializer.SetListenerCone(Command.InnerAngle, Command.OuterAngle, Command.OuterGain);
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Glide(Ref<Voice> Voice, UInt32 Frames)
    {
        if (Voice.Cutoff == Voice.CutoffTarget)
        {
            return;
        }

        const Real32 Count  = static_cast<Real32>(Frames);
        const Real32 Factor = Count / (Count + 0.03f * static_cast<Real32>(kMixerFrequency));

        Voice.Cutoff += (Voice.CutoffTarget - Voice.Cutoff) * Factor;

        if (Abs(Voice.CutoffTarget - Voice.Cutoff) < 1.0f)
        {
            Voice.Cutoff = Voice.CutoffTarget;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Mix(Ref<Voice> Voice, UInt32 Frames)
    {
        const Real32 Submix = mSubmixVolume[Enum::Cast(Voice.Category)].load(std::memory_order_relaxed);

        Real32 TargetLeft;
        Real32 TargetRight;

        if (Voice.Spatial)
        {
            const Gains Gains = mSpatializer.Compute(Voice.Position, Voice.Forward, Voice.Emitter);
            TargetLeft  = Gains.Left  * Voice.Volume * Submix;
            TargetRight = Gains.Right * Voice.Volume * Submix;
        }
        else
        {
            TargetLeft = TargetRight = Voice.Volume * Submix;
        }

        // Publishes what the voice is worth this block, so the game thread can weigh a newcomer against it.
        mRanks[Voice.Handle.GetSlot() - 1].store(
            Rank(Voice.Category, Max(TargetLeft, TargetRight)), std::memory_order_relaxed);

        if (!Voice.Primed)
        {
            Voice.Gain.Left  = TargetLeft;
            Voice.Gain.Right = TargetRight;
            Voice.Primed     = true;
        }

        Glide(Voice, Frames);

        // Under a sixteen-bit step the voice cannot move the output, and it is not still ramping down into one.
        constexpr Real32 kSilent = 1.0f / 32768.0f;

        if (Max(TargetLeft, TargetRight) <= kSilent && Max(Voice.Gain.Left, Voice.Gain.Right) <= kSilent)
        {
            const UInt32 Skipped = Silence(Voice, Frames);

            Voice.Gain.Left  = TargetLeft;
            Voice.Gain.Right = TargetRight;

            if (Skipped < Frames && !Voice.Looping)
            {
                Voice.Reason   = Reason::Completed;
                Voice.Finished = true;
            }
            return;
        }

        const UInt32 Produced = Read(Voice, mScratchLeft.GetData(), mScratchRight.GetData(), Frames);

        if (Produced > 0)
        {
            // Only a voice under the open cutoff pays for a section, and its coefficients follow from that cutoff alone.
            const Bool Occluded = Voice.Cutoff < static_cast<Real32>(kMixerCutoff);

            if (Voice.Spatial)
            {
                if (Occluded)
                {
                    Filter(Voice.Cutoff).Apply(Voice.FilterLeft, mScratchLeft.GetData(), Produced);
                }

                MixAccumulate(mMasterLeft.GetData(),  mScratchLeft.GetData(), Produced, Voice.Gain.Left,  TargetLeft);
                MixAccumulate(mMasterRight.GetData(), mScratchLeft.GetData(), Produced, Voice.Gain.Right, TargetRight);
            }
            else
            {
                if (Occluded)
                {
                    const Filter Section(Voice.Cutoff);

                    Section.Apply(Voice.FilterLeft,  mScratchLeft.GetData(),  Produced);
                    Section.Apply(Voice.FilterRight, mScratchRight.GetData(), Produced);
                }

                MixAccumulate(mMasterLeft.GetData(),  mScratchLeft.GetData(),  Produced, Voice.Gain.Left,  TargetLeft);
                MixAccumulate(mMasterRight.GetData(), mScratchRight.GetData(), Produced, Voice.Gain.Right, TargetRight);
            }

            Voice.Gain.Left  = TargetLeft;
            Voice.Gain.Right = TargetRight;
        }

        // A short read with no looping means the stream reached its end.
        if (Produced < Frames && !Voice.Looping)
        {
            Voice.Reason   = Reason::Completed;
            Voice.Finished = true;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Mixer::Read(Ref<Voice> Voice, Ptr<Real32> Left, Ptr<Real32> Right, UInt32 Frames)
    {
        const UInt32 Capacity = (kMixerBlock * kMixerStride) / Voice.Stride;

        UInt32 Done = 0;

        while (Done < Frames)
        {
            const UInt32 Request = Min(Frames - Done, Capacity) * Voice.Stride;

            UInt32 Decoded = static_cast<UInt32>(Voice.Decoder->Read(Span(mDecode.GetData(), Request)));

            if (Decoded == 0)
            {
                // Restart from the top when looping; otherwise the stream has ended.
                if (Voice.Looping)
                {
                    if (!Voice.Decoder->Seek(0))
                    {
                        Voice.Looping = false;
                        break;
                    }
                }
                else
                {
                    break;
                }

                Decoded = static_cast<UInt32>(Voice.Decoder->Read(Span(mDecode.GetData(), Request)));
                if (Decoded == 0)
                {
                    break;
                }
            }

            // Deinterleave the source frames, folding a spatial voice to the one image its pan reads.
            if (Voice.Spatial)
            {
                if (Voice.Stride == 1)
                {
                    for (UInt32 Frame = 0; Frame < Decoded; ++Frame)
                    {
                        Left[Done + Frame] = mDecode[Frame];
                    }
                }
                else
                {
                    for (UInt32 Frame = 0; Frame < Decoded; ++Frame)
                    {
                        const UInt32 Base = Frame * Voice.Stride;

                        Left[Done + Frame] = 0.5f * (mDecode[Base] + mDecode[Base + 1]);
                    }
                }
            }
            else if (Voice.Stride == 1)
            {
                for (UInt32 Frame = 0; Frame < Decoded; ++Frame)
                {
                    Left[Done + Frame] = Right[Done + Frame] = mDecode[Frame];
                }
            }
            else
            {
                for (UInt32 Frame = 0; Frame < Decoded; ++Frame)
                {
                    const UInt32 Base = Frame * Voice.Stride;

                    Left[Done + Frame]  = mDecode[Base];
                    Right[Done + Frame] = mDecode[Base + 1];
                }
            }
            Done += Decoded;
        }
        return Done;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Mixer::Silence(Ref<Voice> Voice, UInt32 Frames)
    {
        UInt32 Done = 0;

        while (Done < Frames)
        {
            UInt32 Skipped = static_cast<UInt32>(Voice.Decoder->Skip(Frames - Done));

            if (Skipped == 0)
            {
                if (Voice.Looping)
                {
                    if (!Voice.Decoder->Seek(0))
                    {
                        Voice.Looping = false;
                        break;
                    }
                }
                else
                {
                    break;
                }

                Skipped = static_cast<UInt32>(Voice.Decoder->Skip(Frames - Done));

                if (Skipped == 0)
                {
                    break;
                }
            }
            Done += Skipped;
        }
        return Done;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Block(Ptr<Real32> Output, UInt32 Frames)
    {
        for (UInt32 Frame = 0; Frame < Frames; ++Frame)
        {
            mMasterLeft[Frame]  = 0.0f;
            mMasterRight[Frame] = 0.0f;
        }

        mVoices.ForEach([this, Frames](Ref<Voice> Voice)
        {
            if (!Voice.Finished && !Voice.Paused)
            {
                Mix(Voice, Frames);
            }
        });

        const Real32 Master = mMasterVolume.load(std::memory_order_relaxed);

        for (UInt32 Frame = 0; Frame < Frames; ++Frame)
        {
            Output[Frame * kMixerStride + 0] = Clamp(mMasterLeft[Frame]  * Master, -1.0f, 1.0f);
            Output[Frame * kMixerStride + 1] = Clamp(mMasterRight[Frame] * Master, -1.0f, 1.0f);
        }
    }
}