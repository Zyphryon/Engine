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

#include "Zyphryon.Graphic/Types.hpp"

#if   defined(ZY_PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(ZY_PLATFORM_WEB)
#include <emscripten/html5.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Owns the platform's native rendering context and presentation surface for the GLES3 driver.
    class GLES3Context final
    {
    public:

        /// \brief Constructs an empty context that holds no native resources until \ref Initialize succeeds.
        GLES3Context();

        /// \brief Destroys the native rendering context and releases any acquired platform resources.
        ~GLES3Context();

        /// \brief Creates the native rendering context for the given surface and makes it current.
        ///
        /// \param Output The platform window handle the context is bound to (ignored on the web backend).
        /// \param Config The configuration describing the requested color and depth formats.
        /// \return `true` when the context was created and made current, `false` otherwise.
        Bool Initialize(Ptr<void> Output, ConstRef<Config> Config);

        /// \brief Presents the current frame by swapping the surface's buffers.
        void Present();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#if   defined(ZY_PLATFORM_WINDOWS)

        HWND                            mDeviceOutput;
        HDC                             mDeviceContext;
        HGLRC                           mRenderContext;

#elif defined(ZY_PLATFORM_WEB)

        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE mRenderContext;

#endif
    };
}
