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

#include "Kernel.hpp"
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Graphic/Service.hpp"
#include "Zyphryon.Audio/Service.hpp"
#include "Zyphryon.Input/Service.hpp"
#include "Zyphryon.Scene/Service.hpp"

#include "Zyphryon.Content/Mount/Embedded.hpp"
#include "Zyphryon.Content/Loader/Sound/MP3/Loader.hpp"     // TODO: Plugin Based
#include "Zyphryon.Content/Loader/Sound/WAV/Loader.hpp"     // TODO: Plugin Based
#include "Zyphryon.Content/Loader/Texture/STB/Loader.hpp"   // TODO: Plugin Based
#include "Zyphryon.Content/Loader/Material/Loader.hpp"
#include "Zyphryon.Content/Loader/Model/GLTF/Loader.hpp"    // TODO: Plugin Based
#include "Zyphryon.Content/Loader/Pipeline/Loader.hpp"
#include "Zyphryon.Content/Loader/Font/Artery/Loader.hpp"   // TODO: Plugin Based

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
        ZYPHRYON_PROFILE_THREAD("Main Thread");

        // Store the initialization properties.
        mProperties = Move(Properties);

        // Initialize the system mode.
        SetMode(Mode);

        // Initializes common services.
        InitializeCommonServices();

        // Initializes client services.
        if (IsClientMode())
        {
            InitializeClientServices();
            InitializeClientLoaders();
        }

        // Initializes server services.
        if (IsServerMode())
        {
            InitializeServerServices();
        }

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
        mTime.SetAbsolute(Time::GetUptimeInSeconds());

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
        mTime.SetAbsolute(Time::GetUptimeInSeconds());

        // Main application loop.
        while (mActive)
        {
            Poll();
        }

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

    void Kernel::Sync(Ref<Properties> Properties)
    {
        Properties = mProperties;
        Properties.SetWindowWidth(mDevice.GetWidth());
        Properties.SetWindowHeight(mDevice.GetHeight());

        if (ConstTracker<Graphic::Service> Graphics = GetService<Graphic::Service>())
        {
            Properties.SetWindowSamples(Enum::Cast(Graphics->GetDevice().Samples));
        }

        Properties.SetWindowFullscreen(mDevice.IsFullscreen());
        Properties.SetWindowBorderless(mDevice.IsBorderless());
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
        InPlaceConstruct<Device>(& mDevice,
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
        LOG_INFO("Kernel: Creating graphics service");
        ConstTracker<Graphic::Service> Graphics = AddService<Graphic::Service>();

        const Graphic::Backend     Backend = Enum::Cast(mProperties.GetVideoDriver(), Graphic::Backend::None);
        const Graphic::Multisample Samples = static_cast<Graphic::Multisample>(mProperties.GetWindowSamples());

        if (!Graphics->Initialize(Backend, mDevice.GetHandle(), mDevice.GetWidth(), mDevice.GetHeight(), Samples))
        {
            LOG_WARNING("Kernel: Failed to create graphics service.");
        }

        // Initializes audio service.
        LOG_INFO("Kernel: Creating audio service");
        ConstTracker<Audio::Service> AudioService = AddService<Audio::Service>();
        if (! AudioService->Initialize(Audio::Backend::Miniaudio, mProperties.GetAudioDevice()))
        {
            LOG_WARNING("Kernel: Failed to create audio service.");

            // Fallback to no audio.
            AudioService->Initialize(Audio::Backend::None, mProperties.GetAudioDevice());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::InitializeClientLoaders()
    {
        ConstTracker<Content::Service> Content = GetService<Content::Service>();
        LOG_INFO("Kernel: Registering content loaders");

        Content->AddLoader(Tracker<Content::MP3Loader>::Create());
        Content->AddLoader(Tracker<Content::WAVLoader>::Create());
        Content->AddLoader(Tracker<Content::STBLoader>::Create());
        Content->AddLoader(Tracker<Content::MaterialLoader>::Create());
        Content->AddLoader(Tracker<Content::GLTFLoader>::Create());
        Content->AddLoader(Tracker<Content::ArteryFontLoader>::Create());

        Graphic::Device Device = GetService<Graphic::Service>()->GetDevice();
        Content->AddLoader(Tracker<Content::PipelineLoader>::Create(Device.Backend, Device.Version));
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
        ConstTracker<Content::Service> Content = AddService<Content::Service>();
        Content->AddMount("Engine", Tracker<Content::Embedded>::Create());

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