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

#include "ALSA.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAudio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Driver::Backend
    {
        /// The frames pulled from the mixer per write (~5 ms at 48 kHz), which is also the period asked of ALSA.
        static constexpr UInt32 kBlock = 256;

        /// The endpoint the game asked for, or empty for the system default.
        Str              Request;

        /// The endpoint `Device` was opened from, matched literally by ALSA, so the default is lower case.
        Str              Name      = "default";

        /// The playback stream, owned from a successful `Open` until `Close` drains and releases it.
        Ptr<snd_pcm_t>   Device    = nullptr;

        /// The thread that pulls one block through `Callback` and writes it to `Device`, until asked to stop.
        Thread           Worker;

        /// The render callback the service installs, through which the mixer fills `Mix` each block.
        Driver::Callback Callback;

        /// `true` while output is suspended, so the worker writes silence instead of pulling the mixer.
        Atomic<Bool>     Suspended { false };

        /// `true` once the worker abandoned the stream after an unrecoverable write error, `false` while it runs.
        Atomic<Bool>     Faulted   { false };

        /// The seconds left before a faulted stream is next tried again.
        Real64           Retry     = 0.0;

        /// The interleaved stereo scratch the mixer fills each block before it is written to `Device`.
        Real32           Mix[kBlock * kMixerStride];

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static Bool ALSAFill(Ref<Driver::Backend> Backend, UInt32 Frames)
        {
            const Span Output(Backend.Mix, Frames * static_cast<UInt32>(kMixerStride));

            if (Backend.Suspended.load(std::memory_order_relaxed))
            {
                Zero(Output.GetData(), Output.GetSize());
            }
            else
            {
                Backend.Callback(Output, Frames);
            }

            UInt32 Faults = 0;

            for (snd_pcm_uframes_t Cursor = 0; Cursor < Frames; )
            {
                const Ptr<Real32>       Address = Backend.Mix + Cursor * kMixerStride;
                const snd_pcm_sframes_t Written = ::snd_pcm_writei(Backend.Device, Address, Frames - Cursor);

                if (Written < 0)
                {
                    SInt32 Result = ::snd_pcm_recover(Backend.Device, Written, 1);

                    if (Result < 0)
                    {
                        Result = ::snd_pcm_prepare(Backend.Device);
                    }

                    if (Result < 0 || ++Faults > 8)
                    {
                        LOG_E("Audio: Unrecoverable ALSA write error ({0})", StrConvert(::snd_strerror(Result)));
                        return false;
                    }
                    continue;
                }
                Cursor += static_cast<snd_pcm_uframes_t>(Written);
            }
            return true;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static Str ALSASelect(Text Device)
        {
            Str Result;

            Ptr<Ptr<void>> Hints = nullptr;

            if (::snd_device_name_hint(-1, "pcm", AddressOf(Hints)) < 0)
            {
                return Result;
            }

            for (Ptr<Ptr<void>> Hint = Hints; * Hint && Result.IsEmpty(); ++Hint)
            {
                const Ptr<Char> Direction = ::snd_device_name_get_hint(* Hint, "IOID");

                // A null direction means the endpoint is bidirectional, and anything but playback is skipped.
                const Bool      Playable  = (Direction == nullptr) || StrConvert(Direction) == Text("Output");

                if (const Ptr<Char> Name = ::snd_device_name_get_hint(* Hint, "NAME"); Name)
                {
                    // The description carries the human-readable label, so it is matched alongside the name.
                    const Ptr<Char> Label = ::snd_device_name_get_hint(* Hint, "DESC");

                    if (Playable && (StrContains(StrConvert(Name), Device) || (Label && StrContains(StrConvert(Label), Device))))
                    {
                        Result = Str(StrConvert(Name));
                    }

                    ::free(Label);
                    ::free(Name);
                }

                ::free(Direction);
            }

            ::snd_device_name_free_hint(Hints);
            return Result;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static void ALSAThread(ConstRef<Thread::Token> Token, Ptr<Driver::Backend> Backend)
        {
            ZY_PROFILE_THREAD("Audio Thread");

            while (!Token.Stopped())
            {
                if (!ALSAFill(* Backend, kBlock))
                {
                    Backend->Faulted.store(true, std::memory_order_relaxed);

                    LOG_E("Audio: ALSA endpoint '{0}' stopped accepting frames", Backend->Name);
                    break;
                }
            }
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static Bool ALSAStart(Ref<Driver::Backend> Backend)
        {
            Backend.Name = (Backend.Request.IsEmpty() ? Str("default") : Backend.Request);

            // ALSA addresses endpoints by exact name, so the request is honoured verbatim before anything else.
            SInt32 Result = ::snd_pcm_open(AddressOf(Backend.Device), Backend.Name.GetData(), SND_PCM_STREAM_PLAYBACK, 0);

            // A name that does not open verbatim may still be a fragment of one the hint list knows.
            if (Result < 0 && !Backend.Request.IsEmpty())
            {
                if (Str Match = ALSASelect(Backend.Request); !Match.IsEmpty())
                {
                    Backend.Name = Move(Match);

                    Result = ::snd_pcm_open(AddressOf(Backend.Device), Backend.Name.GetData(), SND_PCM_STREAM_PLAYBACK, 0);
                }
            }

            // The default endpoint is the last resort, so a bad request degrades instead of leaving the app mute.
            if (Result < 0 && !Backend.Request.IsEmpty())
            {
                LOG_W("Audio: No ALSA endpoint matches '{0}', falling back to the default", Backend.Request);

                Backend.Name = "default";

                Result = ::snd_pcm_open(AddressOf(Backend.Device), Backend.Name.GetData(), SND_PCM_STREAM_PLAYBACK, 0);
            }

            if (Result < 0)
            {
                LOG_E("Audio: Failed to open ALSA endpoint '{0}' ({1})", Backend.Name, StrConvert(::snd_strerror(Result)));

                // ALSA leaves the handle untouched when the open fails, so drop it before the stop would read it.
                Backend.Device = nullptr;
                return false;
            }

            // The mixer always produces 48 kHz interleaved stereo float, buffered four periods deep.
            Result = ::snd_pcm_set_params(
                Backend.Device,
                SND_PCM_FORMAT_FLOAT_LE,
                SND_PCM_ACCESS_RW_INTERLEAVED,
                kMixerStride,
                kMixerFrequency,
                1,
                (static_cast<UInt64>(kBlock) * 4 * 1000000) / kMixerFrequency);

            if (Result < 0)
            {
                LOG_E("Audio: Failed to configure ALSA endpoint '{0}' ({1})", Backend.Name, StrConvert(::snd_strerror(Result)));
                return false;
            }

            Backend.Worker = Thread([Owner = AddressOf(Backend)](ConstRef<Thread::Token> Token)
            {
                ALSAThread(Token, Owner);
            });
            return true;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static void ALSAStop(Ref<Driver::Backend> Backend)
        {
            if (Backend.Worker.Joinable())
            {
                Backend.Worker.Stop();
                Backend.Worker.Join();
            }

            if (Backend.Device)
            {
                ::snd_pcm_drop(Backend.Device);
                ::snd_pcm_close(Backend.Device);

                Backend.Device = nullptr;
            }
        }
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
        if (!ALSA::Open())
        {
            return false;
        }

        mBackend->Callback = Move(Render);
        mBackend->Request  = Device;

        if (!Backend::ALSAStart(* mBackend))
        {
            Close();
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Close()
    {
        Backend::ALSAStop(* mBackend);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Probe(Ref<Description> Output) const
    {
        Output.Backend = "ALSA";
        Output.Adapter = mBackend->Name;

        Ptr<Ptr<void>> Hints = nullptr;

        if (!ALSA::Open() || ::snd_device_name_hint(-1, "pcm", AddressOf(Hints)) < 0)
        {
            return;
        }

        for (Ptr<Ptr<void>> Hint = Hints; * Hint; ++Hint)
        {
            const Ptr<Char> Direction = ::snd_device_name_get_hint(* Hint, "IOID");

            // A null direction means the endpoint is bidirectional, and anything but playback is skipped.
            const Bool      Playable  = (Direction == nullptr) || StrConvert(Direction) == Text("Output");

            if (const Ptr<Char> Name = ::snd_device_name_get_hint(* Hint, "NAME"); Name)
            {
                if (Playable)
                {
                    Output.Endpoints.Append(Str(StrConvert(Name)));
                }
                ::free(Name);
            }

            ::free(Direction);
        }

        ::snd_device_name_free_hint(Hints);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Advance(Real64 Delta)
    {
        if (!mBackend->Faulted.load(std::memory_order_relaxed))
        {
            return;
        }

        // An endpoint may stay gone for a while, so the stream is tried again once a second rather than every frame.
        mBackend->Retry -= Delta;

        if (mBackend->Retry > 0.0)
        {
            return;
        }
        mBackend->Retry = 1.0;

        Backend::ALSAStop(* mBackend);

        // The flag is lowered before the stream starts, so a fault the new stream raises at once is not lost.
        mBackend->Faulted.store(false, std::memory_order_relaxed);

        if (Backend::ALSAStart(* mBackend))
        {
            mBackend->Retry = 0.0;

            LOG_I("Audio: Moved to endpoint '{0}'", mBackend->Name);
        }
        else
        {
            mBackend->Faulted.store(true, std::memory_order_relaxed);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Suspend()
    {
        mBackend->Suspended.store(true, std::memory_order_relaxed);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Restore()
    {
        mBackend->Suspended.store(false, std::memory_order_relaxed);
    }
}