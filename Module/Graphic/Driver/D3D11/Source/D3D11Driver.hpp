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
#include "D3D11Compiler.hpp"
#include "Zyphryon.Graphic/Driver.hpp"
#include <d3d11_4.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Encapsulate the graphic driver implementation using Direct3D 11.
    class D3D11Driver final : public Driver
    {
    public:

        /// \see Driver::Initialize(Ptr<void>, ConstRef<Config>)
        Bool Initialize(Ptr<void> Output, ConstRef<Config> Config) override;

        /// \see Driver::Reset(UInt16, UInt16)
        void Reset(UInt16 Width, UInt16 Height) override;

        /// \see Driver::Probe(Ref<Description>)
        void Probe(Ref<Description> Output) const override;

        /// \see Driver::CreateBuffer(Object, Storage, Usage, UInt32, ConstSpan<Byte>)
        void CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data) override;

        /// \see Driver::UpdateBuffer(Object, UInt32, ConstSpan<Byte>)
        void UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data) override;

        /// \see Driver::DeleteBuffer(Object)
        void DeleteBuffer(Object ID) override;

        /// \see Driver::CopyBuffer(Object, UInt32, Object, UInt32, UInt32)
        void CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, UInt32 Size) override;

        /// \see Driver::MapBuffer(Object, UInt32, UInt32)
        Ptr<Byte> MapBuffer(Object ID, UInt32 Offset, UInt32 Size) override;

        /// \see Driver::UnmapBuffer(Object)
        void UnmapBuffer(Object ID) override;

        /// \see Driver::CreatePass(Object, ConstSpan<ColorAttachment>, DepthAttachment)
        void CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth) override;

        /// \see Driver::DeletePass(Object)
        void DeletePass(Object ID) override;

        /// \see Driver::CreatePipeline(Object, ConstRef<Program>, ConstRef<States>)
        void CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<States> States) override;

        /// \see Driver::DeletePipeline(Object)
        void DeletePipeline(Object ID) override;

        /// \see Driver::CreateTexture(Object, TextureLayout, TextureFormat, Storage, Usage, UInt16, UInt16, UInt8, Multisample, ConstSpan<Byte>)
        void CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, ConstSpan<Byte> Data) override;

        /// \see Driver::UpdateTexture(Object, UInt8, UInt16, UInt16, UInt16, UInt16, UInt32, ConstSpan<Byte>)
        void UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data) override;

        /// \see Driver::DeleteTexture(Object)
        void DeleteTexture(Object ID) override;

        /// \see Driver::CopyTexture(Object, UInt8, UInt16, UInt16, Object, UInt8, UInt16, UInt16, UInt16, UInt16)
        void CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, UInt16 Width, UInt16 Height) override;

        /// \see Driver::Prepare(Object, ConstRef<Viewport>, ConstSpan<Color>, Real32, UInt8)
        void Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil) override;

        /// \see Driver::Submit(ConstSpan<Command>)
        void Submit(ConstSpan<Command> Commands) override;

        /// \see Driver::Commit(Object)
        void Commit(Object Pass) override;
    private:

        /// \brief Internal wrapper for Direct3D 11 buffer resources.
        using  D3D11Buffer  = ComPtr<ID3D11Buffer>;

        /// \brief Internal wrapper for Direct3D 11 color attachment resources.
        struct D3D11ColorAttachment final
        {
            ComPtr<ID3D11Resource>         Target;
            ComPtr<ID3D11RenderTargetView> TargetResource;
            UInt32                         TargetLevel;
            ComPtr<ID3D11Resource>         Resolve;
            UInt32                         ResolveLevel;
            DXGI_FORMAT                    ResolveFormat;
            Action                         LoadAction;
            Action                         StoreAction;
        };

        /// \brief Internal wrapper for Direct3D 11 depth/stencil attachment resources.
        struct D3D11DepthStencilAttachment final
        {
            ComPtr<ID3D11DepthStencilView> Target;
            Action                         DepthLoadAction;
            Action                         DepthStoreAction;
            Action                         StencilLoadAction;
            Action                         StencilStoreAction;
        };

        /// \brief Internal table for Direct3D 11 multisampling description.
        using D3D11Multisamples = Array<Array<UInt8, 8 + 1>, Enum::Count<TextureFormat>()>;

        /// \brief Internal wrapper for Direct3D 11 pass resources.
        struct D3D11Pass final
        {
            Sequence<D3D11ColorAttachment, kMaxAttachments> Colors;
            D3D11DepthStencilAttachment                     DepthStencil;
        };

        /// \brief Internal wrapper for Direct3D 11 pipeline resources.
        struct D3D11Pipeline final
        {
            ComPtr<ID3D11VertexShader>      VS;
            ComPtr<ID3D11PixelShader>       PS;
            ComPtr<ID3D11BlendState>        BS;
            ComPtr<ID3D11DepthStencilState> DS;
            ComPtr<ID3D11RasterizerState>   RS;
            ComPtr<ID3D11InputLayout>       IL;
            D3D11_PRIMITIVE_TOPOLOGY        PT;
        };

        /// \brief Internal wrapper for Direct3D 11 sampler resources.
        using  D3D11Sampler = ComPtr<ID3D11SamplerState>;

        /// \brief Internal wrapper for Direct3D 11 texture resources.
        struct D3D11Texture final
        {
            ComPtr<ID3D11Texture2D>          Object;
            ComPtr<ID3D11ShaderResourceView> Resource;
            TextureFormat                    Format;
            UINT                             Samples;
        };

        /// \brief Direct3D 11 device capabilities and feature support.
        struct D3D11Properties final
        {
            /// Supported multisampling configurations.
            D3D11Multisamples Multisample;

            /// The preferred color format for the device.
            TextureFormat     ColorFormat = TextureFormat::Unspecified;

            /// The preferred depth format for the device.
            TextureFormat     DepthFormat = TextureFormat::Unspecified;
        };

    private:

        /// \brief Queries available GPU adapters.
        ///
        /// \param Format The DXGI format to use when enumerating display modes.
        void LoadAdapters(DXGI_FORMAT Format);

        /// \brief Queries device capabilities.
        void LoadCapabilities();

        /// \brief Creates a swapchain for the specified display and associates it with a render pass.
        ///
        /// \param Pass   The render pass to associate with the swapchain.
        /// \param Window The native window handle to create the swapchain for.
        /// \param Config The configuration settings for the swapchain.
        void CreateSwapchain(Ref<D3D11Pass> Pass, HWND Window, ConstRef<Config> Config);

        /// \brief Creates the render targets and depth resources for a swapchain-backed render pass.
        ///
        /// \param Pass   The render pass to create resources for.
        /// \param Config The configuration settings for the swapchain.
        void CreateSwapchainResources(Ref<D3D11Pass> Pass, ConstRef<Config> Config) const;

        /// \brief Applies the vertex buffer and index buffer resources for a draw item.
        ///
        /// \param Oldest The oldest draw item that was submitted.
        /// \param Newest The newest draw item that is being submitted.
        void ApplyVertexResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

        /// \brief Applies the sampler state resources for a draw item.
        ///
        /// \param Oldest The oldest draw item that was submitted.
        /// \param Newest The newest draw item that is being submitted.
        void ApplySamplerResources(ConstRef<Command> Oldest, ConstRef<Command> Newest);

        /// \brief Applies the texture resources for a draw item.
        ///
        /// \param Oldest The oldest draw item that was submitted.
        /// \param Newest The newest draw item that is being submitted.
        void ApplyTextureResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

        /// \brief Applies the uniform resources for a draw item.
        ///
        /// \param Oldest The oldest draw item that was submitted.
        /// \param Newest The newest draw item that is being submitted.
        void ApplyUniformResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

        /// \brief Gets an existing sampler state with the specified descriptor or creates a new one if it doesn't exist.
        ///
        /// \param Descriptor The sampler descriptor defining the desired sampler state configuration.
        /// \return A sampler state resource that matches the specified descriptor.
        Ptr<ID3D11SamplerState> GetOrCreateSampler(Sampler Descriptor);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ComPtr<ID3D11Device1>               mDevice;
        ComPtr<ID3D11DeviceContext1>        mDeviceImmediate;
        ComPtr<IDXGIFactory1>               mDeviceFactory;
        D3D11Properties                     mDeviceProperties;
        D3D11Compiler                       mCompiler;
        Description                         mDescription;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ComPtr<IDXGISwapChain>              mSwapchain;
        Array<D3D11Buffer, kMaxBuffers>     mBuffers;
        Array<D3D11Pass, kMaxPasses>        mPasses;
        Array<D3D11Pipeline, kMaxPipelines> mPipelines;
        Table<UInt64, D3D11Sampler>         mSamplers;
        Array<D3D11Texture, kMaxTextures>   mTextures;
    };
}
