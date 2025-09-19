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

#include "Zyphryon.Graphic/Encoder.hpp"
#include "Zyphryon.Graphic/Service.hpp"
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Math/Geometry/Circle.hpp"
#include "Zyphryon.Math/Geometry/Line.hpp"
#include "Zyphryon.Math/Geometry/Rect.hpp"
#include "Zyphryon.Math/Geometry/Quad.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=(Undocumented)=-
    // TODO: This is going to be refactored soon.
    class ShapeRenderer final
    {
    public:

        // -=(Undocumented)=-
        enum class Type : UInt8
        {
            // -=(Undocumented)=-
            Circle,

            // -=(Undocumented)=-
            Ring,

            // -=(Undocumented)=-
            Line,

            // -=(Undocumented)=-
            Rect,

            // -=(Undocumented)=-
            RoundedRect,
        };

        // -=(Undocumented)=-
        static constexpr UInt32 kMaxDrawPerBatch = 4'096;

    public:

        // -=(Undocumented)=-
        ShapeRenderer(Ref<Service::Host> Host);

        // -=(Undocumented)=-
        ZYPHRYON_INLINE void SetGlobalParameters(ConstRef<Stream> Parameters)
        {
            mParameters = Parameters;
        }

        // -=(Undocumented)=-
        void DrawCircle(ConstRef<Circle> Shape, Real32 Depth, UInt32 Tint);

        // -=(Undocumented)=-
        void DrawRing(ConstRef<Circle> Shape, Real32 Depth, UInt32 Tint, Real32 Thickness = 0.1f);

        // -=(Undocumented)=-
        void DrawLine(ConstRef<Line> Shape, Real32 Depth, UInt32 Tint, Real32 Thickness = 1.0f);

        // -=(Undocumented)=-
        void DrawStrokeRect(ConstRef<Rect> Shape, Real32 Depth, UInt32 Tint, Real32 Thickness = 1.0f);

        // -=(Undocumented)=-
        void DrawRect(ConstRef<Rect> Shape, Real32 Depth, UInt32 Tint);

        // -=(Undocumented)=-
        void DrawRoundedRect(ConstRef<Rect> Shape, Real32 Depth, UInt32 Tint, Real32 Radius = 1.0f);

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
            return true;
        }

    private:

        // -=(Undocumented)=-
        struct Layout
        {
            Vector2     Center;
            Real32      Depth;
            UInt32      Color;
            Vector2     Data0;
            Vector2     Data1;
        };

        // -=(Undocumented)=-
        struct Command
        {
            UInt64  Key;
            Type    Type;
            Layout  Instance;
        };

        // -=(Undocumented)=-
        ZYPHRYON_INLINE Ref<Command> Create(Type Kind, UInt64 Key)
        {
            if (mCommandTracker.size() == kMaxDrawPerBatch)
            {
                Flush();
            }

            Ref<Command> Command = mCommands[mCommandTracker.size()];
            Command.Type = Kind;
            Command.Key  = Key;

            mCommandTracker.push_back(&Command);
            return Command;
        }

        // -=(Undocumented)=-
        ZYPHRYON_INLINE UInt64 GetUniqueKey(Type Type, Real32 Depth)
        {
            const UInt32 DepthBits = std::bit_cast<UInt32>(1.0f - Depth);

            return   (static_cast<UInt64>(DepthBits)                  << 32)
                   | (static_cast<UInt64>(Enum::Cast(Type) & 0xFF));
        }

        // -=(Undocumented)=-
        void CreateDefaultResources(ConstTracker<Content::Service> Content);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Tracker<Graphic::Service>                     mGraphics;
        Stream                                        mParameters;
        Array<Command, kMaxDrawPerBatch>              mCommands;
        Vector<Ptr<Command>, kMaxDrawPerBatch>        mCommandTracker;
        Encoder                                       mEncoder;
        Array<Tracker<Pipeline>, Enum::Count<Type>()> mPipelines;
    };

}