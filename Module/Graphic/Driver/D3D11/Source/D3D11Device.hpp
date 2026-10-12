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

#include "D3D11Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    /// \brief Represents the Direct3D 11 device, its immediate context and the DXGI factory, with what they can do.
    class Device final
    {
    public:

        /// \brief Creates the device on the default adapter, falling back to the software rasterizer (WARP).
        ///
        /// \param Format The color format the adapters' display modes are listed in.
        /// \return `true` if the device was created and binds ranges of a constant buffer, `false` otherwise.
        Bool Initialize(TextureFormat Format);

        /// \brief Gets the device resources are created on.
        ///
        /// \return The device, or `nullptr` before \ref Initialize succeeded.
        ZY_INLINE Ptr<ID3D11Device1> GetHandle() const
        {
            return mHandle.Get();
        }

        /// \brief Gets the immediate context commands are recorded into.
        ///
        /// \return The context, or `nullptr` before \ref Initialize succeeded.
        ZY_INLINE Ptr<ID3D11DeviceContext1> GetContext() const
        {
            return mContext.Get();
        }

        /// \brief Gets the factory swapchains are created from.
        ///
        /// \return The factory, or `nullptr` before \ref Initialize succeeded.
        ZY_INLINE Ptr<IDXGIFactory1> GetFactory() const
        {
            return mFactory.Get();
        }

        /// \brief Gets the backend, tier, capabilities and adapters the device was found with.
        ///
        /// \return The description of the device.
        ZY_INLINE ConstRef<Description> GetDescription() const
        {
            return mDescription;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ComPtr<ID3D11Device1>        mHandle;
        ComPtr<ID3D11DeviceContext1> mContext;
        ComPtr<IDXGIFactory1>        mFactory;
        Description                  mDescription;
    };
}