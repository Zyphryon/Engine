// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "ShapeRenderer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ShapeRenderer::ShapeRenderer(Ref<Service::Host> Host)
        : mGraphics { Host.GetService<Graphic::Service>() }
    {
        CreateDefaultResources(Host.GetService<Content::Service>());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ShapeRenderer::DrawCircle(ConstRef<Circle> Shape, Real32 Depth, UInt32 Tint)
    {
        Ref<Command> Command = Create(Type::Circle, GetUniqueKey(Type::Circle, Depth));
        Command.Instance.Center = Shape.GetCenter();
        Command.Instance.Depth  = Depth;
        Command.Instance.Color  = Tint;
        Command.Instance.Data0  = Vector2(Shape.GetRadius());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ShapeRenderer::DrawRing(ConstRef<Circle> Shape, Real32 Depth, UInt32 Tint, Real32 Thickness)
    {
        Ref<Command> Command = Create(Type::Ring, GetUniqueKey(Type::Ring, Depth));
        Command.Instance.Center = Shape.GetCenter();
        Command.Instance.Depth  = Depth;
        Command.Instance.Color  = Tint;
        Command.Instance.Data0  = Vector2(Shape.GetRadius(), Thickness);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ShapeRenderer::DrawLine(ConstRef<Line> Shape, Real32 Depth, UInt32 Tint, float Thickness)
    {
        Ref<Command> Command = Create(Type::Line, GetUniqueKey(Type::Line, Depth));
        Command.Instance.Center = Shape.GetMidpoint();
        Command.Instance.Depth  = Depth;
        Command.Instance.Color  = Tint;
        Command.Instance.Data0  = Shape.GetDirection();
        Command.Instance.Data1  = Vector2(Shape.GetLength() * 0.5f, Thickness);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ShapeRenderer::DrawStrokeRect(ConstRef<Rect> Shape, Real32 Depth, UInt32 Tint, float Thickness)
    {
        const Vector2 BL(Shape.GetMinimumX(), Shape.GetMinimumY());
        const Vector2 TL(Shape.GetMinimumX(), Shape.GetMaximumY());
        const Vector2 TR(Shape.GetMaximumX(), Shape.GetMaximumY());
        const Vector2 BR(Shape.GetMaximumX(), Shape.GetMinimumY());

        const Real32 HalfThickness = Thickness * 0.5f;
        const Vector2 OuterTL = Vector2(TL.GetX() - HalfThickness, TL.GetY());
        const Vector2 OuterBL = Vector2(BL.GetX() - HalfThickness, BL.GetY());
        const Vector2 OuterTR = Vector2(TR.GetX() + HalfThickness, TR.GetY());
        const Vector2 OuterBR = Vector2(BR.GetX() + HalfThickness, BR.GetY());

        DrawLine(Line(BL,      TL),      Depth, Tint, Thickness);
        DrawLine(Line(OuterTL, OuterTR), Depth, Tint, Thickness);
        DrawLine(Line(TR,      BR),      Depth, Tint, Thickness);
        DrawLine(Line(OuterBR, OuterBL), Depth, Tint, Thickness);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ShapeRenderer::DrawRect(ConstRef<Rect> Shape, Real32 Depth, UInt32 Tint)
    {
        Ref<Command> Command = Create(Type::Rect, GetUniqueKey(Type::Rect, Depth));
        Command.Instance.Center = Shape.GetCenter();
        Command.Instance.Depth  = Depth;
        Command.Instance.Color  = Tint;
        Command.Instance.Data0  = Shape.GetSize() * 0.5f;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ShapeRenderer::DrawRoundedRect(ConstRef<Rect> Shape, Real32 Depth, UInt32 Tint, Real32 Radius)
    {
        Ref<Command> Command = Create(Type::RoundedRect, GetUniqueKey(Type::RoundedRect, Depth));
        Command.Instance.Center = Shape.GetCenter();
        Command.Instance.Depth  = Depth;
        Command.Instance.Color  = Tint;
        Command.Instance.Data0  = Shape.GetSize() * 0.5f;
        Command.Instance.Data1  = Vector2(Radius);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ShapeRenderer::Flush()
    {
        if (mCommandTracker.empty())
        {
            return;
        }

        // Sort all command(s) back to front and by material/pipeline
        std::sort(mCommandTracker.begin(), mCommandTracker.end(), [](ConstPtr<Command> Left, ConstPtr<Command> Right)
        {
            return Left->Key > Right->Key;
        });

        // Render all command(s) in group of batches
        for (UInt32 Element = 0, Start = 0, Max = mCommandTracker.size() - 1; Element <= Max; ++Element)
        {
            const ConstPtr<Command> Current = mCommandTracker[Element];
            const ConstPtr<Command> Other   = (Element == Max ? Current : mCommandTracker[Element + 1]);

            // Break batch if material and pipeline differ or we are handling the last element
            if (Current->Type != Other->Type || Current == Other)
            {
                const UInt32 Count = Element - Start + 1;

                auto [VtxPointer, VtxDescription] = mGraphics->Allocate<Layout>(Usage::Vertex, Count);

                for (UInt32 Index = Start; Index < Start + Count; ++Index)
                {
                    *VtxPointer++ = mCommandTracker[Index]->Instance;
                }

                mEncoder.SetUniform(0, mParameters);
                mEncoder.SetVertices(0, VtxDescription);
                mEncoder.SetPipeline(* mPipelines[Enum::Cast(Current->Type)]);
                mEncoder.Draw(4, 0, 0, Count);

                // Continue with the next batch
                Start = Element + 1;
            }
        }

        mGraphics->Submit(mEncoder.GetSubmissions());

        mEncoder.Clear();

        mCommandTracker.clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ShapeRenderer::CreateDefaultResources(ConstTracker<Content::Service> Content)
    {
        mPipelines[0] = Content->Load<Pipeline>("Engine://Pipeline/Shapes/Circle.effect");
        mPipelines[1] = Content->Load<Pipeline>("Engine://Pipeline/Shapes/Ring.effect");
        mPipelines[2] = Content->Load<Pipeline>("Engine://Pipeline/Shapes/Line.effect");
        mPipelines[3] = Content->Load<Pipeline>("Engine://Pipeline/Shapes/Rect.effect");
        mPipelines[4] = Content->Load<Pipeline>("Engine://Pipeline/Shapes/RoundedRect.effect");
    }
}