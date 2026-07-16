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

    Canvas::Canvas(Ref<Engine::Subsystem::Host> Host)
        : mService { Host.GetService<Graphic::Service>() }
    {
        CreateDefaultResources(Host.GetService<Content::Service>());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::SetTechnique(Type Type, ConstRetainer<Graphic::Technique> Technique)
    {
        mTechniques[Enum::Cast(Type)] = Technique;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::Begin()
    {
        Reset();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawCircle(ConstRef<Circle> Shape, Real32 Order, IntColor8 Tint)
    {
        Ref<ShapeCommand> Command = mShapes.Append();
        Command.Layout.Center = Shape.GetCenter();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0.SetX(Shape.GetRadius());

        constexpr UInt32              Type     = Enum::Cast(Type::Circle);
        constexpr Collector::Priority Priority = Collector::Priority::Transparent;

        const UInt16 Technique = mTechniques[Type]->GetHandle();
        mCollector.Push(Collector::Object(Type, mShapes.GetSize() - 1), Priority, Order, 0u, Technique, 0u);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawRing(ConstRef<Circle> Shape, Real32 Order, IntColor8 Tint, Real32 Thickness)
    {
        Ref<ShapeCommand> Command = mShapes.Append();
        Command.Layout.Center = Shape.GetCenter();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0.Set(Shape.GetRadius(), Thickness);

        constexpr UInt32              Type     = Enum::Cast(Type::Ring);
        constexpr Collector::Priority Priority = Collector::Priority::Transparent;

        const UInt16 Technique = mTechniques[Type]->GetHandle();
        mCollector.Push(Collector::Object(Type, mShapes.GetSize() - 1), Priority, Order, 0u, Technique, 0u);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawLine(ConstRef<Line> Shape, Real32 Order, IntColor8 Tint, Real32 Thickness)
    {
        Ref<ShapeCommand> Command = mShapes.Append();
        Command.Layout.Center = Shape.GetMidpoint();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0  = Shape.GetDirection();
        Command.Layout.Data1.Set(Shape.GetLength() * 0.5f, Thickness);

        constexpr UInt32              Type     = Enum::Cast(Type::Line);
        constexpr Collector::Priority Priority = Collector::Priority::Transparent;

        const UInt16 Technique = mTechniques[Type]->GetHandle();
        mCollector.Push(Collector::Object(Type, mShapes.GetSize() - 1), Priority, Order, 0u, Technique, 0u);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawRect(ConstRef<Rect> Shape, Real32 Order, IntColor8 Tint)
    {
        Ref<ShapeCommand> Command = mShapes.Append();
        Command.Layout.Center = Shape.GetCenter();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0  = Shape.GetSize() * 0.5f;

        constexpr UInt32              Type     = Enum::Cast(Type::Rect);
        constexpr Collector::Priority Priority = Collector::Priority::Transparent;

        const UInt16 Technique = mTechniques[Type]->GetHandle();
        mCollector.Push(Collector::Object(Type, mShapes.GetSize() - 1), Priority, Order, 0u, Technique, 0u);
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
        Ref<ShapeCommand> Command = mShapes.Append();
        Command.Layout.Center = Shape.GetCenter();
        Command.Layout.Order  = Order;
        Command.Layout.Color  = Tint;
        Command.Layout.Data0 = Vector2(Shape.GetSize() * 0.5f);
        Command.Layout.Data1.SetX(Radius);

        constexpr UInt32              Type     = Enum::Cast(Type::RoundedRect);
        constexpr Collector::Priority Priority = Collector::Priority::Transparent;

        const UInt16 Technique = mTechniques[Type]->GetHandle();
        mCollector.Push(Collector::Object(Type, mShapes.GetSize() - 1), Priority, Order, 0u, Technique, 0u);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawText(ConstRef<TextStyle> Style, Text Content, ConstRef<Matrix3x2> Transform, Real32 Order, ConstRef<TextEffect> Effect)
    {
        const GlyphEffect EffectData = InternTextEffect(Effect);

        ConstRetainer<Font> Font = Style.GetFont();

        // Calculate the origin point for the text layout based on the content, size, origin, and padding settings.
        const Real32 Size     = Style.GetSize();
        const Vector2 Spacing = Style.GetSpacing();

        // The line height in font units.
        const Real32 LineHeight = Font->GetLineHeight(Style.GetSize()) + Spacing.GetY() * Style.GetSize();

        const ConstPtr<Graphic::Material> Material = &* Font->GetMaterial();

        // Iterate through each character in the text content and generate draw commands for each glyph.
        Real32 CurrentX = 0.0f;
        Real32 CurrentY = 0.0f;
        UInt32 Previous = 0;

        constexpr UInt32 Type      = Enum::Cast(Type::Glyph);
        const     UInt16 Technique = mTechniques[Type]->GetHandle();

        StrIterateUTF8(Content, [&](UInt32 Codepoint)
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
                        Ref<GlyphCommand> Command = mGlyphs.Append();
                        Command.Generation       = EffectData.Generation;
                        Command.Material         = Material;
                        Command.Layout.Transform.SetData(Transform, Order, CastBit<Real32>(static_cast<UInt32>(EffectData.Slot)));
                        Command.Layout.Frame     = Glyph->AtlasBounds;
                        Command.Layout.Offset.Set(
                            CurrentX + Glyph->LocalBounds.GetX() * Size,
                            CurrentY + Glyph->LocalBounds.GetY() * Size);
                        Command.Layout.Size      = Vector2(
                            Glyph->LocalBounds.GetWidth()  * Size,
                            Glyph->LocalBounds.GetHeight() * Size);
                        Command.Layout.Color     = Style.GetTint();

                        mCollector.Push(
                            Collector::Object(Type, mGlyphs.GetSize() - 1),
                            Collector::Priority::Transparent, Order, EffectData.Generation,
                            Technique,
                            Material->GetHandle());
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
        Ref<SpriteCommand> Command = mSprites.Append();
        Command.Layout.Transform.SetData(Transform, Order);
        Command.Layout.Frame     = Sprite.GetFrame();
        Command.Layout.Size      = Sprite.GetSize();
        Command.Layout.Color     = Sprite.GetTint();
        Command.Material         = AddressOf(* Sprite.GetMaterial());
        Command.Technique        = AddressOf(* mTechniques[Enum::Cast(Type::Sprite)]);

        constexpr UInt32              Type     = Enum::Cast(Type::Sprite);
        constexpr Collector::Priority Priority = Collector::Priority::Opaque;

        const Graphic::Object Material = Command.Material->GetHandle();
        const Graphic::Object Pipeline = Command.Technique->GetHandle();
        mCollector.Push(Collector::Object(Type, mSprites.GetSize() - 1), Priority, Order, 0, Pipeline, Material);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::DrawSpriteAlpha(ConstRef<Sprite> Sprite, ConstRef<Matrix3x2> Transform, Real32 Order)
    {
        Ref<SpriteCommand> Command = mSprites.Append();
        Command.Layout.Transform.SetData(Transform, Order);
        Command.Layout.Frame     = Sprite.GetFrame();
        Command.Layout.Size      = Sprite.GetSize();
        Command.Layout.Color     = Sprite.GetTint();
        Command.Material         = AddressOf(* Sprite.GetMaterial());
        Command.Technique        = AddressOf(* mTechniques[Enum::Cast(Type::SpriteAlpha)]);

        constexpr UInt32              Type     = Enum::Cast(Type::SpriteAlpha);
        constexpr Collector::Priority Priority = Collector::Priority::Transparent;

        const Graphic::Object Material = Command.Material->GetHandle();
        const Graphic::Object Pipeline = Command.Technique->GetHandle();
        mCollector.Push(Collector::Object(Type, mSprites.GetSize() - 1), Priority, Order, 0, Pipeline, Material);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::Flush(Ref<Encoder> Encoder)
    {
        // Prepare any necessary resources or state before processing the collected draw calls.
        Prepare();

        // Iterate through each rendering queue in the collector and encode the batched draw calls into commands.
        mCollector.Poll([&](UInt32 Type, ConstSpan<Collector::Command> Commands)
        {
            switch (static_cast<Canvas::Type>(Type))
            {
            case Type::Circle:
            case Type::Ring:
            case Type::Line:
            case Type::Rect:
            case Type::RoundedRect:
                WriteShapes(Encoder, static_cast<Canvas::Type>(Type), Commands);
                break;
            case Type::Sprite:
            case Type::SpriteAlpha:
                WriteSprites(Encoder, Commands);
                break;
            case Type::Glyph:
                WriteGlyphs(Encoder, Commands);
                break;
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::CreateDefaultResources(ConstRetainer<Content::Service> Content)
    {
        mTechniques[Enum::Cast(Type::Circle)]      = Content->Load<Graphic::Technique>("Resources://Technique/Geometry/Circle.vfx");
        mTechniques[Enum::Cast(Type::Ring)]        = Content->Load<Graphic::Technique>("Resources://Technique/Geometry/Ring.vfx");
        mTechniques[Enum::Cast(Type::Line)]        = Content->Load<Graphic::Technique>("Resources://Technique/Geometry/Line.vfx");
        mTechniques[Enum::Cast(Type::Rect)]        = Content->Load<Graphic::Technique>("Resources://Technique/Geometry/Rect.vfx");
        mTechniques[Enum::Cast(Type::RoundedRect)] = Content->Load<Graphic::Technique>("Resources://Technique/Geometry/RoundedRect.vfx");
        mTechniques[Enum::Cast(Type::Sprite)]      = Content->Load<Graphic::Technique>("Resources://Technique/Sprite/Opaque.vfx");
        mTechniques[Enum::Cast(Type::SpriteAlpha)] = Content->Load<Graphic::Technique>("Resources://Technique/Sprite/Alpha.vfx");
        mTechniques[Enum::Cast(Type::Glyph)]       = Content->Load<Graphic::Technique>("Resources://Technique/Typography/MSDF.vfx");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::Reset()
    {
        // Clear the list of shapes after processing to prepare for the next frame.
        mShapes.Clear();

        // Clear the list of sprites after processing to prepare for the next frame.
        mSprites.Clear();

        // Clear the list of glyphs after processing to prepare for the next frame.
        mGlyphs.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::Prepare()
    {
        for (Ref<GlyphEffectPalette> Palette : mEffectPalettes)
        {
            Graphic::Transient<TextEffect> Slice = mService->AllocateTransientUniforms<TextEffect>(kMaxEffectsPerBatch);
            Slice.Copy<TextEffect>(Palette.Effects);

            Palette.Stream = Slice.GetStream();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Canvas::GlyphEffect Canvas::InternTextEffect(ConstRef<TextEffect> Effect)
    {
        const UInt64 Key = Hash(Effect);

        if (const ConstPtr<GlyphEffect> Existing = mEffectLookup.Find(Key))
        {
            return (* Existing);
        }

        if (mEffectPalettes.IsEmpty() || mEffectPalettes.GetBack().Effects.GetSize() >= kMaxEffectsPerBatch)
        {
            mEffectPalettes.Append();
        }

        Ref<GlyphEffectPalette> Palette = mEffectPalettes.GetBack();

        const GlyphEffect Result {
            .Slot       = static_cast<UInt16>(Palette.Effects.GetSize()),
            .Generation = static_cast<UInt16>(mEffectPalettes.GetSize() - 1)
        };

        Palette.Effects.Append(Effect);
        mEffectLookup.Assign(Key, Result);
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::WriteShapes(Ref<Encoder> Encoder, Type Shape, ConstSpan<Collector::Command> Commands)
    {
        // Gather each shape's per-instance layout into a single transient instance stream.
        Graphic::Transient<ShapeLayout> Instances = mService->AllocateTransientVertices<ShapeLayout>(Commands.GetSize());

        for (UInt32 Element = 0; Element < Commands.GetSize(); ++Element)
        {
            Instances[Element] = mShapes[Commands[Element].Entry.Slot].Layout;
        }

        // Draw the batch as a single instanced quad; the encoder binds the pipeline and global uniforms.
        const Graphic::Invocation Invocation {
            .Count     = 4,
            .Base      = 0,
            .Offset    = 0,
            .Instances = static_cast<UInt32>(Commands.GetSize())
        };
        Encoder.Draw(* mTechniques[Enum::Cast(Shape)], nullptr, Instances.GetStream(), Invocation);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::WriteSprites(Ref<Encoder> Encoder, ConstSpan<Collector::Command> Commands)
    {
        // Every draw call in this batch shares the same technique and material.
        Ref<SpriteCommand> First = mSprites[Commands.GetFront().Entry.Slot];

        // Gather each sprite's per-instance layout into a single transient instance stream.
        Graphic::Transient<SpriteLayout> Instances = mService->AllocateTransientVertices<SpriteLayout>(Commands.GetSize());

        for (UInt32 Element = 0; Element < Commands.GetSize(); ++Element)
        {
            Instances[Element] = mSprites[Commands[Element].Entry.Slot].Layout;
        }

        const Graphic::Invocation Invocation {
            .Count     = 4,
            .Base      = 0,
            .Offset    = 0,
            .Instances = static_cast<UInt32>(Commands.GetSize())
        };
        Encoder.Draw(* First.Technique, First.Material, Instances.GetStream(), Invocation);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Canvas::WriteGlyphs(Ref<Encoder> Encoder, ConstSpan<Collector::Command> Commands)
    {
        // Every draw call in this batch shares the same font material and effect palette.
        Ref<GlyphCommand> First = mGlyphs[Commands.GetFront().Entry.Slot];

        // Gather each glyph's per-instance layout into a single transient instance stream.
        Graphic::Transient<GlyphLayout> Instances = mService->AllocateTransientVertices<GlyphLayout>(Commands.GetSize());

        for (UInt32 Element = 0; Element < Commands.GetSize(); ++Element)
        {
            Instances[Element] = mGlyphs[Commands[Element].Entry.Slot].Layout;
        }

        const Graphic::Stream     Palette  = mEffectPalettes[First.Generation].Stream;
        const Graphic::Invocation Invocation {
            .Count     = 4,
            .Base      = 0,
            .Offset    = 0,
            .Instances = static_cast<UInt32>(Commands.GetSize())
        };
        Encoder.Draw(* mTechniques[Enum::Cast(Type::Glyph)], First.Material, Instances.GetStream(), Palette, Invocation);
    }
}