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

#include "D3D11Device.hpp"
#include <dxgi1_5.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    /// \brief Represents the flip-model swapchain of a window.
    class Swapchain final
    {
    public:

        /// \brief Constructs a swapchain that presents nothing until \ref Create.
        Swapchain();

        /// \brief Closes the handle the swapchain signals when it can take another frame.
        ~Swapchain();

        /// \brief Creates the swapchain for a window.
        ///
        /// \param Device The device the swapchain presents for.
        /// \param Window The native window handle to present into.
        /// \param Config The size, formats and presentation mode to start with.
        /// \return `true` if the swapchain was created, `false` otherwise.
        Bool Create(ConstRef<Device> Device, HWND Window, ConstRef<Configuration> Config);

        /// \brief Resizes the back buffers, keeping their old size when that fails.
        ///
        /// \note No view of the back buffer may be alive, bound or held, when this is called.
        ///
        /// \param Width  The new width of the back buffers in pixels.
        /// \param Height The new height of the back buffers in pixels.
        void Resize(UInt16 Width, UInt16 Height);

        /// \brief Presents the back buffer, then waits until the swapchain can take another frame.
        ///
        /// \note Presentation waits for the vertical blank when it is tearless.
        void Present();

        /// \brief Gets the back buffer the display pass renders into.
        ///
        /// \return The back buffer, or empty when it could not be obtained.
        ComPtr<ID3D11Texture2D> GetBuffer() const;

        /// \brief Changes whether presentation waits for the vertical blank.
        ///
        /// \param Tearless `true` to wait for the vertical blank, `false` to present as soon as a frame is ready.
        ZY_INLINE void SetTearless(Bool Tearless)
        {
            mConfig.Tearless = Tearless;
        }

        /// \brief Checks whether the back buffers already have the given size.
        ///
        /// \param Width  The width to compare against, in pixels.
        /// \param Height The height to compare against, in pixels.
        /// \return `true` if both match, `false` otherwise.
        ZY_INLINE Bool Matches(UInt16 Width, UInt16 Height) const
        {
            return mConfig.Width == Width && mConfig.Height == Height;
        }

        /// \brief Gets the size, formats and presentation mode the window was granted.
        ///
        /// \return The configuration of the swapchain.
        ZY_INLINE ConstRef<Configuration> GetConfig() const
        {
            return mConfig;
        }

        /// \brief Gets the format the display pass views the back buffer through.
        ///
        /// \return The view format, which may differ from the configured one in channel order alone.
        ZY_INLINE DXGI_FORMAT GetFormat() const
        {
            return mFormat;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ComPtr<IDXGISwapChain2> mHandle;
        HANDLE                  mWaitable;
        Configuration           mConfig;
        DXGI_FORMAT             mFormat;
        Bool                    mTearing;
    };
}