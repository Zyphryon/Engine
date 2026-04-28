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

        /// \brief Initializes the graphics driver with the given window and parameters.
        ///
        /// \param Window  The native window handle for presentation.
        /// \param Width   The initial surface width in pixels.
        /// \param Height  The initial surface height in pixels.
        /// \param Samples The initial multisampling level.
        /// \return `true` if initialization succeeded, `false` otherwise.
        virtual Bool Initialize(Ptr<SDL_Window> Window, UInt16 Width, UInt16 Height, Multisample Samples) = 0;

        /// \brief Resets the graphics driver with new surface parameters.
        ///
        /// \param Width   The new surface width in pixels.
        /// \param Height  The new surface height in pixels.
        /// \param Samples The new multisampling level.
        virtual void Reset(UInt16 Width, UInt16 Height, Multisample Samples) = 0;

        /// \brief Retrieves the device associated with this driver.
        ///
        /// \return A constant reference to the device.
        virtual ConstRef<Device> GetDevice() const = 0;

        /// \brief Creates a buffer resource with the specified parameters.
        ///
        /// \param ID       The identifier for the buffer.
        /// \param Access   The access pattern for the buffer.
        /// \param Usage    The intended usage of the buffer.
        /// \param Capacity The size of the buffer in bytes.
        /// \param Data     Optional initial data to populate the buffer.
        virtual void CreateBuffer(Object ID, Access Access, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data) = 0;

        /// \brief Updates a region of the specified buffer with new data.
        ///
        /// \param ID        The identifier of the buffer to update.
        /// \param Offset    The byte offset within the buffer to start the update.
        /// \param Invalidate Whether to invalidate the previous contents of the buffer.
        /// \param Data      The new data to write into the buffer.
        virtual void UpdateBuffer(Object ID, UInt32 Offset, Bool Invalidate, ConstSpan<Byte> Data) = 0;

        /// \brief Deletes the specified buffer resource.
        ///
        /// \param ID The identifier of the buffer to delete.
        virtual void DeleteBuffer(Object ID) = 0;

        /// \brief Resizes the specified buffer to a new size.
        ///
        /// \param ID       The identifier of the buffer to resize.
        /// \param Capacity The new size of the buffer in bytes.
        virtual void ResizeBuffer(Object ID, UInt32 Capacity) = 0;

        /// \brief Copies data between two buffers.
        ///
        /// \param SrcBuffer  The source buffer identifier.
        /// \param SrcOffset  The byte offset within the source buffer.
        /// \param DstBuffer  The destination buffer identifier.
        /// \param DstOffset  The byte offset within the destination buffer.
        /// \param Invalidate Whether to invalidate the destination buffer's previous contents.
        /// \param Size       The number of bytes to copy.
        virtual void CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, Bool Invalidate, UInt32 Size) = 0;

        /// \brief Reads data from the specified buffer.
        ///
        /// \param ID     The identifier of the buffer to read from.
        /// \param Offset The byte offset within the buffer to start reading.
        /// \param Size   The number of bytes to read.
        /// \return A blob containing the read data.
        virtual Blob ReadBuffer(Object ID, UInt32 Offset, UInt32 Size) = 0;

        /// \brief Maps a buffer into the CPU address space for direct access.
        ///
        /// \param ID         Identifier of the buffer to map.
        /// \param Offset     Byte offset from the beginning of the buffer where the mapping starts.
        /// \param Size       Number of bytes to map.
        /// \param Invalidate If `true`, discards the buffer’s previous contents before mapping.
        ///
        /// \return Pointer to the mapped memory region, `nullptr` if mapping fails.
        virtual Ptr<Byte> MapBuffer(Object ID, UInt32 Offset, UInt32 Size, Bool Invalidate) = 0;

        /// \brief Unmaps a previously mapped buffer.
        ///
        /// \param ID Identifier of the buffer to unmap.
        virtual void UnmapBuffer(Object ID) = 0;

        /// \brief Creates a render pass with the specified attachments.
        ///
        /// \param ID        The identifier for the render pass.
        /// \param Colors    The color attachments for the render pass.
        /// \param Auxiliary The auxiliary attachment for the render pass (e.g., depth/stencil).
        virtual void CreatePass(Object ID, ConstSpan<Attachment> Colors, ConstRef<Attachment> Auxiliary) = 0;

        /// \brief Deletes the specified render pass.
        ///
        /// \param ID The identifier of the render pass to delete.
        virtual void DeletePass(Object ID) = 0;

        /// \brief Creates a pipeline state object with the given program and descriptor.
        ///
        /// \param ID      The identifier for the pipeline.
        /// \param Shaders The shader program bytecode for the pipeline.
        /// \param States  The fixed-function states for the pipeline.
        virtual void CreatePipeline(Object ID, ConstRef<Shaders> Shaders, ConstRef<States> States) = 0;

        /// \brief Deletes the specified pipeline state object.
        ///
        /// \param ID The identifier of the pipeline to delete.
        virtual void DeletePipeline(Object ID) = 0;

        /// \brief Creates a texture resource with the specified parameters.
        ///
        /// \param ID         The identifier for the texture.
        /// \param Access     The access pattern for the texture.
        /// \param Type       The type of the texture (e.g., 2D, cube).
        /// \param Format     The pixel format of the texture.
        /// \param Layout     The memory layout of the texture.
        /// \param Width      The width of the texture in pixels.
        /// \param Height     The height of the texture in pixels.
        /// \param Mipmaps    The number of mipmap levels.
        /// \param Samples    The multisampling level.
        /// \param Data       Optional initial data to populate the texture.
        virtual void CreateTexture(Object ID, Access Access, TextureType Type, TextureFormat Format, TextureLayout Layout, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, ConstSpan<Byte> Data) = 0;

        /// \brief Updates a region of the specified texture with new data.
        ///
        /// \param ID     The identifier of the texture to update.
        /// \param Level  The mipmap level to update.
        /// \param X      The X offset within the texture to start the update.
        /// \param Y      The Y offset within the texture to start the update.
        /// \param Width  The width of the region to update in pixels.
        /// \param Height The height of the region to update in pixels.
        /// \param Pitch  The number of bytes per row of the source data.
        /// \param Data   The new data to write into the texture.
        virtual void UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data) = 0;

        /// \brief Deletes the specified texture resource.
        ///
        /// \param ID The identifier of the texture to delete.
        virtual void DeleteTexture(Object ID) = 0;

        /// \brief Resizes the specified texture to new dimensions and mipmap levels.
        ///
        /// \param ID      The identifier of the texture to resize.
        /// \param Width   The new width of the texture in pixels.
        /// \param Height  The new height of the texture in pixels.
        /// \param Mipmaps The new number of mipmap levels for the texture.
        virtual void ResizeTexture(Object ID, UInt16 Width, UInt16 Height, UInt8 Mipmaps) = 0;

        /// \brief Copies data between two textures.
        ///
        /// \param SrcTexture The source texture identifier.
        /// \param SrcLevel   The mipmap level of the source texture.
        /// \param SrcX       The X offset within the source texture.
        /// \param SrcY       The Y offset within the source texture.
        /// \param DstTexture The destination texture identifier.
        /// \param DstLevel   The mipmap level of the destination texture.
        /// \param DstX       The X offset within the destination texture.
        /// \param DstY       The Y offset within the destination texture.
        /// \param Invalidate Whether to invalidate the destination texture's previous contents.
        /// \param Width      The width of the region to copy in pixels.
        /// \param Height     The height of the region to copy in pixels.
        virtual void CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, Bool Invalidate, UInt16 Width, UInt16 Height) = 0;

        /// \brief Reads data from the specified texture.
        ///
        /// \param ID     The identifier of the texture to read from.
        /// \param Level  The mipmap level to read.
        /// \param X      The X offset within the texture to start reading.
        /// \param Y      The Y offset within the texture to start reading.
        /// \param Width  The width of the region to read in pixels.
        /// \param Height The height of the region to read in pixels.
        /// \return A blob containing the read texture data.
        virtual Blob ReadTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height) = 0;

        /// \brief Prepares the specified render pass for rendering.
        ///
        /// \param Pass     The render pass to prepare.
        /// \param Viewport The viewport defining the rendering area.
        /// \param Target   The clear target specifying which buffers to clear.
        /// \param Tint     The clear color tint.
        /// \param Depth    The clear depth value.
        /// \param Stencil  The clear stencil value.
        virtual void Prepare(Object Pass, ConstRef<Viewport> Viewport, Clear Target, Color Tint, Real32 Depth, UInt8 Stencil) = 0;

        /// \brief Submits a collection of rendering commands to the GPU.
        ///
        /// \param Submissions The collection of submission commands to execute.
        virtual void Submit(ConstSpan<DrawPacket> Submissions) = 0;

        /// \brief Commits the rendered results of the specified pass to the display.
        ///
        /// \param Pass The render pass to commit.
        virtual void Commit(Object Pass) = 0;
    };
}