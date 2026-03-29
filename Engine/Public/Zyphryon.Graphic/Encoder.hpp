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

#include "Material.hpp"
#include "Pipeline.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Encapsulate draw submission commands to be issued to the GPU.
    class Encoder final
    {
    public:

        /// \brief Default capacity for pre allocated submission storage.
        static constexpr UInt32 kDefaultCapacity = 128;

    public:

        /// \brief Constructs an encoder with optional submission capacity.
        /// 
        /// \param Capacity Number of submissions to reserve space for.
        ZYPHRYON_INLINE explicit Encoder(UInt32 Capacity = kDefaultCapacity)
        {
            mInFlightSubmission.reserve(Capacity);
        }

        /// \brief Clears all recorded submissions and resets internal state.
        ZYPHRYON_INLINE void Clear()
        {
            mInFlightSubmission.clear();
            mInFlightCommand = DrawPacket();
        }

        /// \brief Sets the indices stream to use for this submission.
        /// 
        /// \param Stream Stream containing topology data.
        ZYPHRYON_INLINE void SetIndices(ConstRef<Stream> Stream)
        {
            mInFlightCommand.Indices = Stream;
        }

        /// \brief Sets the indices stream using buffer parameters.
        /// 
        /// \param Buffer GPU buffer handle.
        /// \param Offset Byte offset into the buffer.
        /// \param Stride Stride of each element in the buffer.
        ZYPHRYON_INLINE void SetIndices(Object Buffer, UInt32 Offset, UInt32 Stride)
        {
            SetIndices(Stream(Buffer, Stride, Offset));
        }

        /// \brief Sets a vertices stream at the specified register.
        /// 
        /// \param Register Slot index in the shader.
        /// \param Stream   Vertex buffer stream to bind.
        ZYPHRYON_INLINE void SetVertices(UInt32 Register, ConstRef<Stream> Stream)
        {
            mInFlightCommand.Vertices[Register] = Stream;
        }

        /// \brief Sets a vertices stream using buffer parameters.
        /// 
        /// \param Register Slot index in the shader.
        /// \param Buffer   GPU buffer handle.
        /// \param Offset   Byte offset into the buffer.
        /// \param Stride   Stride of each element in the buffer.
        ZYPHRYON_INLINE void SetVertices(UInt32 Register, Object Buffer, UInt32 Offset, UInt32 Stride)
        {
            SetVertices(Register, Stream(Buffer, Stride, Offset));
        }

        /// \brief Sets a uniform buffer at the specified register.
        /// 
        /// \param Register Slot index in the shader.
        /// \param Stream   Uniform buffer stream to bind.
        ZYPHRYON_INLINE void SetUniform(UInt32 Register, ConstRef<Stream> Stream)
        {
            mInFlightCommand.Uniforms[Register] = Stream;
        }

        /// \brief Sets a uniform buffer using buffer parameters.
        /// 
        /// \param Register Slot index in the shader.
        /// \param Buffer   GPU buffer handle.
        /// \param Offset   Byte offset into the buffer.
        /// \param Stride   Stride of each element in the buffer.
        ZYPHRYON_INLINE void SetUniform(UInt32 Register, Object Buffer, UInt32 Offset, UInt32 Stride)
        {
            SetUniform(Register, Stream(Buffer, Stride, Offset));
        }

        /// \brief Sets the scissor rectangle used for rendering.
        /// 
        /// \param Scissor Scissor region in screen space.
        ZYPHRYON_INLINE void SetScissor(ConstRef<Scissor> Scissor)
        {
            mInFlightCommand.Scissor = Scissor;
        }

        /// \brief Sets the stencil reference value for stencil testing.
        /// 
        /// \param Stencil Reference value to compare against.
        ZYPHRYON_INLINE void SetStencil(UInt8 Stencil)
        {
            mInFlightCommand.Stencil = Stencil;
        }

        /// \brief Sets the pipeline to use for the current submission.
        /// 
        /// \param Pipeline Handle to a compiled graphics pipeline object.
        ZYPHRYON_INLINE void SetPipeline(Object Pipeline)
        {
            mInFlightCommand.Pipeline = Pipeline;
        }

        /// \brief Sets the pipeline to use for the current submission.
        /// 
        /// \param Pipeline Pipeline object wrapper.
        ZYPHRYON_INLINE void SetPipeline(ConstRef<Pipeline> Pipeline)
        {
            SetPipeline(Pipeline.GetID());
        }

        /// \brief Sets a texture at the specified register.
        /// 
        /// \param Register The slot index in the shader.
        /// \param Texture  The texture handle to bind.
        /// \param Sampler  The sampler handle to bind.
        ZYPHRYON_INLINE void SetTexture(UInt32 Register, Object Texture, Sampler Sampler)
        {
            mInFlightCommand.Textures.emplace_back(Entry(Register, Texture));
            mInFlightCommand.Samplers.emplace_back(Entry(Register, Sampler));
        }

        /// \brief Sets a texture at the specified register.
        /// 
        /// \param Register The slot index in the shader.
        /// \param Texture  The texture object wrapper.
        /// \param Sampler  The sampler handle to bind.
        ZYPHRYON_INLINE void SetTexture(UInt32 Register, ConstRef<Texture> Texture, Sampler Sampler)
        {
            SetTexture(Register, Texture.GetID(), Sampler);
        }

        /// \brief Binds textures and samplers from the material using pipeline-defined semantic mappings.
        /// 
        /// \param Pipeline Pipeline that defines expected texture semantics and slots.
        /// \param Material Material providing resources for those semantics.
        ZYPHRYON_INLINE void Bind(ConstRef<Pipeline> Pipeline, ConstRef<Material> Material)
        {
            for (ConstRef<Entry<TextureSemantic>> Binding : Pipeline.GetTextures())
            {
                if (ConstTracker<Texture> Texture = Material.GetTexture(Binding.Resource))
                {
                    SetTexture(Binding.Register, Texture->GetID(), Material.GetSampler(Binding.Resource));
                }
            }
        }

        /// \brief Finalizes the current draw command and adds it to the submission list.
        ///
        /// \param Count     Number of indices or vertices to draw.
        /// \param Base      Base vertex index (or first vertex for non-indexed).
        /// \param Offset    Offset into index buffer or vertex buffer.
        /// \param Instances Number of instances to draw (defaults to 1).
        ZYPHRYON_INLINE void Draw(UInt32 Count, UInt32 Base, UInt32 Offset, UInt32 Instances = 1)
        {
            Ref<DrawCommand> Parameters = mInFlightCommand.Command;
            Parameters.Count     = Count;
            Parameters.Base      = Base;
            Parameters.Offset    = Offset;
            Parameters.Instances = Instances;

            mInFlightSubmission.push_back(mInFlightCommand);
        }

        /// \brief Resets the current in-flight command without clearing recorded submissions.
        ZYPHRYON_INLINE void Reset()
        {
            mInFlightCommand = DrawPacket();
        }

        /// \brief Resets texture and sampler bindings for the current in-flight command.
        ZYPHRYON_INLINE void ResetBindings()
        {
            mInFlightCommand.Textures.clear();
            mInFlightCommand.Samplers.clear();
        }

        /// \brief Returns the list of all recorded submissions.
        /// 
        /// \return Span to the list of finalized submissions.
        ZYPHRYON_INLINE ConstSpan<DrawPacket> GetSubmissions() const
        {
            return mInFlightSubmission;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector<DrawPacket> mInFlightSubmission;
        DrawPacket         mInFlightCommand;
    };
}