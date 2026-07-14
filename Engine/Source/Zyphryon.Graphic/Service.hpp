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

#include "Driver.hpp"
#include "Transient.hpp"
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Manages all GPU resources and rendering operations.
    ///
    /// Dispatch commands to the active driver through a double-buffered \ref Journal for asynchronous GPU execution.
    class Service final : public Engine::Subsystem, public Switchable<Driver>
    {
    public:

        /// \brief Constructs the graphic service and starts the background GPU worker thread.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Advances the service by one tick, flushing the current frame's commands to the GPU thread.
        ///
        /// \param Delta The elapsed time in seconds since the last tick.
        void OnTick(Real64 Delta) override;

        /// \brief Tears down the service by stopping the GPU worker thread and flushing all pending commands.
        void OnTeardown() override;

        /// \brief Initializes the service with the specified graphics adapter and display parameters.
        ///
        /// \param Adapter The name of the graphics adapter to use.
        /// \param Output  The output target to render into.
        /// \param Config  The configuration settings for the graphics device.
        /// \return `true` if initialization succeeded, `false` otherwise.
        Bool Initialize(Text Adapter, Ptr<void> Output, ConstRef<Config> Config);

        /// \brief Resets the rendering surface to the specified resolution.
        ///
        /// \param Width  The new width of the rendering surface in pixels.
        /// \param Height The new height of the rendering surface in pixels.
        void Reset(UInt16 Width, UInt16 Height);

        /// \brief Gets the current description of the graphics service, including backend, adapter, and capabilities.
        ///
        /// \return The current graphics service description.
        ZY_INLINE ConstRef<Description> GetDescription() const
        {
            return mDescription;
        }

        /// \brief Allocates transient vertex data for the current producer frame.
        ///
        /// \param Count The number of vertices to allocate.
        /// \return A transient mapping the CPU memory and the corresponding GPU stream for the allocated vertices.
        template<typename Format>
        ZY_INLINE Transient<Format> AllocateTransientVertices(UInt32 Count)
        {
            return RequestInFlightArena<Format>(mFrames[mProducer].Vertices, sizeof(Format), Count);
        }

        /// \brief Allocates transient index data for the current producer frame.
        ///
        /// \param Count The number of indices to allocate.
        /// \return A transient mapping the CPU memory and the corresponding GPU stream for the allocated indices.
        template<typename Format>
        ZY_INLINE Transient<Format> AllocateTransientIndices(UInt32 Count)
        {
            return RequestInFlightArena<Format>(mFrames[mProducer].Indices, sizeof(Format), Count);
        }

        /// \brief Allocates transient uniform data for the current producer frame.
        ///
        /// \param Count The number of uniform blocks to allocate.
        /// \return A transient mapping the CPU memory and the corresponding GPU stream for the allocated uniforms.
        template<typename Format>
        ZY_INLINE Transient<Format> AllocateTransientUniforms(UInt32 Count)
        {
            const UInt16 Alignment = mDescription.Capabilities.UniformBlockAlignment;

            Count = AlignPowTwo<UInt32>(Count * sizeof(Format), Alignment) / Alignment;
            return RequestInFlightArena<Format>(mFrames[mProducer].Uniforms, Alignment * Count, 1);
        }

        /// \brief Allocates transient command data for the current frame.
        ///
        /// \param Count The number of commands to allocate.
        /// \return A transient span over the allocated command storage.
        ZY_INLINE Span<Command> AllocateTransientCommands(UInt32 Count)
        {
            Ref<Sequence<Command>> Collection = mFrames[mProducer].Commands;

            const UInt Offset = Collection.GetSize();
            Collection.Advance(Count);

            return Span(Collection.GetData() + Offset, Count);;
        }

        /// \brief Creates a buffer resource with the specified parameters and optional initial data.
        ///
        /// \param Storage  The storage type for the buffer.
        /// \param Usage    The intended usage of the buffer.
        /// \param Capacity The capacity of the buffer in bytes.
        /// \param Data     The optional initial data to populate the buffer with.
        /// \return The identifier of the created buffer resource, or zero if creation failed.
        Object CreateBuffer(Storage Storage, Usage Usage, UInt32 Capacity, AnyRef<Blob> Data);

        /// \brief Creates an empty buffer resource with the specified capacity.
        ///
        /// \param Storage  The storage type for the buffer.
        /// \param Usage    The intended usage of the buffer.
        /// \param Capacity The capacity of the buffer in bytes.
        /// \return The identifier of the created buffer resource, or zero if creation failed.
        ZY_INLINE Object CreateBuffer(Storage Storage, Usage Usage, UInt32 Capacity)
        {
            return CreateBuffer(Storage, Usage, Capacity, Blob());
        }

        /// \brief Creates a buffer resource sized and populated from the provided data.
        ///
        /// \param Storage The storage type for the buffer.
        /// \param Usage   The intended usage of the buffer.
        /// \param Data    The initial data to populate the buffer with; the buffer capacity is derived from the data size.
        /// \return The identifier of the created buffer resource, or zero if creation failed.
        ZY_INLINE Object CreateBuffer(Storage Storage, Usage Usage, AnyRef<Blob> Data)
        {
            return CreateBuffer(Storage, Usage, Data.GetSize(), Move(Data));
        }

        /// \brief Updates a region of an existing buffer resource with new data.
        ///
        /// \param ID     The identifier of the buffer resource to update.
        /// \param Offset The byte offset within the buffer to start updating.
        /// \param Data   The new data to write into the buffer.
        void UpdateBuffer(Object ID, UInt32 Offset, AnyRef<Blob> Data);

        /// \brief Deletes a buffer resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the buffer resource to delete.
        void DeleteBuffer(Object ID);

        /// \brief Copies a region of one buffer resource to another.
        ///
        /// \param SrcBuffer The identifier of the source buffer to copy from.
        /// \param SrcOffset The byte offset within the source buffer to start copying from.
        /// \param DstBuffer The identifier of the destination buffer to copy to.
        /// \param DstOffset The byte offset within the destination buffer to start copying to.
        /// \param Size      The number of bytes to copy.
        void CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, UInt32 Size);

        /// \brief Creates a material slot and returns its identifier.
        ///
        /// \return The identifier of the created material slot, or zero if creation failed.
        Object CreateMaterial();

        /// \brief Deletes a material slot, freeing its identifier for reuse.
        ///
        /// \param ID The identifier of the material slot to delete.
        void DeleteMaterial(Object ID);

        /// \brief Creates a render pass resource with the specified color and depth attachments.
        ///
        /// \param Colors The color attachments for the render pass.
        /// \param Depth  The depth/stencil attachment for the render pass.
        /// \return The identifier of the created render pass resource, or zero if creation failed.
        Object CreatePass(ConstSpan<ColorAttachment> Colors, DepthAttachment Depth);

        /// \brief Creates a render pass resource with a single color attachment and a depth attachment.
        ///
        /// \param Color The single color attachment for the render pass.
        /// \param Depth The depth/stencil attachment for the render pass.
        /// \return The identifier of the created render pass resource, or zero if creation failed.
        ZY_INLINE Object CreatePass(ColorAttachment Color, DepthAttachment Depth)
        {
            return CreatePass(ConstSpan(Color), Depth);
        }

        /// \brief Deletes a render pass resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the render pass resource to delete.
        void DeletePass(Object ID);

        /// \brief Creates a pipeline resource with the specified shader program and pipeline states.
        ///
        /// \param Program The shader program to use for the pipeline.
        /// \param States  The fixed-function pipeline states to configure.
        /// \return The identifier of the created pipeline resource, or zero if creation failed.
        Object CreatePipeline(AnyRef<Program> Program, ConstRef<States> States);

        /// \brief Deletes a pipeline resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the pipeline resource to delete.
        void DeletePipeline(Object ID);

        /// \brief Creates a texture resource with the specified parameters and optional initial data.
        ///
        /// \param Layout  The layout of the texture.
        /// \param Format  The pixel format of the texture.
        /// \param Storage The storage type for the texture.
        /// \param Usage   The intended usage of the texture.
        /// \param Width   The width of the texture in pixels.
        /// \param Height  The height of the texture in pixels.
        /// \param Mipmaps The number of mipmap levels.
        /// \param Samples The multisample count.
        /// \param Data    The optional initial image data to populate the texture with.
        /// \return The identifier of the created texture resource, or zero if creation failed.
        Object CreateTexture(TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, AnyRef<Blob> Data);

        /// \brief Creates a 2D texture resource for sampling.
        ///
        /// \param Layout  The layout of the texture.
        /// \param Format  The pixel format of the texture.
        /// \param Width   The width of the texture in pixels.
        /// \param Height  The height of the texture in pixels.
        /// \param Mipmaps The number of mipmap levels (defaults to 1).
        /// \param Data    The optional initial image data to populate the texture with.
        /// \return The identifier of the created texture resource, or zero if creation failed.
        ZY_INLINE Object CreateTexture(TextureLayout Layout, TextureFormat Format, UInt16 Width, UInt16 Height, UInt8 Mipmaps, AnyRef<Blob> Data)
        {
            constexpr Usage Usage = Usage::Sample;
            return CreateTexture(Layout, Format, Storage::Immutable, Usage, Width, Height, Mipmaps, Multisample::X1, Move(Data));
        }

        /// \brief Creates a 2D texture resource for sampling and rendering.
        ///
        /// \param Format  The pixel format of the texture.
        /// \param Width   The width of the texture in pixels.
        /// \param Height  The height of the texture in pixels.
        /// \param Mipmaps The number of mipmap levels (defaults to 1).
        /// \param Samples The multisample count (defaults to \c Multisample::X1).
        /// \return The identifier of the created texture resource, or zero if creation failed.
        ZY_INLINE Object CreateTexture(TextureFormat Format, UInt16 Width, UInt16 Height, UInt8 Mipmaps = 1, Multisample Samples = Multisample::X1)
        {
            constexpr Usage Usage = Usage::Target | Usage::Sample;
            return CreateTexture(TextureLayout::Texture2D, Format, Storage::Stream, Usage, Width, Height, Mipmaps, Samples, {});
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
        void UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, AnyRef<Blob> Data);

        /// \brief Deletes a texture resource, freeing associated GPU memory.
        ///
        /// \param ID The identifier of the texture resource to delete.
        void DeleteTexture(Object ID);

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
        void CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, UInt16 Width, UInt16 Height);

        /// \brief Prepares the specified render pass for rendering by setting the viewport and clearing attachments.
        ///
        /// \param Pass     The render pass to prepare.
        /// \param Viewport The viewport defining the rendering area.
        /// \param Colors   The clear colors for each color attachment.
        /// \param Depth    The depth clear value.
        /// \param Stencil  The stencil clear value.
        void Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil);

        /// \brief Prepares the specified render pass with a single clear color for rendering.
        ///
        /// \param Pass     The render pass to prepare.
        /// \param Viewport The viewport defining the rendering area.
        /// \param Color    The clear color for the single color attachment.
        /// \param Depth    The depth clear value.
        /// \param Stencil  The stencil clear value.
        ZY_INLINE void Prepare(Object Pass, ConstRef<Viewport> Viewport, Color Color, Real32 Depth, UInt8 Stencil)
        {
            Prepare(Pass, Viewport, ConstSpan(Color), Depth, Stencil);
        }

        /// \brief Commits all pending rendering commands for the currently prepared render pass.
        void Commit();

    private:

        /// \brief Initializes the driver, then creates the in-flight resources.
        ///
        /// \param Output The output target to render into.
        /// \param Config The configuration settings for the graphics device.
        void Setup(Ptr<void> Output, ConstRef<Config> Config);

        /// \brief Swaps the producer and consumer journals and signals the GPU thread to begin processing the current frame.
        void Flush();

        /// \brief Consumes and executes all pending commands in the current consumer journal.
        void Consume();

        /// \brief Entry point for the background GPU worker thread; processes commands until a stop is requested.
        ///
        /// \param Token The stop token used to request termination of the GPU worker thread.
        void OnWorkerThread(ConstRef<std::stop_token> Token);

        /// \brief Represents an in-flight render pass.
        struct InFlightPass final
        {
            /// The handle to the in-flight render pass.
            Object Handle;

            /// The current cursor position within the render pass.
            UInt32 Cursor;
        };

        /// \brief Holds the CPU-side linear allocator and the corresponding GPU buffer for one transient stream.
        struct InFlightArena final
        {
            /// The CPU-side linear allocator that accumulates transient data for the frame.
            Sequence<Byte> Memory;

            /// The GPU buffer handle that receives the uploaded data each frame.
            Object         Buffer;

            /// The current capacity of the GPU buffer in bytes.
            UInt32         Capacity;
        };

        /// \brief Groups all per-frame resources required to record and execute one GPU frame.
        struct InFlightFrame final
        {
            /// The command journal for this frame.
            Journal            Journal;

            /// The transient vertex buffer arena for this frame.
            InFlightArena      Vertices;

            /// The transient index buffer arena for this frame.
            InFlightArena      Indices;

            /// The transient uniform buffer arena for this frame.
            InFlightArena      Uniforms;

            /// The submission arena for this frame, which accumulates draw commands to submit.
            Sequence<Command>  Commands;

            /// The in-flight render pass for this frame.
            InFlightPass       Pass;
        };

        /// \brief Returns a reference to the consumer journal currently being processed by the GPU thread.
        ///
        /// \return The consumer \ref Journal for the current frame.
        ZY_INLINE Ref<Journal> GetConsumer()
        {
            return mFrames[mConsumer].Journal;
        }

        /// \brief Returns a reference to the producer journal being written by the CPU thread.
        ///
        /// \return The producer \ref Journal for the current frame.
        ZY_INLINE Ref<Journal> GetProducer()
        {
            return mFrames[mProducer].Journal;
        }

        /// \brief Records a deferred driver method call into the producer journal for asynchronous GPU execution.
        ///
        /// \param Parameters The arguments to forward to the driver method when the command is executed.
        template<auto Method, typename... Arguments>
        ZY_INLINE void Enqueue(AnyRef<Arguments>... Parameters)
        {
            GetProducer().Record<Method>(mDriver.Grab(), Forward<Arguments>(Parameters)...);
        }

        /// \brief Returns a reference to the in-flight render pass for the current frame.
        ///
        /// \return The in-flight \ref InFlightPass for the current frame.
        ZY_INLINE Ref<InFlightPass> GetInFlightPass()
        {
            return mFrames[mProducer].Pass;
        }

        void SubmitInFlightPass(UInt32 Offset, UInt32 Count);

        /// \brief Initializes a single \ref InFlightArena by allocating a CPU writer and a GPU buffer.
        ///
        /// \param Arena    The in-flight arena to initialize.
        /// \param Usage    The intended GPU usage of the buffer backing this arena.
        /// \param Capacity The initial capacity of the GPU buffer in bytes.
        void CreateInFlightArena(Ref<InFlightArena> Arena, Usage Usage, UInt32 Capacity);

        /// \brief Uploads the accumulated CPU data of a single \ref InFlightArena to its GPU buffer and resets the writer.
        ///
        /// \param Arena The in-flight arena to upload and reset.
        /// \param Usage The intended GPU usage of the buffer backing this arena.
        void UpdateInFlightArena(Ref<InFlightArena> Arena, Usage Usage);

        /// \brief Allocates a region of transient CPU memory from the given arena.
        ///
        /// \param Arena  The in-flight arena from which to allocate the transient data.
        /// \param Stride The byte stride of each element, used for alignment and reservation.
        /// \param Count  The number of elements to allocate.
        /// \return A transient mapping the allocated CPU memory and the corresponding GPU stream.
        template<typename Type>
        ZY_INLINE Transient<Type> RequestInFlightArena(Ref<InFlightArena> Arena, UInt32 Stride, UInt32 Count)
        {
            Ref<Sequence<Byte>> Buffer = Arena.Memory;

            const UInt32 Offset = AlignPowTwo(static_cast<UInt32>(Buffer.GetSize()), Stride);
            const UInt32 Size   = Count * Stride;

            Buffer.Advance(Offset - Buffer.GetSize() + Size);

            const Ptr<Type> Address = reinterpret_cast<Ptr<Type>>(Buffer.GetData() + Offset);
            return Transient<Type>(Address, Stream(Arena.Buffer, Stride, Offset));
        }

        /// \brief Initializes all in-flight resources for each frame.
        ///
        /// \param Frame The in-flight frame to initialize.
        void CreateInFlightFrame(Ref<InFlightFrame> Frame);

        /// \brief Uploads all transient data of the given in-flight frame to the GPU.
        ///
        /// \param Frame The in-flight frame to update.
        void UploadInFlightFrame(Ref<InFlightFrame> Frame);

        /// \brief Registers built-in resource loaders for shaders, textures, and other assets.
        void RegisterBuiltinLoaders();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Unique<Driver>                   mDriver;
        Description                      mDescription;
        Thread                           mWorker;
        Atomic<Bool>                     mSignal;
        UInt8                            mProducer = 0;
        UInt8                            mConsumer = kMaxFrames - 1;
        Array<InFlightFrame, kMaxFrames> mFrames;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Freelist<kMaxBuffers>            mBuffers;
        Freelist<kMaxTextures>           mMaterials;
        Freelist<kMaxPasses>             mPasses;
        Freelist<kMaxPipelines>          mPipelines;
        Freelist<kMaxTextures>           mTextures;
    };
}