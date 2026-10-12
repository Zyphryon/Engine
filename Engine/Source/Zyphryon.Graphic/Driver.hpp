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

#include "Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    /// \brief Defines the interface for graphics drivers that manage rendering operations.
    class Driver
    {
    public:

        /// \brief Ensures derived class can be destroyed polymorphically.
        virtual ~Driver() = default;

        /// \brief Initializes the driver with the specified display and rendering parameters.
        ///
        /// \param Output The native window handle to present into.
        /// \param Config The surface size, formats and presentation mode to start with.
        /// \return `true` if initialization succeeded, `false` otherwise.
        virtual Bool Initialize(Ptr<void> Output, ConstRef<Configuration> Config)
        {
            return true;
        }

        /// \brief Resets the rendering surface to the specified resolution.
        ///
        /// \param Width    The new width of the rendering surface in pixels.
        /// \param Height   The new height of the rendering surface in pixels.
        /// \param Tearless `true` to wait for the vertical blank, `false` to present as soon as a frame is ready.
        virtual void Reset(UInt16 Width, UInt16 Height, Bool Tearless)
        {

        }

        /// \brief Queries the driver and fills the provided structure with device capabilities and adapter information.
        ///
        /// \param Output Receives the driver's device information and capabilities.
        virtual void Probe(Ref<Description> Output) const
        {

        }

        /// \brief Creates a buffer resource with the specified parameters and optional initial data.
        ///
        /// \param ID       The handle the service allocated for the buffer.
        /// \param Storage  The CPU access pattern of the buffer.
        /// \param Usage    The ways the buffer is bound, such as vertex or uniform.
        /// \param Capacity The capacity of the buffer in bytes, a multiple of the uniform block alignment for a uniform buffer.
        /// \param Data     The optional initial data, either empty or exactly \p Capacity bytes long.
        virtual void CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data)
        {

        }

        /// \brief Updates a region of an existing buffer resource with new data.
        ///
        /// \param ID     The identifier of the buffer resource to update.
        /// \param Offset The byte offset within the buffer to start updating.
        /// \param Data   The new data to write into the buffer.
        virtual void UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data)
        {

        }

        /// \brief Deletes a buffer resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the buffer resource to delete.
        virtual void DeleteBuffer(Object ID)
        {

        }

        /// \brief Maps a region of a buffer resource into CPU address space for direct access.
        ///
        /// \param ID     The identifier of the buffer resource to map.
        /// \param Offset The byte offset within the buffer to start mapping from.
        /// \param Size   The number of bytes to map.
        /// \return A pointer to the mapped region in CPU address space, or `nullptr` if mapping fails.
        virtual Ptr<Byte> MapBuffer(Object ID, UInt32 Offset, UInt32 Size)
        {
            return nullptr;
        }

        /// \brief Unmaps a previously mapped buffer resource, releasing CPU access.
        ///
        /// \param ID   The identifier of the buffer resource to unmap.
        /// \param Size The number of bytes written from the start of the mapped region.
        virtual void UnmapBuffer(Object ID, UInt32 Size)
        {

        }

        /// \brief Creates a render pass resource with the specified color and depth attachments.
        ///
        /// \param ID     The handle the service allocated for the render pass.
        /// \param Colors The color attachments for the render pass.
        /// \param Depth  The depth/stencil attachment for the render pass.
        virtual void CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth)
        {

        }

        /// \brief Deletes a render pass resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the render pass resource to delete.
        virtual void DeletePass(Object ID)
        {

        }

        /// \brief Creates a pipeline resource with the specified shader program, resource interface, and states.
        ///
        /// \param ID        The handle the service allocated for the pipeline.
        /// \param Program   The shader program to use for the pipeline.
        /// \param Signature The vertex attributes the program's input stage consumes.
        /// \param States    The fixed-function pipeline states to configure.
        virtual void CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States)
        {

        }

        /// \brief Deletes a pipeline resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the pipeline resource to delete.
        virtual void DeletePipeline(Object ID)
        {

        }

        /// \brief Creates a sampler resource with the specified filtering and addressing state.
        ///
        /// \param ID         The handle the service allocated for the sampler.
        /// \param Descriptor The filtering, addressing, and comparison state to create.
        virtual void CreateSampler(Object ID, Sampler Descriptor)
        {

        }

        /// \brief Deletes a sampler resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the sampler resource to delete.
        virtual void DeleteSampler(Object ID)
        {

        }

        /// \brief Creates a texture resource with the specified parameters and optional initial data.
        ///
        /// \param ID      The handle the service allocated for the texture.
        /// \param Layout  The shape of the texture, as 2D, array or cube.
        /// \param Format  The pixel format of the texture.
        /// \param Storage The CPU access pattern of the texture.
        /// \param Usage   The ways the texture is bound, such as sample or target.
        /// \param Width   The width of the texture in pixels.
        /// \param Height  The height of the texture in pixels.
        /// \param Layers  The number of array slices.
        /// \param Levels  The number of mipmap levels.
        /// \param Data    The optional initial image data to populate the texture with, ordered slice-major.
        virtual void CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data)
        {

        }

        /// \brief Updates a region of an existing texture resource with new data.
        ///
        /// \param ID     The identifier of the texture resource to update.
        /// \param Level  The mipmap level to update.
        /// \param Layer  The array slice to update.
        /// \param X      The X offset within the texture to start updating.
        /// \param Y      The Y offset within the texture to start updating.
        /// \param Width  The width of the region to update in pixels.
        /// \param Height The height of the region to update in pixels.
        /// \param Pitch  The number of bytes per row of the source data, including any padding.
        /// \param Data   The new texture data to write into the specified region.
        virtual void UpdateTexture(Object ID, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
        {

        }

        /// \brief Deletes a texture resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the texture resource to delete.
        virtual void DeleteTexture(Object ID)
        {

        }

        /// \brief Prepares the specified render pass for rendering by setting the viewport and clearing attachments.
        ///
        /// \param Pass     The render pass to prepare.
        /// \param Name     The name a profiler and a frame capture show the pass under.
        /// \param Viewport The viewport defining the rendering area.
        /// \param Colors   The clear colors for each color attachment.
        /// \param Depth    The depth clear value.
        /// \param Stencil  The stencil clear value.
        virtual void Prepare(Object Pass, Text Name, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
        {

        }

        /// \brief Submits a list of draw commands to the currently prepared render pass for execution.
        ///
        /// \param Submission The list of draw commands to submit.
        virtual void Submit(ConstSpan<Command> Submission)
        {

        }

        /// \brief Commits all pending rendering commands for the given render pass.
        ///
        /// \param Pass The render pass to commit.
        virtual void Commit(Object Pass)
        {

        }
    };
}