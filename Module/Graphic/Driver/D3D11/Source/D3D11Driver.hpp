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

#if defined(ZY_PROFILE_BACKEND_TRACY)
#include <tracy/TracyD3D11.hpp>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    /// \brief Represents the graphic driver implemented over Direct3D 11.
    class D3D11Driver final : public Driver
    {
    public:

        /// \see Driver::Initialize(Ptr<void>, ConstRef<Configuration>)
        Bool Initialize(Ptr<void> Output, ConstRef<Configuration> Config) override;

        /// \see Driver::Reset(UInt16, UInt16, Bool)
        void Reset(UInt16 Width, UInt16 Height, Bool Tearless) override;

        /// \see Driver::Probe(Ref<Description>)
        void Probe(Ref<Description> Output) const override;

        /// \see Driver::CreateBuffer(Object, Storage, Usage, UInt32, ConstSpan<Byte>)
        void CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data) override;

        /// \see Driver::UpdateBuffer(Object, UInt32, ConstSpan<Byte>)
        void UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data) override;

        /// \see Driver::DeleteBuffer(Object)
        void DeleteBuffer(Object ID) override;

        /// \see Driver::MapBuffer(Object, UInt32, UInt32)
        Ptr<Byte> MapBuffer(Object ID, UInt32 Offset, UInt32 Size) override;

        /// \see Driver::UnmapBuffer(Object)
        void UnmapBuffer(Object ID) override;

        /// \see Driver::CreatePass(Object, ConstSpan<ColorAttachment>, DepthAttachment)
        void CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth) override;

        /// \see Driver::DeletePass(Object)
        void DeletePass(Object ID) override;

        /// \see Driver::CreatePipeline(Object, ConstRef<Program>, ConstRef<Signature>, ConstRef<States>)
        void CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States) override;

        /// \see Driver::DeletePipeline(Object)
        void DeletePipeline(Object ID) override;

        /// \see Driver::CreateSampler(Object, Sampler)
        void CreateSampler(Object ID, Sampler Descriptor) override;

        /// \see Driver::DeleteSampler(Object)
        void DeleteSampler(Object ID) override;

        /// \see Driver::CreateTexture(Object, TextureLayout, TextureFormat, Storage, Usage, UInt16, UInt16, UInt16, UInt8, ConstSpan<Byte>)
        void CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data) override;

        /// \see Driver::UpdateTexture(Object, UInt8, UInt16, UInt16, UInt16, UInt16, UInt16, UInt32, ConstSpan<Byte>)
        void UpdateTexture(Object ID, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data) override;

        /// \see Driver::DeleteTexture(Object)
        void DeleteTexture(Object ID) override;

        /// \see Driver::Prepare(Object, Text, ConstRef<Viewport>, ConstSpan<Color>, Real32, UInt8)
        void Prepare(Object Pass, Text Name, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil) override;

        /// \see Driver::Submit(ConstSpan<Command>)
        void Submit(ConstSpan<Command> Commands) override;

        /// \see Driver::Commit(Object)
        void Commit(Object Pass) override;

    private:

        /// \brief Represents a Direct3D 11 buffer.
        using D3D11Buffer = ComPtr<ID3D11Buffer>;

        /// \brief Represents a single Direct3D 11 color attachment of a render pass.
        struct D3D11ColorAttachment final
        {
            /// The view the pass renders into.
            ComPtr<ID3D11RenderTargetView> Target;

            /// The operation performed on the attachment when the pass begins.
            Action                         LoadAction  = Action::Clear;

            /// The operation performed on the attachment when the pass ends.
            Action                         StoreAction = Action::Discard;
        };

        /// \brief Represents a Direct3D 11 render pass.
        struct D3D11Pass final
        {
            /// The color attachments, in binding order.
            Sequence<D3D11ColorAttachment, kMaxAttachments> Colors;

            /// The depth-stencil view, empty when the pass has no depth attachment.
            ComPtr<ID3D11DepthStencilView>                  DepthStencil;

            /// The load and store actions of the depth attachment.
            DepthAttachment                                 Depth;
        };

        /// \brief Represents a Direct3D 11 pipeline, its compiled shaders plus fixed-function state.
        struct D3D11Pipeline final
        {
            /// The vertex shader.
            ComPtr<ID3D11VertexShader>      VS;

            /// The pixel shader.
            ComPtr<ID3D11PixelShader>       PS;

            /// The blend state.
            ComPtr<ID3D11BlendState>        BS;

            /// The depth-stencil state.
            ComPtr<ID3D11DepthStencilState> DS;

            /// The rasterizer state.
            ComPtr<ID3D11RasterizerState>   RS;

            /// The input layout, empty when the pipeline reads no vertex attributes.
            ComPtr<ID3D11InputLayout>       IL;

            /// The primitive topology.
            D3D11_PRIMITIVE_TOPOLOGY        PT = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        };

        /// \brief Represents a Direct3D 11 sampler.
        using D3D11Sampler = ComPtr<ID3D11SamplerState>;

        /// \brief Represents a Direct3D 11 texture.
        struct D3D11Texture final
        {
            /// The texture resource.
            ComPtr<ID3D11Texture2D>          Object;

            /// The shader resource view, empty when the texture is not sampled.
            ComPtr<ID3D11ShaderResourceView> Resource;

            /// The format the texture was created with.
            TextureFormat                    Format = TextureFormat::Unspecified;

            /// The number of mip levels.
            UINT                             Levels = 1;

            /// The number of array slices, six for a cube.
            UINT                             Layers = 1;
        };

        /// \brief Holds the swapchain settings the driver keeps across resets.
        struct D3D11Properties final
        {
            /// The configuration the swapchain was last built or resized with.
            Configuration Config { };

            /// `true` when the factory allows tearing presentation.
            Bool          Tearing = false;
        };

        /// \brief Represents the profiler's view of the device, and the one zone a pass is timed by.
        struct D3D11Profiler final
        {
#if defined(ZY_PROFILE_BACKEND_TRACY)
            /// The Tracy context the zones belong to.
            TracyD3D11Ctx                        Context = nullptr;

            /// The storage of the zone that times the open pass.
            ZY_ALIGN(tracy::D3D11ZoneScope) Byte Zone[sizeof(tracy::D3D11ZoneScope)];

            /// `true` while a zone is open.
            Bool                                 Opened  = false;
#endif

            /// \brief Creates the profiler context for the device.
            ///
            /// \param Device    The device the zones are timed on.
            /// \param Immediate The context the zones are recorded into.
            ZY_INLINE void Initialize(Ptr<ID3D11Device> Device, Ptr<ID3D11DeviceContext> Immediate)
            {
#if defined(ZY_PROFILE_BACKEND_TRACY)
                Context = TracyD3D11Context(Device, Immediate);
#endif
            }

            /// \brief Opens the zone a pass is timed by.
            ///
            /// \param Name The name the zone is shown with.
            ZY_INLINE void Open(Text Name)
            {
#if defined(ZY_PROFILE_BACKEND_TRACY)
                ZY_ASSERT(!Opened, "A render pass was prepared while another was still open");

                static constexpr Char kZone[] = "D3D11Driver::Prepare";

                Construct<tracy::D3D11ZoneScope>(
                    reinterpret_cast<Ptr<tracy::D3D11ZoneScope>>(Zone),
                    Context,
                    static_cast<UInt32>(__LINE__),
                    __FILE__,
                    sizeof(__FILE__) - 1,
                    kZone,
                    sizeof(kZone) - 1,
                    Name.GetData(),
                    Name.GetSize(),
                    true);
                Opened = true;
#endif
            }

            /// \brief Closes the zone opened by \ref Open, if any.
            ZY_INLINE void Close()
            {
#if defined(ZY_PROFILE_BACKEND_TRACY)
                if (Opened)
                {
                    Destruct(* reinterpret_cast<Ptr<tracy::D3D11ZoneScope>>(Zone));
                    Opened = false;
                }
#endif
            }

            /// \brief Gathers whatever the timing queries have answered.
            ZY_INLINE void Collect()
            {
#if defined(ZY_PROFILE_BACKEND_TRACY)
                TracyD3D11Collect(Context);
#endif
            }
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
        /// \param Config The size and formats of the swapchain.
        void CreateSwapchain(Ref<D3D11Pass> Pass, HWND Window, ConstRef<Configuration> Config);

        /// \brief Creates the render targets and depth resources for a swapchain-backed render pass.
        ///
        /// \param Pass   The render pass to create resources for.
        /// \param Config The size and formats of the swapchain.
        void CreateSwapchainResources(Ref<D3D11Pass> Pass, ConstRef<Configuration> Config) const;

        /// \brief Applies the vertex buffer bindings for a draw item.
        ///
        /// \param Oldest The previously submitted draw item.
        /// \param Newest The draw item being submitted.
        void ApplyVertexResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

        /// \brief Applies the sampler state resources for a draw item.
        ///
        /// \param Oldest The previously submitted draw item.
        /// \param Newest The draw item being submitted.
        void ApplySamplerResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

        /// \brief Applies the texture resources for a draw item.
        ///
        /// \param Oldest The previously submitted draw item.
        /// \param Newest The draw item being submitted.
        void ApplyTextureResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

        /// \brief Applies the uniform resources for a draw item.
        ///
        /// \param Oldest The previously submitted draw item.
        /// \param Newest The draw item being submitted.
        void ApplyUniformResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ComPtr<ID3D11Device1>                   mDevice;
        ComPtr<ID3D11DeviceContext1>            mDeviceImmediate;
        ComPtr<IDXGIFactory1>                   mDeviceFactory;
        D3D11Properties                         mDeviceProperties;
        D3D11Compiler                           mCompiler;
        Description                             mDescription;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        D3D11Profiler                           mProfiler;
        ComPtr<IDXGISwapChain>                  mSwapchain;
        Array<D3D11Buffer,   kMaxBuffers   + 1> mBuffers;
        Array<D3D11Pass,     kMaxPasses    + 1> mPasses;
        Array<D3D11Pipeline, kMaxPipelines + 1> mPipelines;
        Array<D3D11Sampler,  kMaxSamplers  + 1> mSamplers;
        Array<D3D11Texture,  kMaxTextures  + 1> mTextures;
    };
}