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
#include <windows.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Platform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host },
          mWindow   { mDispatcher }
    {
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
        // Poll system messages (platform-specific).
        Poll();

        // Process system notifications.
        if (mDispatcher.IsNotified(Dispatcher::Notification::Monitor))
        {
            mDisplay.Poll();
        }

        // Forwards all queued input events to the input service.
        if (ConstRetainer<Input::Service> Input = GetHost().GetService<Input::Service>())
        {
            const Dispatcher Snapshot = mDispatcher;
            Input->Process(Snapshot.GetInputEvents());
        }
        mDispatcher.Reset();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Initialize(Text Target, Text Title, UInt32 Width, UInt32 Height, Bool Borderless, Bool Fullscreen)
    {
        ConstRef<Monitor> Monitor = mDisplay.GetMonitor(Target);

        if (!Target.IsEmpty() && Target != Monitor.GetName())
        {
            LOG_W("Can't find monitor '{0}', default to '{1}'", Target, Monitor.GetName());
        }

        Width  = static_cast<Real32>(Width)  * Monitor.GetScale();
        Height = static_cast<Real32>(Height) * Monitor.GetScale();

        const SInt32 PositionX = Monitor.GetX() + (Monitor.GetWidth() - Width) / 2;
        const SInt32 PositionY = Monitor.GetY() + (Monitor.GetHeight() - Height) / 2;
        return mWindow.Initialize(Title, PositionX, PositionY, Width, Height, Borderless, Fullscreen);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Poll()
    {
#ifdef    ZY_PLATFORM_WINDOWS

        MSG Event = { };

        while (::PeekMessageW(AddressOf(Event), nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(AddressOf(Event));
            ::DispatchMessageW(AddressOf(Event));

            if (Event.message == WM_QUIT)
            {
                break;
            }
        }

#endif // ZY_PLATFORM_WINDOWS

    }
}