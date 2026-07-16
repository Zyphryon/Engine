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
#include "Zyphryon.Audio/Service.hpp"
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Graphic/Service.hpp"
#include "Zyphryon.Input/Service.hpp"
#include "Zyphryon.Job/Service.hpp"
#include "Zyphryon.Platform/Service.hpp"
#include "Zyphryon.Scene/Service.hpp"

#if   defined(ZY_PLATFORM_WEB)
#include <emscripten.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Engine
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Kernel::Kernel()
        : mAlive { }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Run(AnyRef<Config> Config, AnyRef<Modules> Modules)
    {
        ZY_PROFILE_THREAD("Main Thread");

        // Store the provided configuration for use throughout the engine lifecycle.
        mConfig  = Move(Config);
        mModules = Move(Modules);

        // Initialize all services, modules, and the application before entering the main loop.
        Initialize();

        // Capture the current time in seconds using high-resolution nanosecond timer.
        mTimer.Reset();

#if !defined(ZY_PLATFORM_WEB)

        while (Tick())
        {
            // Intentionally empty - all logic runs inside Tick()
        }
        Terminate();

#else

        const auto OnLoop = [](Ptr<void> Instance)
        {
            const Ptr<Kernel> Executor = static_cast<Ptr<Kernel>>(Instance);

            if (const Bool Continue = Executor->Tick(); !Continue)
            {
                Executor->Terminate();
            }
        };
        emscripten_set_main_loop_arg(OnLoop, this, 0, true);

#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Quit()
    {
        mAlive = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Initialize()
    {
#if !defined(ZY_MODE_HEADLESS)

        LOG_I("Kernel: Creating platform service");
        ConstRetainer<Platform::Service> Platform = Register<Platform::Service>();
        if (!Platform->Initialize(
            mConfig.GetWindowMonitor(),
            mConfig.GetWindowTitle(),
            mConfig.GetWindowWidth(),
            mConfig.GetWindowHeight(),
            mConfig.IsWindowBorderless(),
            mConfig.IsWindowFullscreen()))
        {
            LOG_D("Kernel: Failed to initialize platform service");
            return;
        }

        LOG_I("Kernel: Creating input service");
        ConstRetainer<Input::Service> Input = Register<Input::Service>();
        Input->OnWindowExit.AddMethod<& Kernel::OnWindowExit>(this);
        Input->OnWindowFocus.AddMethod<& Kernel::OnWindowFocus>(this);
        Input->OnWindowResize.AddMethod<& Kernel::OnWindowResize>(this);

#endif

        LOG_I("Kernel: Creating job service");
        Register<Job::Service>();

        LOG_I("Kernel: Creating content service");
        Register<Content::Service>();

        LOG_I("Kernel: Creating scene service");
        Register<Scene::Service>();

#if !defined(ZY_MODE_HEADLESS)

        LOG_I("Kernel: Creating graphic service");
        ConstRetainer<Graphic::Service> Graphic = Register<Graphic::Service>();

        LOG_I("Kernel: Creating audio service");
        ConstRetainer<Audio::Service> Audio = Register<Audio::Service>();

#endif

        // Attaches external modules to the engine, allowing them to register their own services and systems.
        for (Ref<Unique<Module>> Module : mModules)
        {
            LOG_I("Kernel: Attaching module '{0}' v.{1}", Module->GetName(), Module->GetVersion());
            Module->OnAttach(* this);
        }

#if !defined(ZY_MODE_HEADLESS)

        Ref<Platform::Window> Window = Platform->GetWindow();

        LOG_I("Kernel: Initializing graphic service");
        Graphic::Config GraphicsConfig;
        GraphicsConfig.Width       = Window.GetWidth();
        GraphicsConfig.Height      = Window.GetHeight();
        GraphicsConfig.ColorFormat = mConfig.GetGraphicsColorFormat();
        GraphicsConfig.DepthFormat = mConfig.GetGraphicsDepthFormat();

        if (GraphicsConfig.ColorFormat == Graphic::TextureFormat::Unspecified)
        {
            const ConstPtr<Platform::Monitor> Monitor = Platform->GetDisplay().GetMonitor(Window.GetX(), Window.GetY());

            GraphicsConfig.ColorFormat = (Monitor && Monitor->IsHDR())
                ? Graphic::TextureFormat::RGBA16Float
                : Graphic::TextureFormat::RGBA8UIntNorm_sRGB;
        }

        Graphic->Initialize(mConfig.GetGraphicsDriver(), Window.GetHandle(), GraphicsConfig);

        LOG_I("Kernel: Initializing audio service");
        Audio->Initialize(mConfig.GetAudioDriver(), mConfig.GetAudioAdapter());

#endif

        // Initialize the application after all services are ready.
        mAlive = OnInitialize();

#if !defined(ZY_MODE_HEADLESS)

        // Make sure the display is visible after everything is set up.
        Window.SetVisible(mAlive);

#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::Tick()
    {
        const Real64 Delta = mTimer.Step();

        // Dispatch the tick to the application-defined update logic.
        OnTick(Delta);

        // Update all registered services with the current frame delta.
        Subsystem::Host::Tick(Delta);

        ZY_PROFILE_FRAME;
        return mAlive;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Terminate()
    {
        // Invoke the application-defined shutdown logic before services are torn down.
        OnTerminate();

        // Detaches external modules to the engine, allowing them to clean up any resources they allocated.
        for (Ref<Unique<Module>> Module : mModules)
        {
            Module->OnDetach(* this);
        }

        // Destroy all registered services and release remaining engine resources.
        Teardown();

#if defined(ZY_PLATFORM_WEB)

        // Unregister the main loop callback to stop the engine from ticking after shutdown.
        emscripten_cancel_main_loop();

#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::OnWindowExit()
    {
        Quit();
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::OnWindowResize(UInt32 Width, UInt32 Height)
    {
        if (ConstRetainer<Graphic::Service> Graphic = GetService<Graphic::Service>())
        {
            Graphic->Reset(Width, Height);
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::OnWindowFocus(Bool Focused)
    {
        if (mConfig.IsAudioPauseOnFocusLost())
        {
            if (ConstRetainer<Audio::Service> Audio = GetService<Audio::Service>())
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