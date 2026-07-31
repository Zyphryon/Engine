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

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <functiondiscoverykeys.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Driver::Backend
    {
        Str                      Name       = "Default";
        Ptr<IMMDeviceEnumerator> Enumerator = nullptr;
        Ptr<IMMDevice>           Endpoint   = nullptr;
        Ptr<IAudioClient>        Client     = nullptr;
        Ptr<IAudioRenderClient>  Renderer   = nullptr;
        HANDLE                   Event      = nullptr;
        HANDLE                   Thread     = nullptr;
        Atomic<Bool>             Running    = false;
        UInt32                   Frames     = 0;
        Driver::Callback         Callback;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static void WASAPIFill(Ref<Driver::Backend> Backend, UInt32 Frames)
        {
            Ptr<BYTE> Buffer = nullptr;

            if (SUCCEEDED(Backend.Renderer->GetBuffer(Frames, AddressOf(Buffer))))
            {
                const Ptr<Real32> Output = reinterpret_cast<Ptr<Real32>>(Buffer);
                Backend.Callback(Span(Output, Frames * static_cast<UInt32>(kMixerStride)), Frames);

                Backend.Renderer->ReleaseBuffer(Frames, 0);
            }
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static Str WASAPIName(Ptr<IMMDevice> Device)
        {
            Str Name;

            Ptr<IPropertyStore> Store = nullptr;

            if (SUCCEEDED(Device->OpenPropertyStore(STGM_READ, AddressOf(Store))))
            {
                PROPVARIANT Variant = { };

                if (SUCCEEDED(Store->GetValue(PKEY_Device_FriendlyName, AddressOf(Variant))) && Variant.vt == VT_LPWSTR)
                {
                    Name = Str::ConvertFromUTF16(StrConvert(Variant.pwszVal));
                }
                PropVariantClear(AddressOf(Variant));

                Store->Release();
            }
            return Name;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static Ptr<IMMDevice> WASAPISelect(Ptr<IMMDeviceEnumerator> Enumerator, Text Device)
        {
            Ptr<IMMDevice> Result = nullptr;

            if (!Device.IsEmpty())
            {
                Ptr<IMMDeviceCollection> Collection = nullptr;

                if (SUCCEEDED(Enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, AddressOf(Collection))))
                {
                    UINT Count = 0;
                    Collection->GetCount(AddressOf(Count));

                    for (UINT Element = 0; Element < Count && Result == nullptr; ++Element)
                    {
                        Ptr<IMMDevice> Candidate = nullptr;

                        if (FAILED(Collection->Item(Element, AddressOf(Candidate))))
                        {
                            continue;
                        }

                        const Str Name = WASAPIName(Candidate);

                        if (StrContains(Text(Name.GetData(), Name.GetSize()), Device))
                        {
                            Result = Candidate;
                        }
                        else
                        {
                            Candidate->Release();
                        }
                    }
                    Collection->Release();
                }

                if (Result == nullptr)
                {
                    LOG_W("Audio: No endpoint matches '{0}', falling back to the default", Device);
                }
            }

            // The default endpoint serves both an empty request and a named one that matched nothing.
            if (Result == nullptr && FAILED(Enumerator->GetDefaultAudioEndpoint(eRender, eConsole, AddressOf(Result))))
            {
                return nullptr;
            }
            return Result;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static DWORD WINAPI WASAPIThread(LPVOID Parameter)
        {
            Ref<Driver::Backend> Backend = * static_cast<Ptr<Driver::Backend>>(Parameter);

            if (FAILED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
            {
                LOG_E("Audio: Failed to initialize COM");
                return -1;
            }

            DWORD        Index = 0;
            const HANDLE Task  = ::AvSetMmThreadCharacteristicsW(L"Pro Audio", AddressOf(Index));

            while (Backend.Running.load(std::memory_order_acquire))
            {
                if (::WaitForSingleObject(Backend.Event, 2000) != WAIT_OBJECT_0)
                {
                    continue;
                }

                UInt32 Padding = 0;
                if (SUCCEEDED(Backend.Client->GetCurrentPadding(AddressOf(Padding))))
                {
                    const UInt32 Available = Backend.Frames - Padding;
                    if (Available > 0)
                    {
                        WASAPIFill(Backend, Available);
                    }
                }
            }

            if (Task)
            {
                ::AvRevertMmThreadCharacteristics(Task);
            }

            ::CoUninitialize();
            return 0;
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

        if (FAILED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
        {
            LOG_E("Audio: Failed to initialize COM");

            Close();
            return false;
        }

        if (FAILED(::CoCreateInstance(
            __uuidof(MMDeviceEnumerator),
            nullptr,
            CLSCTX_ALL,
            __uuidof(IMMDeviceEnumerator),
            reinterpret_cast<Ptr<LPVOID>>(AddressOf(mBackend->Enumerator)))))
        {
            LOG_E("Audio: Failed to create WASAPI device enumerator");

            Close();
            return false;
        }

        mBackend->Endpoint = Backend::WASAPISelect(mBackend->Enumerator, Device);

        if (mBackend->Endpoint == nullptr)
        {
            LOG_E("Audio: Failed to obtain a WASAPI render endpoint");

            Close();
            return false;
        }

        // The requested name may be empty or inexact, so report the endpoint that actually carries the audio.
        if (Str Name = Backend::WASAPIName(mBackend->Endpoint); !Name.IsEmpty())
        {
            mBackend->Name = Move(Name);
        }

        if (FAILED(mBackend->Endpoint->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            reinterpret_cast<Ptr<LPVOID>>(AddressOf(mBackend->Client)))))
        {
            LOG_E("Audio: Failed to activate WASAPI audio client");

            Close();
            return false;
        }

        // The mixer always produces 48 kHz interleaved stereo float.
        WAVEFORMATEX Format { };
        Format.wFormatTag      = WAVE_FORMAT_IEEE_FLOAT;
        Format.nChannels       = static_cast<WORD>(kMixerStride);
        Format.nSamplesPerSec  = kMixerFrequency;
        Format.wBitsPerSample  = 32;
        Format.nBlockAlign     = static_cast<WORD>(Format.nChannels * Format.wBitsPerSample / 8);
        Format.nAvgBytesPerSec = Format.nSamplesPerSec * Format.nBlockAlign;

        constexpr DWORD Flags  =
              AUDCLNT_STREAMFLAGS_EVENTCALLBACK
            | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
            | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

        if (FAILED(mBackend->Client->Initialize(AUDCLNT_SHAREMODE_SHARED, Flags, 0, 0, AddressOf(Format), nullptr)))
        {
            LOG_E("Audio: Failed to initialize WASAPI audio client");

            Close();
            return false;
        }

        if (FAILED(mBackend->Client->GetBufferSize(AddressOf(mBackend->Frames))))
        {
            Close();
            return false;
        }

        if (FAILED(mBackend->Client->GetService(
            __uuidof(IAudioRenderClient),
            reinterpret_cast<Ptr<LPVOID>>(AddressOf(mBackend->Renderer)))))
        {
            Close();
            return false;
        }

        mBackend->Event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (mBackend->Event == nullptr || FAILED(mBackend->Client->SetEventHandle(mBackend->Event)))
        {
            Close();
            return false;
        }

        // Prime the buffer with one block of silence so playback starts without an underrun.
        Backend::WASAPIFill(* mBackend, mBackend->Frames);

        mBackend->Running.store(true, std::memory_order_release);
        mBackend->Thread = CreateThread(nullptr, 0, Backend::WASAPIThread, mBackend.Grab(), 0, nullptr);

        if (mBackend->Thread == nullptr || FAILED(mBackend->Client->Start()))
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
        if (mBackend->Running.exchange(false, std::memory_order_acq_rel) && mBackend->Thread)
        {
            SetEvent(mBackend->Event);
            WaitForSingleObject(mBackend->Thread, INFINITE);
        }

        if (mBackend->Thread)
        {
            CloseHandle(mBackend->Thread);
            mBackend->Thread = nullptr;
        }
        if (mBackend->Client)
        {
            mBackend->Client->Stop();
        }
        if (mBackend->Event)
        {
            CloseHandle(mBackend->Event);
            mBackend->Event = nullptr;
        }
        if (mBackend->Renderer)
        {
            mBackend->Renderer->Release();
            mBackend->Renderer = nullptr;
        }
        if (mBackend->Client)
        {
            mBackend->Client->Release();
            mBackend->Client = nullptr;
        }
        if (mBackend->Endpoint)
        {
            mBackend->Endpoint->Release();
            mBackend->Endpoint = nullptr;
        }
        if (mBackend->Enumerator)
        {
            mBackend->Enumerator->Release();
            mBackend->Enumerator = nullptr;
        }

        ::CoUninitialize();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Probe(Ref<Description> Output) const
    {
        Output.Backend = "WASAPI";
        Output.Adapter = mBackend->Name;

        Ptr<IMMDeviceCollection> Collection = nullptr;
        if (FAILED(mBackend->Enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, AddressOf(Collection))))
        {
            return;
        }

        UINT Count = 0;
        Collection->GetCount(AddressOf(Count));

        for (UINT Element = 0; Element < Count; ++Element)
        {
            Ptr<IMMDevice> Device = nullptr;
            if (FAILED(Collection->Item(Element, AddressOf(Device))))
            {
                continue;
            }

            if (Str Name = Backend::WASAPIName(Device); !Name.IsEmpty())
            {
                Output.Endpoints.Append(Move(Name));
            }
            Device->Release();
        }
        Collection->Release();
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
        if (mBackend->Client)
        {
            if (FAILED(mBackend->Client->Stop()))
            {
                LOG_W("WASAPI: Failed to stop WASAPI audio client");
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Restore()
    {
        if (mBackend->Client)
        {
            if (FAILED(mBackend->Client->Start()))
            {
                LOG_W("WASAPI: Failed to start WASAPI audio client");
            }
        }
    }
}