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
            ConstRef<ZyPlatform::Window>  Window  = Platform->GetWindow();
            ConstRef<ZyPlatform::Display> Display = Platform->GetDisplay();

            Real32 Scale = 1.0f;

            if (const ConstPtr<ZyPlatform::Monitor> Monitor = Display.GetMonitor(Window.GetX(), Window.GetY()))
            {
                mStartup.SetWindowMonitor(Monitor->GetName());
                Scale = Monitor->GetScale();
            }
            else if (!Display.GetMonitors().IsEmpty())
            {
                Scale = Display.GetMonitor(mStartup.GetWindowMonitor()).GetScale();
            }

            mStartup.SetWindowWidth(static_cast<UInt32>(Round(Window.GetWidth() / Scale)));
            mStartup.SetWindowHeight(static_cast<UInt32>(Round(Window.GetHeight() / Scale)));
            mStartup.SetWindowFullscreen(Window.IsFullscreen());
        }

        if (ConstRetainer<ZyGraphic::Service> Graphic = GetService<ZyGraphic::Service>())
        {
            mStartup.SetGraphicsTearless(Graphic->IsTearless());
        }

#endif

        mStartup.Save(Root);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Run(UInt Count, ConstPtr<ConstPtr<Char>> Arguments, AnyRef<ZyEngine::Modules> Modules)
    {
        ZY_PROFILE_THREAD("Main Thread");

        // Parse the command line first, so the application can consult it while it configures itself.
        mEnvironment.Parse(Count, Arguments);

        // Let the application overwrite the defaults before any service reads them.
        OnConfigure(mStartup);

        // A service the application needs failed, so the application never started and is not told to stop.
        if (!Initialize(Move(Modules)))
        {
            Shutdown();
            return;
        }

        // Start the frame clock now, so the first delta leaves out the time spent starting up.
        mTimer.Reset();

#if !defined(ZY_PLATFORM_WEB)

        // An application that failed to initialize is terminated without ever being ticked.
        while (mAlive && Tick())
        {
            // Every frame runs inside Tick.
        }

#else

        // The browser owns the loop from here, and never returns to this frame.
        if (mAlive)
        {
            emscripten_set_main_loop_arg(& Kernel::OnFrame, this, 0, true);
        }

#endif

        Terminate();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Quit()
    {
        mAlive = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::Initialize(AnyRef<ZyEngine::Modules> Modules)
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
            LOG_E("Kernel: Failed to initialize platform service");
            return false;
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
        for (Ref<Unique<ZyEngine::Module>> Module : Modules)
        {
            LOG_I("Kernel: Attaching module '{0}' v.{1}", Module->GetName(), Module->GetVersion());
            Module->OnAttach(* this);
        }

        // Only attached modules are kept, so a startup that fails before this point has none to detach.
        mModules = Move(Modules);

#if !defined(ZY_MODE_HEADLESS)

        Ref<ZyPlatform::Window> Window = Platform->GetWindow();

        LOG_I("Kernel: Initializing graphic service");
        ZyGraphic::Configuration Config;
        Config.Tearless    = mStartup.IsGraphicsTearless();
        Config.Width       = Window.GetWidth();
        Config.Height      = Window.GetHeight();
        Config.ColorFormat = mStartup.GetGraphicsColorFormat();
        Config.DepthFormat = mStartup.GetGraphicsDepthFormat();

        if (Config.ColorFormat == ZyGraphic::TextureFormat::Unspecified)
        {
            const ConstPtr<ZyPlatform::Monitor> Monitor = Platform->GetDisplay().GetMonitor(Window.GetX(), Window.GetY());

            Config.ColorFormat = (Monitor && Monitor->IsHDR())
                ? ZyGraphic::TextureFormat::RGBA16Float
                : ZyGraphic::TextureFormat::RGBA8UIntNorm_sRGB;
        }

        if (!Graphic->Initialize(mStartup.GetGraphicsDriver(), Window.GetHandle(), Config))
        {
            LOG_E("Kernel: Failed to initialize graphic service");
            return false;
        }

        LOG_I("Kernel: Initializing audio service");

        // Plenty of machines have no output device, and a game without sound still plays.
        if (!Audio->Initialize(mStartup.GetAudioAdapter()))
        {
            LOG_W("Kernel: Failed to initialize audio service, continuing without sound");
        }

#endif

        mAlive = OnInitialize();

#if !defined(ZY_MODE_HEADLESS)

        // Show the window only once the application has started.
        Window.SetVisible(mAlive);

#endif

        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Kernel::Tick()
    {
        const Real64 Delta = mTimer.Step();
        mMeter.Record(Delta);

        OnTick(Delta);

        ZyEngine::Subsystem::Host::Tick(Delta);

        ZY_PROFILE_FRAME;
        return mAlive;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Terminate()
    {
        OnTerminate();

        Shutdown();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::Shutdown()
    {
        // Modules detach while the services they registered with are still alive.
        for (Ref<Unique<ZyEngine::Module>> Module : mModules)
        {
            Module->OnDetach(* this);
        }

        Teardown();

        // The application no longer owns `main`, so this is the last place its log tail can be written out.
        ZyLog::Flush();

#if defined(ZY_PLATFORM_WEB)

        // Unregister the main loop callback to stop the engine from ticking after shutdown.
        emscripten_cancel_main_loop();

#endif
    }

#if defined(ZY_PLATFORM_WEB)

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Kernel::OnFrame(Ptr<void> Instance)
    {
        const Ptr<Kernel> Executor = static_cast<Ptr<Kernel>>(Instance);

        if (!Executor->Tick())
        {
            Executor->Terminate();
        }
    }

#endif

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
        ConstRetainer<ZyAudio::Service> Audio = GetService<ZyAudio::Service>();

        if (Audio && mStartup.IsAudioPauseOnFocusLost())
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
        return false;
    }

#endif
}