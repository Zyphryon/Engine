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

#include "D3D11Pass.hpp"
#include "Zyphryon.Graphic/Metadata.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Pass::Pass()
        : mStencil { false }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Pass::Create(ConstRef<Device> Device, ConstRef<Swapchain> Swapchain)
    {
        const Ptr<ID3D11Device1>      Handle = Device.GetHandle();
        ConstRef<Configuration>       Config = Swapchain.GetConfig();
        const ComPtr<ID3D11Texture2D> Buffer = Swapchain.GetBuffer();

        if (!Buffer)
        {
            return false;
        }

        Ref<Attachment> Color = mColors.Append();

        // An sRGB format names the view alone, as the flip model keeps the buffer itself in its linear twin.
        const CD3D11_RENDER_TARGET_VIEW_DESC View(D3D11_RTV_DIMENSION_TEXTURE2D, Swapchain.GetFormat());

        if (!Check(Handle->CreateRenderTargetView(Buffer.Get(), AddressOf(View), Color.View.GetAddressOf())))
        {
            return false;
        }

        if (Config.DepthFormat == TextureFormat::Unspecified)
        {
            return true;
        }

        mDepth.DepthLoadAction    = Action::Clear;
        mDepth.DepthStoreAction   = Action::Discard;
        mDepth.StencilLoadAction  = Action::Clear;
        mDepth.StencilStoreAction = Action::Discard;
        mStencil                  = GetTextureMetadata(Config.DepthFormat).IsStencil;

        const CD3D11_TEXTURE2D_DESC Description(
            Convert(Config.DepthFormat),
            Config.Width,
            Config.Height,
            1,
            1,
            D3D11_BIND_DEPTH_STENCIL);

        ComPtr<ID3D11Texture2D> DepthBuffer;

        if (!Check(Handle->CreateTexture2D(AddressOf(Description), nullptr, DepthBuffer.GetAddressOf())))
        {
            return false;
        }

        const CD3D11_DEPTH_STENCIL_VIEW_DESC DepthView(D3D11_DSV_DIMENSION_TEXTURE2D, ConvertDepth(Config.DepthFormat));
        return Check(Handle->CreateDepthStencilView(DepthBuffer.Get(), AddressOf(DepthView), mView.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Pass::Create(ConstRef<Device> Device, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth, ConstSpan<Texture> Textures)
    {
        const Ptr<ID3D11Device1> Handle = Device.GetHandle();

        for (ConstRef<ColorAttachment> Color : Colors)
        {
            ConstRef<Texture> Source = Textures[Color.Target];

            Ref<Attachment> Attachment = mColors.Append();
            Attachment.Load  = Color.LoadAction;
            Attachment.Store = Color.StoreAction;

            const D3D11_RTV_DIMENSION Dimension = (Source.GetLayers() > 1)
                ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY
                : D3D11_RTV_DIMENSION_TEXTURE2D;

            const CD3D11_RENDER_TARGET_VIEW_DESC Description(
                Dimension,
                ConvertView(Source.GetFormat()),
                Color.TargetLevel,
                Color.TargetLayer,
                1);

            if (!Check(Handle->CreateRenderTargetView(Source.GetHandle(), AddressOf(Description), Attachment.View.GetAddressOf())))
            {
                return false;
            }
        }

        if (!Depth.Target)
        {
            return true;
        }

        ConstRef<Texture> Source = Textures[Depth.Target];

        mDepth   = Depth;
        mStencil = GetTextureMetadata(Source.GetFormat()).IsStencil;

        const D3D11_DSV_DIMENSION Dimension = (Source.GetLayers() > 1)
            ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY
            : D3D11_DSV_DIMENSION_TEXTURE2D;

        const CD3D11_DEPTH_STENCIL_VIEW_DESC Description(
            Dimension,
            ConvertDepth(Source.GetFormat()),
            Depth.TargetLevel,
            Depth.TargetLayer,
            1);

        return Check(Handle->CreateDepthStencilView(Source.GetHandle(), AddressOf(Description), mView.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pass::Delete()
    {
        * this = Pass();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pass::Prepare(ConstRef<Device> Device, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        const Ptr<ID3D11DeviceContext1> Context = Device.GetContext();

        Sequence<Ptr<ID3D11RenderTargetView>, kMaxAttachments> Views;
        for (ConstRef<Attachment> Attachment : mColors)
        {
            Views.Append(Attachment.View.Get());
        }
        Context->OMSetRenderTargets(Views.GetSize(), Views.GetData(), mView.Get());

        // An attachment past the last clear color handed over is left as it was, even when it asks to be cleared.
        for (UInt32 Slot = 0; Slot < Views.GetSize(); ++Slot)
        {
            if (mColors[Slot].Load == Action::Clear && Slot < Colors.GetSize())
            {
                Context->ClearRenderTargetView(Views[Slot], reinterpret_cast<ConstPtr<FLOAT>>(AddressOf(Colors[Slot])));
            }
        }

        if (mView)
        {
            UINT Mode = 0;
            if (mDepth.DepthLoadAction == Action::Clear)
            {
                Mode |= D3D11_CLEAR_DEPTH;
            }
            if (mDepth.StencilLoadAction == Action::Clear)
            {
                Mode |= D3D11_CLEAR_STENCIL;
            }

            if (Mode != 0)
            {
                Context->ClearDepthStencilView(mView.Get(), Mode, Depth, Stencil);
            }
        }

        const CD3D11_VIEWPORT Rect(
            Viewport.X,
            Viewport.Y,
            Viewport.Width,
            Viewport.Height,
            Viewport.MinDepth,
            Viewport.MaxDepth);
        Context->RSSetViewports(1, AddressOf(Rect));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pass::Commit(ConstRef<Device> Device)
    {
        const Ptr<ID3D11DeviceContext1> Context = Device.GetContext();

        // What the pass does not keep is discarded, so the driver need not preserve it past the pass.
        for (ConstRef<Attachment> Attachment : mColors)
        {
            if (Attachment.Store != Action::Store)
            {
                Context->DiscardView(Attachment.View.Get());
            }
        }

        // Discarding the view drops both planes, so it waits until neither is kept.
        const Bool DiscardDepth   = (mDepth.DepthStoreAction == Action::Discard);
        const Bool DiscardStencil = (!mStencil || mDepth.StencilStoreAction == Action::Discard);

        if (mView && DiscardDepth && DiscardStencil)
        {
            Context->DiscardView(mView.Get());
        }
    }
}