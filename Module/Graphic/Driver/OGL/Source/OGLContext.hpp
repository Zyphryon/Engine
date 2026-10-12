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

#include "OGLDevice.hpp"
#include "OGLLoader.hpp"

#if   defined(ZY_PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(ZY_PLATFORM_WEB)
#include <emscripten/html5.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Represents the platform's OpenGL context and the window it presents to.
    class Context final
    {
    public:

        /// The core profile versions a desktop context is created at, newest first.
        static constexpr UInt8 kLadder[] = { 46, 45, 44, 43, 42, 41, 40, 33 };

    public:

        /// \brief Constructs an empty context that holds no native resources until \ref Initialize succeeds.
        Context();

        /// \brief Destroys the native rendering context and releases any acquired platform resources.
        ~Context();

        /// \brief Creates the native rendering context for the window and makes it current.
        ///
        /// \param Output The platform window handle the context is bound to (ignored on the web backend).
        /// \param Config The configuration describing the requested color and depth formats.
        /// \param Loader The loader the window system's functions are found through, which outlives the context.
        /// \param Device The device that records what the platform enabled on its own.
        /// \return `true` when the context was created and made current, `false` otherwise.
        Bool Initialize(Ptr<void> Output, ConstRef<Configuration> Config, ConstRef<Loader> Loader, Ref<Device> Device);

        /// \brief Checks what the driver requires of the device and sets the state the context never changes.
        ///
        /// \param Device The device, already read and described.
        /// \return `true` when the context offers everything the driver requires, `false` otherwise.
        Bool Prepare(ConstRef<Device> Device);

        /// \brief Records the size the window's back buffer now has, which the window resizes on its own.
        ///
        /// \param Width  The width of the back buffer, in pixels.
        /// \param Height The height of the back buffer, in pixels.
        void Resize(UInt16 Width, UInt16 Height);

        /// \brief Presents the current frame by swapping the window's buffers.
        void Present();

        /// \brief Sets whether presentation waits for the vertical blank.
        ///
        /// \param Tearless `true` to wait for the vertical blank, `false` to present as soon as a frame is ready.
        void SetTearless(Bool Tearless);

        /// \brief Makes the context current on the calling thread, before teardown deletes what it holds.
        ///
        /// \return `true` when the context exists and is now current on the calling thread, `false` otherwise.
        Bool Acquire();

        /// \brief Checks whether the back buffer already has a size.
        ///
        /// \param Width  The width to compare against, in pixels.
        /// \param Height The height to compare against, in pixels.
        /// \return `true` when the back buffer has that size, otherwise `false`.
        ZY_INLINE Bool Matches(UInt16 Width, UInt16 Height) const
        {
            return mConfig.Width == Width && mConfig.Height == Height;
        }

        /// \brief Gets the configuration the window was granted.
        ///
        /// \return The requested configuration, with the color format the window actually presents.
        ZY_INLINE ConstRef<Configuration> GetConfig() const
        {
            return mConfig;
        }

    private:

        /// \brief Creates the platform's rendering context for the window and makes it current.
        ///
        /// \param Output The platform window handle the context is bound to (ignored on the web backend).
        /// \param Device The device that records what the platform enabled on its own.
        /// \return `true` when the context was created and made current, `false` otherwise.
        Bool Create(Ptr<void> Output, Ref<Device> Device);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstPtr<Loader>                mLoader;
        Configuration                   mConfig;

#if   defined(ZY_PLATFORM_WINDOWS)
        HWND                            mWindow;
        HDC                             mSurface;
        HGLRC                           mHandle;
        Ptr<void>                       mInterval;
#elif defined(ZY_PLATFORM_LINUX)
        UInt                            mWindow;
        Ptr<void>                       mConnection;
        Ptr<void>                       mHandle;
        Ptr<void>                       mSwap;
        Ptr<void>                       mInterval;
        Bool                            mMesa;
#elif defined(ZY_PLATFORM_WEB)
        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE mHandle;
#endif
    };
}