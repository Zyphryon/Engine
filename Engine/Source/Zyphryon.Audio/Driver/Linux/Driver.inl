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

#include <alsa/asoundlib.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Driver::Backend
    {
        /// The endpoint \c Device was opened from; ALSA matches it literally, so the default is lower case.
        Str              Name      = "default";

        /// The playback stream, owned from a successful \c Open until \c Close drains and releases it.
        Ptr<snd_pcm_t>   Device    = nullptr;

        /// The thread that pulls one block through \c Callback and writes it to \c Device, until asked to stop.
        Thread           Worker;

        /// The render callback the service installs; the mixer fills \c Mix through it each block.
        Driver::Callback Callback;

        /// Interleaved stereo scratch the mixer fills each block before it is written to \c Device.
        Real32           Mix[kMixerPeriod * kMixerStride];

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static Bool ALSAFill(Ref<Driver::Backend> Backend, UInt32 Frames)
        {
            const Span Output(Backend.Mix, kMixerPeriod * kMixerStride);
            Backend.Callback(Output, Frames);

            for (snd_pcm_uframes_t Cursor = 0; Cursor < Frames; )
            {
                const Ptr<Real32>       Address = Backend.Mix + Cursor * kMixerStride;
                const snd_pcm_sframes_t Written = ::snd_pcm_writei(Backend.Device, Address, Frames - Cursor);

                if (Written < 0)
                {
                    if (const SInt32 Result = ::snd_pcm_recover(Backend.Device, Written, 1); Result < 0)
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

                // A null direction means the endpoint is bidirectional; anything but playback is skipped.
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
        mBackend->Name     = (Device.IsEmpty() ? Str("default") : Str(Device));

        // ALSA addresses endpoints by exact name, so the request is honoured verbatim before anything else.
        SInt32 Result = ::snd_pcm_open(AddressOf(mBackend->Device), mBackend->Name.GetData(), SND_PCM_STREAM_PLAYBACK, 0);

        // A name that does not open verbatim may still be a fragment of one the hint list knows.
        if (Result < 0 && !Device.IsEmpty())
        {
            if (Str Match = Backend::ALSASelect(Device); !Match.IsEmpty())
            {
                mBackend->Name = Move(Match);

                Result = ::snd_pcm_open(AddressOf(mBackend->Device), mBackend->Name.GetData(), SND_PCM_STREAM_PLAYBACK, 0);
            }
        }

        // The default endpoint is the last resort, so a bad request degrades instead of leaving the app mute.
        if (Result < 0 && !Device.IsEmpty())
        {
            LOG_W("Audio: No ALSA endpoint matches '{0}', falling back to the default", Device);

            mBackend->Name = "default";

            Result = ::snd_pcm_open(AddressOf(mBackend->Device), mBackend->Name.GetData(), SND_PCM_STREAM_PLAYBACK, 0);
        }

        if (Result < 0)
        {
            LOG_E("Audio: Failed to open ALSA endpoint '{0}' ({1})", mBackend->Name, StrConvert(::snd_strerror(Result)));

            // ALSA leaves the handle untouched when the open fails, so drop it before Close would read it.
            mBackend->Device = nullptr;

            Close();
            return false;
        }

        // The mixer always produces 48 kHz interleaved stereo float, buffered four periods deep.
        Result = ::snd_pcm_set_params(
            mBackend->Device,
            SND_PCM_FORMAT_FLOAT_LE,
            SND_PCM_ACCESS_RW_INTERLEAVED,
            kMixerStride,
            kMixerFrequency,
            1,
            (static_cast<UInt64>(kMixerPeriod) * 4 * 1000000) / kMixerFrequency);

        if (Result < 0)
        {
            LOG_E("Audio: Failed to configure ALSA endpoint '{0}' ({1})", mBackend->Name, StrConvert(::snd_strerror(Result)));

            Close();
            return false;
        }

        mBackend->Worker = Thread([this](std::stop_token Token)
        {
            ZY_PROFILE_THREAD("Audio Thread");

            while (!Token.stop_requested() && Backend::ALSAFill(* mBackend, kMixerPeriod))
            {
            }
        });

        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Close()
    {
        if (mBackend->Worker.joinable())
        {
            mBackend->Worker.request_stop();
            mBackend->Worker.join();
        }

        if (mBackend->Device)
        {
            ::snd_pcm_drain(mBackend->Device);
            ::snd_pcm_close(mBackend->Device);

            mBackend->Device = nullptr;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Probe(Ref<Description> Output) const
    {
        Output.Backend = "ALSA";
        Output.Adapter = mBackend->Name;

        Ptr<Ptr<void>> Hints = nullptr;

        if (::snd_device_name_hint(-1, "pcm", AddressOf(Hints)) < 0)
        {
            return;
        }

        for (Ptr<Ptr<void>> Hint = Hints; * Hint; ++Hint)
        {
            const Ptr<Char> Direction = ::snd_device_name_get_hint(* Hint, "IOID");

            // A null direction means the endpoint is bidirectional; anything but playback is skipped.
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
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Suspend()
    {
        snd_pcm_pause(mBackend->Device, 1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Restore()
    {
        snd_pcm_pause(mBackend->Device, 0);
    }
}
