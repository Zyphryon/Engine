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
    /// \brief Encapsulates draw submission commands to be issued to the GPU.
    class Encoder final
    {
    public:

        /// \brief Default capacity for pre-allocated submission storage.
        static constexpr UInt32 kDefaultCapacity = 128;

    public:

        /// \brief Constructs an encoder with the specified submission storage capacity.
        ///
        /// \param Capacity The initial reserved capacity for draw submissions.
        ZYPHRYON_INLINE explicit Encoder(UInt32 Capacity = kDefaultCapacity)
        {
            mInFlightSubmission.reserve(Capacity);
        }

        /// \brief Clears all recorded submissions and resets internal state.
        ZYPHRYON_INLINE void Clear()
        {
            mInFlightSubmission.clear();
            mInFlightCommand = DrawItem();
        }

        /// \brief Sets the index buffer stream for indexed draw calls.
        ///
        /// \param Stream The stream containing the index buffer, offset, and stride information.
        ZYPHRYON_INLINE void SetIndices(Stream Stream)
        {
            mInFlightCommand.Indices = Stream;
        }

        /// \brief Sets the index buffer stream for indexed draw calls.
        ///
        /// \param Buffer The buffer object containing the index data.
        /// \param Stride The byte stride between consecutive indices in the buffer.
        /// \param Offset The byte offset into the buffer where index data begins.
        ZYPHRYON_INLINE void SetIndices(Object Buffer, UInt16 Stride, UInt32 Offset)
        {
            SetIndices(Stream(Buffer, Stride, Offset));
        }

        /// \brief Sets a vertex buffer stream for a specific shader register.
        ///
        /// \param Register The shader register index to bind the vertex stream to.
        /// \param Stream   The stream containing the vertex buffer, offset, and stride information.
        ZYPHRYON_INLINE void SetVertices(UInt32 Register, Stream Stream)
        {
            mInFlightCommand.Vertices[Register] = Stream;
        }

        /// \brief Sets a vertex buffer stream for a specific shader register.
        ///
        /// \param Register The shader register index to bind the vertex stream to.
        /// \param Buffer   The buffer object containing the vertex data.
        /// \param Stride   The byte stride between consecutive vertices in the buffer.
        /// \param Offset   The byte offset into the buffer where vertex data begins.
        ZYPHRYON_INLINE void SetVertices(UInt32 Register, Object Buffer, UInt16 Stride, UInt32 Offset)
        {
            SetVertices(Register, Stream(Buffer, Stride, Offset));
        }

        /// \brief Sets a uniform buffer stream for a specific shader register.
        ///
        /// \param Register The shader register index to bind the uniform stream to.
        /// \param Stream   The stream containing the uniform buffer, offset, and stride information.
        ZYPHRYON_INLINE void SetUniform(UInt32 Register, Stream Stream)
        {
            mInFlightCommand.Uniforms[Register] = Stream;
        }

        /// \brief Sets a uniform buffer stream for a specific shader register.
        ///
        /// \param Register The shader register index to bind the uniform stream to.
        /// \param Buffer   The buffer object containing the uniform data.
        /// \param Stride   The byte stride between consecutive uniform elements in the buffer.
        /// \param Offset   The byte offset into the buffer where uniform data begins.
        ZYPHRYON_INLINE void SetUniform(UInt32 Register, Object Buffer, UInt16 Stride, UInt32 Offset)
        {
            SetUniform(Register, Stream(Buffer, Stride, Offset));
        }

        /// \brief Sets the scissor rectangle for the draw call, defining the pixel area affected by rendering.
        ///
        /// \param Scissor The scissor rectangle to set for the draw call.
        ZYPHRYON_INLINE void SetScissor(Scissor Scissor)
        {
            mInFlightCommand.Scissor = Scissor;
        }

        /// \brief Sets the stencil reference value for stencil testing and operations during the draw call.
        ///
        /// \param Stencil The stencil reference value to set for the draw call.
        ZYPHRYON_INLINE void SetStencil(UInt8 Stencil)
        {
            mInFlightCommand.Stencil = Stencil;
        }

        /// \brief Sets the pipeline state object for the draw call, defining fixed-function state and shader program.
        ///
        /// \param Pipeline The pipeline object to set for the draw call.
        ZYPHRYON_INLINE void SetPipeline(Object Pipeline)
        {
            mInFlightCommand.Pipeline = Pipeline;
        }

        /// \brief Sets the pipeline state object for the draw call, defining fixed-function state and shader program.
        ///
        /// \param Pipeline The pipeline resource whose GPU ID will be bound for the draw call.
        ZYPHRYON_INLINE void SetPipeline(ConstRef<Pipeline> Pipeline)
        {
            SetPipeline(Pipeline.GetID());
        }

        /// \brief Binds a texture and sampler to a specific shader register for the draw call.
        ///
        /// \param Register The shader register index to bind the texture and sampler to.
        /// \param Texture  The texture object to bind to the specified register.
        /// \param Sampler  The sampler state to use when sampling the bound texture in the shader.
        ZYPHRYON_INLINE void SetTexture(UInt32 Register, Object Texture, Sampler Sampler)
        {
            mInFlightCommand.Textures.emplace_back(Entry(Register, Texture));
            mInFlightCommand.Samplers.emplace_back(Entry(Register, Sampler));
        }

        /// \brief Binds a texture and sampler to a specific shader register for the draw call.
        ///
        /// \param Register The shader register index to bind the texture and sampler to.
        /// \param Texture  The texture object to bind to the specified register.
        /// \param Sampler  The sampler state to use when sampling the bound texture in the shader.
        ZYPHRYON_INLINE void SetTexture(UInt32 Register, ConstRef<Texture> Texture, Sampler Sampler)
        {
            SetTexture(Register, Texture.GetID(), Sampler);
        }

        /// \brief Binds textures and samplers from the material using pipeline-defined semantic mappings.
        ///
        /// \param Pipeline The pipeline that defines expected texture semantics and register slots.
        /// \param Material The material providing texture and sampler resources for those semantics.
        ZYPHRYON_INLINE void Bind(ConstRef<Pipeline> Pipeline, ConstRef<Material> Material)
        {
            for (ConstRef<Entry<TextureSemantic>> Binding : Pipeline.GetTextures())
            {
                if (ConstTracker<Texture> Texture = Material.GetTexture(Binding.Resource))
                {
                    SetTexture(Binding.Register, Texture->GetID(), Material.GetSampler(Binding.Resource));
                }
                else
                {
                    SetTexture(Binding.Register, 0u, Sampler());
                }
            }
        }

        /// \brief Finalizes the current draw command and adds it to the submission list.
        ///
        /// \param Count     The number of indices or vertices to draw.
        /// \param Base      The base vertex index (or first vertex for non-indexed draws).
        /// \param Offset    The byte offset into the index buffer or vertex buffer.
        /// \param Instances The number of instances to draw (defaults to 1).
        ZYPHRYON_INLINE void Draw(UInt32 Count, SInt32 Base, UInt32 Offset, UInt32 Instances = 1)
        {
            mInFlightCommand.Parameters = DrawCall(Count, Base, Offset, Instances);
            mInFlightSubmission.push_back(mInFlightCommand);
        }

        /// \brief Resets the current in-flight command to default state for the next submission.
        ZYPHRYON_INLINE void Reset()
        {
            mInFlightCommand = DrawItem();
        }

        /// \brief Clears texture and sampler bindings from the current in-flight command.
        ZYPHRYON_INLINE void ResetBindings()
        {
            mInFlightCommand.Textures.clear();
            mInFlightCommand.Samplers.clear();
        }

        /// \brief Gets a read-only view of the list of recorded draw submissions.
        ///
        /// \return A constant span containing the recorded draw items ready for submission to the GPU.
        ZYPHRYON_INLINE ConstSpan<DrawItem> GetSubmissions() const
        {
            return mInFlightSubmission;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector<DrawItem> mInFlightSubmission;
        DrawItem         mInFlightCommand;
    };
}