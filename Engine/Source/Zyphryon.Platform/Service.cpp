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

#include "Service.hpp"
#include "Zyphryon.Input/Service.hpp"

#if   defined(ZY_PLATFORM_WINDOWS)
#include "Driver/Windows/Service.inl"
#elif defined(ZY_PLATFORM_LINUX)
#include "Driver/Linux/Service.inl"
#elif defined(ZY_PLATFORM_WEB)
#include "Driver/Emscripten/Service.inl"
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyPlatform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host },
          mWindow   { mDispatcher }
    {
        // What the monitors report can depend on how the process was prepared, so it comes before they are polled.
        Prepare();

        mDisplay.Poll();

        for (ConstRef<Monitor> Monitor : mDisplay.GetMonitors())
        {
            LOG_I("Platform: Found Monitor '{0}' {1}x{2} @ {3} Hz, DPI {4:.2f}",
                Monitor.GetName(),
                Monitor.GetWidth(),
                Monitor.GetHeight(),
                Monitor.GetFrequency(),
                Monitor.GetScale());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Real64 Delta)
    {
        ZY_PROFILE_SCOPE("Platform::Tick");

        // Poll system messages (platform-specific).
        mWindow.Poll();

        // Process system notifications.
        if (mDispatcher.IsNotified(Dispatcher::Notification::Monitor))
        {
            mDisplay.Poll();
        }

        // Forwards all queued input events to the input service.
        if (ConstRetainer<ZyInput::Service> Input = GetHost().GetService<ZyInput::Service>())
        {
            mDispatcher.Drain(mEvents);

            Input->Process(mEvents);

            mEvents.Clear();
        }
        mDispatcher.Reset();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Initialize(Text Target, Text Title, UInt32 Width, UInt32 Height, Bool Borderless, Bool Fullscreen)
    {
        if (mDisplay.GetMonitors().IsEmpty())
        {
            LOG_W("Platform: No monitor was reported, placing the window at the origin");
            return mWindow.Initialize(Title, 0, 0, Width, Height, Borderless, Fullscreen);
        }

        ConstRef<Monitor> Monitor = mDisplay.GetMonitor(Target);

        if (!Target.IsEmpty() && Target != Monitor.GetName())
        {
            LOG_W("Platform: Can't find monitor '{0}', default to '{1}'", Target, Monitor.GetName());
        }

        Width  = Min(static_cast<UInt32>(Round(Width * Monitor.GetScale())), Monitor.GetWidth());
        Height = Min(static_cast<UInt32>(Round(Height * Monitor.GetScale())), Monitor.GetHeight());

        const SInt32 PositionX = Monitor.GetX() + static_cast<SInt32>(Monitor.GetWidth() - Width) / 2;
        const SInt32 PositionY = Monitor.GetY() + static_cast<SInt32>(Monitor.GetHeight() - Height) / 2;
        return mWindow.Initialize(Title, PositionX, PositionY, Width, Height, Borderless, Fullscreen);
    }
}