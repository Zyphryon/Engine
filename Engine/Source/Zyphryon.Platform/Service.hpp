// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Display.hpp"
#include "Window.hpp"
#include "../Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Platform
{
    /// \brief Manages platform windows and pumps OS events each frame.
    class Service : public Engine::Subsystem
    {
    public:

        /// \brief Constructs the platform service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Destroys the platform service and shuts down the underlying windowing system.
        ~Service() override;

        /// \brief Processes platform events and updates window state each frame.
        ///
        /// \param Delta The elapsed time in seconds since the previous tick.
        void OnTick(Real64 Delta) override;

        /// \brief Gets the display managed by the platform service.
        ///
        /// \return A reference to the display.
        ZY_INLINE Ref<Display> GetDisplay()
        {
            return mDisplay;
        }

        /// \brief Initializes the platform window with the specified configuration.
        ///
        /// \param Target     The target monitor on which to create the window.
        /// \param Title      The title to display in the window's title bar.
        /// \param Width      The width of the window in pixels.
        /// \param Height     The height of the window in pixels.
        /// \param Borderless Whether to create the window without borders or title bar.
        /// \param Fullscreen Whether to create the window in fullscreen mode.
        /// \return `true` if the window was successfully initialized, otherwise `false`.
        Bool Initialize(Text Target, Text Title, UInt32 Width, UInt32 Height, Bool Borderless, Bool Fullscreen);

        /// \brief Gets the primary window managed by the platform service.
        ///
        /// \return A reference to the primary window.
        ZY_INLINE Ref<Window> GetWindow()
        {
            return mWindow;
        }

    private:

        /// \brief Pumps and dispatches pending platform messages from the OS event queue.
        void Poll();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Dispatcher mDispatcher;
        Window     mWindow;
        Display    mDisplay;
    };
}