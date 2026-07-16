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

#include "GLES3Driver.hpp"
#include "Zyphryon.Graphic/Format.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLES3Driver::~GLES3Driver()
    {
        for (Ref<GLES3Buffer> Buffer : mBuffers)
        {
            if (Buffer.Object)
            {
                glDeleteBuffers(1, AddressOf(Buffer.Object));
            }
        }

        for (Ref<GLES3Texture> Texture : mTextures)
        {
            if (Texture.Object)
            {
                if (Texture.Target == GL_RENDERBUFFER)
                {
                    glDeleteRenderbuffers(1, AddressOf(Texture.Object));
                }
                else
                {
                    glDeleteTextures(1, AddressOf(Texture.Object));
                }
            }
        }

        for (Ref<GLES3Pipeline> Pipeline : mPipelines)
        {
            if (Pipeline.Program)
            {
                glDeleteProgram(Pipeline.Program);
            }
        }

        for (Ref<GLES3Pass> Pass : mPasses)
        {
            if (Pass.Framebuffer)
            {
                glDeleteFramebuffers(1, AddressOf(Pass.Framebuffer));
            }
            if (Pass.Resolver)
            {
                glDeleteFramebuffers(1, AddressOf(Pass.Resolver));
            }
        }

        for (Ref<decltype(mSamplers)::Pair> Pair : mSamplers)
        {
            const GLuint Object = Pair.Second;
            glDeleteSamplers(1, AddressOf(Object));
        }

        if (mGlobalReadFramebuffer)
        {
            glDeleteFramebuffers(1, AddressOf(mGlobalReadFramebuffer));
        }
        if (mGlobalDrawFramebuffer)
        {
            glDeleteFramebuffers(1, AddressOf(mGlobalDrawFramebuffer));
        }
        if (mGlobalVAO)
        {
            glDeleteVertexArrays(1, AddressOf(mGlobalVAO));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool GLES3Driver::Initialize(Ptr<void> Output, ConstRef<Config> Config)
    {
        if (!mContext.Initialize(Output, Config))
        {
            LOG_E("GLES3Driver: Failed to initialize OpenGL context");
            return false;
        }

        LoadCapabilities();

        // Core profiles mandate a bound vertex array object; a single one is kept current for the driver's lifetime
        // while attribute pointers are re-specified per pipeline / vertex-stream change.
        glGenVertexArrays(1, AddressOf(mGlobalVAO));
        glBindVertexArray(mGlobalVAO);

        // Tightly-packed uploads, matching the engine's row layout, and the Direct3D winding convention.
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glFrontFace(GL_CW);

        const Bool HasDepth   = (Config.DepthFormat != TextureFormat::Unspecified);
        const Bool HasStencil = GetFormatDescription(Config.DepthFormat).IsStencil;

        Ref<GLES3Pass> Display = mPasses[kDisplay];
        Display.Framebuffer              = 0;
        Display.Width                    = Config.Width;
        Display.Height                   = Config.Height;
        Display.Depth.DepthLoadAction    = HasDepth   ? Action::Clear : Action::Discard;
        Display.Depth.DepthStoreAction   = Action::Discard;
        Display.Depth.StencilLoadAction  = HasStencil ? Action::Clear : Action::Discard;
        Display.Depth.StencilStoreAction = Action::Discard;

        Ref<ColorAttachment> Color = Display.Colors.Append();
        Color.LoadAction  = Action::Clear;
        Color.StoreAction = Action::Store;

        // Persistent framebuffers reused by CopyTexture as the blit read/draw endpoints.
        glGenFramebuffers(1, AddressOf(mGlobalReadFramebuffer));
        glGenFramebuffers(1, AddressOf(mGlobalDrawFramebuffer));
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::Reset(UInt16 Width, UInt16 Height)
    {
        mPasses[kDisplay].Width  = Width;
        mPasses[kDisplay].Height = Height;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::Probe(Ref<Description> Output) const
    {
        Output = mDescription;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data)
    {
        Ref<GLES3Buffer> Buffer = mBuffers[ID];
        Buffer.Usage    = GLES3Convert(Storage);
        Buffer.Capacity = (Usage == Usage::Uniform) ? Align(Capacity, mDescription.Capabilities.UniformBlockAlignment) : Capacity;

        glGenBuffers(1, AddressOf(Buffer.Object));
        glBindBuffer(GL_COPY_WRITE_BUFFER, Buffer.Object);
        glBufferData(GL_COPY_WRITE_BUFFER, Buffer.Capacity, Data.IsEmpty() ? nullptr : Data.GetData(), Buffer.Usage);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data)
    {
        glBindBuffer(GL_COPY_WRITE_BUFFER, mBuffers[ID].Object);
        glBufferSubData(GL_COPY_WRITE_BUFFER, Offset, Data.GetSizeInBytes(), Data.GetData());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::DeleteBuffer(Object ID)
    {
        // Deleting a buffer that is bound to GL_ARRAY_BUFFER resets that binding to zero.
        if (mSnapshot.Vertices == mBuffers[ID].Object)
        {
            mSnapshot.Vertices = 0;
        }

        glDeleteBuffers(1, AddressOf(mBuffers[ID].Object));
        mBuffers[ID] = GLES3Buffer();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, UInt32 Size)
    {
        glBindBuffer(GL_COPY_READ_BUFFER,  mBuffers[SrcBuffer].Object);
        glBindBuffer(GL_COPY_WRITE_BUFFER, mBuffers[DstBuffer].Object);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, SrcOffset, DstOffset, Size);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> GLES3Driver::MapBuffer(Object ID, UInt32 Offset, UInt32 Size)
    {
#if defined(ZY_PLATFORM_WEB)
        ZY_ASSERT(false, "GLES3Driver: MapBuffer is not supported on WebGL");
        return nullptr;
#else
        constexpr GLbitfield Access = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

        glBindBuffer(GL_COPY_WRITE_BUFFER, mBuffers[ID].Object);
        return static_cast<Ptr<Byte>>(glMapBufferRange(GL_COPY_WRITE_BUFFER, Offset, Size, Access));
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::UnmapBuffer(Object ID)
    {
#if defined(ZY_PLATFORM_WEB)
        ZY_ASSERT(false, "GLES3Driver: UnmapBuffer is not supported on WebGL");
#else
        glBindBuffer(GL_COPY_WRITE_BUFFER, mBuffers[ID].Object);
        glUnmapBuffer(GL_COPY_WRITE_BUFFER);
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth)
    {
        Ref<GLES3Pass> Pass = mPasses[ID];

        // All attachments share a common size, the mip-adjusted height of the first one flips window coordinates.
        if (!Colors.IsEmpty())
        {
            Pass.Width  = static_cast<UInt16>(Max(1, mTextures[Colors[0].Target].Width  >> Colors[0].TargetLevel));
            Pass.Height = static_cast<UInt16>(Max(1, mTextures[Colors[0].Target].Height >> Colors[0].TargetLevel));
        }
        else if (Depth.Target)
        {
            Pass.Width  = static_cast<UInt16>(Max(1, mTextures[Depth.Target].Width  >> Depth.TargetLevel));
            Pass.Height = static_cast<UInt16>(Max(1, mTextures[Depth.Target].Height >> Depth.TargetLevel));
        }

        glGenFramebuffers(1, AddressOf(Pass.Framebuffer));
        glBindFramebuffer(GL_FRAMEBUFFER, Pass.Framebuffer);

        Bool NeedsResolve = false;

        Sequence<GLenum, kMaxAttachments> Buffers;

        for (const ColorAttachment Color : Colors)
        {
            const GLenum           Point  = GL_COLOR_ATTACHMENT0 + Buffers.GetSize();
            ConstRef<GLES3Texture> Target = mTextures[Color.Target];

            Pass.Colors.Append(Color);

            if (Target.Target == GL_RENDERBUFFER)
            {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, Point, GL_RENDERBUFFER, Target.Object);
            }
            else
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, Point, GL_TEXTURE_2D, Target.Object, Color.TargetLevel);
            }

            Buffers.Append(Point);
            NeedsResolve |= (Color.Resolve != 0);
        }

        glDrawBuffers(Buffers.GetSize(), Buffers.GetData());

        if (Depth.Target)
        {
            ConstRef<GLES3Texture> Target = mTextures[Depth.Target];
            const GLenum           Point  = GetFormatDescription(Target.Format).IsStencil
                ? GL_DEPTH_STENCIL_ATTACHMENT
                : GL_DEPTH_ATTACHMENT;

            if (Target.Target == GL_RENDERBUFFER)
            {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, Point, GL_RENDERBUFFER, Target.Object);
            }
            else
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, Point, GL_TEXTURE_2D, Target.Object, Depth.TargetLevel);
            }

            Pass.Depth = Depth;                                         // Retain the attachment verbatim.
        }

        // Multisampled attachments are backed by render buffers, a companion framebuffer receives the resolve blit.
        if (NeedsResolve)
        {
            glGenFramebuffers(1, AddressOf(Pass.Resolver));
            glBindFramebuffer(GL_FRAMEBUFFER, Pass.Resolver);

            UInt32 Index = 0;
            for (ConstRef<ColorAttachment> Attachment : Pass.Colors)
            {
                if (Attachment.Resolve)
                {
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Index, GL_TEXTURE_2D,
                        mTextures[Attachment.Resolve].Object, Attachment.ResolveLevel);
                }
                ++Index;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::DeletePass(Object ID)
    {
        Ref<GLES3Pass> Pass = mPasses[ID];

        if (Pass.Framebuffer)
        {
            glDeleteFramebuffers(1, AddressOf(Pass.Framebuffer));
        }
        if (Pass.Resolver)
        {
            glDeleteFramebuffers(1, AddressOf(Pass.Resolver));
        }
        Pass = GLES3Pass();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<States> States)
    {
        Ref<GLES3Pipeline> Pipeline = mPipelines[ID];

        // Program.
        Pipeline.Program = mCompiler.Compile(Program);

        // Blend.
        Pipeline.BlendEnable        = !(
               States.BlendSrcColor      == BlendFactor::One
            && States.BlendSrcAlpha      == BlendFactor::One
            && States.BlendDstColor      == BlendFactor::Zero
            && States.BlendDstAlpha      == BlendFactor::Zero
            && States.BlendEquationColor == BlendFunction::Add
            && States.BlendEquationAlpha == BlendFunction::Add);
        Pipeline.BlendSrcColor      = GLES3Convert(States.BlendSrcColor);
        Pipeline.BlendDstColor      = GLES3Convert(States.BlendDstColor);
        Pipeline.BlendEquationColor = GLES3Convert(States.BlendEquationColor);
        Pipeline.BlendSrcAlpha      = GLES3Convert(States.BlendSrcAlpha);
        Pipeline.BlendDstAlpha      = GLES3Convert(States.BlendDstAlpha);
        Pipeline.BlendEquationAlpha = GLES3Convert(States.BlendEquationAlpha);
        Pipeline.Channel            = Enum::Cast(States.Channel);
        Pipeline.AlphaToCoverage    = States.AlphaToCoverage;

        // Depth.
        Pipeline.DepthEnable        = States.DepthTest != TestCondition::Always || States.DepthMask;
        Pipeline.DepthMask          = States.DepthMask;
        Pipeline.DepthFunction      = GLES3Convert(States.DepthTest);
        Pipeline.DepthClip          = States.DepthClip;
        Pipeline.DepthBias          = States.DepthBias;
        Pipeline.DepthBiasSlope     = States.DepthBiasSlope;

        // Stencil.
        Pipeline.StencilEnable        = States.StencilFrontTest      != TestCondition::Always
                                     || States.StencilFrontFail      != TestAction::Keep
                                     || States.StencilFrontDepthFail != TestAction::Keep
                                     || States.StencilFrontDepthPass != TestAction::Keep
                                     || States.StencilBackTest       != TestCondition::Always
                                     || States.StencilBackFail       != TestAction::Keep
                                     || States.StencilBackDepthFail  != TestAction::Keep
                                     || States.StencilBackDepthPass  != TestAction::Keep;
        Pipeline.StencilReadMask      = States.StencilReadMask;
        Pipeline.StencilWriteMask     = States.StencilWriteMask;
        Pipeline.StencilFrontFunction = GLES3Convert(States.StencilFrontTest);
        Pipeline.StencilFrontFail     = GLES3Convert(States.StencilFrontFail);
        Pipeline.StencilFrontDepthFail= GLES3Convert(States.StencilFrontDepthFail);
        Pipeline.StencilFrontDepthPass= GLES3Convert(States.StencilFrontDepthPass);
        Pipeline.StencilBackFunction  = GLES3Convert(States.StencilBackTest);
        Pipeline.StencilBackFail      = GLES3Convert(States.StencilBackFail);
        Pipeline.StencilBackDepthFail = GLES3Convert(States.StencilBackDepthFail);
        Pipeline.StencilBackDepthPass = GLES3Convert(States.StencilBackDepthPass);

        // Rasterizer.
        Pipeline.CullEnable    = States.Cull != Cull::None;
        Pipeline.CullMode      = GLES3Convert(States.Cull);
        Pipeline.ScissorEnable = States.Scissor;
        Pipeline.FillMode      = States.Fill == Fill::Wireframe ? 0x1B01 : 0x1B02;

        // Input assembler.
        Pipeline.Primitive = GLES3Convert(States.InputPrimitive);

        for (ConstRef<Attribute> Attribute : States.InputAttributes)
        {
            Pipeline.Attributes.Append(Attribute);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::DeletePipeline(Object ID)
    {
        if (mPipelines[ID].Program)
        {
            glDeleteProgram(mPipelines[ID].Program);
        }
        mPipelines[ID] = GLES3Pipeline();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, ConstSpan<Byte> Data)
    {
        Ref<GLES3Texture> Texture = mTextures[ID];
        Texture.Format  = Format;
        Texture.Width   = Width;
        Texture.Height  = Height;
        Texture.Samples = Enum::Cast(Samples);

        const GLES3Format Description = GLES3Convert(Format);

        // Multisampled render targets are represented as render buffers so the same path serves ES 3.0.
        if (Samples != Multisample::X1 && HasBit(Usage, Usage::Target) && !HasBit(Usage, Usage::Sample))
        {
            Texture.Target = GL_RENDERBUFFER;

            glGenRenderbuffers(1, AddressOf(Texture.Object));
            glBindRenderbuffer(GL_RENDERBUFFER, Texture.Object);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, Texture.Samples, Description.Internal, Width, Height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            return;
        }

        Texture.Target = GL_TEXTURE_2D;
        glGenTextures(1, AddressOf(Texture.Object));
        glBindTexture(GL_TEXTURE_2D, Texture.Object);

        const TextureFormatDescription FormatDescription = GetFormatDescription(Format);

        ConstPtr<Byte> Bytes  = Data.GetData();
        const UInt8    Levels = Max<UInt8>(1, Mipmaps);

        for (UInt8 Level = 0; Level < Levels; ++Level)
        {
            const UInt32 BlockWidth  = (Width  + FormatDescription.BlockSize - 1) / FormatDescription.BlockSize;
            const UInt32 BlockHeight = (Height + FormatDescription.BlockSize - 1) / FormatDescription.BlockSize;
            const UInt32 Size        = BlockWidth
                * BlockHeight
                * (FormatDescription.BitsPerPixel * FormatDescription.BlockSize * FormatDescription.BlockSize / 8);

            if (Description.Compressed)
            {
                glCompressedTexImage2D(GL_TEXTURE_2D, Level, Description.Internal, Width, Height, 0, Size, Bytes);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, Level, Description.Internal, Width, Height, 0, Description.External, Description.Type, Bytes);
            }

            if (Bytes)
            {
                Bytes += Size;
            }
            Width  = Max<UInt16>(1, Width  >> 1);
            Height = Max<UInt16>(1, Height >> 1);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  Levels - 1);

        // A request for a full chain without explicit level data derives the remaining levels from the base image.
        if (Mipmaps < 1 && Bytes)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
    {
        ConstRef<GLES3Texture> Texture     = mTextures[ID];
        const GLES3Format      Description = GLES3Convert(Texture.Format);

        glBindTexture(GL_TEXTURE_2D, Texture.Object);

        if (Description.Compressed)
        {
            const TextureFormatDescription FormatDescription = GetFormatDescription(Texture.Format);

            const UInt32 BlockWidth  = (Width  + FormatDescription.BlockSize - 1) / FormatDescription.BlockSize;
            const UInt32 BlockHeight = (Height + FormatDescription.BlockSize - 1) / FormatDescription.BlockSize;
            const UInt32 Size        = BlockWidth
                * BlockHeight
                * (FormatDescription.BitsPerPixel * FormatDescription.BlockSize * FormatDescription.BlockSize / 8);

            glCompressedTexSubImage2D(
                GL_TEXTURE_2D, Level, X, Y, Width, Height, Description.Internal, Size, Data.GetData());
        }
        else
        {
            const UInt32 BytesPerPixel = GetFormatDescription(Texture.Format).BitsPerPixel / 8;

            glPixelStorei(GL_UNPACK_ROW_LENGTH, BytesPerPixel ? Pitch / BytesPerPixel : 0);
            glTexSubImage2D(
                GL_TEXTURE_2D, Level, X, Y, Width, Height, Description.External, Description.Type, Data.GetData());
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::DeleteTexture(Object ID)
    {
        Ref<GLES3Texture> Texture = mTextures[ID];

        if (Texture.Target == GL_RENDERBUFFER)
        {
            glDeleteRenderbuffers(1, AddressOf(Texture.Object));
        }
        else
        {
            glDeleteTextures(1, AddressOf(Texture.Object));
        }
        Texture = GLES3Texture();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, UInt16 Width, UInt16 Height)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mGlobalReadFramebuffer);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[SrcTexture].Object, SrcLevel);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mGlobalDrawFramebuffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[DstTexture].Object, DstLevel);

        glBlitFramebuffer(SrcX, SrcY, SrcX + Width, SrcY + Height, DstX, DstY, DstX + Width, DstY + Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        Ref<GLES3Pass> Target = mPasses[Pass];
        glBindFramebuffer(GL_FRAMEBUFFER, Target.Framebuffer);

        mSnapshot.Pass = AddressOf(Target);

        glViewport(
            static_cast<GLint>(Viewport.X),
            static_cast<GLint>(Target.Height - (Viewport.Y + Viewport.Height)),
            static_cast<GLsizei>(Viewport.Width),
            static_cast<GLsizei>(Viewport.Height));

#if defined(ZY_PLATFORM_WEB)
        glDepthRangef(Viewport.MinDepth, Viewport.MaxDepth);
#else
        glDepthRange(Viewport.MinDepth, Viewport.MaxDepth);
#endif

        // Clears must ignore the pipeline scissor and write masks; the subsequent pipeline bind restores them.
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glStencilMask(0xFF);

        // Keep the pipeline-state cache consistent with the masks the clear just forced.
        mSnapshot.Pipeline.ScissorEnable    = false;
        mSnapshot.Pipeline.Channel          = 0x0F;
        mSnapshot.Pipeline.DepthMask        = true;
        mSnapshot.Pipeline.StencilWriteMask = 0xFF;

        for (UInt32 Index = 0; Index < Target.Colors.GetSize(); ++Index)
        {
            if (Target.Colors[Index].LoadAction == Action::Clear && Index < Colors.GetSize())
            {
                glClearBufferfv(GL_COLOR, Index, reinterpret_cast<ConstPtr<GLfloat>>(AddressOf(Colors[Index])));
            }
        }

        const Bool ClearDepth   = Target.Depth.DepthLoadAction   == Action::Clear;
        const Bool ClearStencil = Target.Depth.StencilLoadAction == Action::Clear;

        if (ClearDepth && ClearStencil)
        {
            glClearBufferfi(GL_DEPTH_STENCIL, 0, Depth, Stencil);
        }
        else if (ClearDepth)
        {
            glClearBufferfv(GL_DEPTH, 0, AddressOf(Depth));
        }
        else if (ClearStencil)
        {
            const GLint Value = Stencil;
            glClearBufferiv(GL_STENCIL, 0, AddressOf(Value));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::Submit(ConstSpan<Command> Commands)
    {
        static constexpr Command kEmptyCommand { };

        for (UInt32 Batch = 0; Batch < Commands.GetSize(); ++Batch)
        {
            ConstRef<Command> Newest = Commands[Batch];
            ConstRef<Command> Oldest = Batch > 0 ? Commands[Batch - 1] : kEmptyCommand;

            ConstRef<GLES3Pipeline> Pipeline = mPipelines[Newest.Pipeline];

            const Bool IsPipelineDirty = (Batch == 0) || (Oldest.Pipeline != Newest.Pipeline);

            if (IsPipelineDirty || Oldest.Stencil != Newest.Stencil)
            {
                ApplyPipeline(Pipeline, Newest.Stencil);
            }

            // Scissor rectangle (only meaningful while the pipeline enables the scissor test).
            if (Pipeline.ScissorEnable)
            {
                const Bool IsScissorDirty = IsPipelineDirty
                    || Oldest.Scissor.X      != Newest.Scissor.X
                    || Oldest.Scissor.Y      != Newest.Scissor.Y
                    || Oldest.Scissor.Width  != Newest.Scissor.Width
                    || Oldest.Scissor.Height != Newest.Scissor.Height;

                if (IsScissorDirty)
                {
                    const GLint Y = mSnapshot.Pass->Height - (Newest.Scissor.Y + Newest.Scissor.Height);
                    glScissor(Newest.Scissor.X, Y, Newest.Scissor.Width, Newest.Scissor.Height);
                }
            }

            // Vertex streams and the attribute layout that reads from them.
            Bool VerticesChanged = IsPipelineDirty || Oldest.Vertices.GetSize() != Newest.Vertices.GetSize();

            for (UInt32 Index = 0; !VerticesChanged && Index < Newest.Vertices.GetSize(); ++Index)
            {
                VerticesChanged =
                       Oldest.Vertices[Index].Buffer != Newest.Vertices[Index].Buffer
                    || Oldest.Vertices[Index].Offset != Newest.Vertices[Index].Offset
                    || Oldest.Vertices[Index].Stride != Newest.Vertices[Index].Stride;
            }
            if (VerticesChanged)
            {
                ApplyVertexResources(Pipeline, Newest, IsPipelineDirty);
            }

            // Index stream (part of the vertex-array element binding).
            if (IsPipelineDirty
                || Oldest.Indices.Buffer != Newest.Indices.Buffer
                || Oldest.Indices.Offset != Newest.Indices.Offset)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Newest.Indices.Buffer ? mBuffers[Newest.Indices.Buffer].Object : 0);
            }

            ApplyUniformResources(Oldest, Newest);
            ApplyTextureResources(Oldest, Newest);

            // Issue draw command.
            const UInt32 Count     = Newest.Parameters.Count;
            const SInt32 Base      = Newest.Parameters.Base;
            const UInt32 Offset    = Newest.Parameters.Offset;
            const UInt32 Instances = Newest.Parameters.Instances;

            if (Newest.Indices.Buffer)
            {
                const GLenum         Type   = GLES3ConvertIndex(Newest.Indices.Stride);
                const ConstPtr<void> Cursor = reinterpret_cast<ConstPtr<void>>(
                    static_cast<UInt>(Newest.Indices.Offset) + static_cast<UInt>(Offset) * Newest.Indices.Stride);

                if (Instances > 1)
                {
#if defined(ZY_PLATFORM_WEB)
                    glDrawElementsInstanced(Pipeline.Primitive, Count, Type, Cursor, Instances);
#else
                    glDrawElementsInstancedBaseVertex(Pipeline.Primitive, Count, Type, Cursor, Instances, Base);
#endif
                }
                else
                {
#if defined(ZY_PLATFORM_WEB)
                    glDrawElements(Pipeline.Primitive, Count, Type, Cursor);
#else
                    glDrawElementsBaseVertex(Pipeline.Primitive, Count, Type, Cursor, Base);
#endif
                }
            }
            else
            {
                const GLint First = static_cast<GLint>(Offset) + Base;

                if (Instances > 1)
                {
                    glDrawArraysInstanced(Pipeline.Primitive, First, Count, Instances);
                }
                else
                {
                    glDrawArrays(Pipeline.Primitive, First, Count);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::Commit(Object Pass)
    {
        Ref<GLES3Pass> Target = mPasses[Pass];

        // Resolve multisampled color attachments into their single-sample companions.
        if (Target.Resolver)
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, Target.Framebuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Target.Resolver);

            UInt32 Index = 0;
            for (ConstRef<ColorAttachment> Attachment : Target.Colors)
            {
                if (Attachment.Resolve && Attachment.StoreAction == Action::Store)
                {
                    const GLenum Buffer = GL_COLOR_ATTACHMENT0 + Index;

                    glReadBuffer(Buffer);
                    glDrawBuffers(1, AddressOf(Buffer));

                    ConstRef<GLES3Texture> Source = mTextures[Attachment.Target];
                    glBlitFramebuffer(
                        0,  0, Source.Width, Source.Height,
                        0,  0, Source.Width, Source.Height,
                        GL_COLOR_BUFFER_BIT, GL_NEAREST);
                }
                ++Index;
            }
        }

        // Present the default framebuffer when committing the display pass.
        if (Pass == kDisplay)
        {
            mContext.Present();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::LoadCapabilities()
    {
        mDescription.Backend  = "GLES3";
        mDescription.Language = ShaderLanguage::GLSL;

        GLint Major = 0;
        GLint Minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, AddressOf(Major));
        glGetIntegerv(GL_MINOR_VERSION, AddressOf(Minor));

        Ref<Capabilities> Limits = mDescription.Capabilities;

#if defined(ZY_PLATFORM_WEB)
        const Bool ExtendedTier = (Major > 3) || (Major == 3 && Minor >= 1);   // OpenGL ES 3.1+

        Limits.SupportsBaseVertex = HasExtension("GL_OES_draw_elements_base_vertex")
                                 || HasExtension("GL_EXT_draw_elements_base_vertex")
                                 || HasExtension("WEBGL_draw_instanced_base_vertex_base_instance");

        Limits.SupportsFormatETC2 = true;                                      // Guaranteed by OpenGL ES 3.0.
#else
        const Bool ExtendedTier = (Major > 4) || (Major == 4 && Minor >= 3);   // OpenGL 4.3+
        Limits.SupportsBaseVertex = true;                                      // Core since OpenGL 3.2.
        Limits.SupportsFormatRGTC = true;                                      // Core since OpenGL 3.0.
        Limits.SupportsFormatETC2 = true;                                      // Core since OpenGL 4.3.
#endif

        mDescription.Tier = ExtendedTier ? Tier::Level3 : Tier::Level2;

        // Extension-gated features.
        if (HasExtension("GL_EXT_texture_filter_anisotropic") || HasExtension("GL_ARB_texture_filter_anisotropic"))
        {
            GLfloat MaxAnisotropy = 1.0f;
            glGetFloatv(0x84FF, AddressOf(MaxAnisotropy));

            Limits.MaxAnisotropy = static_cast<UInt8>(MaxAnisotropy);
        }

        Limits.SupportsBorderClamp =
               HasExtension("GL_EXT_texture_border_clamp")
            || HasExtension("GL_OES_texture_border_clamp");
        Limits.SupportsFormatS3TC  = HasExtension("GL_EXT_texture_compression_s3tc");
        Limits.SupportsFormatBPTC  =
               HasExtension("GL_EXT_texture_compression_bptc")
            || HasExtension("GL_ARB_texture_compression_bptc");
        Limits.SupportsFormatRGTC  = Limits.SupportsFormatRGTC
            || HasExtension("GL_EXT_texture_compression_rgtc")
            || HasExtension("GL_ARB_texture_compression_rgtc");

#if !defined(ZY_PLATFORM_WEB)
        Limits.SupportsBorderClamp = true; // Core since OpenGL 1.3.
#endif

        // Device limits.
        GLint Value = 0;

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, AddressOf(Value));
        Limits.MaxTextureDimension = Value;

        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, AddressOf(Value));
        Limits.MaxTextureLayers  = Value;
        Limits.MaxTextureMipmaps = kMaxMipmaps;

        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, AddressOf(Value));
        Limits.MaxTextureSlots = Value;

        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, AddressOf(Value));
        Limits.MaxRenderTargets = Value;

        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, AddressOf(Value));
        Limits.MaxVertexAttributes = Value;
        Limits.MaxVertexStreams    = Command::kMaxVertices;

        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, AddressOf(Value));
        Limits.UniformBlockAlignment = static_cast<UInt16>(Value);

        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, AddressOf(Value));
        Limits.UniformBlockCapacity  = Value;

        // Adapter description.
        Ref<Adapter> Adapter = mDescription.Endpoints.Append();
        Adapter.Description  = StrConvert(reinterpret_cast<ConstPtr<Char>>(glGetString(GL_RENDERER)));
        Adapter.Memory       = 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool GLES3Driver::HasExtension(ConstPtr<Char> Name) const
    {
        GLint Count = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, AddressOf(Count));

        for (GLint Index = 0; Index < Count; ++Index)
        {
            if (ConstPtr<Char> Extension = reinterpret_cast<ConstPtr<Char>>(glGetStringi(GL_EXTENSIONS, Index)))
            {
                while (* Extension && (* Extension == * Name))
                {
                    ++Extension;
                    ++Name;
                }
                return (* Extension == * Name);
            }
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::ApplyPipeline(ConstRef<GLES3Pipeline> Pipeline, UInt8 Stencil)
    {
        Ref<GLES3Pipeline> State = mSnapshot.Pipeline;

        if (State.Program != Pipeline.Program)
        {
            glUseProgram(Pipeline.Program);
            State.Program = Pipeline.Program;
        }

        // Blend.
        if (State.BlendEnable != Pipeline.BlendEnable)
        {
            Pipeline.BlendEnable ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
            State.BlendEnable = Pipeline.BlendEnable;
        }
        if (Pipeline.BlendEnable)
        {
            if (State.BlendSrcColor != Pipeline.BlendSrcColor || State.BlendDstColor != Pipeline.BlendDstColor
             || State.BlendSrcAlpha != Pipeline.BlendSrcAlpha || State.BlendDstAlpha != Pipeline.BlendDstAlpha)
            {
                glBlendFuncSeparate(Pipeline.BlendSrcColor, Pipeline.BlendDstColor, Pipeline.BlendSrcAlpha, Pipeline.BlendDstAlpha);
                State.BlendSrcColor = Pipeline.BlendSrcColor;
                State.BlendDstColor = Pipeline.BlendDstColor;
                State.BlendSrcAlpha = Pipeline.BlendSrcAlpha;
                State.BlendDstAlpha = Pipeline.BlendDstAlpha;
            }
            if (State.BlendEquationColor != Pipeline.BlendEquationColor || State.BlendEquationAlpha != Pipeline.BlendEquationAlpha)
            {
                glBlendEquationSeparate(Pipeline.BlendEquationColor, Pipeline.BlendEquationAlpha);
                State.BlendEquationColor = Pipeline.BlendEquationColor;
                State.BlendEquationAlpha = Pipeline.BlendEquationAlpha;
            }
        }

        if (State.Channel != Pipeline.Channel)
        {
            glColorMask(
                (Pipeline.Channel & 0x01) ? GL_TRUE : GL_FALSE,
                (Pipeline.Channel & 0x02) ? GL_TRUE : GL_FALSE,
                (Pipeline.Channel & 0x04) ? GL_TRUE : GL_FALSE,
                (Pipeline.Channel & 0x08) ? GL_TRUE : GL_FALSE);
            State.Channel = Pipeline.Channel;
        }

        if (State.AlphaToCoverage != Pipeline.AlphaToCoverage)
        {
            Pipeline.AlphaToCoverage ? glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE) : glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
            State.AlphaToCoverage = Pipeline.AlphaToCoverage;
        }

        // Depth.
        if (State.DepthEnable != Pipeline.DepthEnable)
        {
            Pipeline.DepthEnable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
            State.DepthEnable = Pipeline.DepthEnable;
        }
        if (State.DepthMask != Pipeline.DepthMask)
        {
            glDepthMask(Pipeline.DepthMask ? GL_TRUE : GL_FALSE);
            State.DepthMask = Pipeline.DepthMask;
        }
        if (State.DepthFunction != Pipeline.DepthFunction)
        {
            glDepthFunc(Pipeline.DepthFunction);
            State.DepthFunction = Pipeline.DepthFunction;
        }

#if !defined(ZY_PLATFORM_WEB)
        if (State.DepthClip != Pipeline.DepthClip)
        {
            Pipeline.DepthClip ? glDisable(GL_DEPTH_CLAMP) : glEnable(GL_DEPTH_CLAMP);
            State.DepthClip = Pipeline.DepthClip;
        }
#endif

        if (State.DepthBias != Pipeline.DepthBias || State.DepthBiasSlope != Pipeline.DepthBiasSlope)
        {
            if (Pipeline.DepthBias != 0.0f || Pipeline.DepthBiasSlope != 0.0f)
            {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(Pipeline.DepthBiasSlope, Pipeline.DepthBias);
            }
            else
            {
                glDisable(GL_POLYGON_OFFSET_FILL);
            }
            State.DepthBias      = Pipeline.DepthBias;
            State.DepthBiasSlope = Pipeline.DepthBiasSlope;
        }

        if (State.StencilEnable != Pipeline.StencilEnable)
        {
            Pipeline.StencilEnable ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
            State.StencilEnable = Pipeline.StencilEnable;
        }
        if (Pipeline.StencilEnable)
        {
            if (mSnapshot.Stencil != Stencil
                || State.StencilFrontFunction != Pipeline.StencilFrontFunction
                || State.StencilBackFunction  != Pipeline.StencilBackFunction
                || State.StencilReadMask      != Pipeline.StencilReadMask)
            {
                glStencilFuncSeparate(GL_FRONT, Pipeline.StencilFrontFunction, Stencil, Pipeline.StencilReadMask);
                glStencilFuncSeparate(GL_BACK,  Pipeline.StencilBackFunction,  Stencil, Pipeline.StencilReadMask);
                State.StencilFrontFunction = Pipeline.StencilFrontFunction;
                State.StencilBackFunction  = Pipeline.StencilBackFunction;
                State.StencilReadMask      = Pipeline.StencilReadMask;
                mSnapshot.Stencil          = Stencil;
            }
            if (State.StencilFrontFail != Pipeline.StencilFrontFail
             || State.StencilFrontDepthFail != Pipeline.StencilFrontDepthFail
             || State.StencilFrontDepthPass != Pipeline.StencilFrontDepthPass
             || State.StencilBackFail != Pipeline.StencilBackFail
             || State.StencilBackDepthFail != Pipeline.StencilBackDepthFail
             || State.StencilBackDepthPass != Pipeline.StencilBackDepthPass)
            {
                glStencilOpSeparate(GL_FRONT, Pipeline.StencilFrontFail, Pipeline.StencilFrontDepthFail, Pipeline.StencilFrontDepthPass);
                glStencilOpSeparate(GL_BACK,  Pipeline.StencilBackFail,  Pipeline.StencilBackDepthFail,  Pipeline.StencilBackDepthPass);
                State.StencilFrontFail      = Pipeline.StencilFrontFail;
                State.StencilFrontDepthFail = Pipeline.StencilFrontDepthFail;
                State.StencilFrontDepthPass = Pipeline.StencilFrontDepthPass;
                State.StencilBackFail       = Pipeline.StencilBackFail;
                State.StencilBackDepthFail  = Pipeline.StencilBackDepthFail;
                State.StencilBackDepthPass  = Pipeline.StencilBackDepthPass;
            }
            if (State.StencilWriteMask != Pipeline.StencilWriteMask)
            {
                glStencilMask(Pipeline.StencilWriteMask);
                State.StencilWriteMask = Pipeline.StencilWriteMask;
            }
        }

        if (State.CullEnable != Pipeline.CullEnable)
        {
            Pipeline.CullEnable ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
            State.CullEnable = Pipeline.CullEnable;
        }
        if (Pipeline.CullEnable && (State.CullMode != Pipeline.CullMode))
        {
            glCullFace(Pipeline.CullMode);
            State.CullMode = Pipeline.CullMode;
        }

        if (State.ScissorEnable != Pipeline.ScissorEnable)
        {
            Pipeline.ScissorEnable ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
            State.ScissorEnable = Pipeline.ScissorEnable;
        }

#if !defined(ZY_PLATFORM_WEB)
        if (State.FillMode != Pipeline.FillMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, Pipeline.FillMode);
            State.FillMode = Pipeline.FillMode;
        }
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::ApplyVertexResources(ConstRef<GLES3Pipeline> Pipeline, ConstRef<Command> Command, Bool IsPipelineDirty)
    {
        UInt32 Enabled = 0;

        for (ConstRef<Attribute> Attribute : Pipeline.Attributes)
        {
            if (Attribute.Stream >= Command.Vertices.GetSize())
            {
                continue;
            }

            ConstRef<Stream> Stream = Command.Vertices[Attribute.Stream];
            Enabled |= (1u << Attribute.Location);

            if (const GLuint Object = Stream.Buffer ? mBuffers[Stream.Buffer].Object : 0; mSnapshot.Vertices != Object)
            {
                glBindBuffer(GL_ARRAY_BUFFER, Object);
                mSnapshot.Vertices = Object;
            }

            if (IsPipelineDirty)
            {
                glEnableVertexAttribArray(Attribute.Location);
                glVertexAttribDivisor(Attribute.Location, Attribute.Divisor);
            }

            const GLES3Attribute Format = GLES3Convert(Attribute.Format);
            const ConstPtr<void> Cursor = reinterpret_cast<ConstPtr<void>>(
                static_cast<UInt>(Stream.Offset) + Attribute.Offset);

            if (Format.Integer)
            {
                glVertexAttribIPointer(Attribute.Location, Format.Size, Format.Type, Stream.Stride, Cursor);
            }
            else
            {
                glVertexAttribPointer(Attribute.Location, Format.Size, Format.Type, Format.Normalized, Stream.Stride, Cursor);
            }
        }

        // The enable-set changes only with the pipeline; disable whatever a previously bound pipeline left enabled.
        if (IsPipelineDirty)
        {
            for (UInt32 Location = 0, Stale = mSnapshot.Attributes & ~Enabled; Stale; ++Location, Stale >>= 1)
            {
                if (Stale & 1u)
                {
                    glDisableVertexAttribArray(Location);
                }
            }
            mSnapshot.Attributes = Enabled;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::ApplyUniformResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const
    {
        for (UInt32 Slot = 0; Slot < Command::kMaxUniforms; ++Slot)
        {
            ConstRef<Stream> Old = Oldest.Uniforms[Slot];
            ConstRef<Stream> New = Newest.Uniforms[Slot];

            if (Old.Buffer == New.Buffer && Old.Offset == New.Offset && Old.Stride == New.Stride)
            {
                continue;
            }

            if (New.Buffer)
            {
                glBindBufferRange(GL_UNIFORM_BUFFER, Slot, mBuffers[New.Buffer].Object, New.Offset, New.Stride);
            }
            else
            {
                glBindBufferBase(GL_UNIFORM_BUFFER, Slot, 0);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Driver::ApplyTextureResources(ConstRef<Command> Oldest, ConstRef<Command> Newest)
    {
        const UInt32 OldTextures = Oldest.Textures.GetSize();
        const UInt32 NewTextures = Newest.Textures.GetSize();

        for (UInt32 Slot = 0; Slot < NewTextures; ++Slot)
        {
            const Object Old = Slot < OldTextures ? Oldest.Textures[Slot] : Object { };
            const Object New = Newest.Textures[Slot];

            if (Old != New)
            {
                ConstRef<GLES3Texture> Texture = mTextures[New];

                glActiveTexture(GL_TEXTURE0 + Slot);
                glBindTexture(GL_TEXTURE_2D, Texture.Object);
            }
        }

        const UInt32 OldSamplers = Oldest.Samplers.GetSize();
        const UInt32 NewSamplers = Newest.Samplers.GetSize();

        for (UInt32 Slot = 0; Slot < NewSamplers; ++Slot)
        {
            const Sampler Old = Slot < OldSamplers ? Oldest.Samplers[Slot] : Sampler { };
            const Sampler New = Newest.Samplers[Slot];

            if (Slot >= OldSamplers || Hash(Old) != Hash(New))
            {
                glBindSampler(Slot, GetOrCreateSampler(New));
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLuint GLES3Driver::GetOrCreateSampler(Sampler Descriptor)
    {
        Ref<GLuint> Object = mSamplers.FindOrInsert(Hash(Descriptor));

        if (Object == 0)
        {
            glGenSamplers(1, AddressOf(Object));
            glSamplerParameteri(Object, GL_TEXTURE_WRAP_S, GLES3Convert(Descriptor.AddressModeU));
            glSamplerParameteri(Object, GL_TEXTURE_WRAP_T, GLES3Convert(Descriptor.AddressModeV));
            glSamplerParameteri(Object, GL_TEXTURE_WRAP_R, GLES3Convert(Descriptor.AddressModeW));
            glSamplerParameteri(Object, GL_TEXTURE_MIN_FILTER, GLES3ConvertMinFilter(Descriptor.Filter));
            glSamplerParameteri(Object, GL_TEXTURE_MAG_FILTER, GLES3ConvertMagFilter(Descriptor.Filter));

            if (Descriptor.Comparison != TestCondition::Always)
            {
                glSamplerParameteri(Object, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                glSamplerParameteri(Object, GL_TEXTURE_COMPARE_FUNC, GLES3Convert(Descriptor.Comparison));
            }
            else
            {
                glSamplerParameteri(Object, GL_TEXTURE_COMPARE_MODE, GL_NONE);
            }

            if (mDescription.Capabilities.SupportsBorderClamp)
            {
                glSamplerParameterfv(Object, 0x1004, GLES3Convert(Descriptor.Border));
            }

            if (mDescription.Capabilities.MaxAnisotropy > 0)
            {
                const Real32 Anisotropy = Min(
                    GLES3ConvertAnisotropy(Descriptor.Filter), mDescription.Capabilities.MaxAnisotropy);
                glSamplerParameterf(Object, 0x84FE, Anisotropy);
            }
        }
        return Object;
    }
}
