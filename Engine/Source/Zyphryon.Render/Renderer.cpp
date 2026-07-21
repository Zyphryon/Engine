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

#include "Renderer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Renderer::Renderer(Ref<Engine::Subsystem::Host> Host)
        : mService { Host.GetService<Graphic::Service>() },
          mEncoder { * mService }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Renderer::Resize(UInt16 Width, UInt16 Height)
    {
        // Recreate every managed target at the new output size.
        for (ConstRef<Unique<Target>> Holder : mTargets)
        {
            Ref<Target> Resource = (* Holder);

            Resource.Release(* mService);
            Resource.Realize(* mService, Width, Height);
        }

        // Rebuild each pass handle from its declared attachments; passes that render to the display are left as-is.
        for (ConstRef<Unique<Pass>> Holder : mPasses)
        {
            Ref<Pass>                        Stage  = (* Holder);
            ConstSpan<Pass::ColorAttachment> Colors = Stage.GetColors();

            // A pass whose primary color attachment has no managed target renders to the display surface.
            if (Colors.IsEmpty() || Colors.GetFront().Target == nullptr)
            {
                Stage.SetViewport(Graphic::Viewport(0.0f, 0.0f, Width, Height));
                continue;
            }

            if (Stage.GetTarget() != Graphic::kDisplay)
            {
                mService->DeletePass(Stage.GetTarget());
            }

            // Resolve the declared attachments into RHI attachments over the freshly realized textures.
            Sequence<Graphic::ColorAttachment, Graphic::kMaxAttachments> Resolved;

            for (ConstRef<Pass::ColorAttachment> Color : Colors)
            {
                Ref<Graphic::ColorAttachment> Slot = Resolved.Append();
                Slot.Target      = Color.Target  ? Color.Target->GetTexture()  : 0;
                Slot.Resolve     = Color.Resolve ? Color.Resolve->GetTexture() : 0;
                Slot.LoadAction  = Color.Load;
                Slot.StoreAction = Color.Store;
            }

            Graphic::DepthAttachment Depth;

            if (ConstRef<Pass::DepthAttachment> Source = Stage.GetDepth(); Source.Target)
            {
                Depth.Target             = Source.Target->GetTexture();
                Depth.DepthLoadAction    = Source.DepthLoad;
                Depth.DepthStoreAction   = Source.DepthStore;
                Depth.StencilLoadAction  = Source.StencilLoad;
                Depth.StencilStoreAction = Source.StencilStore;
            }

            Stage.SetTarget(mService->CreatePass(Resolved, Depth));

            // The pass viewport tracks its first color target's size.
            ConstPtr<Target> Primary = Colors.GetFront().Target;
            Stage.SetViewport(Graphic::Viewport(0.0f, 0.0f, Primary->GetWidth(), Primary->GetHeight()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Renderer::Run(ConstRef<Graphic::Stream> Global)
    {
        // Bind the frame-global uniforms shared by every pass and draw this frame.
        mEncoder.SetGlobal(Global);

        for (ConstRef<Unique<Pass>> Holder : mPasses)
        {
            Ref<Pass> Stage = (* Holder);

            if (Stage.IsActive())
            {
                // Gather the per-attachment clear colors and the depth attachment's clears.
                Sequence<Color, Graphic::kMaxAttachments> Clears;

                for (ConstRef<Pass::ColorAttachment> Color : Stage.GetColors())
                {
                    Clears.Append(Color.Tint);
                }

                ConstRef<Pass::DepthAttachment> Depth = Stage.GetDepth();

                mService->Prepare(Stage.GetTarget(), Stage.GetViewport(), Clears, Depth.Depth, Depth.Stencil);
                {
                    mEncoder.Reset();

                    Stage.Run(mEncoder);

                    // Continuations share this pass's open target: append their draws without re-preparing.
                    for (Ptr<Pass> Next = Stage.GetContinuation(); Next; Next = Next->GetContinuation())
                    {
                        if (Next->IsActive())
                        {
                            Next->Run(mEncoder);
                        }
                    }
                }
                mService->Commit();
            }
        }
    }
}
