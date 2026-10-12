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

#include "OGLBinder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    static Bool Differ(Bool Valid, ConstSpan<Type> Old, ConstSpan<Type> New, UInt32 Limit, Ref<UInt32> First, Ref<UInt32> Count)
    {
        // A slot past the end of a list counts as empty, so a shorter list clears what a longer one bound.
        UInt32 Last = 0;
        First = Limit;

        for (UInt32 Slot = 0; Slot < Limit; ++Slot)
        {
            const Type Before = (Slot < Old.GetSize() ? Old[Slot] : Type { });
            const Type After  = (Slot < New.GetSize() ? New[Slot] : Type { });

            if (!Valid || Before != After)
            {
                First = Min(First, Slot);
                Last  = Slot + 1;
            }
        }
        Count = (Last > First ? Last - First : 0);
        return Count > 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt8 Divisor(ConstRef<Attributes> Layout, UInt32 Stream)
    {
        for (ConstRef<Attribute> Attribute : Layout)
        {
            if (Attribute.Stream == Stream)
            {
                return Attribute.Divisor;
            }
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt Locate(ConstRef<Stream> Stream, UInt8 Divisor, SInt32 Base)
    {
        // A per-instance stream is indexed by instance, which the base vertex never moves.
        const SInt64 Shift = (Divisor ? 0 : static_cast<SInt64>(Base) * Stream.Stride);
        return static_cast<UInt>(static_cast<SInt64>(Stream.Offset) + Shift);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void Point(ConstRef<Attribute> Attribute, ConstRef<Stream> Stream, SInt32 Base)
    {
        const AttributeFormat Description = Convert(Attribute.Format);
        const ConstPtr<void>  Cursor      = reinterpret_cast<ConstPtr<void>>(Locate(Stream, Attribute.Divisor, Base) + Attribute.Offset);

        if (Description.Integer)
        {
            glVertexAttribIPointer(Attribute.Location, Description.Size, Description.Type, Stream.Stride, Cursor);
        }
        else
        {
            glVertexAttribPointer(Attribute.Location, Description.Size, Description.Type, Description.Normalized, Stream.Stride, Cursor);
        }
        glVertexAttribDivisor(Attribute.Location, Attribute.Divisor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void Format(ConstRef<Attribute> Attribute)
    {
        const AttributeFormat Description = Convert(Attribute.Format);

        if (Description.Integer)
        {
            glVertexAttribIFormat(Attribute.Location, Description.Size, Description.Type, Attribute.Offset);
        }
        else
        {
            glVertexAttribFormat(Attribute.Location, Description.Size, Description.Type, Description.Normalized, Attribute.Offset);
        }
        glVertexAttribBinding(Attribute.Location, Attribute.Stream);
        glVertexBindingDivisor(Attribute.Stream, Attribute.Divisor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt32 Select(ConstRef<Attributes> Layout, UInt Streams)
    {
        // An attribute whose stream the draw leaves out stays disabled, and reads its constant value.
        UInt32 Wanted = 0;

        for (ConstRef<Attribute> Attribute : Layout)
        {
            if (Attribute.Stream < Streams)
            {
                Wanted |= (1u << Attribute.Location);
            }
        }
        return Wanted;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void Toggle(UInt32 Changed, UInt32 Wanted)
    {
        for (UInt32 Location = 0; Changed; ++Location, Changed >>= 1, Wanted >>= 1)
        {
            if (Changed & 1u)
            {
                if (Wanted & 1u)
                {
                    glEnableVertexAttribArray(Location);
                }
                else
                {
                    glDisableVertexAttribArray(Location);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void ApplyStencil(ConstRef<Stencil> Stencil, UInt8 Reference)
    {
        if (!Stencil.Enable)
        {
            glDisable(GL_STENCIL_TEST);
            return;
        }

        glEnable(GL_STENCIL_TEST);
        glStencilFuncSeparate(GL_FRONT, Stencil.Front.Test, Reference, Stencil.Read);
        glStencilFuncSeparate(GL_BACK,  Stencil.Back.Test,  Reference, Stencil.Read);
        glStencilOpSeparate(GL_FRONT, Stencil.Front.Fail, Stencil.Front.DepthFail, Stencil.Front.DepthPass);
        glStencilOpSeparate(GL_BACK,  Stencil.Back.Fail,  Stencil.Back.DepthFail,  Stencil.Back.DepthPass);
        glStencilMask(Stencil.Write);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Binder::Binder(
        ConstRef<Device>    Device,
        ConstSpan<Buffer>   Buffers,
        ConstSpan<Pipeline> Pipelines,
        ConstSpan<Sampler>  Samplers,
        ConstSpan<Texture>  Textures)
        : mDevice     { Device },
          mBuffers    { Buffers },
          mPipelines  { Pipelines },
          mSamplers   { Samplers },
          mTextures   { Textures },
          mPass       { nullptr },
          mAttributes { 0 },
          mUnit       { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::Prepare(ConstRef<Pass> Pass)
    {
        mPass = AddressOf(Pass);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::Invalidate()
    {
        ConstRef<Pipeline> Empty = mPipelines[0];

        // The context is put in the state of an empty draw, which is what the next draw is compared against.
        mCommand = Command();

        glUseProgram(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        ApplyBlend(Empty.GetBlend(), false);
        ApplyDepth(Empty.GetDepth());
        ApplyStencil(Empty.GetStencil(), mCommand.Stencil);
        ApplyRaster(Empty.GetRaster());
        Clip(mCommand.Scissor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::Apply(ConstRef<Command> Draw)
    {
        ApplyPipeline(Draw);
        ApplyScissor(Draw.Scissor);
        ApplyVertices(Draw);
        ApplyIndices(Draw.Indices);
        ApplyUniforms(Draw.Uniforms);
        ApplySamplers(Draw.Samplers);
        ApplyTextures(Draw.Textures);

        mCommand = Draw;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyPipeline(ConstRef<Command> Draw)
    {
        ConstRef<Pipeline> Old = mPipelines[mCommand.Pipeline];
        ConstRef<Pipeline> New = mPipelines[Draw.Pipeline];

        if (Old.GetHandle() != New.GetHandle())
        {
            glUseProgram(New.GetHandle());
        }

        if (Old.GetBlend() != New.GetBlend())
        {
            ApplyBlend(New.GetBlend(), true);
        }

        if (Old.GetDepth() != New.GetDepth())
        {
            ApplyDepth(New.GetDepth());
        }

        if (Old.GetStencil() != New.GetStencil() || mCommand.Stencil != Draw.Stencil)
        {
            ApplyStencil(New.GetStencil(), Draw.Stencil);
        }

        if (Old.GetRaster() != New.GetRaster())
        {
            ApplyRaster(New.GetRaster());
        }

        // Formats are kept apart from the buffers they read, so they change with the pipeline alone.
        if (mDevice.Has(Feature::VertexBinding) && mCommand.Pipeline != Draw.Pipeline)
        {
            for (ConstRef<Attribute> Attribute : New.GetLayout())
            {
                Format(Attribute);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyScissor(ConstRef<Scissor> Scissor)
    {
        const Bool Changed = (mCommand.Scissor.X      != Scissor.X)
                          || (mCommand.Scissor.Y      != Scissor.Y)
                          || (mCommand.Scissor.Width  != Scissor.Width)
                          || (mCommand.Scissor.Height != Scissor.Height);

        if (!Changed)
        {
            return;
        }

        Clip(Scissor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyVertices(ConstRef<Command> Draw)
    {
        UInt32 First;
        UInt32 Count;

        const SInt32 Base  = Fold(Draw);
        const UInt32 Limit = static_cast<UInt32>(Max(mCommand.Vertices.GetSize(), Draw.Vertices.GetSize()));

        // A new pipeline or a new folded base moves every stream, so only a draw that keeps both binds just what differs.
        const Bool Kept = mCommand.Pipeline == Draw.Pipeline && Fold(mCommand) == Base;

        if (!Differ<Stream>(Kept, mCommand.Vertices, Draw.Vertices, Limit, First, Count))
        {
            return;
        }

        ConstRef<Attributes> Layout = mPipelines[Draw.Pipeline].GetLayout();

        if (mDevice.Has(Feature::VertexBinding))
        {
            for (UInt32 Slot = First; Slot < First + Count; ++Slot)
            {
                const Stream New = (Slot < Draw.Vertices.GetSize() ? Draw.Vertices[Slot] : Stream());

                glBindVertexBuffer(Slot, mBuffers[New.Buffer].GetHandle(), Locate(New, Divisor(Layout, Slot), Base), New.Stride);
            }
        }
        else
        {
            GLuint Bound = 0;

            for (ConstRef<Attribute> Attribute : Layout)
            {
                if (Attribute.Stream >= Draw.Vertices.GetSize())
                {
                    continue;
                }

                ConstRef<Stream> Stream = Draw.Vertices[Attribute.Stream];

                // Pointing an attribute reads GL_ARRAY_BUFFER, so each stream's buffer is bound as the attributes reach it.
                if (const GLuint Handle = mBuffers[Stream.Buffer].GetHandle(); Handle != Bound)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, Handle);
                    Bound = Handle;
                }
                Point(Attribute, Stream, Base);
            }
        }

        // Only the binder enables attributes, so what it last enabled is what the context holds.
        const UInt32 Wanted = Select(Layout, Draw.Vertices.GetSize());

        Toggle(mAttributes ^ Wanted, Wanted);
        mAttributes = Wanted;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyIndices(ConstRef<Stream> Indices)
    {
        // The index buffer is part of the vertex array, which uploads through GL_ELEMENT_ARRAY_BUFFER also change.
        if (mCommand.Indices != Indices)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffers[Indices.Buffer].GetHandle());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyUniforms(ConstSpan<Stream> Uniforms)
    {
        UInt32 First;
        UInt32 Count;

        const UInt32 Limit = static_cast<UInt32>(Max(mCommand.Uniforms.GetSize(), Uniforms.GetSize()));

        if (!Differ<Stream>(true, mCommand.Uniforms, Uniforms, Limit, First, Count))
        {
            return;
        }

        GLuint     Buffers[Command::kMaxUniforms];
        GLintptr   Offsets[Command::kMaxUniforms];
        GLsizeiptr Lengths[Command::kMaxUniforms];

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            const Stream New = (Slot < Uniforms.GetSize() ? Uniforms[Slot] : Stream { });

            Buffers[Slot] = mBuffers[New.Buffer].GetHandle();
            Offsets[Slot] = New.Offset;

            // An empty slot keeps a size the call accepts, and the size is ignored once the buffer is zero.
            Lengths[Slot] = New.Buffer ? New.Stride : 1;
        }

        if (mDevice.Has(Feature::MultiBind))
        {
            glBindBuffersRange(GL_UNIFORM_BUFFER, First, Count, Buffers + First, Offsets + First, Lengths + First);
            return;
        }

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            if (Buffers[Slot])
            {
                glBindBufferRange(GL_UNIFORM_BUFFER, Slot, Buffers[Slot], Offsets[Slot], Lengths[Slot]);
            }
            else
            {
                glBindBufferBase(GL_UNIFORM_BUFFER, Slot, 0);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplySamplers(ConstSpan<Object> Samplers)
    {
        UInt32 First;
        UInt32 Count;

        // A unit the program does not sample is never read, so what an earlier draw left there stays bound.
        const UInt32 Limit = static_cast<UInt32>(Samplers.GetSize());

        if (!Differ<Object>(true, mCommand.Samplers, Samplers, Limit, First, Count))
        {
            return;
        }

        GLuint Handles[Command::kMaxSamplers];

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            Handles[Slot] = mSamplers[Slot < Samplers.GetSize() ? Samplers[Slot] : 0].GetHandle();
        }

        if (mDevice.Has(Feature::MultiBind))
        {
            glBindSamplers(First, Count, Handles + First);
            return;
        }

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            glBindSampler(Slot, Handles[Slot]);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyTextures(ConstSpan<Object> Textures)
    {
        UInt32 First;
        UInt32 Count;

        // A unit the program does not sample is never read, so what an earlier draw left there stays bound.
        const UInt32 Limit = static_cast<UInt32>(Textures.GetSize());

        if (!Differ<Object>(true, mCommand.Textures, Textures, Limit, First, Count))
        {
            return;
        }

        GLuint Handles[Command::kMaxTextures];
        GLenum Targets[Command::kMaxTextures];

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            ConstRef<Texture> New = mTextures[Slot < Textures.GetSize() ? Textures[Slot] : 0];

            Handles[Slot] = New.GetHandle();
            Targets[Slot] = New.GetTarget();
        }

        // Each texture goes to the target it was created for, so one call covers 2D, array and cube alike.
        if (mDevice.Has(Feature::MultiBind))
        {
            glBindTextures(First, Count, Handles + First);
            return;
        }

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            // Only the binder switches units, so the one it left active is still the one the context has.
            if (mUnit != Slot)
            {
                glActiveTexture(GL_TEXTURE0 + Slot);
                mUnit = Slot;
            }
            glBindTexture(Targets[Slot], Handles[Slot]);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::Clip(ConstRef<Scissor> Scissor) const
    {
        // The window counts its rows from the bottom, so only the display flips the rectangle.
        const GLint Y = mPass->IsDisplay() ? mPass->GetHeight() - (Scissor.Y + Scissor.Height) : Scissor.Y;

        glScissor(Scissor.X, Y, Scissor.Width, Scissor.Height);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    SInt32 Binder::Fold(ConstRef<Command> Draw) const
    {
        // Without base vertex entry points an indexed draw's base moves its per-vertex streams instead.
        return (Draw.Indices.Buffer && !mDevice.Has(Feature::BaseVertex)) ? Draw.Parameters.Base : 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyBlend(ConstRef<Blend> Blend, Bool Valid)
    {
        if (!Valid || mBlend.Enable != Blend.Enable)
        {
            if (Blend.Enable)
            {
                glEnable(GL_BLEND);
            }
            else
            {
                glDisable(GL_BLEND);
            }
        }

        if (!Valid || mBlend.Mask != Blend.Mask)
        {
            glColorMask(
                (Blend.Mask & 0x01) ? GL_TRUE : GL_FALSE,
                (Blend.Mask & 0x02) ? GL_TRUE : GL_FALSE,
                (Blend.Mask & 0x04) ? GL_TRUE : GL_FALSE,
                (Blend.Mask & 0x08) ? GL_TRUE : GL_FALSE);
        }

        // A disabled blend never reads its factors or equations, so they keep what the last enabled one set.
        if (Valid && !Blend.Enable)
        {
            mBlend.Enable = Blend.Enable;
            mBlend.Mask   = Blend.Mask;
            return;
        }

        if (!Valid
            || mBlend.SrcColor != Blend.SrcColor
            || mBlend.DstColor != Blend.DstColor
            || mBlend.SrcAlpha != Blend.SrcAlpha
            || mBlend.DstAlpha != Blend.DstAlpha)
        {
            glBlendFuncSeparate(Blend.SrcColor, Blend.DstColor, Blend.SrcAlpha, Blend.DstAlpha);
        }

        if (!Valid || mBlend.EquationColor != Blend.EquationColor || mBlend.EquationAlpha != Blend.EquationAlpha)
        {
            glBlendEquationSeparate(Blend.EquationColor, Blend.EquationAlpha);
        }
        mBlend = Blend;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyDepth(ConstRef<Depth> Depth) const
    {
        if (Depth.Enable)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        glDepthMask(Depth.Write ? GL_TRUE : GL_FALSE);
        glDepthFunc(Depth.Test);

        // Without depth clamping every primitive is clipped, which is what most pipelines ask for anyway.
        if (mDevice.Has(Feature::DepthClamp))
        {
            if (Depth.Clip)
            {
                glDisable(GL_DEPTH_CLAMP);
            }
            else
            {
                glEnable(GL_DEPTH_CLAMP);
            }
        }

        if (Depth.Bias != 0.0f || Depth.Slope != 0.0f)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(Depth.Slope, Depth.Bias);
        }
        else
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyRaster(ConstRef<Raster> Raster) const
    {
        if (Raster.Cull)
        {
            glEnable(GL_CULL_FACE);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        if (Raster.Scissor)
        {
            glEnable(GL_SCISSOR_TEST);
        }
        else
        {
            glDisable(GL_SCISSOR_TEST);
        }

        glCullFace(Raster.Face);

        // The web has no polygon mode, and draws every polygon filled.
        if (mDevice.Has(Feature::PolygonMode))
        {
            glPolygonMode(GL_FRONT_AND_BACK, Raster.Fill);
        }
    }
}