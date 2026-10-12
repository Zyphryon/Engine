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
#include "Loader/MSHLoader.hpp"
#include "Loader/MTLLoader.hpp"
#include "Loader/SHDLoader.hpp"
#include "Loader/TEXLoader.hpp"
#include "Loader/VFXLoader.hpp"
#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host },
          mSignal   { false },
          mProducer { 0 },
          mConsumer { kMaxFrames - 1 }
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

    void Service::OnTick(Real64 Delta)
    {
        ZY_PROFILE_SCOPE("Graphic::Tick");

        Flush();
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

    Bool Service::Initialize(Text Name, Ptr<void> Output, ConstRef<Configuration> Config)
    {
        ZY_PROFILE_SCOPE("Graphic::Initialize");

        if (mDriver)
        {
            return true;
        }

        mDriver = Switch(Name);

        if (mDriver)
        {
            GetProducer().Record<& Service::Setup>(this, Output, Config);

            // The second flush waits out the first, so Setup has run on the GPU thread before the driver is checked.
            Flush();
            Flush();
        }

        if (!mDriver)
        {
            mDriver = Unique<Driver>::Create();

            LOG_W("Graphics: Failed to initialize driver, using default driver");
            return false;
        }

        mDriver->Probe(mDescription);

        mSnapshot.Tearless = Config.Tearless;

        LOG_I("Graphics: Using {0}", mDescription.Backend);
        LOG_I("Graphics: Detected Tier ({0})", ZyEnum::GetName(mDescription.Tier));

        for (ConstRef<ZyGraphic::Adapter> Device : mDescription.Adapters)
        {
            LOG_I("Graphics: Found GPU '{0}'", Device.Description);
            LOG_I("Graphics:     Memory {0} (video)", Device.Memory);
        }

        RegisterBuiltinLoaders();
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Reset(UInt16 Width, UInt16 Height, Bool Tearless)
    {
        mSnapshot.Tearless = Tearless;

        Enqueue<& Driver::Reset>(Width, Height, Tearless);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreateBuffer(Storage Storage, Usage Usage, UInt32 Capacity, AnyRef<Blob> Data)
    {
        ZY_ASSERT(Data.GetSize() == 0 || Data.GetSize() == Capacity, "Initial data must fill the buffer's capacity");

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

    Object Service::CreatePipeline(AnyRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States)
    {
        const Object ID = mPipelines.Allocate();

        if (ID)
        {
            Enqueue<& Driver::CreatePipeline>(ID, Move(Program), Signature, States);
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

    Object Service::ObtainSampler(Sampler Descriptor)
    {
        Ref<Object> Handle = mSnapshot.Samplers.FindOrInsert(Hash(Descriptor));

        if (!Handle)
        {
            Handle = mSamplers.Allocate();

            if (Handle)
            {
                Enqueue<& Driver::CreateSampler>(Handle, Descriptor);
            }
        }
        return Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::CreateTexture(TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, AnyRef<Blob> Data)
    {
        ZY_ASSERT(Layout != TextureLayout::TextureCube || Layers == 6, "A cube map is exactly six slices");

        const Object ID = mTextures.Allocate();

        if (ID)
        {
            Enqueue<& Driver::CreateTexture>(ID, Layout, Format, Storage, Usage, Width, Height, Layers, Levels, Move(Data));
        }
        return ID;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::UpdateTexture(Object ID, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, AnyRef<Blob> Data)
    {
        ZY_ASSERT(mTextures.IsAllocated(ID), "Texture is not valid");

        Enqueue<& Driver::UpdateTexture>(ID, Level, Layer, X, Y, Width, Height, Pitch, Move(Data));
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

    void Service::Prepare(Object Pass, Text Name, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        Enqueue<& Driver::Prepare>(Pass, Name, Viewport, Sequence<Color, kMaxAttachments>(Colors), Depth, Stencil);

        // Commit submits the commands recorded from here on as this pass.
        Ref<InFlightPass> InFlightPass = GetInFlightPass();
        InFlightPass.Handle = Pass;
        InFlightPass.Cursor = mFrames[mProducer].Commands.GetSize();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Commit()
    {
        Ref<InFlightPass> InFlightPass = GetInFlightPass();

        if (const UInt32 Size = mFrames[mProducer].Commands.GetSize(); Size > InFlightPass.Cursor)
        {
            GetProducer().Record<& Service::SubmitInFlightPass>(this, InFlightPass.Cursor, Size - InFlightPass.Cursor);
        }
        Enqueue<& Driver::Commit>(InFlightPass.Handle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Setup(Ptr<void> Output, ConstRef<Configuration> Config)
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

        // The CPU takes the next frame while the GPU takes this one.
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

        UploadInFlightFrame(mFrames[mConsumer]);

        GetConsumer().Run();

        mFrames[mConsumer].Commands.Clear();

#if !defined(ZY_PLATFORM_WEB)
        // The frame goes back to the CPU already mapped, so its next round of writes needs no copy behind it.
        MapInFlightFrame(mFrames[mConsumer]);
#endif
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

#if defined(ZY_PLATFORM_WEB)
        // Every write goes through the CPU writer here, since the buffer is never mapped.
        Arena.Memory.Reserve(Capacity);
#endif

        mDriver->CreateBuffer(Arena.Buffer, Storage::Dynamic, Usage, Capacity, ConstSpan<Byte>());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::UpdateInFlightArena(Ref<InFlightArena> Arena, Usage Usage)
    {
#if !defined(ZY_PLATFORM_WEB)
        if (Arena.Mapping)
        {
            mDriver->UnmapBuffer(Arena.Buffer, Arena.Cursor);

            Arena.Mapping = nullptr;
        }
#endif

        if (Ref<Sequence<Byte>> Data = Arena.Memory; !Data.IsEmpty())
        {
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
                mDriver->UnmapBuffer(Arena.Buffer, Data.GetSize());
#endif
            }

            // The writer keeps its storage, so the next frame that spills reuses it.
            Data.Clear();
        }

        Arena.Cursor = 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::EvictInFlightArena(Ref<InFlightArena> Arena)
    {
        ZY_ASSERT(Arena.Memory.IsEmpty(), "An evicted arena writes through its memory alone");

        // What was written keeps its offsets, so every stream handed out before the spill still holds.
        Arena.Memory.Advance(Arena.Cursor);
        Blit(Arena.Memory.GetData(), Arena.Cursor, Arena.Mapping);

        Arena.Mapping = nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::MapInFlightArena(Ref<InFlightArena> Arena)
    {
        if (Arena.Buffer)
        {
            Arena.Mapping = mDriver->MapBuffer(Arena.Buffer, 0, Arena.Capacity);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::CreateInFlightFrame(Ref<InFlightFrame> Frame)
    {
        CreateInFlightArena(Frame.Vertices, Usage::Vertex,  8 * 1024 * 1024);
        CreateInFlightArena(Frame.Indices,  Usage::Index,   2 * 1024 * 1024);
        CreateInFlightArena(Frame.Uniforms, Usage::Uniform, 2 * 1024 * 1024);
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

    void Service::MapInFlightFrame(Ref<InFlightFrame> Frame)
    {
        // A frame whose buffers were never created, or a driver that maps nothing, leaves the CPU writer in charge.
        if (mDriver)
        {
            MapInFlightArena(Frame.Vertices);
            MapInFlightArena(Frame.Indices);
            MapInFlightArena(Frame.Uniforms);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RegisterBuiltinLoaders()
    {
        ConstRetainer<ZyContent::Service> Content = GetHost().GetService<ZyContent::Service>();

        Content->AddLoader(MSHLoader::kTypes, Retainer<MSHLoader>::Create());
        Content->AddLoader(MTLLoader::kTypes, Retainer<MTLLoader>::Create());
        Content->AddLoader(SHDLoader::kTypes, Retainer<SHDLoader>::Create());
        Content->AddLoader(TEXLoader::kTypes, Retainer<TEXLoader>::Create());
        Content->AddLoader(VFXLoader::kTypes, Retainer<VFXLoader>::Create(mDescription.Language, mDescription.Tier));
    }
}