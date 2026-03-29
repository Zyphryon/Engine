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

        /// \brief Initializes the rendering backend.
        ///
        /// \param Backend   Rendering backend to initialize.
        /// \param Swapchain Native window handle for presentation.
        /// \param Width     Initial surface width in pixels.
        /// \param Height    Initial surface height in pixels.
        /// \param Samples   Initial multisampling level.
        /// \return `true` if the initialization was successful, `false` otherwise.
        Bool Initialize(Backend Backend, Ptr<SDL_Window> Swapchain, UInt16 Width, UInt16 Height, Multisample Samples);

        /// \brief Resets the graphics driver with new surface parameters.
        ///
        /// \param Width   The new surface width in pixels.
        /// \param Height  The new surface height in pixels.
        /// \param Samples The new multisampling level.
        void Reset(UInt16 Width, UInt16 Height, Multisample Samples);

        /// \brief Retrieves the device associated with this driver.
        ///
        /// \return A constant reference to the device.
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

        /// \brief Creates a buffer resource with the specified parameters.
        ///
        /// \param Access The access pattern for the buffer.
        /// \param Usage  The intended usage of the buffer.
        /// \param Length The length of the buffer in bytes.
        /// \param Data   Optional initial data to populate the buffer.
        /// \return The identifier of the created buffer.
        Object CreateBuffer(Access Access, Usage Usage, UInt32 Length, AnyRef<Blob> Data);

        /// \brief Creates a buffer resource with the specified parameters.
        ///
        /// \param Access The access pattern for the buffer.
        /// \param Usage  The intended usage of the buffer.
        /// \param Length The length of the buffer in bytes.
        /// \return The identifier of the created buffer.
        ZYPHRYON_INLINE Object CreateBuffer(Access Access, Usage Usage, UInt32 Length)
        {
            return CreateBuffer(Access, Usage, Length, Blob());
        }

        /// \brief Creates a buffer resource with the specified parameters.
        ///
        /// \param Access The access pattern for the buffer.
        /// \param Usage  The intended usage of the buffer.
        /// \param Data   Optional initial data to populate the buffer.
        /// \return The identifier of the created buffer.
        ZYPHRYON_INLINE Object CreateBuffer(Access Access, Usage Usage, AnyRef<Blob> Data)
        {
            return CreateBuffer(Access, Usage, Data.GetSize(), Move(Data));
        }

        /// \brief Updates a region of the specified buffer with new data.
        ///
        /// \param ID         The identifier of the buffer to update.
        /// \param Offset     The byte offset within the buffer to start the update.
        /// \param Invalidate Whether to invalidate the previous contents of the buffer.
        /// \param Data       The new data to write into the buffer.
        void UpdateBuffer(Object ID, UInt32 Offset, Bool Invalidate, AnyRef<Blob> Data);

        /// \brief Deletes the specified buffer resource.
        ///
        /// \param ID The identifier of the buffer to delete.
        void DeleteBuffer(Object ID);

        /// \brief Resizes the specified buffer to a new size.
        ///
        /// \param ID   The identifier of the buffer to resize.
        /// \param Size The new size of the buffer in bytes.
        void ResizeBuffer(Object ID, UInt32 Size);

        /// \brief Copies data between two buffers.
        ///
        /// \param SrcBuffer  The source buffer identifier.
        /// \param SrcOffset  The byte offset within the source buffer.
        /// \param DstBuffer  The destination buffer identifier.
        /// \param DstOffset  The byte offset within the destination buffer.
        /// \param Invalidate Whether to invalidate the destination buffer's previous contents.
        /// \param Size       The number of bytes to copy.
        void CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, Bool Invalidate, UInt32 Size);

        /// \brief Creates a material resource.
        ///
        /// \return The identifier of the created material.
        Object CreateMaterial();

        /// \brief Deletes the specified material resource.
        ///
        /// \param ID The identifier of the material to delete.
        void DeleteMaterial(Object ID);

        /// \brief Creates a render pass with the specified attachments.
        ///
        /// \param Colors    The color attachments for the render pass.
        /// \param Auxiliary The auxiliary attachment for the render pass (e.g., depth/stencil).
        /// \return The identifier of the created render pass.
        Object CreatePass(ConstSpan<Attachment> Colors, ConstRef<Attachment> Auxiliary);

        /// \brief Deletes the specified render pass.
        ///
        /// \param ID The identifier of the render pass to delete.
        void DeletePass(Object ID);

        /// \brief Creates a pipeline state object with the given program and descriptor.
        ///
        /// \param Shaders The shader program bytecode for the pipeline.
        /// \param States  The fixed-function states for the pipeline.
        /// \return The identifier of the created pipeline.
        Object CreatePipeline(AnyRef<Shaders> Shaders, ConstRef<States> States);

        /// \brief Deletes the specified pipeline state object.
        ///
        /// \param ID The identifier of the pipeline to delete.
        void DeletePipeline(Object ID);

        /// \brief Creates a texture resource with the specified parameters.
        ///
        /// \param Access  The access pattern for the texture.
        /// \param Type    The type of the texture (e.g., 2D, cube).
        /// \param Format  The pixel format of the texture.
        /// \param Layout  The memory layout of the texture.
        /// \param Width   The width of the texture in pixels.
        /// \param Height  The height of the texture in pixels.
        /// \param Mipmaps The number of mipmap levels.
        /// \param Samples The multisampling level.
        /// \param Data    Optional initial data to populate the texture.
        /// \return The identifier of the created texture.
        Object CreateTexture(Access Access, TextureType Type, TextureFormat Format, TextureLayout Layout, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, AnyRef<Blob> Data);

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
        void UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, AnyRef<Blob> Data);

        /// \brief Deletes the specified texture resource.
        ///
        /// \param ID The identifier of the texture to delete.
        void DeleteTexture(Object ID);

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
        void CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, Bool Invalidate, UInt16 Width, UInt16 Height);

        /// \brief Prepares the specified render pass for rendering.
        ///
        /// \param Pass     The render pass to prepare.
        /// \param Viewport The viewport defining the rendering area.
        /// \param Target   The clear target specifying which buffers to clear.
        /// \param Tint     The clear color tint.
        /// \param Depth    The clear depth value.
        /// \param Stencil  The clear stencil value.
        void Prepare(Object Pass, ConstRef<Viewport> Viewport, Clear Target, Color Tint, Real32 Depth, UInt8 Stencil);

        /// \brief Submits a collection of rendering commands to the GPU.
        ///
        /// \param Submissions The collection of submission commands to execute.
        void Submit(ConstSpan<DrawPacket> Submissions);

        /// \brief Commits the rendered results of the specified pass to the display.
        ///
        /// \param Pass The render pass to commit.
        void Commit(Object Pass);

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
            using CreatePass     = Command<CommandType::CreatePass, Object, Vector<Attachment, kMaxAttachments>, Attachment>;
            using DeletePass     = Command<CommandType::DeletePass, Object>;
            using CreatePipeline = Command<CommandType::CreatePipeline, Object, Shaders, States>;
            using DeletePipeline = Command<CommandType::DeletePipeline, Object>;
            using CreateTexture  = Command<CommandType::CreateTexture, Object, Access, TextureType, TextureFormat, TextureLayout, UInt16, UInt16, UInt8, Multisample, Blob>;
            using UpdateTexture  = Command<CommandType::UpdateTexture, Object, UInt8, UInt16, UInt16, UInt16, UInt16, UInt32, Blob>;
            using DeleteTexture  = Command<CommandType::DeleteTexture, Object>;
            using CopyTexture    = Command<CommandType::CopyTexture, Object, UInt8, UInt16, UInt16, Object, UInt8, UInt16, UInt16, Bool, UInt16, UInt16>;
            using Prepare        = Command<CommandType::Prepare, Object, Viewport, Clear, Color, Real32, UInt8>;
            using Commit         = Command<CommandType::Commit, Object>;
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