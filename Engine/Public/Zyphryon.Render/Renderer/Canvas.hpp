            /// The graphics pipeline to use for rendering the sprite.
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

#include "Collector.hpp"
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Graphic/Encoder.hpp"
#include "Zyphryon.Graphic/Service.hpp"
#include "Zyphryon.Math/Matrix3x2.hpp"
#include "Zyphryon.Math/Geometry/Circle.hpp"
#include "Zyphryon.Math/Geometry/Line.hpp"
#include "Zyphryon.Render/Sprite/Sprite.hpp"
#include "Zyphryon.Render/Typography/Text.hpp"
#include "Zyphryon.Render/Typography/TextEffect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief A 2D canvas that draws basic shapes, text, and sprites using a graphics service.
    class Canvas final
    {
    public:

        /// \brief Constructs a canvas with the specified service host.
        ///
        /// \param Host The service host to use for rendering operations.
        Canvas(Ref<Service::Host> Host);

        /// \brief Sets the transformation matrix for the canvas, which will be applied to all during this session.
        ///
        /// \param Transform The transformation matrix to apply to all rendered shapes and sprites during this session.
        void SetTransform(ConstRef<Matrix4x4> Transform);

        /// \brief Sets the scissor rectangle for clipping rendered objects to a specific region.
        ///
        /// \param Scissor The scissor rectangle defining the clipping region for rendered objects.
        void SetScissor(Graphic::Scissor Scissor);

        /// \brief Sets the graphics pipeline to use for subsequent draw calls (only Sprites).
        ///
        /// \param Pipeline The graphics pipeline to set for subsequent draw calls.
        void SetPipeline(ConstTracker<Graphic::Pipeline> Pipeline);

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
        /// \param Radius The radius of the rounded corners (default is 1.0
        void DrawRoundedRect(ConstRef<Rect> Shape, Real32 Order, IntColor8 Tint, Real32 Radius = 1.0f);

        /// \brief Sets the parameters for a text effect at the specified index, which can be applied to text rendering.
        ///
        /// \param Index  The index of the text effect to set, which corresponds to the effect slot used in text rendering.
        /// \param Effect The text effect parameters to set for the specified index.
        void SetTextEffect(UInt8 Index, ConstRef<TextEffect> Effect);

        /// \brief Issues a draw command for drawing text with the specified parameters.
        ///
        /// \param Text      The text to draw, containing font, size, tint, and content information.
        /// \param Transform The transformation matrix to apply to the text for positioning, scaling, and rotation.
        /// \param Order     The depth value for rendering order.
        /// \param Effect    The text effect to apply to the text (default is 0, which means no effect).
        void DrawText(ConstRef<Text> Text, ConstStr8 Content, ConstRef<Matrix3x2> Transform, Real32 Order, UInt8 Effect = 0);

        /// \brief Issues a draw command for drawing a sprite with the specified parameters.
        ///
        /// \param Sprite    The sprite to draw, containing texture and source rectangle information.
        /// \param Transform The transformation matrix to apply to the sprite for positioning, scaling, and rotation.
        /// \param Order     The depth value for rendering order.
        void DrawSprite(ConstRef<Sprite> Sprite, ConstRef<Matrix3x2> Transform, Real32 Order);

        /// \brief Flushes all pending draw commands to the graphics service.
        void Flush();

    private:

        /// \brief Creates default rendering resources such as pipelines and materials for the renderer.
        ///
        /// \param Content The content service used to load necessary resources for rendering.
        void CreateDefaultResources(ConstTracker<Content::Service> Content);

        /// \brief Prepares the renderer for a new frame, resetting internal state and preparing for new draw commands.
        void Prepare();

        /// \brief Resets the renderer's internal state, clearing any pending commands and bindings.
        void Reset();

        /// \brief Binds the specified pipeline and material for subsequent draw calls.
        ///
        /// \param Pipeline The graphics pipeline to bind for rendering.
        /// \param Material The material to bind for rendering, containing shader parameters and resources.
        void Bind(ConstPtr<Graphic::Pipeline> Pipeline, ConstPtr<Graphic::Material> Material);

        /// \brief Issues a draw command using the specified vertex and index formats and a callback to write vertex and index data.
        ///
        /// \param Vertices The number of vertices to allocate for drawing.
        /// \param Indices  The number of indices to allocate for drawing.
        /// \param Callback A callback function that writes vertex and index data into the allocated buffers.
        template<typename Vertex, typename Index, typename Writer>
        ZYPHRYON_INLINE void Draw(UInt32 Vertices, UInt32 Indices, AnyRef<Writer> Callback)
        {
            // Allocate transient buffers for vertex and index data.
            const auto [VtxData, VtxStream] = mService->AllocateTransientBuffer<Vertex>(Graphic::Usage::Vertex, Vertices);
            const auto [IdxData, IdxStream] = mService->AllocateTransientBuffer<Index>(Graphic::Usage::Index, Indices);

            // Invoke the callback to write vertex and index data into the allocated buffers.
            Callback(VtxData, IdxData);

            // Set the vertex and index buffers for drawing and issue the draw command.
            mEncoder.SetVertices(0, VtxStream.Buffer, 0, VtxStream.Stride);
            mEncoder.SetIndices(IdxStream.Buffer, 0, IdxStream.Stride);
            mEncoder.Draw(Indices, VtxStream.Offset / sizeof(Vertex), IdxStream.Offset / sizeof(Index));
            mEncoder.ResetBindings();
        }

        /// \brief Issues a draw command using the specified vertex format and a callback to write instances data.
        ///
        /// \param Instances The number of instances to allocate for drawing.
        /// \param Callback  A callback function that writes vertex data into the allocated buffer.
        template<typename Vertex, typename Writer>
        ZYPHRYON_INLINE void Draw(UInt32 Instances, AnyRef<Writer> Callback)
        {
            // Allocate transient buffers for vertex data.
            const auto [VtxData, VtxStream] = mService->AllocateTransientBuffer<Vertex>(Graphic::Usage::Vertex, Instances);

            // Invoke the callback to write vertex data into the allocated buffers.
            Callback(VtxData);

            // Set the vertex buffers for drawing and issue the draw command.
            mEncoder.SetVertices(0, VtxStream);
            mEncoder.SetIndices(0, 0, 0);
            mEncoder.Draw(4, 0, 0, Instances);
            mEncoder.ResetBindings();
        }

    private:

        /// \brief Enumeration of supported types that can be drawn on the canvas.
        enum class Type : UInt8
        {
            Circle,         ///< A filled circle shape defined by a center point and radius.
            Ring,           ///< A ring shape defined by a center point, inner radius, and outer radius.
            Line,           ///< A line shape defined by a start point and end point.
            Rect,           ///< A filled rectangle shape defined by a center point, width, and height.
            RoundedRect,    ///< A filled rectangle with rounded corners defined by a center point.
            Sprite,         ///< A sprite for drawing textured quads.
            Glyph,          ///< A text glyph for drawing characters from a font atlas.
        };

        /// \brief Defines a type alias for an array of trackers to graphics pipelines, indexed by an enumeration type.
        using Pipelines = Array<Tracker<Graphic::Pipeline>, Enum::Count<Type>()>;

        /// \brief Defines a structure representing a 2x3 transformation matrix with an additional depth value.
        struct ZYPHRYON_ALIGN(16) Matrix2x3Packed final
        {
            /// \brief The first column of the 2x3 transformation matrix.
            Vector3 Column0;

            /// \brief The depth value for rendering order, where lower values are rendered behind higher values.
            Real32  Order;

            /// \brief The second column of the 2x3 transformation matrix.
            Vector3 Column1;

            /// \brief Sets the values of the matrix from a 3x2 transformation matrix and a depth value.
            ///
            /// \param Matrix The 3x2 transformation matrix to convert into the packed format.
            /// \param Depth  The depth value to set for rendering order.
            ZYPHRYON_INLINE void SetData(ConstRef<Matrix3x2> Matrix, Real32 Depth)
            {
                Column0 = Matrix.GetColumn(0);
                Order   = Depth;
                Column1 = Matrix.GetColumn(1);
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

        /// \brief Structure representing a draw command for a shape, containing its input data.
        struct ShapeCommand final
        {
            /// The input data for the sprite.
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

        /// \brief Structure representing a draw command for a sprite, containing its input data.
        struct SpriteCommand final
        {
            /// The graphics pipeline to use for rendering the sprite.
            ConstPtr<Graphic::Pipeline> Pipeline;

            /// The material to use for rendering the sprite, containing shader parameters and resources.
            ConstPtr<Graphic::Material> Material;

            /// The input data for the sprite.
            SpriteLayout                Layout;
        };

        /// \brief Defines a structure representing the input data for drawing a glyph in the GPU.
        struct GlyphLayout final
        {
            /// The transformation matrix to apply to the text for positioning, scaling, and rotation.
            Matrix2x3Packed Transform;

            /// The source rectangle within the text's texture, defining the portion of the texture to use.
            Rect            Frame;

            /// Additional data for the text, such as offset stored as a 2D vector, used for positioning glyphs relative to the text layout.
            Vector2         Offset;

            /// Additional data for the text, such as size stored as a 2D vector.
            Vector2         Size;

            /// Color tint to apply to the text, represented as an 8-bit integer color (RGBA).
            IntColor8       Color;

            /// The index of the text effect to apply to the glyph.
            UInt8           Effect;
        };

        /// \brief Structure representing a draw command for a glyph, containing its input data.
        struct GlyphCommand final
        {
            /// The material to use for rendering the glyph, containing the font atlas and shader parameters.
            ConstPtr<Graphic::Material> Material;

            /// The input data for the glyph.
            GlyphLayout                 Layout;
        };

        /// \brief Writes a batch of shape draw commands to the graphics encoder for rendering.
        ///
        /// \param Shape   The type of shape being drawn, which determines the rendering pipeline used.
        /// \param Commands A span of shape draw commands to be processed and encoded for rendering.
        void WriteShapes(Type Shape, ConstSpan<Collector::Command> Commands);

        /// \brief Writes a batch of sprite draw commands to the graphics encoder for rendering.
        ///
        /// \param Commands A span of sprite draw commands to be processed and encoded for rendering.
        void WriteSprites(ConstSpan<Collector::Command> Commands);

        /// \brief Writes a batch of glyph draw commands to the graphics encoder for rendering.
        ///
        /// \param Commands A span of glyph draw commands to be processed and encoded for rendering.
        void WriteGlyphs(ConstSpan<Collector::Command> Commands);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Tracker<Graphic::Service> mService;
        Graphic::Encoder          mEncoder;
        Collector                 mCollector;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Pipelines                 mPipelines;
        Vector<ShapeCommand>      mShapes;
        Vector<SpriteCommand>     mSprites;
        Vector<GlyphCommand>      mGlyphs;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<TextEffect, 64>     mGlyphEffects;
        Graphic::Stream           mGlyphEffectsStream;
    };
}