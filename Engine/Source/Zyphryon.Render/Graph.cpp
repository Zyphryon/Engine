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

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Graph::Graph(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint)
        : mService   { Host.GetService<ZyGraphic::Service>() },
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
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Graph::Run(ZyGraphic::Stream Frame)
    {
        // Whatever the blueprint restated or the output moved since the last run is realized before anything draws.
        Reconcile();

        mEncoder.SetFrame(Frame);

        for (UInt32 Index = 0, Limit = mBlueprint.mPasses.GetSize(); Index < Limit; ++Index)
        {
            Ref<Pass>      Stage = * mBlueprint.mPasses[Index];
            ConstRef<Step> Entry = mSteps[Index];

            if (!Stage.IsActive())
            {
                continue;
            }

            // The pass opens the target it declared, clearing it as it asked, and closes it once it has drawn.
            Sequence<Color, ZyGraphic::kMaxAttachments> Clears;

            for (ConstRef<Pass::ColorAttachment> Color : Stage.GetColors())
            {
                Clears.Append(Color.Tint);
            }

            ConstRef<Pass::DepthAttachment> Depth = Stage.GetDepth();

            // The display tracks the output, which can move while every texture of the graph's own stays put.
            const ZyGraphic::Viewport Viewport = (Entry.Handle == ZyGraphic::kDisplay)
                ? ZyGraphic::Viewport(0.0f, 0.0f, mWidth, mHeight)
                : Entry.Viewport;

            mService->Prepare(Entry.Handle, Stage.GetName(), Viewport, Clears, Depth.Depth, Depth.Stencil);

            mEncoder.Reset();

            // Whatever the pass reads is handed to every draw it records, under the name its techniques declare.
            for (ConstRef<Pass::InputAttachment> Input : Stage.GetInputs())
            {
                mEncoder.SetInput(Input.Name, GetTexture(Input.Target));
            }

            Stage.Run(mEncoder, * this);

            mService->Commit();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Graph::Reconcile()
    {
        const ConstSpan<Target> Targets = mBlueprint.GetTargets();

        mSlots.Expand(Targets.GetSize());

        // Realize a texture of the graph's own for every target whose shape is not the one it already holds.
        Bool Realized = false;

        for (UInt32 Index = 0; Index < Targets.GetSize(); ++Index)
        {
            ConstRef<Target> Description = Targets[Index];
            Ref<Slot>        Entry       = mSlots[Index];

            UInt16 Width;
            UInt16 Height;

            switch (Description.Sizing)
            {
            case Target::Scale::Full:
                Width  = mWidth;
                Height = mHeight;
                break;
            case Target::Scale::Half:
                Width  = mWidth  / 2;
                Height = mHeight / 2;
                break;
            case Target::Scale::Quarter:
                Width  = mWidth  / 4;
                Height = mHeight / 4;
                break;
            default:
                Width  = Description.Width;
                Height = Description.Height;
                break;
            }

            Width  = Max(1, Width);
            Height = Max(1, Height);

            // A texture already standing in the shape it is asked for again is left where it is.
            if (Entry.Texture && Entry.Format == Description.Format && Entry.Width == Width && Entry.Height == Height)
            {
                continue;
            }

            if (Entry.Texture)
            {
                mService->DeleteTexture(Entry.Texture);
            }

            Entry.Format  = Description.Format;
            Entry.Width   = Width;
            Entry.Height  = Height;
            Entry.Texture = mService->CreateTexture(Description.Format, Width, Height, 1);
            Realized      = true;
        }

        // Every step names the textures it draws into, so they stand only while none of them moved and no pass came.
        if (!Realized && mSteps.GetSize() == mBlueprint.mPasses.GetSize())
        {
            return;
        }

        Unbake();

        // Bake one step per pass over the textures the graph just realized.
        for (UInt32 Index = 0, Limit = mBlueprint.mPasses.GetSize(); Index < Limit; ++Index)
        {
            ConstRef<Pass>                   Stage  = * mBlueprint.mPasses[Index];
            ConstSpan<Pass::ColorAttachment> Colors = Stage.GetColors();
            Ref<Step>                        Entry  = mSteps.Append();

            const Bool IsDepthOnly = Colors.IsEmpty() && Stage.GetDepth().Target != Pass::kNone;

            if (!IsDepthOnly && (Colors.IsEmpty() || Colors.GetFront().Target == Pass::kNone))
            {
                Entry.Handle = ZyGraphic::kDisplay;
                continue;
            }

            Sequence<ZyGraphic::ColorAttachment, ZyGraphic::kMaxAttachments> Resolved;

            for (ConstRef<Pass::ColorAttachment> Color : Colors)
            {
                Ref<ZyGraphic::ColorAttachment> Attachment = Resolved.Append();
                Attachment.Target      = Color.Target != Pass::kNone ? GetTexture(Color.Target) : 0;
                Attachment.LoadAction  = Color.Load;
                Attachment.StoreAction = Color.Store;
            }

            ZyGraphic::DepthAttachment Depth;

            if (ConstRef<Pass::DepthAttachment> Source = Stage.GetDepth(); Source.Target != Pass::kNone)
            {
                Depth.Target             = GetTexture(Source.Target);
                Depth.DepthLoadAction    = Source.DepthLoad;
                Depth.DepthStoreAction   = Source.DepthStore;
                Depth.StencilLoadAction  = Source.StencilLoad;
                Depth.StencilStoreAction = Source.StencilStore;
            }

            Entry.Handle = mService->CreatePass(Resolved, Depth);

            // The pass viewport tracks the size its first target came out at, colors first.
            const UInt32 Primary = IsDepthOnly ? Stage.GetDepth().Target : Colors.GetFront().Target;
            Entry.Viewport = ZyGraphic::Viewport(0.0f, 0.0f, GetWidth(Primary), GetHeight(Primary));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Graph::Unbake()
    {
        for (ConstRef<Step> Entry : mSteps)
        {
            // The display surface is not the graph's to destroy.
            if (Entry.Handle != ZyGraphic::kDisplay)
            {
                mService->DeletePass(Entry.Handle);
            }
        }
        mSteps.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Graph::Release()
    {
        for (ConstRef<Slot> Entry : mSlots)
        {
            mService->DeleteTexture(Entry.Texture);
        }

        Unbake();
        mSlots.Clear();
    }
}