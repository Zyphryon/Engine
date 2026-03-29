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

#include "Zyphryon.Graphic/Driver.hpp"
#include <d3d11_4.h>
#include <d3d12.h>
#include <wrl/client.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Encapsulate the graphic driver implementation using Direct3D 11.
    class D3D11Driver final : public Driver
    {
    public:

        /// \brief Constructs a new driver instance.
        D3D11Driver();

        /// \copydoc Driver::Initialize(Ptr<SDL_Window>, UInt16, UInt16, Multisample)
        Bool Initialize(Ptr<SDL_Window> Window, UInt16 Width, UInt16 Height, Multisample Samples) override;

        /// \copydoc Driver::Reset(UInt16, UInt16, Multisample)
        void Reset(UInt16 Width, UInt16 Height, Multisample Samples) override;

        /// \copydoc Driver::GetDevice()
        ConstRef<Device> GetDevice() const override;

        /// \copydoc Driver::CreateBuffer(Object, Access, Usage, UInt32, ConstSpan<Byte>)
        void CreateBuffer(Object ID, Access Access, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data) override;

        /// \copydoc Driver::UpdateBuffer(Object, UInt32, Bool, ConstSpan<Byte>)
        void UpdateBuffer(Object ID, UInt32 Offset, Bool Invalidate, ConstSpan<Byte> Data) override;

        /// \copydoc Driver::DeleteBuffer(Object)
        void DeleteBuffer(Object ID) override;

        /// \copydoc Driver::ResizeBuffer(Object, UInt32)
        void ResizeBuffer(Object ID, UInt32 Capacity) override;

        /// \copydoc Driver::CopyBuffer(Object, UInt32, Object, UInt32, Bool, UInt32)
        void CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, Bool Invalidate, UInt32 Size) override;

        /// \copydoc Driver::ReadBuffer(Object, UInt32, UInt32)
        Blob ReadBuffer(Object ID, UInt32 Offset, UInt32 Size) override;

        /// \copydoc Driver::MapBuffer(Object, UInt32, UInt32, Bool)
        Ptr<Byte> MapBuffer(Object ID, UInt32 Offset, UInt32 Size, Bool Invalidate) override;

        /// \copydoc Driver::UnmapBuffer(Object)
        void UnmapBuffer(Object ID) override;

        /// \copydoc Driver::CreatePass(Object, ConstSpan<Attachment>, ConstRef<Attachment>)
        void CreatePass(Object ID, ConstSpan<Attachment> Colors, ConstRef<Attachment> Auxiliary) override;

        /// \copydoc Driver::DeletePass(Object)
        void DeletePass(Object ID) override;

        /// \copydoc Driver::CreatePipeline(Object, ConstRef<Shaders>, ConstRef<States>)
        void CreatePipeline(Object ID, ConstRef<Shaders> Shaders, ConstRef<States> States) override;

        /// \copydoc Driver::DeletePipeline(Object)
        void DeletePipeline(Object ID) override;

        /// \copydoc Driver::CreateTexture(Object, Access, TextureType, TextureFormat, TextureLayout, UInt16, UInt16, UInt8, Multisample, ConstSpan<Byte>)
        void CreateTexture(Object ID, Access Access, TextureType Type, TextureFormat Format, TextureLayout Layout, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, ConstSpan<Byte> Data) override;

        /// \copydoc Driver::UpdateTexture(Object, UInt8, UInt16, UInt16, UInt16, UInt16, UInt32, ConstSpan<Byte>)
        void UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data) override;

        /// \copydoc Driver::DeleteTexture(Object)
        void DeleteTexture(Object ID) override;

        /// \copydoc Driver::CopyTexture(Object, UInt8, UInt16, UInt16, Object, UInt8, UInt16, UInt16, Bool, UInt16, UInt16)
        void CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, Bool Invalidate, UInt16 Width, UInt16 Height) override;

        /// \copydoc Driver::ReadTexture(Object, UInt8, UInt16, UInt16, UInt16, UInt16)
        Blob ReadTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height) override;

        /// \copydoc Driver::Prepare(Object, ConstRef<Viewport>, Clear, Color, Real32, UInt8)
        void Prepare(Object Pass, ConstRef<Viewport> Viewport, Clear Target, Color Tint, Real32 Depth, UInt8 Stencil) override;

        /// \copydoc Driver::Submit(ConstSpan<DrawPacket>)
        void Submit(ConstSpan<DrawPacket> Submissions) override;

        /// \copydoc Driver::Commit(Object)
        void Commit(Object Pass) override;

    private:

        /// \brief Smart pointer for managing COM (Component Object Model) objects.
        template<typename Type>
        using ComPtr = Microsoft::WRL::ComPtr<Type>;

        /// \brief Internal wrapper for Direct3D 11 attachment resources.
        struct D3D11Attachment final
        {
            ComPtr<ID3D11RenderTargetView> Resource;
            ComPtr<ID3D11Resource>         Source;
            UInt32                         SourceLevel;
            ComPtr<ID3D11Resource>         Target;
            UInt32                         TargetLevel;
            DXGI_FORMAT                    TargetFormat;
        };

        /// \brief Internal wrapper for Direct3D 11 buffer resources.
        struct D3D11Buffer final
        {
            ComPtr<ID3D11Buffer> Object;
        };

        /// \brief Internal wrapper for Direct3D 11 pass resources.
        struct D3D11Pass final
        {
            ZYPHRYON_INLINE ~D3D11Pass()
            {
                Colors.clear();
            }

            Vector<D3D11Attachment, kMaxAttachments> Colors;
            ComPtr<ID3D11DepthStencilView>           Auxiliary;
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
        struct D3D11Sampler final
        {
            ComPtr<ID3D11SamplerState> Object;
        };

        /// \brief Internal wrapper for Direct3D 11 texture resources.
        struct D3D11Texture final
        {
            ComPtr<ID3D11Texture2D>          Object;
            ComPtr<ID3D11ShaderResourceView> Resource;
            TextureFormat                    Format;
        };

        /// \brief Direct3D 11 device capabilities and feature support.
        struct D3D11Properties final
        {
            /// Supported multisampling configurations.
            Array<DXGI_SAMPLE_DESC, kMaxMultisamples + 1> Multisample;   // TODO Add per format

            /// Indicates support for variable refresh rate (tearing).
            Bool                                          Tearing;
        };

        /// \brief Queries available GPU adapters.
        void LoadAdapters();

        /// \brief Queries device capabilities.
        void LoadCapabilities();

        /// \brief Initializes default rendering states.
        void LoadStates();

        /// \brief Creates a swapchain for the specified display and associates it with a render pass.
        ///
        /// \param Pass    Render pass object that will be linked to the swapchain.
        /// \param Window  Handle to the window for presentation.
        /// \param Width   Width of the swapchain back-buffer in pixels.
        /// \param Height  Height of the swapchain back-buffer in pixels.
        /// \param Samples Multisampling count for anti-aliasing.
        void CreateSwapchain(Ref<D3D11Pass> Pass, HWND Window, UInt16 Width, UInt16 Height, UInt8 Samples);

        /// \brief Creates the render targets and depth resources for a swapchain-backed render pass.
        ///
        /// \param Pass    Render pass object that will own the swapchain attachments.
        /// \param Width   Width of the back-buffer in pixels.
        /// \param Height  Height of the back-buffer in pixels.
        /// \param Samples Multisampling count for anti-aliasing.
        void CreateSwapchainResources(Ref<D3D11Pass> Pass, UInt16 Width, UInt16 Height, UInt8 Samples) const;

        /// \brief Applies vertex buffer bindings from the latest submission.
        ///
        /// \param Oldest Previous submission state used for comparison.
        /// \param Newest Current submission state containing updated resources.
        void ApplyVertexResources(ConstRef<DrawPacket> Oldest, ConstRef<DrawPacket> Newest) const;

        /// \brief Applies sampler bindings from the latest submission.
        ///
        /// \param Oldest Previous submission state used for comparison.
        /// \param Newest Current submission state containing updated resources.
        void ApplySamplerResources(ConstRef<DrawPacket> Oldest, ConstRef<DrawPacket> Newest);

        /// \brief Applies texture bindings from the latest submission.
        ///
        /// \param Oldest Previous submission state used for comparison.
        /// \param Newest Current submission state containing updated resources.
        void ApplyTextureResources(ConstRef<DrawPacket> Oldest, ConstRef<DrawPacket> Newest) const;

        /// \brief Applies uniform buffer bindings from the latest submission.
        ///
        /// \param Oldest Previous submission state used for comparison.
        /// \param Newest Current submission state containing updated resources.
        void ApplyUniformResources(ConstRef<DrawPacket> Oldest, ConstRef<DrawPacket> Newest) const;

        /// \brief Retrieves an existing sampler or creates a new one based on the descriptor.
        ///
        /// \param Descriptor The sampler descriptor parameters.
        /// \return A reference to the sampler object.
        Ref<D3D11Sampler> GetOrCreateSampler(Sampler Descriptor);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ComPtr<ID3D11Device1>               mDevice;
        ComPtr<ID3D11DeviceContext1>        mDeviceImmediate;
        ComPtr<IDXGIFactory1>               mDeviceFactory;
        D3D11Properties                     mProperties;
        Device                              mCapabilities;
        DrawPacket                          mStates;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ComPtr<IDXGISwapChain>              mDisplay;
        Array<D3D11Buffer, kMaxBuffers>     mBuffers;
        Array<D3D11Pass, kMaxPasses>        mPasses;
        Array<D3D11Pipeline, kMaxPipelines> mPipelines;
        Table<UInt64, D3D11Sampler>         mSamplers;
        Array<D3D11Texture, kMaxTextures>   mTextures;
    };
}
