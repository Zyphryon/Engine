// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Kernel.hpp"

#ifdef    __EMSCRIPTEN__
    #include <emscripten.h>
#endif // __EMSCRIPTEN__

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Engine
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Kernel::Kernel()
        : mActive { false }
    {
        Log::Initialize("Zyphryon.log");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Kernel::~Kernel()
    {
        Log::Shutdown();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Initialize(Mode Mode, AnyRef<Properties> Properties)
    {
        ZYPHRYON_THREAD_NAME("Main Thread");

        // Store the initialization properties.
        mProperties = Move(Properties);

        // Initialize the system mode.
        SetMode(Mode);

        // Initializes client services.
        if (IsClientMode())
        {
            InitializeClientServices();
        }

        // Initializes server services.
        if (IsServerMode())
        {
            InitializeServerServices();
        }

        // Initializes common services.
        InitializeCommonServices();

        // Initializes the host and then enable the platform device.
        mActive = OnInitialize();

        if (mDevice.GetHandle() != nullptr)
        {
            mDevice.SetVisible(mActive);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Poll()
    {
        // Updates the current time by calculating the absolute time.
        mTime.SetAbsolute(Time::Elapsed());

        // Tick application.
        OnTick(mTime);

        // Tick services (order matters).
        Tick(mTime);

        ZYPHRYON_PROFILE_FRAME;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Run()
    {
        // Updates the simulation time by calculating the absolute time.
        mTime.SetAbsolute(Time::Elapsed());

#ifdef    __EMSCRIPTEN__

        const auto OnPoll = [](Ptr<void> Instance)
        {
            static_cast<Ptr<Kernel>>(Instance)->Poll();
        };

        emscripten_set_main_loop_arg(OnPoll, this, 0, true);

#else

        while (mActive)
        {
            Poll();
        }

#endif // __EMSCRIPTEN__

        // Performs application-level teardown.
        OnTeardown();

        // Performs subsystem-level teardown.
        Teardown();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Exit()
    {
        mActive = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::InitializeClientServices()
    {
        // Initializes input service.
        LOG_INFO("Kernel: Creating input service");
        ConstTracker<Input::Service> Input = AddService<Input::Service>();
        Input->OnWindowExit.AddMethod<& Kernel::OnWindowExit>(this);
        Input->OnWindowFocus.AddMethod<& Kernel::OnWindowFocus>(this);
        Input->OnWindowResize.AddMethod<& Kernel::OnWindowResize>(this);

        // Initializes device service.
        LOG_INFO("Kernel: Creating device ({}, {})", mProperties.GetWindowWidth(), mProperties.GetWindowHeight());
        InPlaceConstruct<Device>(mDevice,
            mProperties.GetWindowHandle(),
            mProperties.GetWindowTitle(),
            mProperties.GetWindowWidth(),
            mProperties.GetWindowHeight(),
            mProperties.IsWindowFullscreen(),
            mProperties.IsWindowBorderless());

        if (mDevice.GetHandle() == nullptr)
        {
            LOG_ERROR("Failed to create device: '{}'", SDL_GetError());
        }

        // Initializes graphic service.
        const Graphic::Backend Backend = Enum::Cast(mProperties.GetVideoDriver(), Graphic::Backend::None);
        const Graphic::Multisample Samples = Enum::Cast(Format("X{}", mProperties.GetWindowSamples()), Graphic::Multisample::X1);

        LOG_INFO("Kernel: Creating graphics service");
        ConstTracker<Graphic::Service> Graphics = AddService<Graphic::Service>();

        if (!Graphics->Initialize(Backend, mDevice.GetHandle(), mDevice.GetWidth(), mDevice.GetHeight(), Samples))
        {
            LOG_WARNING("Kernel: Failed to create graphics service.");
        }

        // Initializes audio service.
        LOG_INFO("Kernel: Creating audio service");
        ConstTracker<Audio::Service> AudioService = AddService<Audio::Service>();
        if (! AudioService->Initialize(Audio::Backend::FAudio, mProperties.GetAudioDevice()))
        {
            LOG_WARNING("Kernel: Failed to create audio service.");
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::InitializeServerServices()
    {
        // TODO: Implement server-specific services initialization.
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::InitializeCommonServices()
    {
        // Initializes resources service.
        LOG_INFO("Kernel: Creating content service");
        AddService<Content::Service>();

        // Initializes scene service.
        LOG_INFO("Kernel: Creating scene service");
        AddService<Scene::Service>();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::OnWindowExit()
    {
        Exit();
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::OnWindowResize(UInt32 Width, UInt32 Height)
    {
        if (ConstTracker<Graphic::Service> Graphics = GetService<Graphic::Service>())
        {
            Graphics->Reset(Width, Height, Graphics->GetDevice().Samples);
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::OnWindowFocus(Bool Focused)
    {
        if (mProperties.GetAudioPauseOnFocusLost())
        {
            if (ConstTracker<Audio::Service> Audio = GetService<Audio::Service>())
            {
                if (Focused)
                {
                    Audio->Restore();
                }
                else
                {
                    Audio->Suspend();
                }
            }
        }
        return false;
    }
}