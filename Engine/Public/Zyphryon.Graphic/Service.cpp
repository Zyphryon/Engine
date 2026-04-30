// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Service.hpp"

#ifdef    ZYPHRYON_PLATFORM_WINDOWS
    #include <Zyphryon.Graphic/Driver/D3D11/D3D11Driver.hpp>
#endif // ZYPHRYON_PLATFORM_WINDOWS

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : AbstractService(Host)
    {
        mWorker = Thread(Capture<& Service::OnCommandThread>(this));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTeardown()
    {
        ZYPHRYON_PROFILE_SCOPE("Graphic::Teardown");

        mWorker.request_stop();
        FlushCommands(true);
        mWorker.join();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Time Time)
    {
        ZYPHRYON_PROFILE_SCOPE("Graphic::Tick");

        FlushCommands(false);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Initialize(Backend Backend, Ptr<SDL_Window> Display, UInt16 Width, UInt16 Height, Multisample Samples)
    {
        ZYPHRYON_PROFILE_SCOPE("Graphic::Initialize");

        Bool Successful = true;

        if (!mDriver)
        {
            switch (Backend)
            {
#ifdef    ZYPHRYON_PLATFORM_WINDOWS
            case Backend::D3D11:
                mDriver = Unique<D3D11Driver>::Create();
                break;
#endif // ZYPHRYON_PLATFORM_WINDOWS
            default:
                break;
            }

            if (mDriver)
            {
                WriteCommand<CommandTypes::Initialize>(GetProducerFrame(), Display, Width, Height, Samples);
                FlushCommands(false); // Handles the immediate encoding and begins the process of transferring data to the GPU.
                FlushCommands(false); // Ensures that all data has been completely processed and synchronized.
            }

            Successful = mDriver != nullptr;

            if (Successful)
            {
                ConstRef<Device> Device = mDriver->GetDevice();
                LOG_INFO("Graphics: using {}", Enum::Name(Device.Backend));
                LOG_INFO("Graphics: Detected shader model {}", Enum::Cast(Device.Version) + 1);

                for (ConstRef<Adapter> Adapter : Device.Adapters)
                {
                    LOG_INFO("Graphics: Found GPU '{}'", Adapter.Description);
                    LOG_INFO("Graphics:     Memory {} (video)", Adapter.Memory);
                }
            }
        }
        return Successful;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Reset(UInt16 Width, UInt16 Height, Multisample Samples)
    {
        WriteCommand<CommandTypes::Reset>(GetProducerFrame(), Width, Height, Samples);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreateBuffer(Access Access, Usage Usage, UInt32 Capacity, AnyRef<Blob> Data)
    {
        const Object ID = mBuffers.Allocate();

        if (ID)
        {
            WriteCommand<CommandTypes::CreateBuffer>(GetProducerFrame(), ID, Access, Usage, Capacity, Move(Data));
        }
        return ID;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::UpdateBuffer(Object ID, UInt32 Offset, Bool Invalidate, AnyRef<Blob> Data)
    {
        LOG_ASSERT(mBuffers.IsAllocated(ID), "Buffer ID is not valid");

        WriteCommand<CommandTypes::UpdateBuffer>(GetProducerFrame(), ID, Offset, Invalidate, Move(Data));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::DeleteBuffer(Object ID)
    {
        LOG_ASSERT(mBuffers.IsAllocated(ID), "Buffer ID is not valid");

        mBuffers.Free(ID);

        WriteCommand<CommandTypes::DeleteBuffer>(GetProducerFrame(), ID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::ResizeBuffer(Object ID, UInt32 Capacity)
    {
        LOG_ASSERT(mBuffers.IsAllocated(ID), "Buffer ID is not valid");

        WriteCommand<CommandTypes::ResizeBuffer>(GetProducerFrame(), ID, Capacity);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, Bool Invalidate, UInt32 Size)
    {
        LOG_ASSERT(mBuffers.IsAllocated(SrcBuffer), "Source buffer is not valid");
        LOG_ASSERT(mBuffers.IsAllocated(SrcOffset), "Destination buffer is not valid");

        WriteCommand<CommandTypes::CopyBuffer>(GetProducerFrame(), SrcBuffer, SrcOffset, DstBuffer, DstOffset, Invalidate, Size);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreateMaterial()
    {
        return mMaterials.Allocate();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::DeleteMaterial(Object ID)
    {
        LOG_ASSERT(mMaterials.IsAllocated(ID), "Material ID is not valid");

        mMaterials.Free(ID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreatePass(ConstSpan<ColorAttachment> Colors, DepthStencilAttachment DepthStencil)
    {
        const Object ID = mPasses.Allocate();

        if (ID)
        {
            Vector<ColorAttachment, kMaxAttachments> ColorAttachments(Colors.begin(), Colors.end());

            WriteCommand<CommandTypes::CreatePass>(GetProducerFrame(), ID, Move(ColorAttachments), DepthStencil);
        }
        return ID;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::DeletePass(Object ID)
    {
        LOG_ASSERT(mPasses.IsAllocated(ID), "Pass ID is not valid");

        mPasses.Free(ID);

        WriteCommand<CommandTypes::DeletePass>(GetProducerFrame(), ID);
    }
    
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreatePipeline(AnyRef<Program> Program, ConstRef<States> States)
    {
        const Object ID = mPipelines.Allocate();

        if (ID)
        {
            WriteCommand<CommandTypes::CreatePipeline>(GetProducerFrame(), ID, Move(Program), States);
        }
        return ID;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::DeletePipeline(Object ID)
    {
        LOG_ASSERT(mPipelines.IsAllocated(ID), "Pipeline ID is not valid");

        mPipelines.Free(ID);

        WriteCommand<CommandTypes::DeletePipeline>(GetProducerFrame(), ID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreateTexture(TextureType Type, TextureFormat Format, Access Access, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, AnyRef<Blob> Data)
    {
        const Object ID = mTextures.Allocate();

        if (ID)
        {
            WriteCommand<CommandTypes::CreateTexture>(
                GetProducerFrame(), ID, Type, Format, Access, Usage, Width, Height, Mipmaps, Samples, Move(Data));
        }
        return ID;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, Bool Invalidate, AnyRef<Blob> Data)
    {
        LOG_ASSERT(mTextures.IsAllocated(ID), "Texture ID is not valid");

        WriteCommand<CommandTypes::UpdateTexture>(GetProducerFrame(), ID, Level, X, Y, Width, Height, Pitch, Invalidate, Move(Data));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::DeleteTexture(Object ID)
    {
        LOG_ASSERT(mTextures.IsAllocated(ID), "Texture ID is not valid");

        mTextures.Free(ID);

        WriteCommand<CommandTypes::DeleteTexture>(GetProducerFrame(), ID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::ResizeTexture(Object ID, UInt16 Width, UInt16 Height, UInt8 Mipmaps)
    {
        LOG_ASSERT(mTextures.IsAllocated(ID), "Texture ID is not valid");

        WriteCommand<CommandTypes::ResizeTexture>(GetProducerFrame(), ID, Width, Height, Mipmaps);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, Bool Invalidate, UInt16 Width, UInt16 Height)
    {
        LOG_ASSERT(mTextures.IsAllocated(SrcTexture), "Source texture is not valid");
        LOG_ASSERT(mTextures.IsAllocated(DstTexture), "Destination texture is not valid");

        WriteCommand<CommandTypes::CopyTexture>(GetProducerFrame(),
            SrcTexture, SrcLevel, SrcX, SrcY,
            DstTexture, DstLevel, DstX, DstY, Invalidate, Width, Height);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        Vector<Color, kMaxAttachments> ClearColorParams(Colors.begin(), Colors.end());

        WriteCommand<CommandTypes::Prepare>(GetProducerFrame(), Pass, Viewport, ClearColorParams, Depth, Stencil);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Submit(ConstSpan<DrawItem> Items)
    {
        LOG_ASSERT(!Items.empty(), "Requires at least one draw command");

        Ref<Writer> Writer = GetProducerFrame();
        Writer.WriteEnum(CommandType::Submit);
        Writer.WriteBlock<UInt16, DrawItem>(Items);
    }
    
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Commit(Object Pass)
    {
        WriteCommand<CommandTypes::Commit>(GetProducerFrame(), Pass);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::FlushCommands(Bool Abort)
    {
        if (Abort)
        {
            GetProducerFrame().Clear();
        }

        // Wait until the GPU thread signals that the current frame has finished processing.
        mSignal.wait(true, std::memory_order_acquire);

        // Rotate the frame queue so that the next frame becomes writable for the CPU while the previous one
        // moves into the GPU submission pipeline.
        Swap(mProducer, mConsumer);

        // Mark the service as busy again and notify the GPU thread that a new frame is ready for processing.
        mSignal.test_and_set(std::memory_order_release);
        mSignal.notify_one();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnCommandThread(ConstRef<std::stop_token> Token)
    {
        ZYPHRYON_PROFILE_THREAD("GPU Thread");

        while (!Token.stop_requested())
        {
            ZYPHRYON_PROFILE_SCOPE("Graphic::Consume");

            // Put the thread to sleep until the flag indicates there is more work to process.
            mSignal.wait(false, std::memory_order_acquire);

            // Update the in-flight arenas with the latest data from the CPU.
            UpdateInFlightArenas();

            // Continuously process the data as long as there is available data in the decoder.
            Ref<Writer> Frame = GetConsumerFrame();

            Reader Decoder(Frame.GetData());
            while (Decoder.GetAvailable() > 0)
            {
                OnCommandExecute(static_cast<CommandType>(Decoder.ReadUInt8()), Decoder);
            }
            Frame.Clear();

            // Clear the busy flag to signal that the GPU has completed its current tasks.
            mSignal.clear(std::memory_order_release);
            mSignal.notify_one();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnCommandExecute(CommandType Type, Ref<Reader> Frame)
    {
        switch (Type)
        {
            case CommandType::Initialize:
            {
                if (const Bool Succeed = DispatchCommand<&Driver::Initialize, CommandTypes::Initialize>(Frame); Succeed)
                {
                    CreateInFlightArenas();
                }
                else
                {
                    mDriver = nullptr;
                }
                break;
            }
            case CommandType::Reset:
            {
                DispatchCommand<&Driver::Reset, CommandTypes::Reset>(Frame);
                break;
            }
            case CommandType::CreateBuffer:
            {
                DispatchCommand<&Driver::CreateBuffer, CommandTypes::CreateBuffer>(Frame);
                break;
            }
            case CommandType::UpdateBuffer:
            {
                DispatchCommand<&Driver::UpdateBuffer, CommandTypes::UpdateBuffer>(Frame);
                break;
            }
            case CommandType::DeleteBuffer:
            {
                DispatchCommand<&Driver::DeleteBuffer, CommandTypes::DeleteBuffer>(Frame);
                break;
            }
            case CommandType::ResizeBuffer:
            {
                DispatchCommand<&Driver::ResizeBuffer, CommandTypes::ResizeBuffer>(Frame);
                break;
            }
            case CommandType::CopyBuffer:
            {
                DispatchCommand<&Driver::CopyBuffer, CommandTypes::CopyBuffer>(Frame);
                break;
            }
            case CommandType::CreatePass:
            {
                DispatchCommand<&Driver::CreatePass, CommandTypes::CreatePass>(Frame);
                break;
            }
            case CommandType::DeletePass:
            {
                DispatchCommand<&Driver::DeletePass, CommandTypes::DeletePass>(Frame);
                break;
            }
            case CommandType::CreatePipeline:
            {
                DispatchCommand<&Driver::CreatePipeline, CommandTypes::CreatePipeline>(Frame);
                break;
            }
            case CommandType::DeletePipeline:
            {
                DispatchCommand<&Driver::DeletePipeline, CommandTypes::DeletePipeline>(Frame);
                break;
            }
            case CommandType::CreateTexture:
            {
                DispatchCommand<&Driver::CreateTexture, CommandTypes::CreateTexture>(Frame);
                break;
            }
            case CommandType::UpdateTexture:
            {
                DispatchCommand<&Driver::UpdateTexture, CommandTypes::UpdateTexture>(Frame);
                break;
            }
            case CommandType::DeleteTexture:
            {
                DispatchCommand<&Driver::DeleteTexture, CommandTypes::DeleteTexture>(Frame);
                break;
            }
            case CommandType::ResizeTexture:
            {
                DispatchCommand<&Driver::ResizeTexture, CommandTypes::ResizeTexture>(Frame);
                break;
            }
            case CommandType::CopyTexture:
            {
                DispatchCommand<&Driver::CopyTexture, CommandTypes::CopyTexture>(Frame);
                break;
            }
            case CommandType::Prepare:
            {
                DispatchCommand<&Driver::Prepare, CommandTypes::Prepare>(Frame);
                break;
            }
            case CommandType::Submit:
            {
                mDriver->Submit(Frame.ReadBlock<UInt16, DrawItem>());
                break;
            }
            case CommandType::Commit:
            {
                DispatchCommand<&Driver::Commit, CommandTypes::Commit>(Frame);
                break;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::CreateInFlightArenas()
    {
        static auto CreateBuffer = [&](Usage Usage, UInt32 Capacity)
        {
            Ref<InFlightArena> Arena = mArenas[Enum::Cast(Usage)];
            Arena.Handle   = mBuffers.Allocate();
            Arena.Capacity = Capacity;

            mDriver->CreateBuffer(Arena.Handle, Access::Dynamic, Usage, Capacity, ConstSpan<Byte>());

            for (Ref<Writer> Buffer : Arena.Buffers)
            {
                Buffer.Ensure(Capacity);
            }
        };

        CreateBuffer(Usage::Vertex,  kDefaultTransientVertexCapacity);
        CreateBuffer(Usage::Index,   kDefaultTransientIndexCapacity);
        CreateBuffer(Usage::Uniform, kDefaultTransientUniformCapacity);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::UpdateInFlightArenas()
    {
        for (Ref<InFlightArena> Arena : mArenas)
        {
            Ref<Writer> Buffer = Arena.Buffers[mConsumer];

            if (ConstSpan<Byte> Data = Buffer.GetData(); !Data.empty())
            {
                // Resize the GPU buffer if the new data exceeds its current capacity.
                if (Data.size_bytes() > Arena.Capacity)
                {
                    Arena.Capacity = Data.size_bytes();

                    mDriver->ResizeBuffer(Arena.Handle, Arena.Capacity);  // TODO: Resize With Initial Data
                }

                const Ptr<void> Address = mDriver->MapBuffer(Arena.Handle, 0, Data.size_bytes(), true);
                {
                    std::memcpy(Address, Data.data(), Data.size_bytes());
                }
                mDriver->UnmapBuffer(Arena.Handle);

                // Clear the buffer for the next frame.
                Buffer.Clear();
            }
        }
    }
}