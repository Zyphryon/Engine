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

#include "OGLBuffer.hpp"
#include "OGLPass.hpp"
#include "OGLPipeline.hpp"
#include "OGLSampler.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Represents what is bound to the context, so a draw only binds what differs from the last one.
    class Binder final
    {
    public:

        /// \brief Constructs the binder over the driver's tables.
        ///
        /// \param Device    The device of the context, read once it is current.
        /// \param Buffers   The driver's buffers, indexed by the draws' streams.
        /// \param Pipelines The driver's pipelines, indexed by the draws' pipelines.
        /// \param Samplers  The driver's sampler objects, indexed by the draws' samplers.
        /// \param Textures  The driver's textures, indexed by the draws' textures.
        Binder(
            ConstRef<Device>    Device,
            ConstSpan<Buffer>   Buffers,
            ConstSpan<Pipeline> Pipelines,
            ConstSpan<Sampler>  Samplers,
            ConstSpan<Texture>  Textures);

        /// \brief Begins a pass.
        ///
        /// \param Pass The pass being recorded, whose height the window's scissor is flipped against.
        void Prepare(ConstRef<Pass> Pass);

        /// \brief Puts the context in the state of an empty draw, which the next draw is then compared against.
        void Invalidate();

        /// \brief Binds what a draw needs and differs from the last draw bound.
        ///
        /// \param Draw The draw about to be issued.
        void Apply(ConstRef<Command> Draw);

    private:

        /// \brief Binds the program and the fixed-function state of a draw's pipeline, with its stencil reference.
        ///
        /// \param Draw The draw about to be issued.
        void ApplyPipeline(ConstRef<Command> Draw);

        /// \brief Binds the scissor rectangle.
        ///
        /// \param Scissor The rectangle, with its origin at the top-left corner.
        void ApplyScissor(ConstRef<Scissor> Scissor);

        /// \brief Binds the vertex streams and points the pipeline's attributes at them.
        ///
        /// \note Without \ref Feature::BaseVertex an indexed draw's base vertex is folded into the per-vertex streams' offsets.
        ///
        /// \param Draw The draw about to be issued.
        void ApplyVertices(ConstRef<Command> Draw);

        /// \brief Binds the index stream.
        ///
        /// \param Indices The stream the indices are read from.
        void ApplyIndices(ConstRef<Stream> Indices);

        /// \brief Binds the uniform buffer ranges.
        ///
        /// \param Uniforms The range of each uniform slot.
        void ApplyUniforms(ConstSpan<Stream> Uniforms);

        /// \brief Binds the samplers.
        ///
        /// \param Samplers The sampler of each texture unit.
        void ApplySamplers(ConstSpan<Object> Samplers);

        /// \brief Binds the textures.
        ///
        /// \param Textures The texture of each texture unit.
        void ApplyTextures(ConstSpan<Object> Textures);

        /// \brief Sets the scissor rectangle, flipped against the window's height in the display pass.
        ///
        /// \param Scissor The rectangle, with its origin at the top-left corner.
        void Clip(ConstRef<Scissor> Scissor) const;

        /// \brief Gets the base vertex a draw folds into its per-vertex streams, when the context cannot apply it itself.
        ///
        /// \param Draw The draw whose base is folded.
        /// \return The base vertex of an indexed draw without \ref Feature::BaseVertex, otherwise `0`.
        SInt32 Fold(ConstRef<Command> Draw) const;

        /// \brief Sets the blend switch, write mask, factors and equations that differ from what is bound.
        ///
        /// \param Blend The blend state to bind.
        /// \param Valid `true` when what is bound may be trusted, `false` to set everything.
        void ApplyBlend(ConstRef<Blend> Blend, Bool Valid);

        /// \brief Sets the depth test, writes, comparison, clamping and bias.
        ///
        /// \param Depth The depth state to bind.
        void ApplyDepth(ConstRef<Depth> Depth) const;

        /// \brief Sets face culling, the scissor test and the polygon mode.
        ///
        /// \param Raster The rasterizer state to bind.
        void ApplyRaster(ConstRef<Raster> Raster) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstRef<Device>    mDevice;
        Command             mCommand;
        ConstSpan<Buffer>   mBuffers;
        ConstSpan<Pipeline> mPipelines;
        ConstSpan<Sampler>  mSamplers;
        ConstSpan<Texture>  mTextures;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstPtr<Pass>      mPass;
        UInt32              mAttributes;
        UInt32              mUnit;
        Blend               mBlend;
    };
}