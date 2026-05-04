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
#include "Encoder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Provides high-level management of the graphic subsystem.
    class Service final : public AbstractService<Service>
    {
    public:

        /// \brief Default capacity (in bytes) for transient vertex buffers.
        static constexpr UInt32 kDefaultTransientVertexCapacity  = 8 * 1024 * 1024;

        /// \brief Default capacity (in bytes) for transient index buffers.
        static constexpr UInt32 kDefaultTransientIndexCapacity   = 4 * 1024 * 1024;

        /// \brief Default capacity (in bytes) for transient uniform buffers.
        static constexpr UInt32 kDefaultTransientUniformCapacity = 2 * 1024 * 1024;

        /// \brief Represents an allocation of graphic resources.
        template<typename Type>
        struct Allocation final
        {
            /// \brief Pointer to the allocated data.
            Ptr<Type>   Pointer;

            /// \brief The graphic stream descriptor for this allocation.
            Stream      Descriptor;
        };

    public:

        /// \brief Constructs the graphic service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \copydoc Service::OnTeardown()
        void OnTeardown() override;

        /// \copydoc Service::OnTick(Time)
        void OnTick(Time Time) override;

        /// \brief Initializes the driver with the specified display and rendering parameters.
        ///
        /// \param Backend The graphics backend to use for rendering.
        /// \param Display The native display handle to render into.
        /// \param Width   The width of the rendering surface in pixels.
        /// \param Height  The height of the rendering surface in pixels.
        /// \param Samples The multisample count for anti-aliasing.
        /// \return `true` if initialization succeeded, `false` otherwise.
        Bool Initialize(Backend Backend, Ptr<SDL_Window> Display, UInt16 Width, UInt16 Height, Multisample Samples);

        /// \brief Resets the rendering surface to the specified resolution and multisample level.
        ///
        /// \param Width   The new width of the rendering surface in pixels.
        /// \param Height  The new height of the rendering surface in pixels.
        /// \param Samples The new multisample count for anti-aliasing.
        void Reset(UInt16 Width, UInt16 Height, Multisample Samples);

        /// \brief Gets the device associated with this driver.
        ///
        /// \return A constant reference to the device associated with this driver.
        ZYPHRYON_INLINE ConstRef<Device> GetDevice() const
        {
            return mDriver->GetDevice();
        }

        /// \brief Allocates a transient per-frame buffer of the specified format and length.
        ///
        /// \param Usage  The intended buffer usage.
        /// \param Length The number of elements to allocate.
        /// \param Stride The byte stride between elements (defaults to sizeof(Format)).
        /// \return An allocation containing a pointer to the data and its stream descriptor.
        template<typename Format>
        ZYPHRYON_INLINE Allocation<Format> AllocateTransientBuffer(Usage Usage, UInt32 Length, UInt32 Stride = sizeof(Format))
        {
            // Uniform buffers require special alignment and size handling.
            if (Usage == Usage::Uniform)
            {
                Stride = kUniformBlockAlignment;
                Length = Align(Length, kUniformBlockAlignment) / kUniformBlockAlignment;
            }

            Ref<InFlightArena> Arena  = mArenas[Enum::Cast(Usage)];
            Ref<Writer>        Buffer = Arena.Buffers[mProducer];

            // Allocate the memory from the CPU-side arena.
            const UInt32 Offset = Align(Buffer.GetOffset(), Stride);
            Buffer.Reserve(Length * Stride, Stride);

            const Ptr<Format> Address = reinterpret_cast<Ptr<Format>>(Buffer.GetData().data() + Offset);
            return Allocation<Format>(Address, Stream(Arena.Handle, Stride, Offset));
        }

        /// \brief Allocates and populates a transient per-frame buffer with the specified data.
        ///
        /// \param Usage The intended buffer usage.
        /// \param Data  The data to populate the buffer with.
        /// \return The stream descriptor of the allocated buffer.
        template<typename Format>
        ZYPHRYON_INLINE Stream AllocateTransientBuffer(Usage Usage, ConstSpan<Format> Data)
        {
            Allocation<Format> Allocation = AllocateTransientBuffer<Format>(Usage, Data.size());
            std::memcpy(Allocation.Pointer, Data.data(), Data.size_bytes());
            return Allocation.Descriptor;
        }

        /// \brief Creates a buffer resource with the specified parameters and initial data.
        ///
        /// \param Access   The access pattern for the buffer (e.g., read, write).
        /// \param Usage    The intended usage of the buffer (e.g., vertex, index).
        /// \param Capacity The capacity of the buffer in bytes.
        /// \param Data     The initial data to populate the buffer with.
        /// \return The identifier of the created buffer.
        Object CreateBuffer(Access Access, Usage Usage, UInt32 Capacity, AnyRef<Blob> Data);

        /// \brief Creates a buffer resource with the specified parameters and no initial data.
        ///
        /// \param Access   The access pattern for the buffer (e.g., read, write).
        /// \param Usage    The intended usage of the buffer (e.g., vertex, index).
        /// \param Capacity The capacity of the buffer in bytes.
        /// \return The identifier of the created buffer.
        ZYPHRYON_INLINE Object CreateBuffer(Access Access, Usage Usage, UInt32 Capacity)
        {
            return CreateBuffer(Access, Usage, Capacity, Blob());
        }

        /// \brief Creates a buffer resource with the specified parameters and initial data.
        ///
        /// \param Access The access pattern for the buffer (e.g., read, write).
        /// \param Usage  The intended usage of the buffer (e.g., vertex, index).
        /// \param Data   The initial data to populate the buffer with.
        /// \return The identifier of the created buffer.
        ZYPHRYON_INLINE Object CreateBuffer(Access Access, Usage Usage, AnyRef<Blob> Data)
        {
            return CreateBuffer(Access, Usage, Data.GetSize(), Move(Data));
        }

        /// \brief Updates a region of an existing buffer resource with new data.
        ///
        /// \param ID         The identifier of the buffer resource to update.
        /// \param Offset     The byte offset within the buffer to start updating.
        /// \param Invalidate Whether to invalidate the existing buffer data before updating.
        /// \param Data       The new data to write into the buffer.
        void UpdateBuffer(Object ID, UInt32 Offset, Bool Invalidate, AnyRef<Blob> Data);

        /// \brief Deletes a buffer resource, freeing associated memory.
        ///
        /// \param ID The identifier of the buffer resource to delete.
        void DeleteBuffer(Object ID);

        /// \brief Resizes the specified buffer to a new size.
        ///
        /// \param ID       The identifier of the buffer to resize.
        /// \param Capacity The new size of the buffer in bytes.
        void ResizeBuffer(Object ID, UInt32 Capacity);

        /// \brief Copies data from one buffer to another, with optional invalidation of the destination buffer.
        ///
        /// \param SrcBuffer  The identifier of the source buffer to copy from.
        /// \param SrcOffset  The byte offset within the source buffer to start copying from.
        /// \param DstBuffer  The identifier of the destination buffer to copy to.
        /// \param DstOffset  The byte offset within the destination buffer to start copying to.
        /// \param Invalidate Whether to invalidate the existing data in the destination buffer before copying.
        /// \param Size       The number of bytes to copy from the source buffer to the destination.
        void CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, Bool Invalidate, UInt32 Size);

        /// \brief Creates a material resource.
        ///
        /// \return The identifier of the created material.
        Object CreateMaterial();

        /// \brief Deletes a material resource, freeing associated memory and GPU state.
        ///
        /// \param ID The identifier of the material to delete.
        void DeleteMaterial(Object ID);

        /// \brief Creates a render pass resource with the specified attachments.
        ///
        /// \param Colors       The color attachments to use for the render pass.
        /// \param DepthStencil The depth/stencil attachment to use for the render pass.
        /// \return The identifier of the created render pass.
        Object CreatePass(ConstSpan<ColorAttachment> Colors, DepthStencilAttachment DepthStencil);

        /// \brief Deletes a render pass resource, freeing associated memory.
        ///
        /// \param ID The identifier of the render pass resource to delete.
        void DeletePass(Object ID);

        /// \brief Creates a pipeline resource with the specified program and states.
        ///
        /// \param Program The shader program to use for the pipeline.
        /// \param States  The pipeline states to configure for the pipeline.
        /// \return The identifier of the created pipeline.
        Object CreatePipeline(AnyRef<Program> Program, ConstRef<States> States);

        /// \brief Deletes a pipeline resource, freeing associated memory.
        ///
        /// \param ID The identifier of the pipeline resource to delete.
        void DeletePipeline(Object ID);

        /// \brief Creates a texture resource with the specified parameters and optional initial data.
        ///
        /// \param Type    The type of the texture (e.g., 2D, 3D, cube map).
        /// \param Format  The pixel format of the texture (e.g., RGBA8, DXT1).
        /// \param Access  The access pattern for the texture (e.g., read, write).
        /// \param Usage   The intended usage of the texture (e.g., render target, shader resource).
        /// \param Width   The width of the texture in pixels.
        /// \param Height  The height of the texture in pixels.
        /// \param Mipmaps The number of mipmap levels for the texture.
        /// \param Samples The multisample count for the texture, if applicable.
        /// \return The identifier of the created texture.
        Object CreateTexture(TextureType Type, TextureFormat Format, Access Access, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, AnyRef<Blob> Data);

        /// \brief Creates a texture resource for use as a render target.
        ///
        /// param Format   The pixel format of the texture (e.g., RGBA8, DXT1).
        /// param Resource Whether the texture will also be used as a shader resource for sampling.
        /// param Width    The width of the texture in pixels.
        /// param Height   The height of the texture in pixels.
        /// param Mipmaps  The number of mipmap levels for the texture.
        /// param Samples  The multisample count for the texture, if applicable.
        /// \return The identifier of the created texture.
        ZYPHRYON_INLINE Object CreateTexture(TextureFormat Format, Bool Resource, UInt16 Width, UInt16 Height, UInt8 Mipmaps = 1, Multisample Samples = Multisample::X1)
        {
            const Usage Usage = (Resource ? Usage::Target | Usage::Sample : Usage::Target);
            return CreateTexture(TextureType::Texture2D, Format, Access::Stream, Usage, Width, Height, Mipmaps, Samples, {});
        }

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
        void UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, Bool Invalidate, AnyRef<Blob> Data);

        /// \brief Deletes a texture resource, freeing associated memory.
        ///
        /// \param ID The identifier of the texture resource to delete.
        void DeleteTexture(Object ID);

        /// \brief Resizes the specified texture to new dimensions and mipmap levels.
        ///
        /// \param ID      The identifier of the texture to resize.
        /// \param Width   The new width of the texture in pixels.
        /// \param Height  The new height of the texture in pixels.
        /// \param Mipmaps The new number of mipmap levels for the texture.
        void ResizeTexture(Object ID, UInt16 Width, UInt16 Height, UInt8 Mipmaps);

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
        void CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, Bool Invalidate, UInt16 Width, UInt16 Height);

        /// \brief Prepares the specified render pass for rendering.
        ///
        /// \param Pass     The render pass to prepare.
        /// \param Viewport The viewport defining the rendering area.
        /// \param Colors   The clear colors for each color attachment, in order.
        /// \param Depth    The clear depth value (used when the pass has a depth attachment).
        /// \param Stencil  The clear stencil value (used when the pass has a stencil attachment).
        void Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil);

        /// \brief Prepares the specified render pass for rendering.
        ///
        /// \param Pass     The render pass to prepare.
        /// \param Viewport The viewport defining the rendering area.
        /// \param Color    The clear color to use for all color attachments.
        /// \param Depth    The clear depth value (used when the pass has a depth attachment).
        /// \param Stencil  The clear stencil value (used when the pass has a stencil attachment).
        ZYPHRYON_INLINE void Prepare(Object Pass, ConstRef<Viewport> Viewport, Color Color, Real32 Depth, UInt8 Stencil)
        {
            Prepare(Pass, Viewport, Spanify(Color), Depth, Stencil);
        }

        /// \brief Submits a list of draw items to the currently prepared render pass for execution.
        ///
        /// \param Items The list of draw items to submit.
        void Submit(ConstSpan<DrawItem> Items);

        /// \brief Submits the draw items recorded in the specified encoder, then clears the encoder.
        ///
        /// \param Encoder The encoder containing the draw items to submit.
        ZYPHRYON_INLINE void Submit(Ref<Encoder> Encoder)
        {
            if (const ConstSpan<Graphic::DrawItem> Items = Encoder.GetSubmissions(); !Items.empty())
            {
                Submit(Items);
            }
            Encoder.Clear();
        }

        /// \brief Commits all pending rendering commands for the currently prepared render pass.
        void Commit();

    private:

        /// \brief Maximum number of in-flight frames that can be processed concurrently.
        static constexpr UInt kInFlightFrames = 2;

        /// \brief Returns the frame currently open for CPU recording (producer).
        ///
        /// \return Reference to the producer (CPU) frame.
        ZYPHRYON_INLINE Ref<Writer> GetProducerFrame()
        {
            return mFrames[mProducer];
        }

        /// \brief Returns the frame currently being consumed by the GPU.
        ///
        /// \return Reference to the consumer (GPU) frame.
        ZYPHRYON_INLINE Ref<Writer> GetConsumerFrame()
        {
            return mFrames[mConsumer];
        }

        /// \brief Defines the types of commands that can be processed by the graphic service.
        enum class CommandType : UInt8
        {
            Initialize,     ///< \see Driver::Initialize
            Reset,          ///< \see Driver::Reset
            CreateBuffer,   ///< \see Driver::CreateBuffer
            UpdateBuffer,   ///< \see Driver::UpdateBuffer
            DeleteBuffer,   ///< \see Driver::DeleteBuffer
            ResizeBuffer,   ///< \see Driver::ResizeBuffer
            CopyBuffer,     ///< \see Driver::CopyBuffer
            CreatePass,     ///< \see Driver::CreatePass
            DeletePass,     ///< \see Driver::DeletePass
            CreatePipeline, ///< \see Driver::CreatePipeline
            DeletePipeline, ///< \see Driver::DeletePipeline
            CreateTexture,  ///< \see Driver::CreateTexture
            UpdateTexture,  ///< \see Driver::UpdateTexture
            DeleteTexture,  ///< \see Driver::DeleteTexture
            ResizeTexture,  ///< \see Driver::ResizeTexture
            CopyTexture,    ///< \see Driver::CopyTexture
            Prepare,        ///< \see Driver::Prepare
            Submit,         ///< \see Driver::Submit
            Commit,         ///< \see Driver::Commit
        };

        /// \brief Defines a command to be executed by the graphic service.
        template<CommandType Type, typename ...Arguments>
        struct Command final
        {
            /// \brief The return type of the command when executed on the driver.
            template<auto Method>
            using Return = std::invoke_result_t<decltype(Method), Ref<Driver>, Arguments...>;

            /// \brief The type of the command.
            static constexpr CommandType kType = Type;

            /// \brief Constructs a command with the specified parameters.
            ///
            /// \param Parameters The parameters for the command.
            ZYPHRYON_INLINE explicit Command(Arguments... Parameters)
                : Parameters { Move(Parameters)... }
            {
            }

            /// \brief Executes the command on the specified driver instance.
            ///
            /// \param Driver The driver instance to execute the command on.
            /// \return The result of the command execution.
            template<auto Method>
            ZYPHRYON_INLINE decltype(auto) Execute(Ref<Driver> Driver) const
            {
                return std::apply([& Driver]<typename... T0>(AnyRef<T0>... Parameters) {
                    return (Driver.* Method)(Forward<T0>(Parameters)...);
                }, Parameters);
            }

            std::tuple<Arguments...> Parameters;    // TODO: Remove std::tuple
        };

        /// \brief Collection of type aliases for all supported driver commands.
        struct CommandTypes final
        {
            using Initialize     = Command<CommandType::Initialize, Ptr<SDL_Window>, UInt16, UInt16, Multisample>;
            using Reset          = Command<CommandType::Reset, UInt16, UInt16, Multisample>;
            using CreateBuffer   = Command<CommandType::CreateBuffer, Object, Access, Usage, UInt32, Blob>;
            using UpdateBuffer   = Command<CommandType::UpdateBuffer, Object, UInt32, Bool, Blob>;
            using DeleteBuffer   = Command<CommandType::DeleteBuffer, Object>;
            using ResizeBuffer   = Command<CommandType::ResizeBuffer, Object, UInt32>;
            using CopyBuffer     = Command<CommandType::CopyBuffer, Object, UInt32, Object, UInt32, Bool, UInt32>;
            using CreatePass     = Command<CommandType::CreatePass, Object, Vector<ColorAttachment, kMaxAttachments>, DepthStencilAttachment>;
            using DeletePass     = Command<CommandType::DeletePass, Object>;
            using CreatePipeline = Command<CommandType::CreatePipeline, Object, Program, States>;
            using DeletePipeline = Command<CommandType::DeletePipeline, Object>;
            using CreateTexture  = Command<CommandType::CreateTexture, Object, TextureType, TextureFormat, Access, Usage, UInt16, UInt16, UInt8, Multisample, Blob>;
            using UpdateTexture  = Command<CommandType::UpdateTexture, Object, UInt8, UInt16, UInt16, UInt16, UInt16, UInt32, Bool, Blob>;
            using DeleteTexture  = Command<CommandType::DeleteTexture, Object>;
            using ResizeTexture  = Command<CommandType::ResizeTexture, Object, UInt16, UInt16, UInt8>;
            using CopyTexture    = Command<CommandType::CopyTexture, Object, UInt8, UInt16, UInt16, Object, UInt8, UInt16, UInt16, Bool, UInt16, UInt16>;
            using Prepare        = Command<CommandType::Prepare, Object, Viewport, Vector<Color, kMaxAttachments>, Real32, UInt8>;
            using Commit         = Command<CommandType::Commit>;
        };

        /// \brief Writes a GPU command to the specified stream.
        ///
        /// \tparam Command   The command type to write.
        /// \param Stream     The stream to write the command to.
        /// \param Parameters The parameters for the command.
        template<typename Command, typename... Arguments>
        ZYPHRYON_INLINE void WriteCommand(Ref<Writer> Stream, AnyRef<Arguments>... Parameters)
        {
            /// Write the command type identifier.
            Stream.WriteUInt8(static_cast<UInt8>(Command::kType));

            /// Write the command arguments.
            Stream.WriteStruct<Command>(Forward<Arguments>(Parameters)...);
        }

        /// \brief Dispatches execution of a GPU command read from the stream.
        ///
        /// \tparam Method  The driver method to invoke.
        /// \tparam Command The command type to read and execute.
        /// \param Stream   The stream containing the encoded command data.
        template<auto Method, typename Command>
        ZYPHRYON_INLINE decltype(auto) DispatchCommand(Ref<Reader> Stream)
        {
            using Return = Command::template Return<Method>;

            const ConstPtr<Command> Instance = Stream.ReadStruct<Command>();

            if constexpr (IsEqual<Return, void>)
            {
                Instance->template Execute<Method>(* mDriver);
                InPlaceDelete(* Instance);
            }
            else
            {
                Return Result = Instance->template Execute<Method>(* mDriver);
                InPlaceDelete(* Instance);

                return Result;
            }
        }

        /// \brief Flushes all pending GPU commands, optionally aborting ongoing operations.
        ///
        /// \param Abort If `true`, aborts any ongoing GPU operations.
        void FlushCommands(Bool Abort);

        /// \brief Main GPU thread loop for processing and executing queued commands.
        ///
        /// \param Token Stop token used to request thread termination.
        void OnCommandThread(ConstRef<std::stop_token> Token);

        /// \brief Dispatches execution of a single GPU command from the frame stream.
        ///
        /// \param Type  Type of command to run.
        /// \param Frame Source stream containing the encoded command data.
        void OnCommandExecute(CommandType Type, Ref<Reader> Frame);

        /// \brief Defines an in-flight buffer being tracked by the service.
        struct InFlightArena final
        {
            /// \brief The buffer object being tracked.
            Object                         Handle;

            /// \brief The capacity of the buffer at the time it was submitted.
            UInt32                         Capacity;

            /// \brief The collection of in-flight arenas for this buffer.
            Array<Writer, kInFlightFrames> Buffers;
        };

        /// \brief Defines a collection of in-flight buffers categorized by usage.
        using InFlightArenas = Array<InFlightArena, Enum::Count<Usage>()>;

        /// \brief Creates transient arenas for in-flight buffers.
        void CreateInFlightArenas();

        /// \brief Updates the transient arenas with new data for the current frame.
        void UpdateInFlightArenas();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Unique<Driver>                 mDriver;
        Thread                         mWorker;
        Signal                         mSignal;
        Array<Writer, kInFlightFrames> mFrames;
        UInt32                         mProducer = 0;
        UInt32                         mConsumer = 1;
        InFlightArenas                 mArenas;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Slot<kMaxBuffers>              mBuffers;
        Slot<kMaxMaterials>            mMaterials;
        Slot<kMaxPasses>               mPasses;
        Slot<kMaxPipelines>            mPipelines;
        Slot<kMaxTextures>             mTextures;
    };
}