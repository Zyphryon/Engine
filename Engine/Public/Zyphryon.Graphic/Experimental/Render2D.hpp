// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2024 by Agustin Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Graphic/Camera.hpp"
#include "Zyphryon.Graphic/Font.hpp"
#include "ShapeRenderer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=(Undocumented)=-
    // TODO: This is going to be refactored soon.
    class Render2D final
    {
    public:

        // -=(Undocumented)=-
        static constexpr UInt32 kMaxDrawPerBatch = 20'480;

        // -=(Undocumented)=-
        enum class Type : UInt8
        {
            // -=(Undocumented)=-
            Font,

            // -=(Undocumented)=-
            Sprite,
        };

        // -=(Undocumented)=-
        enum class Scope : UInt8
        {
            // -=(Undocumented)=-
            Global = 0,

            // -=(Undocumented)=-
            Effect = 1,

            // -=(Undocumented)=-
            Style  = 2,

            // -=(Undocumented)=-
            Object = 3,
        };

        // -=(Undocumented)=-
        struct ZYPHRYON_ALIGN(16) FontStyleSDF
        {
            Real32 uRounded              = 0;
            Real32 uInvThreshold         = 1.0f - 0.5f;
            Real32 uOutlineBias          = 1.0f/4.0f;
            Real32 uOutlineWidthAbsolute = 1.0f/3.0f;
            Real32 uOutlineWidthRelative = 1.0f/20.0f;
            Real32 uOutlineBlur          = 0.0f;
            Real32 Reserver[2];
            Color  uOuterColor           = Color(0, 0, 0, 1);

            // Hash function
            UInt64 Hash() const
            {
                return HashCombine(
                    uRounded,
                    uInvThreshold,
                    uOutlineBias,
                    uOutlineWidthAbsolute,
                    uOutlineWidthRelative,
                    uOutlineBlur,
                    uOuterColor);
            }
        };

    public:

        // -=(Undocumented)=-
        Render2D(Ref<Service::Host> Host);

        // -=(Undocumented)=-
        template<typename Format>
        ZYPHRYON_INLINE void SetGlobalParameters(ConstSpan<Format> Parameters)
        {
            Graphic::Stream Data = mGraphics->Allocate(Usage::Uniform, Parameters);
            mParameters[Enum::Cast(Scope::Global)] = Data;
            mShapeRenderer.SetGlobalParameters(Data);
        }

        // -=(Undocumented)=-
        template<typename Format>
        ZYPHRYON_INLINE void SetEffectParameters(ConstSpan<Format> Parameters)
        {
            mParameters[Enum::Cast(Scope::Effect)] = mGraphics->Allocate(Usage::Uniform, Parameters);
        }

        // -=(Undocumented)=-
        ZYPHRYON_INLINE void SetPipeline(Type Type, ConstTracker<Pipeline> Pipeline)
        {
            mPipelines[Enum::Cast(Type)] = Pipeline;
        }

        // -=(Undocumented)=-
        void SetFontStyle(ConstRef<FontStyleSDF> Style);

        // -=(Undocumented)=-
        ZYPHRYON_INLINE void DrawCircle(ConstRef<Circle> Shape, Real32 Depth, UInt32 Tint)
        {
            mShapeRenderer.DrawCircle(Shape, Depth, Tint);
        }

        // -=(Undocumented)=-
        ZYPHRYON_INLINE void DrawRing(ConstRef<Circle> Shape, Real32 Depth, UInt32 Tint, Real32 Thickness = 0.1f)
        {
            mShapeRenderer.DrawRing(Shape, Depth, Tint, Thickness);
        }

        // -=(Undocumented)=-
        ZYPHRYON_INLINE void DrawLine(ConstRef<Line> Shape, Real32 Depth, UInt32 Tint, Real32 Thickness = 1.0f)
        {
            mShapeRenderer.DrawLine(Shape, Depth, Tint, Thickness);
        }

        // -=(Undocumented)=-
        ZYPHRYON_INLINE void DrawStrokeRect(ConstRef<Rect> Shape, Real32 Depth, UInt32 Tint, Real32 Thickness = 1.0f)
        {
            mShapeRenderer.DrawStrokeRect(Shape, Depth, Tint, Thickness);
        }

        // -=(Undocumented)=-
        ZYPHRYON_INLINE void DrawRect(ConstRef<Rect> Shape, Real32 Depth, UInt32 Tint)
        {
            mShapeRenderer.DrawRect(Shape, Depth, Tint);
        }

        // -=(Undocumented)=-
        ZYPHRYON_INLINE void DrawRoundedRect(ConstRef<Rect> Shape, Real32 Depth, UInt32 Tint, Real32 Radius = 1.0f)
        {
            mShapeRenderer.DrawRoundedRect(Shape, Depth, Tint, Radius);
        }

        // -=(Undocumented)=-
        void DrawSprite(ConstRef<Rect> Origin, Real32 Depth, ConstRef<Rect> Uv, UInt32 Tint, ConstTracker<Material> Material);

        // -=(Undocumented)=-
        void DrawSprite(ConstRef<Matrix4x4> Transform, ConstRef<Rect> Origin, ConstRef<Rect> Uv, UInt32 Tint, ConstTracker<Material> Material);

        // -=(Undocumented)=-
        void DrawFont(ConstRef<Rect> Origin, Real32 Depth, ConstStr8 Text, Real32 Size, UInt32 Tint, ConstTracker<Font> Font);

        // -=(Undocumented)=-
        void DrawFont(ConstRef<Matrix4x4> Transform, ConstStr8 Text, Pivot Anchor, Real32 Size, UInt32 Tint, ConstTracker<Font> Font);

        // -=(Undocumented)=-
        void Flush();

        // -=(Undocumented)=-
        ZYPHRYON_INLINE Bool IsReady() const
        {
            for (ConstTracker<Graphic::Pipeline> Pipeline : mPipelines)
            {
                if (!Pipeline->HasCompleted())
                {
                    return false;
                }
            }
            return mShapeRenderer.IsReady();
        }

    public:

        ZYPHRYON_INLINE static constexpr FontStyleSDF FontStyleOutline()
        {
            return Graphic::Render2D::FontStyleSDF {
                .uInvThreshold =  1.0f - 0.5f,
                .uOutlineBias = 1.0f/4.0f,
                .uOutlineWidthAbsolute = 1.0f/3.0f,
                .uOutlineWidthRelative = 1.0f/20.0f,
                .uOutlineBlur = 0
            };
        }

        ZYPHRYON_INLINE static constexpr FontStyleSDF FontStyleThick()
        {
            return Graphic::Render2D::FontStyleSDF {
                .uInvThreshold =  1.0f - 0.5f,
                .uOutlineBias = 0.0f,
                .uOutlineWidthAbsolute = 1.0f/16.0f,
                .uOutlineWidthRelative = 1.0f/50.0f,
                .uOutlineBlur = 0
            };
        }

        ZYPHRYON_INLINE static constexpr FontStyleSDF FontStyleThicker()
        {
            return Graphic::Render2D::FontStyleSDF {
                .uInvThreshold =  1.0f - 0.6f,
                .uOutlineBias = 0.0f,
                .uOutlineWidthAbsolute = 1.0f/16.0f,
                .uOutlineWidthRelative = 1.0f/7.0f,
                .uOutlineBlur = 0
            };
        }

        ZYPHRYON_INLINE static constexpr FontStyleSDF FontStyleShadow()
        {
            return Graphic::Render2D::FontStyleSDF {
                .uInvThreshold =  1.0f - 0.5f,
                .uOutlineBias = 0.0f,
                .uOutlineWidthAbsolute = 1.0f/3.0f,
                .uOutlineWidthRelative = 1.0f/5.0f,
                .uOutlineBlur = 1
            };
        }

    private:

        // -=(Undocumented)=-
        void CreateDefaultResources(ConstTracker<Content::Service> Content);

        // -=(Undocumented)=-
        struct Command
        {
            // -=(Undocumented)=-
            UInt64             Key;

            // -=(Undocumented)=-
            Type               Kind;

            // -=(Undocumented)=-
            UInt32             Tint;

            // -=(Undocumented)=-
            Tracker<Pipeline>  Pipeline;

            // -=(Undocumented)=-
            Tracker<Material>  Material;

            // -=(Undocumented)=-
            Quad               Quad;

            // -=(Undocumented)=-
            Real32             Depth;

            // -=(Undocumented)=-
            Rect               Uv;

            // -=(Undocumented)=-
            UInt32             Instance;
        };

        // -=(Undocumented)=-
        ZYPHRYON_INLINE Ref<Command> Create(Type Kind, UInt64 Key)
        {
            if (mCommandTracker.size() == kMaxDrawPerBatch)
            {
                Flush();
            }

            Ref<Command> Command = mCommands[mCommandTracker.size()];
            Command.Key      = Key;
            Command.Kind     = Kind;
            Command.Pipeline = mPipelines[Enum::Cast(Kind)];
            mCommandTracker.push_back(&Command);
            return Command;
        }

        // -=(Undocumented)=-
        ZYPHRYON_INLINE UInt64 GetUniqueKey(Material::Kind Order, Type Type, Real32 Depth, Object Material)
        {
            Object Pipeline = mPipelines[Enum::Cast(Type)]->GetID();

            if (Order == Material::Kind::Opaque)
            {
                const UInt32 DepthBits = std::bit_cast<UInt32>(1.0f - Depth);

                // TODO: Consider Byte Endianness
                return   (static_cast<UInt64>(DepthBits)                  << 32)
                       | (static_cast<UInt64>(Material          & 0xFFFF) << 16)
                       | (static_cast<UInt64>(Pipeline          & 0xFF)   << 8)
                       | (static_cast<UInt64>(Enum::Cast(Order) & 0xFF));
            }
            else
            {
                const UInt32 DepthBits = std::bit_cast<UInt32>(Depth);

                // TODO: Consider Byte Endianness
                return   (static_cast<UInt64>(Material          & 0xFFFF) << 48)
                       | (static_cast<UInt64>(Pipeline          & 0xFF)   << 40)
                       | (static_cast<UInt64>(DepthBits)                  << 8)
                       | (static_cast<UInt64>(Enum::Cast(Order) & 0xFF));
            }
        }

        // -=(Undocumented)=-
        struct Layout
        {
            // -=(Undocumented)=-
            Vector2 Position;

            // -=(Undocumented)=-
            Real32  Depth;

            // -=(Undocumented)=-
            Vector2 Texture;

            // -=(Undocumented)=-
            UInt32  Color;

            // -=(Undocumented)=-
            UInt32  Instance;
        };

        // -=(Undocumented)=-
        void WritePrimitives(UInt32 Offset, UInt32 Count, Ref<Pipeline> Pipeline);

        // -=(Undocumented)=-
        void WriteFonts(UInt32 Offset, UInt32 Count, Ref<Pipeline> Pipeline, Ref<Material> Material);

        // -=(Undocumented)=-
        void WriteSprites(UInt32 Offset, UInt32 Count, Ref<Pipeline> Pipeline, Ref<Material> Material);

        // -=(Undocumented)=-
        void PushFontData();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Tracker<Graphic::Service>                     mGraphics;
        Encoder                                       mEncoder;
        Array<Stream, Enum::Count<Scope>()>           mParameters;
        Array<Tracker<Pipeline>, Enum::Count<Type>()> mPipelines;
        Array<Command, kMaxDrawPerBatch>              mCommands;
        Vector<Ptr<Command>, kMaxDrawPerBatch>        mCommandTracker;
        Table<UInt64, UInt32>                         mFontStylesToUniform;
        Vector<FontStyleSDF, 128>                     mFontStyles;
        UInt32                                        mFontStylesSelected = 0;
        Stream                                        mFontStream;
        ShapeRenderer                                 mShapeRenderer;
    };
}