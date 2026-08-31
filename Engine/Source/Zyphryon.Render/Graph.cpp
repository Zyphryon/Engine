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

#include "Graph.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Graph::Graph(Ref<Engine::Subsystem::Host> Host, Ref<Blueprint> Blueprint)
        : mService   { Host.GetService<Graphic::Service>() },
          mBlueprint { Blueprint },
          mEncoder   { * mService },
          mWidth     { 0 },
          mHeight    { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Graph::~Graph()
    {
        Release();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Graph::Resize(UInt16 Width, UInt16 Height)
    {
        mWidth  = Width;
        mHeight = Height;

        ConstSpan<Target> Targets = mBlueprint.GetTargets();

        while (mSlots.GetSize() < Targets.GetSize())
        {
            mSlots.Append(Slot());
        }

        // Realize a texture of the graph's own for every target whose shape is not the one it already holds.
        Bool Realized = false;

        for (UInt32 Index = 0; Index < Targets.GetSize(); ++Index)
        {
            ConstRef<Target> Description = Targets[Index];
            Ref<Slot>        Entry       = mSlots[Index];

            UInt16 Sized;
            UInt16 Tall;

            switch (Description.Sizing)
            {
            case Target::Scale::Full:
                Sized = Width;
                Tall  = Height;
                break;
            case Target::Scale::Half:
                Sized = Width  / 2;
                Tall  = Height / 2;
                break;
            case Target::Scale::Quarter:
                Sized = Width  / 4;
                Tall  = Height / 4;
                break;
            default:
                Sized = Description.Width;
                Tall  = Description.Height;
                break;
            }

            Sized = Max(1, Sized);
            Tall  = Max(1, Tall);

            // A texture already standing in the shape it is asked for again is left where it is.
            if (Entry.Texture && Entry.Format == Description.Format && Entry.Width == Sized && Entry.Height == Tall)
            {
                continue;
            }

            if (Entry.Texture)
            {
                mService->DeleteTexture(Entry.Texture);
            }

            Entry.Format  = Description.Format;
            Entry.Width   = Sized;
            Entry.Height  = Tall;
            Entry.Texture = mService->CreateTexture(Description.Format, Sized, Tall, 1, Description.Samples);
            Realized      = true;
        }

        // Every step names the textures it draws into, so they stand only while none of them moved.
        if (!Realized && !mSteps.IsEmpty())
        {
            return;
        }

        for (ConstRef<Step> Entry : mSteps)
        {
            // The display surface is not the graph's to destroy, and an inline step never owned its handle.
            if (!Entry.Inline && Entry.Handle != Graphic::kDisplay)
            {
                mService->DeletePass(Entry.Handle);
            }
        }
        mSteps.Clear();

        // Bake one step per pass over the textures the graph just realized.
        for (UInt32 Index = 0, Limit = mBlueprint.mPasses.GetSize(); Index < Limit; ++Index)
        {
            ConstRef<Pass>                   Stage  = (* mBlueprint.mPasses[Index]);
            ConstSpan<Pass::ColorAttachment> Colors = Stage.GetColors();
            Ref<Step>                        Entry  = mSteps.Append();

            // An inline pass draws into what the pass before it opened, borrowing its handle and viewport.
            if (Stage.IsInline())
            {
                ZY_ASSERT(Index > 0, "The first pass has no open target to draw into");
                ZY_ASSERT(Colors.IsEmpty(), "An inline pass inherits the target of the pass before it");

                if (Index > 0)
                {
                    Entry        = mSteps[Index - 1];
                    Entry.Inline = true;
                    continue;
                }
            }

            // A pass whose primary color attachment names no target renders to the display surface.
            if (Colors.IsEmpty() || Colors.GetFront().Target == Pass::kNone)
            {
                Entry.Handle   = Graphic::kDisplay;
                Entry.Viewport = Graphic::Viewport(0.0f, 0.0f, Width, Height);
                continue;
            }

            Sequence<Graphic::ColorAttachment, Graphic::kMaxAttachments> Resolved;

            for (ConstRef<Pass::ColorAttachment> Color : Colors)
            {
                Ref<Graphic::ColorAttachment> Attachment = Resolved.Append();
                Attachment.Target      = Color.Target  != Pass::kNone ? GetTexture(Color.Target)  : 0;
                Attachment.Resolve     = Color.Resolve != Pass::kNone ? GetTexture(Color.Resolve) : 0;
                Attachment.LoadAction  = Color.Load;
                Attachment.StoreAction = Color.Store;
            }

            Graphic::DepthAttachment Depth;

            if (ConstRef<Pass::DepthAttachment> Source = Stage.GetDepth(); Source.Target != Pass::kNone)
            {
                Depth.Target             = GetTexture(Source.Target);
                Depth.DepthLoadAction    = Source.DepthLoad;
                Depth.DepthStoreAction   = Source.DepthStore;
                Depth.StencilLoadAction  = Source.StencilLoad;
                Depth.StencilStoreAction = Source.StencilStore;
            }

            Entry.Handle = mService->CreatePass(Resolved, Depth);

            // The pass viewport tracks the size its first color target came out at.
            const UInt32 Primary = Colors.GetFront().Target;
            Entry.Viewport = Graphic::Viewport(0.0f, 0.0f, GetWidth(Primary), GetHeight(Primary));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Graph::Release()
    {
        for (ConstRef<Slot> Entry : mSlots)
        {
            mService->DeleteTexture(Entry.Texture);
        }

        for (ConstRef<Step> Entry : mSteps)
        {
            // The display surface is not the graph's to destroy, and an inline step never owned its handle.
            if (!Entry.Inline && Entry.Handle != Graphic::kDisplay)
            {
                mService->DeletePass(Entry.Handle);
            }
        }

        mSlots.Clear();
        mSteps.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Graph::Run(Graphic::Stream Frame)
    {
        // Bind the frame-global uniforms shared by every pass and draw this frame.
        mEncoder.SetFrame(Frame);

        // The pass that declared the open target, whose attachments every inline pass after it draws through.
        ConstPtr<Pass> Owner  = nullptr;
        Bool           Opened = false;

        for (UInt32 Index = 0, Limit = mBlueprint.mPasses.GetSize(); Index < Limit; ++Index)
        {
            Ref<Pass>      Stage = (* mBlueprint.mPasses[Index]);
            ConstRef<Step> Entry = mSteps[Index];

            // A pass that opens a target of its own closes whatever the passes before it were drawing into.
            if (!Entry.Inline)
            {
                if (Opened)
                {
                    mService->Commit();

                    Opened = false;
                }
                Owner = AddressOf(Stage);
            }

            if (!Stage.IsActive())
            {
                continue;
            }

            // The group opens on its first active pass, clearing as the pass that declared the target asked.
            if (!Opened)
            {
                Sequence<Color, Graphic::kMaxAttachments> Clears;

                for (ConstRef<Pass::ColorAttachment> Color : Owner->GetColors())
                {
                    Clears.Append(Color.Tint);
                }

                ConstRef<Pass::DepthAttachment> Depth = Owner->GetDepth();

                mService->Prepare(Entry.Handle, Entry.Viewport, Clears, Depth.Depth, Depth.Stencil);

                mEncoder.Reset();

                Opened = true;
            }

            Stage.Run(mEncoder, * this);
        }

        if (Opened)
        {
            mService->Commit();
        }
    }
}