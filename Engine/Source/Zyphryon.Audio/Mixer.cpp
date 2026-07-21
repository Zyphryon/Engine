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

        /// Frees voices that finished during the current render and reports their handles.
        mVoices.ForEach([this](Ref<Voice> Voice)
        {
            if (Voice.Finished && mCompletions.Push(Voice.Handle))
            {
                mVoices.Free(Voice.Handle);
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mixer::Drain(Ref<Sequence<Object>> Output)
    {
        for (Object Handle; mCompletions.Pop(Handle); )
        {
            Output.Append(Handle);
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

    Bool Mixer::Play(Object Handle, Category Category, Ptr<Decoder> Decoder, UInt32 Stride, Real32 Volume, ConstRef<Emitter> Emitter, ConstRef<Matrix4x4> Transform)
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
        Command.Transform = Transform;
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

    void Mixer::SetTransform(Object Handle, ConstRef<Matrix4x4> Transform)
    {
        Command Command { };
        Command.Kind      = Op::Move;
        Command.Handle    = Handle;
        Command.Transform = Transform;
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

    void Mixer::SetListener(ConstRef<Matrix4x4> Transform)
    {
        Command Command { };
        Command.Kind      = Op::Listener;
        Command.Transform = Transform;
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
            mVoices.Acquire(Command.Handle);

            Ref<Voice> Voice = mVoices[Command.Handle];
            Voice.Handle     = Command.Handle;
            Voice.Decoder    = Unique(Command.Decoder);
            Voice.Category   = Command.Category;
            Voice.Stride     = static_cast<UInt16>(Command.Stride);
            Voice.Spatial    = Command.Spatial;
            Voice.Volume     = Command.Volume;
            Voice.Looping    = false;
            Voice.Paused     = false;
            Voice.Finished   = false;
            Voice.Primed     = false;
            Voice.Gain.Left  = 1.0f;
            Voice.Gain.Right = 1.0f;

            if (Command.Spatial)
            {
                Voice.Emitter  = Command.Emitter;
                Voice.Position = Command.Transform.GetTranslation();
                Voice.Forward  = Vector3::Normalize(Command.Transform.GetForward());
            }
            break;
        }
        case Op::Stop:
            if (const Ptr<Voice> Voice = mVoices.TryGet(Command.Handle))
            {
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
        case Op::Move:
            if (const Ptr<Voice> Voice = mVoices.TryGet(Command.Handle))
            {
                Voice->Position = Command.Transform.GetTranslation();
                Voice->Forward  = Vector3::Normalize(Command.Transform.GetForward());
            }
            break;
        case Op::Listener:
            mSpatializer.SetListener(
                Command.Transform.GetTranslation(),
                Vector3::Normalize(Command.Transform.GetForward()),
                Vector3::Normalize(Command.Transform.GetRight()));
            break;
        case Op::Cone:
            mSpatializer.SetListenerCone(Command.InnerAngle, Command.OuterAngle, Command.OuterGain);
            break;
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

        const UInt32 Produced = Read(Voice, mScratchLeft.GetData(), mScratchRight.GetData(), Frames);

        if (!Voice.Primed)
        {
            Voice.Gain.Left  = TargetLeft;
            Voice.Gain.Right = TargetRight;
            Voice.Primed     = true;
        }

        if (Produced > 0)
        {
            if (Voice.Spatial)
            {
                // Collapse the stereo source to mono, then pan it into both channels.
                for (UInt32 Frame = 0; Frame < Produced; ++Frame)
                {
                    mScratchLeft[Frame] = 0.5f * (mScratchLeft[Frame] + mScratchRight[Frame]);
                }
                MixAccumulate(mMasterLeft.GetData(),  mScratchLeft.GetData(), Produced, Voice.Gain.Left,  TargetLeft);
                MixAccumulate(mMasterRight.GetData(), mScratchLeft.GetData(), Produced, Voice.Gain.Right, TargetRight);
            }
            else
            {
                MixAccumulate(mMasterLeft.GetData(),  mScratchLeft.GetData(),  Produced, Voice.Gain.Left,  TargetLeft);
                MixAccumulate(mMasterRight.GetData(), mScratchRight.GetData(), Produced, Voice.Gain.Right, TargetRight);
            }

            Voice.Gain.Left  = TargetLeft;
            Voice.Gain.Right = TargetRight;
        }

        // A short read with no looping means the stream reached its end.
        if (Produced < Frames && !Voice.Looping)
        {
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
            const UInt32 Want = Min(Frames - Done, Capacity);

            UInt32 Got = static_cast<UInt32>(Voice.Decoder->Read(Span(mDecode.GetData(), Want)));

            if (Got == 0)
            {
                // Restart from the top when looping; otherwise the stream has ended.
                if (!Voice.Looping || !Voice.Decoder->Seek(0))
                {
                    break;
                }

                Got = static_cast<UInt32>(Voice.Decoder->Read(Span(mDecode.GetData(), Want)));
                if (Got == 0)
                {
                    break;
                }
            }

            // Deinterleave the source frames into planar stereo.
            if (Voice.Stride == 1)
            {
                for (UInt32 Frame = 0; Frame < Got; ++Frame)
                {
                    Left[Done + Frame] = Right[Done + Frame] = mDecode[Frame];
                }
            }
            else
            {
                for (UInt32 Frame = 0; Frame < Got; ++Frame)
                {
                    const UInt32 Base = Frame * Voice.Stride;

                    Left[Done + Frame]  = mDecode[Base];
                    Right[Done + Frame] = mDecode[Base + 1];
                }
            }
            Done += Got;
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
