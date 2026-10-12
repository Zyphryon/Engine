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

#include "D3D11Binder.hpp"
#include "D3D11Pass.hpp"
#include "D3D11Profiler.hpp"
#include "Zyphryon.Graphic/Driver.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    /// \brief Represents the graphic driver implemented over Direct3D 11.
    class Driver final : public ZyGraphic::Driver
    {
    public:

        /// \brief Constructs a driver that holds no device until \ref Initialize creates one.
        Driver();

        /// \see ZyGraphic::Driver::Initialize(Ptr<void>, ConstRef<Configuration>)
        Bool Initialize(Ptr<void> Output, ConstRef<Configuration> Config) override;

        /// \see ZyGraphic::Driver::Reset(UInt16, UInt16, Bool)
        void Reset(UInt16 Width, UInt16 Height, Bool Tearless) override;

        /// \see ZyGraphic::Driver::Probe(Ref<Description>)
        void Probe(Ref<Description> Output) const override;

        /// \see ZyGraphic::Driver::CreateBuffer(Object, Storage, Usage, UInt32, ConstSpan<Byte>)
        void CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data) override;

        /// \see ZyGraphic::Driver::UpdateBuffer(Object, UInt32, ConstSpan<Byte>)
        void UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data) override;

        /// \see ZyGraphic::Driver::DeleteBuffer(Object)
        void DeleteBuffer(Object ID) override;

        /// \see ZyGraphic::Driver::MapBuffer(Object, UInt32, UInt32)
        Ptr<Byte> MapBuffer(Object ID, UInt32 Offset, UInt32 Size) override;

        /// \see ZyGraphic::Driver::UnmapBuffer(Object, UInt32)
        void UnmapBuffer(Object ID, UInt32 Size) override;

        /// \see ZyGraphic::Driver::CreatePass(Object, ConstSpan<ColorAttachment>, DepthAttachment)
        void CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth) override;

        /// \see ZyGraphic::Driver::DeletePass(Object)
        void DeletePass(Object ID) override;

        /// \see ZyGraphic::Driver::CreatePipeline(Object, ConstRef<Program>, ConstRef<Signature>, ConstRef<States>)
        void CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States) override;

        /// \see ZyGraphic::Driver::DeletePipeline(Object)
        void DeletePipeline(Object ID) override;

        /// \see ZyGraphic::Driver::CreateSampler(Object, Sampler)
        void CreateSampler(Object ID, ZyGraphic::Sampler Descriptor) override;

        /// \see ZyGraphic::Driver::DeleteSampler(Object)
        void DeleteSampler(Object ID) override;

        /// \see ZyGraphic::Driver::CreateTexture(Object, TextureLayout, TextureFormat, Storage, Usage, UInt16, UInt16, UInt16, UInt8, ConstSpan<Byte>)
        void CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data) override;

        /// \see ZyGraphic::Driver::UpdateTexture(Object, UInt8, UInt16, UInt16, UInt16, UInt16, UInt16, UInt32, ConstSpan<Byte>)
        void UpdateTexture(Object ID, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data) override;

        /// \see ZyGraphic::Driver::DeleteTexture(Object)
        void DeleteTexture(Object ID) override;

        /// \see ZyGraphic::Driver::Prepare(Object, Text, ConstRef<Viewport>, ConstSpan<Color>, Real32, UInt8)
        void Prepare(Object ID, Text Name, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil) override;

        /// \see ZyGraphic::Driver::Submit(ConstSpan<Command>)
        void Submit(ConstSpan<Command> Commands) override;

        /// \see ZyGraphic::Driver::Commit(Object)
        void Commit(Object ID) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Device                             mDevice;
        Swapchain                          mSwapchain;
        Compiler                           mCompiler;
        Profiler                           mProfiler;
        Binder                             mBinder;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Buffer,   kMaxBuffers   + 1> mBuffers;
        Array<Pass,     kMaxPasses    + 1> mPasses;
        Array<Pipeline, kMaxPipelines + 1> mPipelines;
        Array<Sampler,  kMaxSamplers  + 1> mSamplers;
        Array<Texture,  kMaxTextures  + 1> mTextures;
    };
}