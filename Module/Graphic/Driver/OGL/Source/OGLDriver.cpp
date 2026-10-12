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

#include "OGLDriver.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Driver::Driver()
        : mCompiler { mDevice },
          mBinder   { mDevice, mBuffers, mPipelines, mSamplers, mTextures }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Driver::~Driver()
    {
        // The service destroys the driver on the drawing thread; a context that never came up holds nothing to delete.
        if (!mContext.Acquire())
        {
            return;
        }

        for (Ref<Buffer> Buffer : mBuffers)
        {
            if (Buffer.GetHandle())
            {
                Buffer.Delete();
            }
        }

        for (Ref<Texture> Texture : mTextures)
        {
            if (Texture.GetHandle())
            {
                Texture.Delete();
            }
        }

        for (Ref<Pipeline> Pipeline : mPipelines)
        {
            Pipeline.Delete(mCompiler);
        }

        for (Ref<Pass> Pass : mPasses)
        {
            Pass.Delete();
        }

        for (Ref<Sampler> Sampler : mSamplers)
        {
            if (Sampler.GetHandle())
            {
                Sampler.Delete();
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Driver::Initialize(Ptr<void> Output, ConstRef<Configuration> Config)
    {
        if (!mLoader.Open())
        {
            LOG_E("Graphics: Failed to load the system's OpenGL library (is a GPU driver installed?)");
            return false;
        }

        if (!mContext.Initialize(Output, Config, mLoader, mDevice))
        {
            return false;
        }

        // The baseline comes first, since the version and the extensions are read through it.
        if (!mLoader.Load())
        {
            LOG_E("Graphics: The context lacks entry points every OpenGL 3.3 and OpenGL ES 3.0 context has");
            return false;
        }

        if (!mDevice.Initialize())
        {
            return false;
        }

        mLoader.Load(mDevice);
        mDevice.Describe();

        if (!mContext.Prepare(mDevice))
        {
            return false;
        }

        mCompiler.Initialize();

        // Made once the context is current and its functions are loaded, on the thread every command is run on.
        mProfiler.Initialize(mDevice);

        mPasses[kDisplay].Create(mContext.GetConfig());
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Reset(UInt16 Width, UInt16 Height, Bool Tearless)
    {
        mContext.SetTearless(Tearless);

        if (!mContext.Matches(Width, Height))
        {
            mContext.Resize(Width, Height);

            mPasses[kDisplay].Delete();
            mPasses[kDisplay].Create(mContext.GetConfig());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Probe(Ref<Description> Output) const
    {
        Output = mDevice.GetDescription();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data)
    {
        mBuffers[ID].Create(mDevice, Storage, Usage, Capacity, Data);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data)
    {
        mBuffers[ID].Update(Offset, Data);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::DeleteBuffer(Object ID)
    {
        mBuffers[ID].Delete();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> Driver::MapBuffer(Object ID, UInt32 Offset, UInt32 Size)
    {
        return mBuffers[ID].Map(Offset, Size);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::UnmapBuffer(Object ID, UInt32 Size)
    {
        mBuffers[ID].Unmap(Size);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth)
    {
        if (!mPasses[ID].Create(Colors, Depth, mTextures))
        {
            LOG_E("Graphics: Failed to create pass {0}", ID);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::DeletePass(Object ID)
    {
        mPasses[ID].Delete();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States)
    {
        if (!mPipelines[ID].Create(mDevice, mCompiler, Program, Signature, States))
        {
            LOG_E("Graphics: Failed to create pipeline {0}, its draws are skipped", ID);

            mPipelines[ID].Delete(mCompiler);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::DeletePipeline(Object ID)
    {
        mPipelines[ID].Delete(mCompiler);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::CreateSampler(Object ID, ZyGraphic::Sampler Descriptor)
    {
        mSamplers[ID].Create(mDevice, Descriptor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::DeleteSampler(Object ID)
    {
        mSamplers[ID].Delete();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage, Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data)
    {
        mTextures[ID].Create(mDevice, Layout, Format, Width, Height, Layers, Levels, Data);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::UpdateTexture(Object ID, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
    {
        mTextures[ID].Update(Level, Layer, X, Y, Width, Height, Pitch, Data);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::DeleteTexture(Object ID)
    {
        mTextures[ID].Delete();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Prepare(Object ID, Text Name, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        mProfiler.Open(Name);

        mPasses[ID].Prepare(mDevice, Viewport, Colors, Depth, Stencil);
        mBinder.Prepare(mPasses[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Submit(ConstSpan<Command> Commands)
    {
        // Whatever changed the context or reused an id happened before the pass's draws arrived, so they start from nothing.
        mBinder.Invalidate();

        for (ConstRef<Command> Draw : Commands)
        {
            Ref<Pipeline> Pipeline = mPipelines[Draw.Pipeline];

            // A program still linking is skipped rather than waited for, so a load never stalls the frame.
            if (!Pipeline.IsReady() && !Pipeline.Settle(mCompiler))
            {
                continue;
            }

            mBinder.Apply(Draw);

            const UInt32 Count     = Draw.Parameters.Count;
            const SInt32 Base      = Draw.Parameters.Base;
            const UInt32 Offset    = Draw.Parameters.Offset;
            const UInt32 Instances = Draw.Parameters.Instances;

            if (Draw.Indices.Buffer)
            {
                const GLenum         Type   = ConvertIndex(Draw.Indices.Stride);
                const ConstPtr<void> Cursor = reinterpret_cast<ConstPtr<void>>(
                    static_cast<UInt>(Draw.Indices.Offset) + static_cast<UInt>(Offset) * Draw.Indices.Stride);

                // Without base vertex entry points the binder has already moved the per-vertex streams by the base.
                const Bool Based = mDevice.Has(Feature::BaseVertex);

                if (Instances > 1)
                {
                    if (Based)
                    {
                        glDrawElementsInstancedBaseVertex(Pipeline.GetTopology(), Count, Type, Cursor, Instances, Base);
                    }
                    else
                    {
                        glDrawElementsInstanced(Pipeline.GetTopology(), Count, Type, Cursor, Instances);
                    }
                }
                else
                {
                    if (Based)
                    {
                        glDrawElementsBaseVertex(Pipeline.GetTopology(), Count, Type, Cursor, Base);
                    }
                    else
                    {
                        glDrawElements(Pipeline.GetTopology(), Count, Type, Cursor);
                    }
                }
            }
            else
            {
                const GLint First = static_cast<GLint>(Offset) + Base;

                if (Instances > 1)
                {
                    glDrawArraysInstanced(Pipeline.GetTopology(), First, Count, Instances);
                }
                else
                {
                    glDrawArrays(Pipeline.GetTopology(), First, Count);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Driver::Commit(Object ID)
    {
        mPasses[ID].Commit(mDevice);

        // Closed once the pass has resolved, so what it timed is the pass and not the wait for the display.
        mProfiler.Close();

        if (ID == kDisplay)
        {
            mContext.Present();

            // The frame is over, so whatever the queries have answered is gathered before the next one opens.
            mProfiler.Collect();
        }
    }
}