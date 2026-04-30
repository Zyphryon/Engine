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
#include <SDL3/SDL_video.h>

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
        /// \param Display The native display handle to render into.
        /// \param Width   The width of the rendering surface in pixels.
        /// \param Height  The height of the rendering surface in pixels.
        /// \param Samples The multisample count for anti-aliasing.
        /// \return `true` if initialization succeeded, `false` otherwise.
        virtual Bool Initialize(Ptr<SDL_Window> Display, UInt16 Width, UInt16 Height, Multisample Samples) = 0;

        /// \brief Resets the rendering surface to the specified resolution and multisample level.
        ///
        /// \param Width   The new width of the rendering surface in pixels.
        /// \param Height  The new height of the rendering surface in pixels.
        /// \param Samples The new multisample count for anti-aliasing.
        virtual void Reset(UInt16 Width, UInt16 Height, Multisample Samples) = 0;

        /// \brief Gets the device associated with this driver.
        ///
        /// \return A constant reference to the device associated with this driver.
        virtual ConstRef<Device> GetDevice() const = 0;

        /// \brief Creates a buffer resource with the specified parameters and optional initial data.
        ///
        /// \param ID       The identifier for the buffer resource.
        /// \param Access   The access pattern for the buffer (e.g., read, write).
        /// \param Usage    The intended usage of the buffer (e.g., vertex, index).
        /// \param Capacity The capacity of the buffer in bytes.
        /// \param Data     Optional initial data to populate the buffer with.
        virtual void CreateBuffer(Object ID, Access Access, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data) = 0;

        /// \brief Updates a region of an existing buffer resource with new data.
        ///
        /// \param ID         The identifier of the buffer resource to update.
        /// \param Offset     The byte offset within the buffer to start updating.
        /// \param Invalidate Whether to invalidate the existing buffer data before updating.
        /// \param Data       The new data to write into the buffer.
        virtual void UpdateBuffer(Object ID, UInt32 Offset, Bool Invalidate, ConstSpan<Byte> Data) = 0;

        /// \brief Deletes a buffer resource, freeing associated memory.
        ///
        /// \param ID The identifier of the buffer resource to delete.
        virtual void DeleteBuffer(Object ID) = 0;

        /// \brief Resizes the specified buffer to a new size.
        ///
        /// \param ID       The identifier of the buffer to resize.
        /// \param Capacity The new size of the buffer in bytes.
        virtual void ResizeBuffer(Object ID, UInt32 Capacity) = 0;

        /// \brief Copies data from one buffer to another, with optional invalidation of the destination buffer.
        ///
        /// \param SrcBuffer  The identifier of the source buffer to copy from.
        /// \param SrcOffset  The byte offset within the source buffer to start copying from.
        /// \param DstBuffer  The identifier of the destination buffer to copy to.
        /// \param DstOffset  The byte offset within the destination buffer to start copying to.
        /// \param Invalidate Whether to invalidate the existing data in the destination buffer before copying.
        /// \param Size       The number of bytes to copy from the source buffer to the destination.
        virtual void CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, Bool Invalidate, UInt32 Size) = 0;

        /// \brief Reads data from a buffer resource into a CPU buffer.
        ///
        /// \param ID     The identifier of the buffer to read from.
        /// \param Offset The byte offset within the buffer to start reading from.
        /// \param Size   The number of bytes to read from the buffer.
        /// \return The CPU buffer containing the data read from the specified buffer resource.
        virtual Blob ReadBuffer(Object ID, UInt32 Offset, UInt32 Size) = 0;

        /// \brief Maps a region of a buffer resource into CPU address space for direct access.
        ///
        /// \param ID         The identifier of the buffer to map.
        /// \param Offset     The byte offset within the buffer to start mapping from.
        /// \param Size       The number of bytes to map from the buffer.
        /// \param Invalidate Whether to invalidate the existing buffer data before mapping.
        /// \return A pointer to the mapped buffer region in CPU address space, or `nullptr` if mapping fails.
        virtual Ptr<Byte> MapBuffer(Object ID, UInt32 Offset, UInt32 Size, Bool Invalidate) = 0;

        /// \brief Unmaps a previously mapped buffer resource, releasing CPU access.
        ///
        /// \param ID The identifier of the buffer to unmap.
        virtual void UnmapBuffer(Object ID) = 0;

        /// \brief Creates a render pass resource with the specified attachments.
        ///
        /// \param ID           The identifier for the render pass resource.
        /// \param Colors       The color attachments to use for the render pass.
        /// \param DepthStencil The depth/stencil attachment to use for the render pass.
        virtual void CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthStencilAttachment DepthStencil) = 0;

        /// \brief Deletes a render pass resource, freeing associated memory.
        ///
        /// \param ID The identifier of the render pass resource to delete.
        virtual void DeletePass(Object ID) = 0;

        /// \brief Creates a pipeline resource with the specified program and states.
        ///
        /// \param ID      The identifier for the pipeline resource.
        /// \param Program The shader program to use for the pipeline.
        /// \param States  The pipeline states to configure for the pipeline.
        virtual void CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<States> States) = 0;

        /// \brief Deletes a pipeline resource, freeing associated memory.
        ///
        /// \param ID The identifier of the pipeline resource to delete.
        virtual void DeletePipeline(Object ID) = 0;

        /// \brief Creates a texture resource with the specified parameters and optional initial data.
        ///
        /// \param ID      The identifier for the texture resource.
        /// \param Type    The type of the texture (e.g., 2D, 3D, cube map).
        /// \param Format  The pixel format of the texture (e.g., RGBA8, DXT1).
        /// \param Access  The access pattern for the texture (e.g., read, write).
        /// \param Usage   The intended usage of the texture (e.g., render target, shader resource).
        /// \param Width   The width of the texture in pixels.
        /// \param Height  The height of the texture in pixels.
        /// \param Mipmaps The number of mipmap levels for the texture.
        /// \param Samples The multisample count for the texture, if applicable.
        /// \param Data    Optional initial data to populate the texture with, organized by mipmap level and array layer.
        virtual void CreateTexture(Object ID, TextureType Type, TextureFormat Format, Access Access, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, ConstSpan<Byte> Data) = 0;

        /// \brief Updates a region of an existing texture resource with new data.
        ///
        /// \param ID         The identifier of the texture resource to update.
        /// \param Level      The mipmap level of the texture to update.
        /// \param X          The X offset within the texture to start updating.
        /// \param Y          The Y offset within the texture to start updating.
        /// \param Width      The width of the region to update in pixels.
        /// \param Height     The height of the region to update in pixels.
        /// \param Pitch      The number of bytes in a row of the texture data, including any padding.
        /// \param Invalidate Whether to invalidate the existing texture data in the specified region before updating.
        /// \param Data       The new texture data to write into the specified region, organized by mipmap level and array layer.
        virtual void UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, Bool Invalidate, ConstSpan<Byte> Data) = 0;

        /// \brief Deletes a texture resource, freeing associated memory.
        ///
        /// \param ID The identifier of the texture resource to delete.
        virtual void DeleteTexture(Object ID) = 0;

        /// \brief Resizes the specified texture to new dimensions and mipmap levels.
        ///
        /// \param ID      The identifier of the texture to resize.
        /// \param Width   The new width of the texture in pixels.
        /// \param Height  The new height of the texture in pixels.
        /// \param Mipmaps The new number of mipmap levels for the texture.
        virtual void ResizeTexture(Object ID, UInt16 Width, UInt16 Height, UInt8 Mipmaps) = 0;

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
        /// \param Invalidate Whether to invalidate the existing texture data in the destination region before copying.
        /// \param Width      The width of the region to copy in pixels.
        /// \param Height     The height of the region to copy in pixels.
        virtual void CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, Bool Invalidate, UInt16 Width, UInt16 Height) = 0;

        /// \brief Reads a region of a texture resource into a CPU buffer.
        ///
        /// \param ID     The identifier of the texture to read from.
        /// \param Level  The mipmap level of the texture to read from.
        /// \param X      The X offset within the texture to start reading from.
        /// \param Y      The Y offset within the texture to start reading from.
        /// \param Width  The width of the region to read in pixels.
        /// \param Height The height of the region to read in pixels.
        /// \return The CPU buffer containing the texture data read from the specified region of the texture
        virtual Blob ReadTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height) = 0;

        /// \brief Prepares the specified render pass for rendering.
        ///
        /// \param Pass     The render pass to prepare.
        /// \param Viewport The viewport defining the rendering area.
        /// \param Colors   The clear colors for each color attachment, in order.
        /// \param Depth    The clear depth value (used when the pass has a depth attachment).
        /// \param Stencil  The clear stencil value (used when the pass has a stencil attachment).
        virtual void Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil) = 0;

        /// \brief Submits a list of draw items to the currently prepared render pass for execution.
        ///
        /// \param Items The list of draw items to submit.
        virtual void Submit(ConstSpan<DrawItem> Items) = 0;

        /// \brief Commits the rendered results of the specified pass.
        ///
        /// \param Pass The render pass to commit.
        virtual void Commit(Object Pass) = 0;
    };
}