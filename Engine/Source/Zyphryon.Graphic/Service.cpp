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
#include "Loader/MTLLoader.hpp"
#include "Loader/VFXLoader.hpp"
#include "Loader/SHDLoader.hpp"
#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host }
    {
#if !defined(ZY_PLATFORM_WEB)
        mWorker = Thread([this](std::stop_token Token)
        {
            OnWorkerThread(Token);
        });
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTeardown()
    {
        ZY_PROFILE_SCOPE("Graphic::Teardown");

#if !defined(ZY_PLATFORM_WEB)
        mWorker.request_stop();
        Flush();
        mWorker.join();
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Real64 Delta)
    {
        ZY_PROFILE_SCOPE("Graphic::Tick");

        Flush();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Initialize(Text Adapter, Ptr<void> Output, ConstRef<Config> Config)
    {
        ZY_PROFILE_SCOPE("Graphic::Initialize");

        Bool Successful = true;

        if (!mDriver)
        {
            mDriver = Switch(Adapter);

            if (mDriver)
            {
                GetProducer().Record<& Service::Setup>(this, Output, Config);
                Flush(); // Handles the immediate encoding and begins transferring data to the GPU.
                Flush(); // Ensures that all data has been completely processed and synchronized.
            }

            Successful = (mDriver != nullptr);

            if (!Successful)
            {
                mDriver = Unique<Driver>::Create();

                LOG_W("Graphics: Failed to initialize driver, using default driver");
            }
            else
            {
                mDriver->Probe(mDescription);

                LOG_I("Graphics: Using {0}", mDescription.Backend);
                LOG_I("Graphics: Detected Tier ({0})", Enum::GetName(mDescription.Tier));

                for (ConstRef<Graphic::Adapter> Endpoint : mDescription.Endpoints)
                {
                    LOG_I("Graphics: Found GPU '{0}'", Endpoint.Description);
                    LOG_I("Graphics:     Memory {0} (video)", Endpoint.Memory);
                }

                RegisterBuiltinLoaders();
            }
        }
        return Successful;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Reset(UInt16 Width, UInt16 Height)
    {
        Enqueue<& Driver::Reset>(Width, Height);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreateBuffer(Storage Storage, Usage Usage, UInt32 Capacity, AnyRef<Blob> Data)
    {
        const Object ID = mBuffers.Allocate();

        if (ID)
        {
            Enqueue<& Driver::CreateBuffer>(ID, Storage, Usage, Capacity, Move(Data));
        }
        return ID;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::UpdateBuffer(Object ID, UInt32 Offset, AnyRef<Blob> Data)
    {
        ZY_ASSERT(mBuffers.IsAllocated(ID), "Buffer is not valid");

        Enqueue<& Driver::UpdateBuffer>(ID, Offset, Move(Data));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::DeleteBuffer(Object ID)
    {
        mBuffers.Free(ID);

        Enqueue<& Driver::DeleteBuffer>(ID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, UInt32 Size)
    {
        ZY_ASSERT(mBuffers.IsAllocated(SrcBuffer), "Source buffer is not valid");
        ZY_ASSERT(mBuffers.IsAllocated(DstBuffer), "Destination buffer is not valid");

        Enqueue<& Driver::CopyBuffer>(SrcBuffer, SrcOffset, DstBuffer, DstOffset, Size);
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
        mMaterials.Free(ID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreatePass(ConstSpan<ColorAttachment> Colors, DepthAttachment Depth)
    {
        const Object ID = mPasses.Allocate();

        if (ID)
        {
            Enqueue<& Driver::CreatePass>(ID, Sequence<ColorAttachment, kMaxAttachments>(Colors), Depth);
        }
        return ID;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::DeletePass(Object ID)
    {
        mPasses.Free(ID);

        Enqueue<& Driver::DeletePass>(ID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreatePipeline(AnyRef<Program> Program, ConstRef<States> States)
    {
        const Object ID = mPipelines.Allocate();

        if (ID)
        {
            Enqueue<& Driver::CreatePipeline>(ID, Move(Program), States);
        }
        return ID;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::DeletePipeline(Object ID)
    {
        mPipelines.Free(ID);

        Enqueue<& Driver::DeletePipeline>(ID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreateTexture(TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, AnyRef<Blob> Data)
    {
        const Object ID = mTextures.Allocate();

        if (ID)
        {
            Enqueue<& Driver::CreateTexture>(ID, Layout, Format, Storage, Usage, Width, Height, Mipmaps, Samples, Move(Data));
        }
        return ID;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, AnyRef<Blob> Data)
    {
        ZY_ASSERT(mTextures.IsAllocated(ID), "Texture is not valid");

        Enqueue<& Driver::UpdateTexture>(ID, Level, X, Y, Width, Height, Pitch, Move(Data));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::DeleteTexture(Object ID)
    {
        mTextures.Free(ID);

        Enqueue<& Driver::DeleteTexture>(ID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, UInt16 Width, UInt16 Height)
    {
        ZY_ASSERT(mTextures.IsAllocated(SrcTexture), "Source texture is not valid");
        ZY_ASSERT(mTextures.IsAllocated(DstTexture), "Destination texture is not valid");

        Enqueue<& Driver::CopyTexture>(SrcTexture, SrcLevel, SrcX, SrcY, DstTexture, DstLevel, DstX, DstY, Width, Height);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        Enqueue<& Driver::Prepare>(Pass, Viewport, Sequence<Color, kMaxAttachments>(Colors), Depth, Stencil);

        // Mark the beginning of a new render pass, recording the pass handle and the current command buffer
        // offset so that commit can later slice the commands belonging to this pass.
        Ref<InFlightPass> InFlightPass = GetInFlightPass();
        InFlightPass.Handle = Pass;
        InFlightPass.Cursor = mFrames[mProducer].Commands.GetSize();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Commit()
    {
        // Submit all commands recorded since the last render pass was prepared, then commit the current render pass.
        Ref<InFlightPass> InFlightPass = GetInFlightPass();

        if (const UInt32 Size = mFrames[mProducer].Commands.GetSize(); Size > InFlightPass.Cursor)
        {
            GetProducer().Record<& Service::SubmitInFlightPass>(this, InFlightPass.Cursor, Size - InFlightPass.Cursor);
        }
        Enqueue<& Driver::Commit>(InFlightPass.Handle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Setup(Ptr<void> Output, ConstRef<Config> Config)
    {
        if (mDriver->Initialize(Output, Config))
        {
            for (Ref<InFlightFrame> InFlightFrame : mFrames)
            {
                CreateInFlightFrame(InFlightFrame);
            }
        }
        else
        {
            mDriver = nullptr;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Flush()
    {
#if !defined(ZY_PLATFORM_WEB)
        // Wait until the GPU thread signals that the current frame has finished processing.
        mSignal.wait(true, std::memory_order_acquire);
#endif

        // Rotate the frame queue so that the next frame becomes writable for the CPU while the previous one
        // moves into the GPU submission pipeline.
        Swap(mProducer, mConsumer);

#if !defined(ZY_PLATFORM_WEB)
        // Mark the service as busy again and notify the GPU thread that a new frame is ready for processing.
        mSignal.exchange(true, std::memory_order_release);
        mSignal.notify_one();
#else
        Consume();
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Consume()
    {
        ZY_PROFILE_SCOPE("Graphic::Consume");

        // Update the in-flight arenas with the latest data from the CPU.
        UploadInFlightFrame(mFrames[mConsumer]);

        // Execute all commands in the consumer buffer, then reset it for reuse.
        GetConsumer().Run();

        // Clears the in-flight command queue.
        mFrames[mConsumer].Commands.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnWorkerThread(ConstRef<std::stop_token> Token)
    {
        ZY_PROFILE_THREAD("GPU Thread");

        while (!Token.stop_requested())
        {
            // Put the thread to sleep until the flag indicates there is more work to process.
            mSignal.wait(false, std::memory_order_acquire);

            // Consume all pending commands in the current frame's consumer buffer.
            Consume();

            // Clear the busy flag to signal that the GPU has completed its current tasks.
            mSignal.store(false, std::memory_order_release);
            mSignal.notify_one();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SubmitInFlightPass(UInt32 Offset, UInt32 Count)
    {
        mDriver->Submit(ConstSpan(mFrames[mConsumer].Commands.GetData() + Offset, Count));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::CreateInFlightArena(Ref<InFlightArena> Arena, Usage Usage, UInt32 Capacity)
    {
        Arena.Buffer   = mBuffers.Allocate();
        Arena.Capacity = Capacity;
        Arena.Memory.Reserve(Capacity);

        mDriver->CreateBuffer(Arena.Buffer, Storage::Dynamic, Usage, Capacity, ConstSpan<Byte>());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::UpdateInFlightArena(Ref<InFlightArena> Arena, Usage Usage)
    {
        if (Ref<Sequence<Byte>> Data = Arena.Memory; !Data.IsEmpty())
        {
            // Ensure GPU buffer has enough capacity to hold the new data.
            if (Data.GetSize() > Arena.Capacity)
            {
                Arena.Capacity = Data.GetSize();

                mDriver->DeleteBuffer(Arena.Buffer);
                mDriver->CreateBuffer(Arena.Buffer, Storage::Dynamic, Usage, Arena.Capacity, Data);
            }
            else
            {

#if defined(ZY_PLATFORM_WEB)
                mDriver->UpdateBuffer(Arena.Buffer, 0, Data);
#else
                const Ptr<void> Address = mDriver->MapBuffer(Arena.Buffer, 0, Data.GetSize());
                Blit(Address, Data.GetSize(), Data.GetData());
                mDriver->UnmapBuffer(Arena.Buffer);
#endif
            }

            // Clear the data after uploading to the GPU to free up memory on the CPU side.
            Data.Clear();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=- =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::CreateInFlightFrame(Ref<InFlightFrame> Frame)
    {
        CreateInFlightArena(Frame.Vertices, Usage::Vertex,  8 * 1024 * 1024);    // Initial 8mb
        CreateInFlightArena(Frame.Indices,  Usage::Index,   2 * 1024 * 1024);    // Initial 2mb
        CreateInFlightArena(Frame.Uniforms, Usage::Uniform, 2 * 1024 * 1024);    // Initial 2mb
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::UploadInFlightFrame(Ref<InFlightFrame> Frame)
    {
        UpdateInFlightArena(Frame.Vertices, Usage::Vertex);
        UpdateInFlightArena(Frame.Indices,  Usage::Index);
        UpdateInFlightArena(Frame.Uniforms, Usage::Uniform);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RegisterBuiltinLoaders()
    {
        ConstRetainer<Content::Service> Content = GetHost().GetService<Content::Service>();

        Content->AddLoader(MTLLoader::kTypes, Retainer<MTLLoader>::Create());
        Content->AddLoader(SHDLoader::kTypes, Retainer<SHDLoader>::Create());
        Content->AddLoader(VFXLoader::kTypes, Retainer<VFXLoader>::Create(mDescription.Language));
    }
}