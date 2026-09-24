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

#include "Startup.hpp"
#include "Zyphryon.Engine/Module.hpp"
#include "Zyphryon.Platform/Timer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRuntime
{
    /// \brief Represents the application itself, bringing the services up, running the frame loop and tearing them down.
    class ZY_API Kernel : public ZyEngine::Subsystem::Host
    {
    public:

        /// \brief Constructs a kernel that is not running yet.
        Kernel();

        /// \brief Destructs the kernel and releases all associated resources.
        virtual ~Kernel() = default;

        /// \brief Kernels are not copied, since each one owns the modules it was handed.
        Kernel(ConstRef<Kernel> Other) = delete;

        /// \brief Writes the parameters a user is allowed to change, as they currently stand.
        ///
        /// \param Root The object to write the parameters under.
        void Save(JsonObject Root);

        /// \brief Parses the command line, configures the application, and runs it to completion.
        ///
        /// \note Invoked by \ref ZY_APPLICATION, so an application never calls this itself.
        ///
        /// \param Count     The number of entries in \p Arguments.
        /// \param Arguments The argument vector, as handed to the platform entry point.
        /// \param Modules   The modules to attach and run.
        void Run(UInt Count, ConstPtr<ConstPtr<Char>> Arguments, AnyRef<ZyEngine::Modules> Modules);

        /// \brief Signals the application to exit gracefully.
        void Quit();

        /// \brief Gets the switches the application was launched with.
        ///
        /// \return The parsed command line.
        ZY_INLINE ConstRef<Environment> GetEnvironment() const
        {
            return mEnvironment;
        }

        /// \brief Kernels are not copied, since each one owns the modules it was handed.
        Ref<Kernel> operator=(ConstRef<Kernel> Other) = delete;

    protected:

        /// \brief Called before any service exists, to let applications set their startup parameters.
        ///
        /// \param Startup Receives the parameters the engine starts with, pre-filled with defaults.
        virtual void OnConfigure(Ref<Startup> Startup)
        {

        }

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

        /// \brief Brings every service up, attaches the modules, then initializes the application.
        ///
        /// \param Modules The modules to attach, kept by the kernel once they are.
        /// \return `true` once the application has been initialized, `false` when a service it needs failed first.
        Bool Initialize(AnyRef<ZyEngine::Modules> Modules);

        /// \brief Advances the application by one frame.
        ///
        /// \return `true` if the application should continue running, `false` if termination was requested.
        Bool Tick();

        /// \brief Terminates the application, then shuts every service down.
        void Terminate();

        /// \brief Detaches the modules that were attached and tears down every service that was registered.
        void Shutdown();

#if defined(ZY_PLATFORM_WEB)

        /// \brief Advances one frame from the browser's main loop, tearing everything down once the kernel stops.
        ///
        /// \param Instance The kernel being run.
        static void OnFrame(Ptr<void> Instance);

#endif

    private:

#if !defined(ZY_MODE_HEADLESS)

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

#endif

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bool              mAlive;
        ZyPlatform::Timer mTimer;
        Environment       mEnvironment;
        Startup           mStartup;
        ZyEngine::Modules mModules;
    };
}