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

#include "Canvas.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Canvas::Canvas(Ref<Service::Host> Host)
        : mService { Host.GetService<Graphic::Service>() }
    {
        CreateDefaultResources(Host.GetService<Content::Service>());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::SetTransform(ConstRef<Matrix4x4> Transform)
    {
        const Graphic::Stream Scene
            = mService->AllocateTransientBuffer(Graphic::Usage::Uniform, ConstSpan(& Transform, 1));
        mEncoder.SetUniform(Enum::Cast(Graphic::Pipeline::Tier::Global), Scene);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::SetScissor(Graphic::Scissor Scissor)
    {
        mEncoder.SetScissor(Scissor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::SetPipeline(ConstTracker<Graphic::Pipeline> Pipeline)
    {
        mPipelines[Enum::Cast(Type::Sprite)] = Pipeline;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawCircle(ConstRef<Circle> Shape, Real32 Order, IntColor8 Tint)
    {
        Ref<ShapeCommand> Command = mShapes.emplace_back();
        Command.Layout.Center = Shape.GetCenter();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0.SetX(Shape.GetRadius());

        constexpr UInt32              Type     = Enum::Cast(Type::Circle);
        constexpr Collector::Priority Priority = Collector::Priority::Transparent;

        const UInt16 Pipeline = mPipelines[Type]->GetID();
        mCollector.Push(Collector::Object(Type, mShapes.size() - 1), Priority, Order, 0u, Pipeline, 0u);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawRing(ConstRef<Circle> Shape, Real32 Order, IntColor8 Tint, Real32 Thickness)
    {
        Ref<ShapeCommand> Command = mShapes.emplace_back();
        Command.Layout.Center = Shape.GetCenter();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0.Set(Shape.GetRadius(), Thickness);

        constexpr UInt32              Type     = Enum::Cast(Type::Ring);
        constexpr Collector::Priority Priority = Collector::Priority::Transparent;

        const UInt16 Pipeline = mPipelines[Type]->GetID();
        mCollector.Push(Collector::Object(Type, mShapes.size() - 1), Priority, Order, 0u, Pipeline, 0u);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawLine(ConstRef<Line> Shape, Real32 Order, IntColor8 Tint, Real32 Thickness)
    {
        Ref<ShapeCommand> Command = mShapes.emplace_back();
        Command.Layout.Center = Shape.GetMidpoint();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0  = Shape.GetDirection();
        Command.Layout.Data1.Set(Shape.GetLength() * 0.5f, Thickness);

        constexpr UInt32              Type     = Enum::Cast(Type::Line);
        const     Collector::Priority Priority = Tint.IsOpaque()
            ? Collector::Priority::Opaque
            : Collector::Priority::Transparent;

        const UInt16 Pipeline = mPipelines[Type]->GetID();
        mCollector.Push(Collector::Object(Type, mShapes.size() - 1), Priority, Order, 0u, Pipeline, 0u);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawRect(ConstRef<Rect> Shape, Real32 Order, IntColor8 Tint)
    {
        Ref<ShapeCommand> Command = mShapes.emplace_back();
        Command.Layout.Center = Shape.GetCenter();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0  = Shape.GetSize() * 0.5f;

        constexpr UInt32              Type     = Enum::Cast(Type::Rect);
        const     Collector::Priority Priority = Tint.IsOpaque()
            ? Collector::Priority::Opaque
            : Collector::Priority::Transparent;

        const UInt16 Pipeline = mPipelines[Type]->GetID();
        mCollector.Push(Collector::Object(Type, mShapes.size() - 1), Priority, Order, 0u, Pipeline, 0u);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawStrokeRect(ConstRef<Rect> Shape, Real32 Order, IntColor8 Tint, Real32 Thickness)
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

        DrawLine(Line(BL,      TL),      Order, Tint, Thickness);
        DrawLine(Line(OuterTL, OuterTR), Order, Tint, Thickness);
        DrawLine(Line(TR,      BR),      Order, Tint, Thickness);
        DrawLine(Line(OuterBR, OuterBL), Order, Tint, Thickness);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawRoundedRect(ConstRef<Rect> Shape, Real32 Order, IntColor8 Tint, Real32 Radius)
    {
        Ref<ShapeCommand> Command = mShapes.emplace_back();
        Command.Layout.Center = Shape.GetCenter();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0 = Vector2(Shape.GetSize() * 0.5f);
        Command.Layout.Data1.SetX(Radius);

        constexpr UInt32              Type     = Enum::Cast(Type::RoundedRect);
        constexpr Collector::Priority Priority = Collector::Priority::Transparent;

        const UInt16 Pipeline = mPipelines[Type]->GetID();
        mCollector.Push(Collector::Object(Type, mShapes.size() - 1), Priority, Order, 0u, Pipeline, 0u);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::SetTextEffect(UInt8 Index, ConstRef<TextEffect> Effect)
    {
        LOG_ASSERT(Index < mGlyphEffects.size(), "Exceeded maximum number of text effects.");

        mGlyphEffects[Index] = Effect;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawText(ConstRef<Text> Text, ConstStr8 Content, ConstRef<Matrix3x2> Transform, Real32 Order, UInt32 Effect)
    {
        ConstTracker<Font> Font = Text.GetFont();

        // Calculate the origin point for the text layout based on the content, size, origin, and padding settings.
        const Real32 Size = Text.GetSize();

        // The line height in font units.
        const Real32 LineHeight = Font->GetLineHeight(Text.GetSize(), Text.GetPadding());

        const ConstPtr<Graphic::Material> Material = Font->GetMaterial();

        // Iterate through each character in the text content and generate draw commands for each glyph.
        Real32 CurrentX = 0.0f;
        Real32 CurrentY = 0.0f;
        UInt32 Previous = 0;

        constexpr UInt32 Type     = Enum::Cast(Type::Glyph);
        const     UInt16 Pipeline = mPipelines[Type]->GetID();

        IterateUTF8(Content, [&](UInt32 Codepoint)
        {
            switch (Codepoint)
            {
            case '\r':
                CurrentX = 0.0f;
                break;
            case '\n':
                CurrentY -= LineHeight;
                break;
            default:
                const ConstPtr<Font::Glyph> Glyph = Font->GetGlyph(Codepoint);

                if (Glyph)
                {
                    if (Glyph->LocalBounds.GetWidth() > 0 && Glyph->LocalBounds.GetHeight() > 0)
                    {
                        Ref<GlyphCommand> Command = mGlyphs.emplace_back();

                        Command.Material         = Material;
                        Command.Layout.Transform.SetData(Transform, Order, BitCast<Real32>(Effect));
                        Command.Layout.Frame     = Glyph->AtlasBounds;
                        Command.Layout.Offset.Set(
                            CurrentX + Glyph->LocalBounds.GetX() * Size,
                            CurrentY + Glyph->LocalBounds.GetY() * Size);
                        Command.Layout.Size      = Vector2(
                            Glyph->LocalBounds.GetWidth()  * Size,
                            Glyph->LocalBounds.GetHeight() * Size);
                        Command.Layout.Color     = Text.GetTint();

                        mCollector.Push(
                            Collector::Object(Type, mGlyphs.size() - 1),
                            Collector::Priority::Transparent, Order, 0u,
                            Pipeline,
                            Material->GetID());
                    }
                    CurrentX += (Font->GetKerning(Previous, Codepoint) + Glyph->Advance) * Size;
                }
                break;
            }
            Previous = Codepoint;
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawSprite(ConstRef<Sprite> Sprite, ConstRef<Matrix3x2> Transform, Real32 Order)
    {
        const Collector::Priority Priority = Sprite.GetTint().IsOpaque()
            ? Collector::Priority::Opaque
            : Collector::Priority::Transparent;

        Ref<SpriteCommand> Command = mSprites.emplace_back();
        Command.Layout.Transform.SetData(Transform, Order);
        Command.Layout.Frame     = Sprite.GetFrame();
        Command.Layout.Size      = Sprite.GetSize();
        Command.Layout.Color     = Sprite.GetTint();
        Command.Material         = Sprite.GetMaterial();
        Command.Pipeline         = mPipelines[Enum::Cast(Type::Sprite)];

        constexpr UInt32 Type = Enum::Cast(Type::Sprite);

        mCollector.Push(Collector::Object(Type, mSprites.size() - 1), Priority, Order, 0,
            Command.Pipeline->GetID(),
            Command.Material->GetID());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::Flush()
    {
        // Prepare any necessary resources or state before processing the collected rendering commands.
        Prepare();

        // Iterate through each rendering queue in the collector and process the commands for submission to the GPU.
        mCollector.Poll([this](UInt32 Type, ConstSpan<Collector::Command> Commands)
        {
            switch (static_cast<Canvas::Type>(Type))
            {
            case Type::Circle:
            case Type::Ring:
            case Type::Line:
            case Type::Rect:
            case Type::RoundedRect:
                WriteShapes(static_cast<Canvas::Type>(Type), Commands);
                break;
            case Type::Sprite:
                WriteSprites(Commands);
                break;
            case Type::Glyph:
                WriteGlyphs(Commands);
                break;
            }
        });

        // Clear the collector after processing all commands to prepare for the next frame.
        Reset();

        // Submit all encoded draw packets to the graphics service for rendering.
        if (const ConstSpan<Graphic::DrawPacket> Submissions = mEncoder.GetSubmissions(); !Submissions.empty())
        {
            mService->Submit(Submissions);
        }
        mEncoder.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::CreateDefaultResources(ConstTracker<Content::Service> Content)
    {
        // TODO: Non-Opaque Rect/Line pipelines.

        mPipelines[Enum::Cast(Type::Circle)]      = Content->Load<Graphic::Pipeline>("Engine://Pipeline/Shapes/Circle.effect");
        mPipelines[Enum::Cast(Type::Ring)]        = Content->Load<Graphic::Pipeline>("Engine://Pipeline/Shapes/Ring.effect");
        mPipelines[Enum::Cast(Type::Line)]        = Content->Load<Graphic::Pipeline>("Engine://Pipeline/Shapes/Line.effect");
        mPipelines[Enum::Cast(Type::Rect)]        = Content->Load<Graphic::Pipeline>("Engine://Pipeline/Shapes/Rect.effect");
        mPipelines[Enum::Cast(Type::RoundedRect)] = Content->Load<Graphic::Pipeline>("Engine://Pipeline/Shapes/RoundedRect.effect");
        mPipelines[Enum::Cast(Type::Sprite)]      = Content->Load<Graphic::Pipeline>("Engine://Pipeline/Sprites/Opaque.effect");
        mPipelines[Enum::Cast(Type::Glyph)]       = Content->Load<Graphic::Pipeline>("Engine://Pipeline/Typography/MSDF.effect");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::Prepare()
    {
        mGlyphEffectsStream = mService->AllocateTransientBuffer(
            Graphic::Usage::Uniform, ConstSpan<TextEffect>(mGlyphEffects.data(), mGlyphEffects.size()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::Reset()
    {
        // Clear the list of shapes after processing to prepare for the next frame.
        mShapes.clear();

        // Clear the list of sprites after processing to prepare for the next frame.
        mSprites.clear();

        // Clear the list of glyphs after processing to prepare for the next frame.
        mGlyphs.clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::Bind(ConstPtr<Graphic::Pipeline> Pipeline, ConstPtr<Graphic::Material> Material)
    {
        mEncoder.SetPipeline(* Pipeline);

        if (Material)
        {
            if (const ConstSpan<Byte> Parameters = Material->GetParameters(); !Parameters.empty())
            {
                const Graphic::Stream Binding = mService->AllocateTransientBuffer(Graphic::Usage::Uniform, Parameters);
                mEncoder.SetUniform(Enum::Cast(Graphic::Pipeline::Tier::Style), Binding);
            }
            mEncoder.Bind(* Pipeline, * Material);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::WriteShapes(Type Shape, ConstSpan<Collector::Command> Commands)
    {
        // Since all commands in this batch share the same pipeline and material, we bind them once for the entire batch.
        Bind(mPipelines[Enum::Cast(Shape)], nullptr);

        // Draw all shapes in the batch using a single draw call with a callback to write each instance.
        Draw<ShapeLayout>(Commands.size(), [&](Ptr<ShapeLayout> Instances)
        {
            for (UInt32 Element = 0; Element < Commands.size(); ++Element, ++Instances)
            {
                // Fetch the command for the current element.
                Ref<ShapeCommand> Command = mShapes[Commands[Element].Entry.Slot];

                // Write the vertex data for the current shape command.
                (* Instances) = Command.Layout;
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::WriteSprites(ConstSpan<Collector::Command> Commands)
    {
        // Since all commands in this batch share the same pipeline and material, we bind them once for the entire batch.
        Ref<SpriteCommand> First = mSprites[Commands.front().Entry.Slot];
        Bind(First.Pipeline, First.Material);

        // Draw all shapes in the batch using a single draw call with a callback to write each instance.
        Draw<SpriteLayout>(Commands.size(), [&](Ptr<SpriteLayout> Instances)
        {
            for (UInt32 Element = 0; Element < Commands.size(); ++Element, ++Instances)
            {
                // Fetch the command for the current element.
                Ref<SpriteCommand> Command = mSprites[Commands[Element].Entry.Slot];

                // Write the vertex data for the current sprite command.
                (* Instances) = Command.Layout;
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::WriteGlyphs(ConstSpan<Collector::Command> Commands)
    {
        // Since all commands in this batch share the same pipeline and material, we bind them once for the entire batch.
        Ref<GlyphCommand> First = mGlyphs[Commands.front().Entry.Slot];
        Bind(mPipelines[Enum::Cast(Type::Glyph)], First.Material);

        // Set the text effects uniform for the glyph pipeline, which contains the array of text effects to be applied.
        mEncoder.SetUniform(Enum::Cast(Graphic::Pipeline::Tier::Object), mGlyphEffectsStream);

        // Draw all glyphs in the batch using a single draw call with a callback to write each instance.
        Draw<GlyphLayout>(Commands.size(), [&](Ptr<GlyphLayout> Instances)
        {
            for (UInt32 Element = 0; Element < Commands.size(); ++Element, ++Instances)
            {
                // Fetch the command for the current element.
                Ref<GlyphCommand> Command = mGlyphs[Commands[Element].Entry.Slot];

                // Write the vertex data for the current glyph command.
                (* Instances) = Command.Layout;
            }
        });
    }
}