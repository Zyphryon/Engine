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

#include "OGLPass.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Pass::Pass()
        : mHandle { 0 },
          mHeight { 0 },
          mEncode { false }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pass::Create(ConstRef<Configuration> Config)
    {
        const Bool HasDepth   = (Config.DepthFormat != TextureFormat::Unspecified);
        const Bool HasStencil = GetTextureMetadata(Config.DepthFormat).IsStencil;

        mHandle                   = 0;
        mHeight                   = static_cast<UInt16>(Config.Height);
        mEncode                   = (Config.ColorFormat == TextureFormat::RGBA8UIntNorm_sRGB);
        mDepth.DepthLoadAction    = HasDepth   ? Action::Clear : Action::Discard;
        mDepth.DepthStoreAction   = Action::Discard;
        mDepth.StencilLoadAction  = HasStencil ? Action::Clear : Action::Discard;
        mDepth.StencilStoreAction = Action::Discard;

        // The window has one color buffer, cleared when the frame begins and kept to be presented.
        mColors.Append();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Pass::Create(ConstSpan<ColorAttachment> Colors, DepthAttachment Depth, ConstSpan<Texture> Textures)
    {
        glGenFramebuffers(1, AddressOf(mHandle));
        glBindFramebuffer(GL_FRAMEBUFFER, mHandle);

        // An sRGB attachment is written gamma-encoded, as D3D does; a linear one ignores the switch.
        mEncode = true;

        Sequence<GLenum, kMaxAttachments> Points;

        for (ConstRef<ColorAttachment> Color : Colors)
        {
            const GLenum Point = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(Points.GetSize());

            Textures[Color.Target].Attach(GL_FRAMEBUFFER, Point, Color.TargetLevel, Color.TargetLayer);

            Points.Append(Point);
            mColors.Append(Color.LoadAction, Color.StoreAction);
        }

        glDrawBuffers(static_cast<GLsizei>(Points.GetSize()), Points.GetData());

        // A pass that writes depth alone has no color to read back either.
        if (Points.IsEmpty())
        {
            glReadBuffer(GL_NONE);
        }

        if (Depth.Target)
        {
            ConstRef<Texture> Texture = Textures[Depth.Target];
            const GLenum      Point   = GetTextureMetadata(Texture.GetFormat()).IsStencil
                ? GL_DEPTH_STENCIL_ATTACHMENT
                : GL_DEPTH_ATTACHMENT;

            Texture.Attach(GL_FRAMEBUFFER, Point, Depth.TargetLevel, Depth.TargetLayer);

            mDepth = Depth;
        }
        return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pass::Delete()
    {
        if (mHandle)
        {
            glDeleteFramebuffers(1, AddressOf(mHandle));
        }

        mHandle = 0;
        mHeight = 0;
        mColors.Clear();
        mDepth  = DepthAttachment();
        mEncode = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pass::Prepare(ConstRef<Device> Device, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mHandle);

        // Only desktop has the switch; ES and WebGL always encode writes into sRGB attachments.
        if (Device.GetProfile() == Profile::Desktop)
        {
            if (mEncode)
            {
                glEnable(GL_FRAMEBUFFER_SRGB);
            }
            else
            {
                glDisable(GL_FRAMEBUFFER_SRGB);
            }
        }

        // A framebuffer keeps its rows top-down as D3D does, so only the window is drawn from the lower-left.
        glClipControl(IsDisplay() ? GL_LOWER_LEFT : GL_UPPER_LEFT, GL_ZERO_TO_ONE);

        const Real32 Y = IsDisplay() ? mHeight - (Viewport.Y + Viewport.Height) : Viewport.Y;

        glViewport(
            static_cast<GLint>(Viewport.X),
            static_cast<GLint>(Y),
            static_cast<GLsizei>(Viewport.Width),
            static_cast<GLsizei>(Viewport.Height));
        glDepthRange(Viewport.MinDepth, Viewport.MaxDepth);

        Clear(Colors, Depth, Stencil);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pass::Commit(ConstRef<Device> Device)
    {
        // Whatever the pass does not keep is dropped now, which spares a tiled GPU the write of the tile back to memory.
        Sequence<GLenum, kMaxAttachments + 2> Discards;

        for (UInt32 Slot = 0; Slot < mColors.GetSize(); ++Slot)
        {
            if (mColors[Slot].Store == Action::Discard)
            {
                Discards.Append(IsDisplay() ? GL_COLOR : GL_COLOR_ATTACHMENT0 + Slot);
            }
        }

        if (mDepth.DepthStoreAction == Action::Discard)
        {
            Discards.Append(IsDisplay() ? GL_DEPTH : GL_DEPTH_ATTACHMENT);
        }

        if (mDepth.StencilStoreAction == Action::Discard)
        {
            Discards.Append(IsDisplay() ? GL_STENCIL : GL_STENCIL_ATTACHMENT);
        }

        // A discard is only a hint, so a context without it simply keeps everything.
        if (!Discards.IsEmpty() && Device.Has(Feature::Invalidate))
        {
            glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
            glInvalidateFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(Discards.GetSize()), Discards.GetData());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pass::Clear(ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil) const
    {
        // A clear honours the scissor and the write masks, so it opens them all; the next draw sets its own again.
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glStencilMask(0xFF);

        for (UInt32 Slot = 0; Slot < mColors.GetSize(); ++Slot)
        {
            if (mColors[Slot].Load == Action::Clear && Slot < Colors.GetSize())
            {
                glClearBufferfv(GL_COLOR, Slot, reinterpret_cast<ConstPtr<GLfloat>>(AddressOf(Colors[Slot])));
            }
        }

        const Bool ClearDepth   = mDepth.DepthLoadAction   == Action::Clear;
        const Bool ClearStencil = mDepth.StencilLoadAction == Action::Clear;

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
}