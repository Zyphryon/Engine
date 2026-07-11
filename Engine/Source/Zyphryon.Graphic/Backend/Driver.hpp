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

#include "Common.hpp"
#include "Zyphryon.Math/Color.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Interface for graphics drivers that manage rendering operations.
    class Driver
    {
    public:

        /// \brief Ensures derived class can be destroyed polymorphically.
        virtual ~Driver() = default;

        /// \brief Initializes the driver with the specified display and rendering parameters.
        ///
        /// \param Output The output target to render into.
        /// \param Config The configuration settings for the graphics device.
        /// \return `true` if initialization succeeded, `false` otherwise.
        virtual Bool Initialize(Ptr<void> Output, ConstRef<Config> Config)
        {
            return true;
        }

        /// \brief Resets the rendering surface to the specified resolution.
        ///
        /// \param Width  The new width of the rendering surface in pixels.
        /// \param Height The new height of the rendering surface in pixels.
        virtual void Reset(UInt16 Width, UInt16 Height)
        {

        }

        /// \brief Queries the driver and fills the provided structure with device capabilities and adapter information.
        ///
        /// \param Output The structure to populate with the driver's current device information and capabilities.
        virtual void Probe(Ref<Description> Output) const
        {

        }

        /// \brief Creates a buffer resource with the specified parameters and optional initial data.
        ///
        /// \param ID       The identifier for the buffer resource.
        /// \param Storage  The storage type for the buffer.
        /// \param Usage    The intended usage of the buffer.
        /// \param Capacity The capacity of the buffer in bytes.
        /// \param Data     The optional initial data to populate the buffer with.
        virtual void CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data)
        {

        }

        /// \brief Updates a region of an existing buffer resource with new data.
        ///
        /// \param ID     The identifier of the buffer resource to update.
        /// \param Offset The byte offset within the buffer to start updating.
        /// \param Data   Th e new data to write into the buffer.
        virtual void UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data)
        {

        }

        /// \brief Deletes a buffer resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the buffer resource to delete.
        virtual void DeleteBuffer(Object ID)
        {

        }

        /// \brief Copies a region of one buffer resource to another.
        ///
        /// \param SrcBuffer The identifier of the source buffer to copy from.
        /// \param SrcOffset The byte offset within the source buffer to start copying from.
        /// \param DstBuffer The identifier of the destination buffer to copy to.
        /// \param DstOffset The byte offset within the destination buffer to start copying to.
        /// \param Size      The number of bytes to copy.
        virtual void CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, UInt32 Size)
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
        /// \param ID The identifier of the buffer resource to unmap.
        virtual void UnmapBuffer(Object ID)
        {

        }

        /// \brief Creates a render pass resource with the specified color and depth attachments.
        ///
        /// \param ID     The identifier for the render pass resource.
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

        /// \brief Creates a pipeline resource with the specified shader program and pipeline states.
        ///
        /// \param ID      The identifier for the pipeline resource.
        /// \param Program The shader program to use for the pipeline.
        /// \param States  The fixed-function pipeline states to configure.
        virtual void CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<States> States)
        {

        }

        /// \brief Deletes a pipeline resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the pipeline resource to delete.
        virtual void DeletePipeline(Object ID)
        {

        }

        /// \brief Creates a texture resource with the specified parameters and optional initial data.
        ///
        /// \param ID      The identifier for the texture resource.
        /// \param Layout  The layout of the texture.
        /// \param Format  The pixel format of the texture.
        /// \param Storage The storage type for the texture.
        /// \param Usage   The intended usage of the texture.
        /// \param Width   The width of the texture in pixels.
        /// \param Height  The height of the texture in pixels.
        /// \param Mipmaps The number of mipmap levels.
        /// \param Samples The multisample count.
        /// \param Data    The optional initial image data to populate the texture with.
        virtual void CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, ConstSpan<Byte> Data)
        {

        }

        /// \brief Updates a region of an existing texture resource with new data.
        ///
        /// \param ID     The identifier of the texture resource to update.
        /// \param Level  The mipmap level to update.
        /// \param X      The X offset within the texture to start updating.
        /// \param Y      The Y offset within the texture to start updating.
        /// \param Width  The width of the region to update in pixels.
        /// \param Height The height of the region to update in pixels.
        /// \param Pitch  The number of bytes per row of the source data, including any padding.
        /// \param Data   The new texture data to write into the specified region.
        virtual void UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
        {

        }

        /// \brief Deletes a texture resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the texture resource to delete.
        virtual void DeleteTexture(Object ID)
        {

        }

        /// \brief Copies a region of one texture resource to another.
        ///
        /// \param SrcTexture The identifier of the source texture to copy from.
        /// \param SrcLevel   The mipmap level of the source texture to copy from.
        /// \param SrcX       The X offset within the source texture to start copying from.
        /// \param SrcY       The Y offset within the source texture to start copying from.
        /// \param DstTexture The identifier of the destination texture to copy to.
        /// \param DstLevel   The mipmap level of the destination texture to copy to.
        /// \param DstX       The X offset within the destination texture to start copying to.
        /// \param DstY       The Y offset within the destination texture to start copying to.
        /// \param Width      The width of the region to copy in pixels.
        /// \param Height     The height of the region to copy in pixels.
        virtual void CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, UInt16 Width, UInt16 Height)
        {

        }

        /// \brief Prepares the specified render pass for rendering by setting the viewport and clearing attachments.
        ///
        /// \param Pass     The render pass to prepare.
        /// \param Viewport The viewport defining the rendering area.
        /// \param Colors   The clear colors for each color attachment.
        /// \param Depth    The depth clear value.
        /// \param Stencil  The stencil clear value.
        virtual void Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
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

