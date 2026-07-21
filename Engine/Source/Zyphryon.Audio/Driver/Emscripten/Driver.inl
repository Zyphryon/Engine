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

#include <emscripten/emscripten.h>

#if defined(ZY_WEB_THREAD)
#include <emscripten/webaudio.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Driver::Backend
    {
#if defined(ZY_WEB_THREAD)
        /// The fixed Web Audio render quantum: 128 interleaved frames per worklet process call.
        static constexpr UInt32 kBlock     = 128;
#else
        /// Frames pulled from the mixer per scheduled buffer (~21 ms at 48 kHz).
        static constexpr UInt32 kBlock     = 1024;
#endif

        /// The render callback the service installs; the mixer fills \c Mix through it each block.
        Driver::Callback Callback;

        /// Interleaved stereo scratch the mixer fills each block before it is handed to Web Audio.
        Real32           Mix[kBlock * kMixerStride];

#if defined(ZY_WEB_THREAD)

        /// The stack size, in bytes, handed to the dedicated audio worklet thread.
        static constexpr UInt32 kStackSize = 64 * 1024;

        /// Handle to the Web Audio context that owns the worklet graph.
        EMSCRIPTEN_WEBAUDIO_T Context = 0;

        /// Handle to the worklet node that drives the mixer render callback.
        EMSCRIPTEN_WEBAUDIO_T Node    = 0;

        /// Signals the worklet callback to render audio; \c false makes it emit silence.
        Atomic<Bool>          Running = false;

        /// Dedicated stack for the audio worklet thread (must stay alive for the driver's lifetime).
        ZY_ALIGN(16) Byte     Stack[kStackSize];

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL WorkletProcess(int, ConstPtr<AudioSampleFrame>, int Outputs, Ptr<AudioSampleFrame> Output,
            int, ConstPtr<AudioParamFrame>, Ptr<void> Parameter)
        {
            Ref<Driver::Backend> Backend = * static_cast<Ptr<Driver::Backend>>(Parameter);

            if (Outputs > 0)
            {
                Ref<AudioSampleFrame> Frame    = Output[0];
                const UInt32          Channels = static_cast<UInt32>(Frame.numberOfChannels);

                if (Backend.Running.load(std::memory_order_acquire))
                {
                    // Ask the mixer for one interleaved quantum, then deinterleave into the planar output channels.
                    Backend.Callback(Span(Backend.Mix, kBlock * static_cast<UInt32>(kMixerStride)), kBlock);

                    for (UInt32 Sample = 0; Sample < kBlock; ++Sample)
                    {
                        for (UInt32 Channel = 0; Channel < Channels; ++Channel)
                        {
                            const UInt32 Lane = (Channel < kMixerStride ? Channel : kMixerStride - 1);
                            Frame.data[Channel * kBlock + Sample] = Backend.Mix[Sample * kMixerStride + Lane];
                        }
                    }
                }
                else
                {
                    // Not yet started (or suspended): emit silence so the graph keeps pulling without artifacts.
                    for (UInt32 Index = 0; Index < kBlock * Channels; ++Index)
                    {
                        Frame.data[Index] = 0.0f;
                    }
                }
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static void OnNodeReady(EMSCRIPTEN_WEBAUDIO_T Context, EM_BOOL Success, Ptr<void> Parameter)
        {
            Ref<Driver::Backend> Backend = * static_cast<Ptr<Driver::Backend>>(Parameter);

            if (!Success)
            {
                LOG_E("Audio: Failed to create the audio worklet processor");
                return;
            }

            // A single stereo output; the mixer always produces interleaved 48 kHz stereo float.
            int OutputChannels[]                              = { kMixerStride };
            EmscriptenAudioWorkletNodeCreateOptions Options   = { };
            Options.numberOfInputs                            = 0;
            Options.numberOfOutputs                           = 1;
            Options.outputChannelCounts                       = OutputChannels;

            Backend.Node = emscripten_create_wasm_audio_worklet_node(
                Context, "zyphryon-mixer", AddressOf(Options), WorkletProcess, AddressOf(Backend));

            // Route the node to the context's speakers and allow the render callback to produce audio.
            emscripten_audio_node_connect(Backend.Node, Context, 0, 0);
            Backend.Running.store(true, std::memory_order_release);

            // Browsers start an AudioContext suspended until a user gesture; try now and again on the first input.
            emscripten_resume_audio_context_async(Context, nullptr, nullptr);

            MAIN_THREAD_EM_ASM(
            {
                const Context = emscriptenGetAudioObject($0);
                if (Context)
                {
                    const Resume = () => { Context.resume(); };
                    document.addEventListener('click',      Resume, { once : true });
                    document.addEventListener('keydown',    Resume, { once : true });
                    document.addEventListener('touchstart', Resume, { once : true });
                }
            }, Context);
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static void OnProcessorReady(EMSCRIPTEN_WEBAUDIO_T Context, EM_BOOL Success, Ptr<void> Parameter)
        {
            if (!Success)
            {
                LOG_E("Audio: Failed to bootstrap the audio worklet processor");
                return;
            }

            WebAudioWorkletProcessorCreateOptions Options = { };
            Options.name = "zyphryon-mixer";

            emscripten_create_wasm_audio_worklet_processor_async(Context, AddressOf(Options), OnNodeReady, Parameter);
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static void OnThreadReady(EMSCRIPTEN_WEBAUDIO_T Context, EM_BOOL Success, Ptr<void> Parameter)
        {
            if (!Success)
            {
                LOG_E("Audio: Failed to start the audio worklet thread");
                return;
            }

            OnProcessorReady(Context, EM_TRUE, Parameter);
        }

#else

        /// Seconds of audio kept scheduled ahead of the context clock to absorb main-loop jitter.
        static constexpr Real64 kLookahead = 0.150;

        /// Context-clock time, in seconds, up to which audio blocks have already been scheduled.
        Real64 Scheduled = 0.0;

#endif
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Driver::Driver()
        : mBackend { Unique<Backend>::Create() }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Driver::~Driver()
    {
        Close();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Driver::Open(Text Device, AnyRef<Callback> Render)
    {
        mBackend->Callback = Move(Render);

#if defined(ZY_WEB_THREAD)

        // Force the context to the mixer's fixed clock so no browser-side resampling (and pitch shift) is needed.
        EmscriptenWebAudioCreateAttributes Attributes = { };
        Attributes.latencyHint = "interactive";
        Attributes.sampleRate  = kMixerFrequency;

        mBackend->Context = emscripten_create_audio_context(AddressOf(Attributes));

        if (mBackend->Context <= 0)
        {
            LOG_E("Audio: Failed to create the Web Audio context");
            return false;
        }

        // Spin up the dedicated audio worklet thread; the render node is created once it signals ready.
        emscripten_start_wasm_audio_worklet_thread_async(
            mBackend->Context, mBackend->Stack, Backend::kStackSize, Backend::OnThreadReady, mBackend.Grab());

#else

        // The context is locked to the mixer's fixed clock so no browser-side resampling (and pitch shift) occurs.
        const SInt32 Created = EM_ASM_INT(
        {
            try
            {
                const Constructor = window.AudioContext || window.webkitAudioContext;
                const Context     = new Constructor({ sampleRate : $0, latencyHint : 'interactive' });
                Module['zyWebAudio'] = Context;

                // Browsers start a context suspended until a user gesture; resume on the first interaction.
                const Resume = function() { Context.resume(); };
                document.addEventListener('click',      Resume, { once : true });
                document.addEventListener('keydown',    Resume, { once : true });
                document.addEventListener('touchstart', Resume, { once : true });
                return 1;
            }
            catch (Error)
            {
                return 0;
            }
        }, kMixerFrequency);

        if (!Created)
        {
            LOG_E("Audio: Failed to create the Web Audio context");
            return false;
        }

        mBackend->Scheduled = 0.0;

#endif
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Close()
    {
#if defined(ZY_WEB_THREAD)
        emscripten_destroy_audio_context(mBackend->Context);
#else
        EM_ASM({ Module['zyWebAudio'].close(); }); // ctx.close() inside
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Probe(Ref<Description> Output) const
    {
        Output.Backend = "WebAudio";
        Output.Adapter = "Default";
        Output.Endpoints.Append("Default");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Advance([[maybe_unused]] Real64 Delta)
    {
#if !defined(ZY_WEB_THREAD)

        const Real64 Now = EM_ASM_DOUBLE(
        {
            return Module['zyWebAudio'].currentTime;
        });

        // Recover the cursor after a stall (or before the first user gesture) so blocks are never scheduled behind.
        if (mBackend->Scheduled < Now)
        {
            mBackend->Scheduled = Now;
        }

        const Real64 Horizon = Now + Backend::kLookahead;

        while (mBackend->Scheduled < Horizon)
        {
            mBackend->Callback(Span(mBackend->Mix, Backend::kBlock * static_cast<UInt32>(kMixerStride)), Backend::kBlock);

            // Copy one interleaved-stereo block from the wasm heap into a fresh buffer and schedule it at 'When'.
            EM_ASM(
            {
                const Context = Module['zyWebAudio'];
                const Frames  = $1;
                const Buffer  = Context.createBuffer(2, Frames, Context.sampleRate);
                const Left    = Buffer.getChannelData(0);
                const Right   = Buffer.getChannelData(1);
                const Base    = $0 >> 2;

                for (var Index = 0; Index < Frames; ++Index)
                {
                    Left[Index]  = HEAPF32[Base + Index * 2 + 0];
                    Right[Index] = HEAPF32[Base + Index * 2 + 1];
                }

                const Source  = Context.createBufferSource();
                Source.buffer = Buffer;
                Source.connect(Context.destination);
                Source.start($2);
            }, mBackend->Mix, Backend::kBlock, mBackend->Scheduled);

            mBackend->Scheduled += static_cast<Real64>(Backend::kBlock) / static_cast<Real64>(kMixerFrequency);
        }

#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Suspend()
    {
#if defined(ZY_WEB_THREAD)
        MAIN_THREAD_EM_ASM({
            emscriptenGetAudioObject($0).suspend();
        }, mBackend->Context);
#else
        EM_ASM({ Module['zyWebAudio'].suspend(); });
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Restore()
    {
#if defined(ZY_WEB_THREAD)
        emscripten_resume_audio_context_async(mBackend->Context, nullptr, nullptr);
#else
        EM_ASM({ Module['zyWebAudio'].resume(); });
#endif
    }
}
