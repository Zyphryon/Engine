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

#include "D3D11Buffer.hpp"
#include "D3D11Pipeline.hpp"
#include "D3D11Sampler.hpp"
#include "D3D11Texture.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    /// \brief Represents what is bound to the immediate context, so a draw only binds what differs from the last one.
    class Binder final
    {
    public:

        /// \brief Constructs a binder over the driver's tables, comparing the first draw against an empty one.
        ///
        /// \param Device    The device whose immediate context the state is bound on.
        /// \param Buffers   The buffer table the draws' streams index.
        /// \param Pipelines The pipeline table the draws index.
        /// \param Samplers  The sampler table the draws' samplers index.
        /// \param Textures  The texture table the draws' textures index.
        Binder(ConstRef<Device> Device, ConstSpan<Buffer> Buffers, ConstSpan<Pipeline> Pipelines, ConstSpan<Sampler> Samplers, ConstSpan<Texture> Textures);

        /// \brief Forgets the last draw, so the next one is compared against an empty one.
        void Invalidate();

        /// \brief Binds whatever a draw needs that differs from the last draw, clearing the slots it leaves empty.
        ///
        /// \param Draw The draw being bound, whose pipeline is ready.
        void Apply(ConstRef<Command> Draw);

    private:

        /// \brief Binds the shaders and fixed-function states of the draw's pipeline, with its stencil reference.
        ///
        /// \param Draw The draw being bound.
        void ApplyPipeline(ConstRef<Command> Draw);

        /// \brief Binds the scissor rectangle.
        ///
        /// \param Scissor The scissor rectangle to bind.
        void ApplyScissor(ConstRef<Scissor> Scissor);

        /// \brief Binds the vertex buffers, clearing the slots past the last stream.
        ///
        /// \param Vertices The vertex streams to bind.
        void ApplyVertices(ConstSpan<Stream> Vertices);

        /// \brief Binds the index buffer, or none when the stream names no buffer.
        ///
        /// \param Indices The index stream to bind.
        void ApplyIndices(ConstRef<Stream> Indices);

        /// \brief Binds the uniform blocks a stage reads, clearing the slots it does not.
        ///
        /// \param Stage The stage whose slots are bound.
        /// \param Draw  The draw being bound.
        void ApplyUniforms(ShaderStage Stage, ConstRef<Command> Draw);

        /// \brief Binds the samplers a stage reads, clearing the slots it does not.
        ///
        /// \param Stage The stage whose slots are bound.
        /// \param Draw  The draw being bound.
        void ApplySamplers(ShaderStage Stage, ConstRef<Command> Draw);

        /// \brief Binds the textures a stage reads, clearing the slots it does not.
        ///
        /// \param Stage The stage whose slots are bound.
        /// \param Draw  The draw being bound.
        void ApplyTextures(ShaderStage Stage, ConstRef<Command> Draw);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstRef<Device>    mDevice;
        Command             mCommand;
        ConstSpan<Buffer>   mBuffers;
        ConstSpan<Pipeline> mPipelines;
        ConstSpan<Sampler>  mSamplers;
        ConstSpan<Texture>  mTextures;
    };
}