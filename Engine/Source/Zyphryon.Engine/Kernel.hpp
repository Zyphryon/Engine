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

#include "Config.hpp"
#include "Module.hpp"
#include "Zyphryon.Platform/Timer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Engine
{
    /// \brief Entry point and main coordinator of the application runtime.
    class Kernel : public Subsystem::Host
    {
    public:

        /// \brief Default constructor.
        Kernel();

        /// \brief Destructs the kernel and releases all associated resources.
        virtual ~Kernel() = default;

        /// \brief Starts the application main loop with the given configuration and modules.
        ///
        /// \param Config  The application configuration.
        /// \param Modules The modules to attach and run.
        void Run(AnyRef<Config> Config, AnyRef<Modules> Modules);

        /// \brief Signals the application to exit gracefully.
        void Quit();

    protected:

        /// \brief Called during initialization to allow applications to perform custom setup.
        ///
        /// \return `true` if initialization succeeded, `false` otherwise.
        virtual Bool OnInitialize()
        {
            return true;
        }

        /// \brief Called once per frame to allow applications to update their state.
        ///
        /// \param Delta The elapsed time in seconds since the last tick.
        virtual void OnTick(Real64 Delta)
        {

        }

        /// \brief Called during shutdown to allow applications to release resources.
        virtual void OnTerminate()
        {

        }

    private:

        /// \brief Performs internal setup before the application loop begins.
        void Initialize();

        /// \brief Advances the application by one frame.
        ///
        /// \return `true` if the application should continue running, `false` if termination was requested.
        Bool Tick();

        /// \brief Performs internal cleanup after the application loop ends.
        void Terminate();

    private:

        /// \brief Invoked when the window requests to close.
        ///
        /// \return `true` to allow the window to close, `false` to prevent it.
        Bool OnWindowExit();

        /// \brief Invoked when the window is resized.
        ///
        /// \param Width  The new window width in pixels.
        /// \param Height The new window height in pixels.
        /// \return `true` if the resize was handled successfully, `false` otherwise.
        Bool OnWindowResize(UInt32 Width, UInt32 Height);

        /// \brief Invoked when the window gains or loses focus.
        ///
        /// \param Focused `true` if the window gained focus, `false` if it lost focus.
        /// \return `true` if the focus change was handled successfully, `false` otherwise.
        Bool OnWindowFocus(Bool Focused);

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bool            mAlive;
        Platform::Timer mTimer;
        Config          mConfig;
        Modules         mModules;
    };
}