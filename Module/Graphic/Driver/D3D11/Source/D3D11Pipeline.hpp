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

#include "D3D11Compiler.hpp"
#include "D3D11Device.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    /// \brief Represents a Direct3D 11 pipeline, its shaders plus fixed-function state.
    class Pipeline final
    {
    public:

        /// \brief Describes the registers one shader stage reads, one bit per slot.
        struct Usage final
        {
            /// The uniform block registers the stage reads.
            UInt32 Uniforms = 0;

            /// The sampler registers the stage reads.
            UInt32 Samplers = 0;

            /// The texture registers the stage reads.
            UInt32 Textures = 0;
        };

    public:

        /// \brief Constructs an empty pipeline that holds no shaders until \ref Create.
        Pipeline();

        /// \brief Waits for a compile still running, since it reads what the pipeline handed it.
        ~Pipeline();

        /// \brief Starts building the pipeline, its stages compiling on the system's thread pool meanwhile.
        ///
        /// \param Device    The device whose tier picks the shader model.
        /// \param Compiler  The compiler the program's modules are compiled with.
        /// \param Program   The shader program to compile.
        /// \param Signature The vertex attributes the program's input stage consumes.
        /// \param States    The fixed-function states to create.
        void Create(ConstRef<Device> Device, ConstRef<Compiler> Compiler, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States);

        /// \brief Waits for a compile still running, then releases the shaders and states.
        void Delete();

        /// \brief Checks whether a draw through the pipeline has shaders to run, creating them once its compile is done.
        ///
        /// \param Device   The device the shaders and states are created on.
        /// \param Compiler The compiler the stages' registers are reflected with.
        /// \return `true` if the pipeline holds its shaders, `false` while it compiles or once it failed.
        Bool Resolve(ConstRef<Device> Device, ConstRef<Compiler> Compiler);

        /// \brief Gets the vertex shader.
        ///
        /// \return The vertex shader, or `nullptr` when the pipeline failed to build.
        ZY_INLINE Ptr<ID3D11VertexShader> GetVertex() const
        {
            return mVertex.Get();
        }

        /// \brief Gets the pixel shader.
        ///
        /// \return The pixel shader.
        ZY_INLINE Ptr<ID3D11PixelShader> GetFragment() const
        {
            return mFragment.Get();
        }

        /// \brief Gets the blend state, with the channels written.
        ///
        /// \return The blend state.
        ZY_INLINE Ptr<ID3D11BlendState> GetBlend() const
        {
            return mBlend.Get();
        }

        /// \brief Gets the depth-stencil state.
        ///
        /// \return The depth-stencil state.
        ZY_INLINE Ptr<ID3D11DepthStencilState> GetDepth() const
        {
            return mDepth.Get();
        }

        /// \brief Gets the rasterizer state.
        ///
        /// \return The rasterizer state.
        ZY_INLINE Ptr<ID3D11RasterizerState> GetRaster() const
        {
            return mRaster.Get();
        }

        /// \brief Gets the input layout.
        ///
        /// \return The input layout, or `nullptr` when the pipeline reads no vertex attributes.
        ZY_INLINE Ptr<ID3D11InputLayout> GetLayout() const
        {
            return mLayout.Get();
        }

        /// \brief Gets the primitive topology.
        ///
        /// \return The topology the pipeline's draws assemble.
        ZY_INLINE D3D11_PRIMITIVE_TOPOLOGY GetTopology() const
        {
            return mTopology;
        }

        /// \brief Gets the registers a stage reads, so a draw binds the stage only what it reads.
        ///
        /// \param Stage The stage whose registers are returned.
        /// \return The registers the stage reads, one bit per slot.
        ZY_INLINE ConstRef<Usage> GetUsage(ShaderStage Stage) const
        {
            return mUsages[ZyEnum::Cast(Stage)];
        }

    private:

        /// \brief Holds what a compile reads and writes, owned apart from the pipeline while the pool runs it.
        struct Build final
        {
            /// The compiler the stages are compiled with.
            ConstPtr<Compiler>                                  Compiler = nullptr;

            /// The feature tier, which picks the shader model.
            ZyGraphic::Tier                                     Tier     = ZyGraphic::Tier::Level3;

            /// The source of each stage, copied so the compile owns what it reads.
            Array<Sequence<Byte>, ZyEnum::Count<ShaderStage>()> Sources;

            /// The macros the program defines.
            Sequence<Macro>                                     Macros;

            /// The vertex attributes the program's input stage consumes.
            ZyGraphic::Signature                                Signature;

            /// The fixed-function states to create.
            ZyGraphic::States                                   States;

            /// The vertex stage's bytecode, or `nullptr` when it failed.
            ComPtr<ID3DBlob>                                    Vertex;

            /// The fragment stage's bytecode, or `nullptr` when it failed.
            ComPtr<ID3DBlob>                                    Fragment;

            /// `true` once the compile finished, whether it succeeded or not.
            Atomic<Bool>                                        Done     = false;
        };

        /// \brief Compiles a build's stages on a thread of the system's pool.
        ///
        /// \param Instance The callback instance the pool runs it in.
        /// \param Context  The build to compile.
        static void CALLBACK OnCompile(PTP_CALLBACK_INSTANCE Instance, PVOID Context);

        /// \brief Creates the shaders and states from a finished build, stopping at the first that fails.
        ///
        /// \param Device   The device the shaders and states are created on.
        /// \param Compiler The compiler the stages' registers are reflected with.
        /// \param Source   The finished build.
        /// \return `true` if every shader and state was created, `false` otherwise.
        Bool Assemble(ConstRef<Device> Device, ConstRef<Compiler> Compiler, ConstRef<Build> Source);

        /// \brief Waits until the pool is done with the build, which a compile cannot be called off from.
        void Await() const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Unique<Build>                              mBuild;
        ComPtr<ID3D11VertexShader>                 mVertex;
        ComPtr<ID3D11PixelShader>                  mFragment;
        ComPtr<ID3D11BlendState>                   mBlend;
        ComPtr<ID3D11DepthStencilState>            mDepth;
        ComPtr<ID3D11RasterizerState>              mRaster;
        ComPtr<ID3D11InputLayout>                  mLayout;
        D3D11_PRIMITIVE_TOPOLOGY                   mTopology;
        Array<Usage, ZyEnum::Count<ShaderStage>()> mUsages;
    };
}