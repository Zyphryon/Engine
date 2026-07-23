// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Sprite.hpp"
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Graphic/Technique.hpp"
#include "Zyphryon.Math/Color.hpp"
#include "Zyphryon.Math/Geometry/Circle.hpp"
#include "Zyphryon.Math/Geometry/Line.hpp"
#include "Zyphryon.Render/Collector.hpp"
#include "Zyphryon.Render/Encoder.hpp"
#include "Zyphryon.Render/Typography/Font.hpp"
#include "Zyphryon.Render/Typography/FontEffect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief A 2D draw-call generator for basic shapes, text, and sprites.
    ///
    /// Emits sorted, batched draw calls into a \ref Collector during a frame, then encodes them through an
    /// \ref Encoder — it never opens a target nor binds uniforms itself. Intended to be driven from a pass's Run.
    class Canvas final
    {
    public:

        /// \brief Enumeration of supported types that can be drawn on the canvas.
        enum class Type : UInt8
        {
            Circle,         ///< A filled circle shape defined by a center point and radius.
            Ring,           ///< A ring shape defined by a center point, inner radius, and outer radius.
            Line,           ///< A line shape defined by a start point and end point.
            Rect,           ///< A filled rectangle shape defined by a center point, width, and height.
            RoundedRect,    ///< A filled rectangle with rounded corners defined by a center point.
            Sprite,         ///< A sprite for drawing textured quads.
            SpriteAlpha,    ///< A transparent sprite for drawing textured quads.
            Glyph,          ///< A text glyph for drawing characters from a font atlas.
        };

    public:

        /// \brief Constructs a canvas with the specified service host.
        ///
        /// \param Host The service host to use for rendering operations.
        Canvas(Ref<Engine::Subsystem::Host> Host);

        /// \brief Override the technique to use for subsequent draw calls.
        ///
        /// \param Type      The type to change the technique.
        /// \param Technique The technique to set for subsequent draw calls.
        void SetTechnique(Type Type, ConstRetainer<Graphic::Technique> Technique);

        /// \brief Begins a new frame, binding the collector draw calls are emitted into.
        void Begin();

        /// \brief Issues a draw command for drawing a circle shape with the specified parameters.
        ///
        /// \param Shape The circle shape to draw.
        /// \param Order The depth value for rendering order.
        /// \param Tint  The tint color to apply to the circle.
        void DrawCircle(ConstRef<Circle> Shape, Real32 Order, IntColor8 Tint);

        /// \brief Issues a draw command for drawing a ring shape (circle outline) with the specified parameters.
        ///
        /// \param Shape     The circle shape defining the ring to draw.
        /// \param Order     The depth value for rendering order.
        /// \param Tint      The tint color to apply to the ring.
        /// \param Thickness The thickness of the ring outline (default is 0.1f).
        void DrawRing(ConstRef<Circle> Shape, Real32 Order, IntColor8 Tint, Real32 Thickness = 0.1f);

        /// \brief Issues a draw command for drawing a line segment with the specified parameters.
        ///
        /// \param Shape     The line segment to draw.
        /// \param Order     The depth value for rendering order.
        /// \param Tint      The tint color to apply to the line.
        /// \param Thickness The thickness of the line (default is 1.0f).
        void DrawLine(ConstRef<Line> Shape, Real32 Order, IntColor8 Tint, Real32 Thickness = 1.0f);

        /// \brief Issues a draw command for drawing a quadrilateral shape with the specified parameters.
        ///
        /// \param Shape The quadrilateral shape to draw.
        /// \param Order The depth value for rendering order.
        /// \param Tint  The tint color to apply to the quadrilateral.
        void DrawRect(ConstRef<Rect> Shape, Real32 Order, IntColor8 Tint);

        /// \brief Issues a draw command for drawing a rectangle outline with the specified parameters.
        ///
        /// \param Shape     The rectangle shape defining the outline to draw.
        /// \param Order     The depth value for rendering order.
        /// \param Tint      The tint color to apply to the rectangle outline.
        /// \param Thickness The thickness of the rectangle outline (default is 1.0f).
        void DrawStrokeRect(ConstRef<Rect> Shape, Real32 Order, IntColor8 Tint, Real32 Thickness = 1.0f);

        /// \brief Issues a draw command for drawing a rounded rectangle shape with the specified parameters.
        ///
        /// \param Shape  The rectangle shape defining the rounded rectangle to draw.
        /// \param Order  The depth value for rendering order.
        /// \param Tint   The tint color to apply to the rounded rectangle.
        /// \param Radius The radius of the rounded corners (default is 1.0f).
        void DrawRoundedRect(ConstRef<Rect> Shape, Real32 Order, IntColor8 Tint, Real32 Radius = 1.0f);

        /// \brief Issues a draw command for drawing text with the specified parameters.
        ///
        /// \param Font      The font resource providing the glyph metrics and atlas used to render the text.
        /// \param Size      The font size, in points.
        /// \param Spacing   The horizontal (between characters) and vertical (between lines) spacing adjustments.
        /// \param Content   The UTF-8 encoded string content of the text to draw.
        /// \param Transform The transformation matrix to apply to the text for positioning, scaling, and rotation.
        /// \param Order     The depth value for rendering order.
        /// \param Tint      The tint color to apply to the glyphs.
        /// \param Effect    The text effect to apply to the text.
        void DrawText(ConstRetainer<Font> Font, Real32 Size, Vector2 Spacing, Text Content, ConstRef<Matrix3x2> Transform, Real32 Order, IntColor8 Tint, ConstRef<FontEffect> Effect);

        /// \brief Issues a draw command for drawing a sprite with the specified parameters.
        ///
        /// \param Sprite    The sprite to draw, containing texture and source rectangle information.
        /// \param Transform The transformation matrix to apply to the sprite for positioning, scaling, and rotation.
        /// \param Order     The depth value for rendering order.
        void DrawSprite(ConstRef<Sprite> Sprite, ConstRef<Matrix3x2> Transform, Real32 Order);

        /// \brief Issues a draw command for drawing an alpha sprite with the specified parameters.
        ///
        /// \param Sprite    The sprite to draw, containing texture and source rectangle information.
        /// \param Transform The transformation matrix to apply to the sprite for positioning, scaling, and rotation.
        /// \param Order     The depth value for rendering order.
        void DrawSpriteAlpha(ConstRef<Sprite> Sprite, ConstRef<Matrix3x2> Transform, Real32 Order);

        /// \brief Drains the collected draw calls, encoding them as batched draw commands.
        ///
        /// \param Encoder The encoder that builds and binds the resulting draw commands.
        void Flush(Ref<Encoder> Encoder);

    private:

        /// \brief Maximum number of text effects that can be batched together in a single draw call.
        static constexpr UInt32 kMaxEffectsPerBatch = 64;

        /// \brief Defines a type alias for an array of trackers to graphics techniques, indexed by an enumeration type.
        using Techniques = Array<Retainer<Graphic::Technique>, Enum::Count<Type>()>;

        /// \brief Defines a structure representing a 2x3 transformation matrix with an additional depth value.
        struct Matrix2x3Packed final
        {
            /// \brief The first column of the 2x3 transformation matrix.
            Vector3 Column0;

            /// \brief The depth value for rendering order, where lower values are rendered behind higher values.
            Real32  Order;

            /// \brief The second column of the 2x3 transformation matrix.
            Vector3 Column1;

            /// \brief Additional user custom data that can be used for various purposes.
            Real32  Custom;

            /// \brief Sets the values of the matrix from a 3x2 transformation matrix and a depth value.
            ///
            /// \param Matrix The 3x2 transformation matrix to convert into the packed format.
            /// \param Depth  The depth value to set for rendering order.
            /// \param Data   The additional custom data to set (default is 0.0f).
            ZY_INLINE void SetData(ConstRef<Matrix3x2> Matrix, Real32 Depth, Real32 Data = 0.0f)
            {
                Column0 = Matrix.GetColumn(0);
                Order   = Depth;
                Column1 = Matrix.GetColumn(1);
                Custom  = Data;
            }
        };

        /// \brief Defines a structure representing the input data for drawing a shape in the GPU.
        struct ShapeLayout final
        {
            /// Center point of the shape, used for positioning and transformations.
            Vector2     Center;

            /// Depth value for rendering order, where lower values are rendered behind higher values.
            Real32      Order;

            /// Color tint to apply to the shape, represented as an 8-bit integer color (RGBA).
            IntColor8   Color;

            /// Additional data for the shape, such as radius for circles or size for rectangles, stored as a 2D vector.
            Vector2     Data0;

            /// Additional data for the shape, such as thickness for rings or lines, stored as a 2D vector.
            Vector2     Data1;
        };

        /// \brief Defines a draw command for a shape, containing its input data.
        struct ShapeCommand final
        {
            /// The input data for the shape.
            ShapeLayout Layout;
        };

        /// \brief Defines a structure representing the input data for drawing a sprite in the GPU.
        struct SpriteLayout final
        {
            /// The transformation matrix to apply to the sprite for positioning, scaling, and rotation.
            Matrix2x3Packed Transform;

            /// The source rectangle within the sprite's texture, defining the portion of the texture to use.
            Rect            Frame;

            /// Additional data for the sprite, such as size stored as a 2D vector.
            Vector2         Size;

            /// Color tint to apply to the sprite, represented as an 8-bit integer color (RGBA).
            IntColor8       Color;
        };

        /// \brief Defines a draw command for a sprite, containing its input data.
        struct SpriteCommand final
        {
            /// The graphics technique to use for rendering the sprite.
            ConstPtr<Graphic::Technique> Technique;

            /// The material to use for rendering the sprite, containing shader parameters and resources.
            ConstPtr<Graphic::Material>  Material;

            /// The input data for the sprite.
            SpriteLayout                 Layout;
        };

        /// \brief Defines a structure representing the input data for drawing a glyph in the GPU.
        struct GlyphLayout final
        {
            /// The transformation matrix to apply to the text for positioning, scaling, and rotation.
            Matrix2x3Packed Transform;

            /// The source rectangle within the text's texture, defining the portion of the texture to use.
            Rect            Frame;

            /// Additional data for the text, such as offset stored as a 2D vector, used for positioning glyphs relative to the text layout.
            Vector2         Offset;     // TODO: UInt16x2

            /// Additional data for the text, such as size stored as a 2D vector.
            Vector2         Size;       // TODO: UInt16x2

            /// Color tint to apply to the text, represented as an 8-bit integer color (RGBA).
            IntColor8       Color;
        };

        /// \brief Structure representing a draw command for a glyph, containing its input data.
        struct GlyphCommand final
        {
            /// The material to use for rendering the glyph, containing the font atlas and shader parameters.
            ConstPtr<Graphic::Material> Material;

            /// The generation of the glyph, used for tracking updates and changes.
            UInt16                      Generation;

            /// The input data for the glyph.
            GlyphLayout                 Layout;
        };

        /// \brief Structure representing a text effect applied to a glyph.
        struct GlyphEffect
        {
            /// The slot index of the text effect, used for managing multiple effects.
            UInt16 Slot;

            /// The generation of the text effect, used for tracking updates and changes.
            UInt16 Generation;
        };

        /// \brief Structure representing a palette of text effects, containing the stream, lookup table, and sequence of effects.
        struct GlyphEffectPalette
        {
            /// The stream containing the text effect data.
            Graphic::Stream           Stream;

            /// The lookup table mapping text effects to their corresponding indices.
            Table<FontEffect, UInt32> Lookup;

            /// The sequence of text effects in the palette.
            Sequence<FontEffect>      Effects;
        };

        /// \brief Creates default rendering resources such as pipelines and materials for the renderer.
        ///
        /// \param Content The content service used to load necessary resources for rendering.
        void CreateDefaultResources(ConstRetainer<Content::Service> Content);

        /// \brief Resets the renderer's internal state, clearing any pending commands and bindings.
        void Reset();

        /// \brief Prepares the canvas for rendering by setting up internal state and resources.
        void Prepare();

        /// \brief Interns a text effect, returning a glyph effect that can be used for rendering.
        ///
        /// \param Effect The text effect to intern.
        /// \return The interned glyph effect.
        GlyphEffect InternFontEffect(ConstRef<FontEffect> Effect);

        /// \brief Writes a batch of shape draw calls as a single instanced command through the encoder.
        ///
        /// \param Encoder  The encoder that builds the resulting draw command.
        /// \param Shape    The type of shape being drawn, which determines the rendering pipeline used.
        /// \param Commands The span of shape draw calls to be processed and encoded for rendering.
        void WriteShapes(Ref<Encoder> Encoder, Type Shape, ConstSpan<Collector::Command> Commands);

        /// \brief Writes a batch of sprite draw calls as a single instanced command through the encoder.
        ///
        /// \param Encoder  The encoder that builds the resulting draw command.
        /// \param Commands The span of sprite draw calls to be processed and encoded for rendering.
        void WriteSprites(Ref<Encoder> Encoder, ConstSpan<Collector::Command> Commands);

        /// \brief Writes a batch of glyph draw calls as a single instanced command through the encoder.
        ///
        /// \param Encoder  The encoder that builds the resulting draw command.
        /// \param Commands The span of glyph draw calls to be processed and encoded for rendering.
        void WriteGlyphs(Ref<Encoder> Encoder, ConstSpan<Collector::Command> Commands);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Graphic::Service>     mService;
        Collector                      mCollector;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Techniques                     mTechniques;
        Sequence<ShapeCommand>         mShapes;
        Sequence<SpriteCommand>        mSprites;
        Sequence<GlyphCommand>         mGlyphs;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<GlyphEffectPalette>   mEffectPalettes;
        Table<UInt64, GlyphEffect>     mEffectLookup;
    };
}