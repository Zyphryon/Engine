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

#include "SH2Loader.hpp"
#include "Zyphryon.Render/Sprite/Sheet2D.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Motion2D::Key ReadKey(ConstRef<JsonObject> Definition)
    {
        Motion2D::Key Key;
        Key.Row      = Definition.GetNumber<UInt8>("Row");
        Key.Keyframe = Definition.GetNumber<UInt8>("Frame");
        Key.Roll     = Angle::FromDegrees(Definition.GetNumber<Real32>("Angle"));
        Key.Turn     = Angle::FromDegrees(Definition.GetNumber<Real32>("Turn"));
        Key.Scale    = Definition.GetNumber<Real32>("Scale", 1.0f);
        Key.Layer    = Definition.GetNumber<SInt8>("Layer", 1);

        if (const JsonArray Offset = Definition.GetArray("Offset"); Offset.GetSize() == 2)
        {
            Key.Offset = Vector2(Offset.GetNumber<Real32>(0), Offset.GetNumber<Real32>(1));
        }
        return Key;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ReadFrames(ConstRef<JsonObject> Definition, Ref<Animation2D> Frames)
    {
        const Easing Curve = Definition.GetEnum("Easing", Easing::Linear);

        if (!Animation2D::IsMonotonic(Curve))
        {
            return false;
        }

        Frames.SetStatus(Definition.GetEnum("Status", Animation2D::Status::Repeat));
        Frames.SetEasing(Curve);
        Frames.SetDirections(Definition.GetNumber<UInt8>("Directions", 1));
        Frames.SetMirrored(Definition.GetBool("Mirrored"));
        Frames.SetHeading(Angle::FromDegrees(Definition.GetNumber<Real32>("Heading", 270.0f)));
        Frames.SetHysteresis(Definition.GetNumber<Real32>("Hysteresis", Animation2D::kHysteresis));

        if (const JsonArray Stride = Definition.GetArray("Stride"); !Stride.IsNullOrEmpty() && Stride.GetSize() == 2)
        {
            Frames.SetStride(Vector2(Stride.GetNumber<Real32>(0), Stride.GetNumber<Real32>(1)));
        }

        const JsonArray List = Definition.GetArray("Frames");

        for (UInt Index = 0; Index < List.GetSize(); ++Index)
        {
            const JsonObject Frame = List.GetObject(Index);
            const JsonArray  Crop  = Frame.GetArray("Crop");

            Rect Region;

            if (Crop.GetSize() == 4)
            {
                Region = Rect(Crop.GetNumber<Real32>(0),
                              Crop.GetNumber<Real32>(1),
                              Crop.GetNumber<Real32>(2),
                              Crop.GetNumber<Real32>(3));
            }

            if (!Frames.AddFrame(Region, Frame.GetNumber<Real32>("Duration")))
            {
                return false;
            }
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void ReadMotion(ConstRef<JsonObject> Definition, Ref<Motion2D> Motion)
    {
        Motion.SetCycle(Definition.GetNumber<UInt32>("Cycle"));

        if (const JsonArray Lanes = Definition.GetArray("Lanes"); !Lanes.IsNullOrEmpty())
        {
            for (UInt Slot = 0; Slot < Lanes.GetSize(); ++Slot)
            {
                const JsonObject Track = Lanes.GetObject(Slot);
                const JsonArray  Keys  = Track.GetArray("Keys");

                Ref<Motion2D::Lane> Lane = Motion.AddLane(Hash(Track.GetString("Bone")));
                Lane.Keys.Reserve(Keys.GetSize());

                for (UInt Step = 0; Step < Keys.GetSize(); ++Step)
                {
                    Lane.Keys.Append(ReadKey(Keys.GetObject(Step)));
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool SH2Loader::Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Sheet2D> Asset = Retainer<Sheet2D>::Cast(Scope.GetResource());

        JsonValue        Document = JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()));
        const JsonObject Root(Document);

        if (!Root.IsValid())
        {
            LOG_W("'{0}' is not a valid 2D sheet", Scope.GetResource()->GetKey());
            return false;
        }

        Sequence<Skeleton2D::Bone> Bones;

        if (const JsonArray List = Root.GetArray("Bones"); !List.IsNullOrEmpty())
        {
            for (UInt Index = 0; Index < List.GetSize(); ++Index)
            {
                const Text Name = List.GetString(Index);

                Ref<Skeleton2D::Bone> Bone = Bones.Append();
                Bone.Name  = Hash(Name);
                Bone.Label = Name;
            }
        }

        Skeleton2D Skeleton;
        Skeleton.SetBones(Move(Bones));

        Sequence<Sheet2D::Clip> Clips;

        if (const JsonArray List = Root.GetArray("Clips"); !List.IsNullOrEmpty())
        {
            for (UInt Index = 0; Index < List.GetSize(); ++Index)
            {
                const JsonObject Definition = List.GetObject(Index);
                const Text       Name       = Definition.GetString("Name");

                Ref<Sheet2D::Clip> Clip = Clips.Append();
                Clip.Name  = Hash(Name);
                Clip.Label = Name;

                if (!ReadFrames(Definition, Clip.Frames))
                {
                    LOG_W("'{0}' has a clip with too many frames or an easing that steps back", Scope.GetResource()->GetKey());
                    return false;
                }
                ReadMotion(Definition, Clip.Motion);
            }
        }

        // Bones first, so the clips are laid out over them once, as they arrive.
        Asset->SetSkeleton(Move(Skeleton));

        if (!Asset->SetClips(Move(Clips)))
        {
            LOG_W("'{0}' has lanes for bones it does not list, or two lanes for one bone", Scope.GetResource()->GetKey());
        }
        return true;
    }
}