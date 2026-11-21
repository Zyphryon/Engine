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

namespace ZyAudio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    static void WASAPIRelease(Ref<Ptr<Type>> Instance)
    {
        if (Instance)
        {
            Instance->Release();
            Instance = nullptr;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Driver::Backend final : public IMMNotificationClient
    {
        /// The endpoint the game asked for, or empty for the system default.
        Str                      Request;

        /// The endpoint actually opened, reported by `Probe`.
        Str                      Name       = "Default";

        /// The device enumerator, owned from a successful `Open` until `Close`.
        Ptr<IMMDeviceEnumerator> Enumerator = nullptr;

        /// The render endpoint the audio client is activated on.
        Ptr<IMMDevice>           Endpoint   = nullptr;

        /// The shared-mode audio client that runs the stream.
        Ptr<IAudioClient>        Client     = nullptr;

        /// The render service the buffer is written through.
        Ptr<IAudioRenderClient>  Renderer   = nullptr;

        /// The event WASAPI signals when the buffer has room for more frames.
        HANDLE                   Event      = nullptr;

        /// The thread that refills the buffer each time `Event` is signaled.
        Thread                   Worker;

        /// The size of the endpoint buffer, in frames.
        UInt32                   Frames     = 0;

        /// The render callback the service installs, through which the mixer fills the buffer.
        Driver::Callback         Callback;

        /// `true` once the stream stopped taking frames and has to be opened again, `false` while it runs.
        Atomic<Bool>             Faulted    = false;

        /// `true` once the system named another default endpoint, `false` once that has been acted on.
        Atomic<Bool>             Moved      = false;

        /// `true` while output is suspended, so a stream opened again stays stopped, `false` while it plays.
        Bool                     Suspended  = false;

        /// The seconds left before a faulted stream is next tried again.
        Real64                   Retry      = 0.0;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return 1;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ULONG STDMETHODCALLTYPE Release() override
        {
            return 1;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID Identifier, Ptr<Ptr<void>> Output) override
        {
            if (Identifier == __uuidof(IUnknown) || Identifier == __uuidof(IMMNotificationClient))
            {
                (* Output) = static_cast<Ptr<IMMNotificationClient>>(this);
                return S_OK;
            }

            (* Output) = nullptr;
            return E_NOINTERFACE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow Flow, ERole Role, LPCWSTR Device) override
        {
            if (Flow == eRender && Role == eConsole)
            {
                Moved.store(true, std::memory_order_relaxed);
            }
            return S_OK;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR Device, DWORD State) override
        {
            return S_OK;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR Device) override
        {
            return S_OK;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR Device) override
        {
            return S_OK;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR Device, const PROPERTYKEY Key) override
        {
            return S_OK;
        }

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
            else
            {
                Backend.Faulted.store(true, std::memory_order_relaxed);
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
                ::PropVariantClear(AddressOf(Variant));

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

                        if (StrContains(Name, Device))
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

        static void WASAPIThread(ConstRef<Thread::Token> Token, Ptr<Driver::Backend> Backend)
        {
            ZY_PROFILE_THREAD("Audio Thread");

            if (FAILED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
            {
                LOG_E("Audio: Failed to initialize COM");
                return;
            }

            DWORD        Index = 0;
            const HANDLE Task  = ::AvSetMmThreadCharacteristicsW(L"Pro Audio", AddressOf(Index));

            while (!Token.Stopped())
            {
                // A wait that runs out fills nothing, yet still asks the endpoint whether it is there at all.
                const Bool Signaled = (::WaitForSingleObject(Backend->Event, 2000) == WAIT_OBJECT_0);

                UInt32 Padding = 0;
                if (FAILED(Backend->Client->GetCurrentPadding(AddressOf(Padding))))
                {
                    // An endpoint unplugged or set up anew refuses every call from here on, until it is opened again.
                    Backend->Faulted.store(true, std::memory_order_relaxed);
                }
                else if (Signaled && Backend->Frames > Padding)
                {
                    WASAPIFill(* Backend, Backend->Frames - Padding);
                }
            }

            if (Task)
            {
                ::AvRevertMmThreadCharacteristics(Task);
            }

            ::CoUninitialize();
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static Bool WASAPIStart(Ref<Driver::Backend> Backend)
        {
            Backend.Endpoint = WASAPISelect(Backend.Enumerator, Backend.Request);

            if (Backend.Endpoint == nullptr)
            {
                LOG_E("Audio: Failed to obtain a WASAPI render endpoint");
                return false;
            }

            // The requested name may be empty or inexact, so report the endpoint that actually carries the audio.
            if (Str Name = WASAPIName(Backend.Endpoint); !Name.IsEmpty())
            {
                Backend.Name = Move(Name);
            }

            if (FAILED(Backend.Endpoint->Activate(
                __uuidof(IAudioClient),
                CLSCTX_ALL,
                nullptr,
                reinterpret_cast<Ptr<LPVOID>>(AddressOf(Backend.Client)))))
            {
                LOG_E("Audio: Failed to activate WASAPI audio client");
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

            constexpr DWORD Flags =
                  AUDCLNT_STREAMFLAGS_EVENTCALLBACK
                | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
                | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

            if (FAILED(Backend.Client->Initialize(AUDCLNT_SHAREMODE_SHARED, Flags, 0, 0, AddressOf(Format), nullptr)))
            {
                LOG_E("Audio: Failed to initialize WASAPI audio client");
                return false;
            }

            if (FAILED(Backend.Client->GetBufferSize(AddressOf(Backend.Frames))))
            {
                LOG_E("Audio: Failed to query the WASAPI buffer size");
                return false;
            }

            if (FAILED(Backend.Client->GetService(
                __uuidof(IAudioRenderClient),
                reinterpret_cast<Ptr<LPVOID>>(AddressOf(Backend.Renderer)))))
            {
                LOG_E("Audio: Failed to obtain the WASAPI render client");
                return false;
            }

            Backend.Event = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
            if (Backend.Event == nullptr || FAILED(Backend.Client->SetEventHandle(Backend.Event)))
            {
                LOG_E("Audio: Failed to create the WASAPI wake event");
                return false;
            }

            // Fill the whole buffer once so playback does not open on an underrun.
            WASAPIFill(Backend, Backend.Frames);

            Backend.Worker = Thread([Owner = AddressOf(Backend)](ConstRef<Thread::Token> Token)
            {
                WASAPIThread(Token, Owner);
            });

            // A stream opened again while output is suspended waits for the restore to start it.
            if (!Backend.Suspended && FAILED(Backend.Client->Start()))
            {
                LOG_E("Audio: Failed to start the WASAPI audio client");
                return false;
            }
            return true;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static void WASAPIStop(Ref<Driver::Backend> Backend)
        {
            if (Backend.Worker.Joinable())
            {
                Backend.Worker.Stop();
                ::SetEvent(Backend.Event);
                Backend.Worker.Join();
            }

            if (Backend.Client)
            {
                Backend.Client->Stop();
            }
            if (Backend.Event)
            {
                ::CloseHandle(Backend.Event);
                Backend.Event = nullptr;
            }

            WASAPIRelease(Backend.Renderer);
            WASAPIRelease(Backend.Client);
            WASAPIRelease(Backend.Endpoint);
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
        mBackend->Request  = Device;

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

        // The system tells of a new default endpoint from a thread of its own, which only raises a flag.
        mBackend->Enumerator->RegisterEndpointNotificationCallback(mBackend.Grab());

        if (!Backend::WASAPIStart(* mBackend))
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
        Backend::WASAPIStop(* mBackend);

        if (mBackend->Enumerator)
        {
            mBackend->Enumerator->UnregisterEndpointNotificationCallback(mBackend.Grab());
        }

        WASAPIRelease(mBackend->Enumerator);

        ::CoUninitialize();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Probe(Ref<Description> Output) const
    {
        Output.Backend = "WASAPI";
        Output.Adapter = mBackend->Name;

        // A failed open has already released the enumerator, so no endpoint can be listed.
        if (mBackend->Enumerator == nullptr)
        {
            return;
        }

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
        // A new default endpoint only matters to a stream that follows the default.
        if (mBackend->Moved.exchange(false, std::memory_order_relaxed) && mBackend->Request.IsEmpty())
        {
            mBackend->Faulted.store(true, std::memory_order_relaxed);
        }

        if (mBackend->Enumerator == nullptr || !mBackend->Faulted.load(std::memory_order_relaxed))
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

        Backend::WASAPIStop(* mBackend);

        // The flag is lowered before the stream starts, so a fault the new stream raises at once is not lost.
        mBackend->Faulted.store(false, std::memory_order_relaxed);

        if (Backend::WASAPIStart(* mBackend))
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
        mBackend->Suspended = true;

        if (mBackend->Client)
        {
            if (FAILED(mBackend->Client->Stop()))
            {
                LOG_W("Audio: Failed to stop the WASAPI audio client");
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Restore()
    {
        mBackend->Suspended = false;

        if (mBackend->Client)
        {
            // An endpoint lost while output was suspended only shows here, so it is left to be opened again.
            if (FAILED(mBackend->Client->Start()))
            {
                LOG_W("Audio: Failed to start the WASAPI audio client");

                mBackend->Faulted.store(true, std::memory_order_relaxed);
            }
        }
    }
}