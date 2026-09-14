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
#include "Zyphryon.Job/Service.hpp"
#include "Zyphryon.Network/Service.hpp"
#include "Zyphryon.Scene/Service.hpp"

#if !defined(ZY_MODE_HEADLESS)
#include "Zyphryon.Audio/Service.hpp"
#include "Zyphryon.Graphic/Service.hpp"
#include "Zyphryon.Input/Service.hpp"
#include "Zyphryon.Platform/Service.hpp"
#include "Zyphryon.Render/Service.hpp"
#endif

#if   defined(ZY_PLATFORM_WEB)
#include <emscripten.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRuntime
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Kernel::Kernel()
        : mAlive { }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Save(JsonObject Root)
    {
#if !defined(ZY_MODE_HEADLESS)
        if (ConstRetainer<ZyPlatform::Service> Platform = GetService<ZyPlatform::Service>())
        {
            ConstRef<ZyPlatform::Window> Window = Platform->GetWindow();

            mStartup.SetWindowWidth(Window.GetWidth());
            mStartup.SetWindowHeight(Window.GetHeight());
            mStartup.SetWindowFullscreen(Window.IsFullscreen());

            if (const ConstPtr<ZyPlatform::Monitor> Monitor = Platform->GetDisplay().GetMonitor(Window.GetX(), Window.GetY()))
            {
                mStartup.SetWindowMonitor(Monitor->GetName());
            }
        }

        if (ConstRetainer<ZyGraphic::Service> Graphics = GetService<ZyGraphic::Service>())
        {
            mStartup.SetGraphicsTearless(Graphics->IsTearless());
        }
#endif

        mStartup.Save(Root);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Run(UInt Count, ConstPtr<ConstPtr<Char>> Arguments, AnyRef<ZyEngine::Modules> Modules)
    {
        ZY_PROFILE_THREAD("Main Thread");

        mModules = Move(Modules);

        // Parse the command line first, so the application can consult it while it configures itself.
        mEnvironment.Parse(Count, Arguments);

        // Let the application overwrite the defaults before any service reads them.
        OnConfigure(mStartup);

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
        ConstRetainer<ZyPlatform::Service> Platform = Register<ZyPlatform::Service>();
        if (!Platform->Initialize(
            mStartup.GetWindowMonitor(),
            mStartup.GetWindowTitle(),
            mStartup.GetWindowWidth(),
            mStartup.GetWindowHeight(),
            mStartup.IsWindowBorderless(),
            mStartup.IsWindowFullscreen()))
        {
            LOG_D("Kernel: Failed to initialize platform service");
            return;
        }

        LOG_I("Kernel: Creating input service");
        ConstRetainer<ZyInput::Service> Input = Register<ZyInput::Service>();
        Input->OnWindowExit.AddMethod<& Kernel::OnWindowExit>(this);
        Input->OnWindowFocus.AddMethod<& Kernel::OnWindowFocus>(this);
        Input->OnWindowResize.AddMethod<& Kernel::OnWindowResize>(this);

#endif

        LOG_I("Kernel: Creating job service");
        Register<ZyJob::Service>();

        LOG_I("Kernel: Creating network service");
        if (ConstRetainer<ZyNetwork::Service> Network = Register<ZyNetwork::Service>(); !Network->Initialize())
        {
            Unregister<ZyNetwork::Service>();

            LOG_W("Kernel: Failed to initialize network service");
        }

        LOG_I("Kernel: Creating content service");
        Register<ZyContent::Service>();

        LOG_I("Kernel: Creating scene service");
        Register<ZyScene::Service>();

#if !defined(ZY_MODE_HEADLESS)

        LOG_I("Kernel: Creating graphic service");
        ConstRetainer<ZyGraphic::Service> Graphic = Register<ZyGraphic::Service>();

        LOG_I("Kernel: Creating audio service");
        ConstRetainer<ZyAudio::Service> Audio = Register<ZyAudio::Service>();

        LOG_I("Kernel: Creating render service");
        Register<ZyRender::Service>();

#endif

        // Attaches external modules to the engine, allowing them to register their own services and systems.
        for (Ref<Unique<ZyEngine::Module>> Module : mModules)
        {
            LOG_I("Kernel: Attaching module '{0}' v.{1}", Module->GetName(), Module->GetVersion());
            Module->OnAttach(* this);
        }

#if !defined(ZY_MODE_HEADLESS)

        Ref<ZyPlatform::Window> Window = Platform->GetWindow();

        LOG_I("Kernel: Initializing graphic service");
        ZyGraphic::Configuration GraphicsConfig;
        GraphicsConfig.Tearless    = mStartup.IsGraphicsTearless();
        GraphicsConfig.Width       = Window.GetWidth();
        GraphicsConfig.Height      = Window.GetHeight();
        GraphicsConfig.ColorFormat = mStartup.GetGraphicsColorFormat();
        GraphicsConfig.DepthFormat = mStartup.GetGraphicsDepthFormat();

        if (GraphicsConfig.ColorFormat == ZyGraphic::TextureFormat::Unspecified)
        {
            const ConstPtr<ZyPlatform::Monitor> Monitor = Platform->GetDisplay().GetMonitor(Window.GetX(), Window.GetY());

            GraphicsConfig.ColorFormat = (Monitor && Monitor->IsHDR())
                ? ZyGraphic::TextureFormat::RGBA16Float
                : ZyGraphic::TextureFormat::RGBA8UIntNorm_sRGB;
        }

        Graphic->Initialize(mStartup.GetGraphicsDriver(), Window.GetHandle(), GraphicsConfig);

        LOG_I("Kernel: Initializing audio service");
        Audio->Initialize(mStartup.GetAudioAdapter());

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
        ZyEngine::Subsystem::Host::Tick(Delta);

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
        for (Ref<Unique<ZyEngine::Module>> Module : mModules)
        {
            Module->OnDetach(* this);
        }

        // Destroy all registered services and release remaining engine resources.
        Teardown();

        // The application no longer owns `main`, so this is the last place its log tail can be written out.
        ZyLog::Flush();

#if defined(ZY_PLATFORM_WEB)

        // Unregister the main loop callback to stop the engine from ticking after shutdown.
        emscripten_cancel_main_loop();

#endif
    }

#if !defined(ZY_MODE_HEADLESS)

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
        if (ConstRetainer<ZyGraphic::Service> Graphic = GetService<ZyGraphic::Service>())
        {
            Graphic->Reset(Width, Height, Graphic->IsTearless());
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::OnWindowFocus(Bool Focused)
    {
        if (mStartup.IsAudioPauseOnFocusLost())
        {
            if (ConstRetainer<ZyAudio::Service> Audio = GetService<ZyAudio::Service>())
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

#endif
}