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

#include "Animation.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    static Bool Agrees(ConstRef<Track<Type>> Track, Real32 Rate, UInt Count)
    {
        return Track.IsEmpty() || (Track.GetRate() == Rate && Track.GetSize() == Count);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool IsLockstep(ConstRef<Animation::Lane> Entry)
    {
        const UInt   Count = Max(Entry.Position.GetSize(),
                             Max(Entry.Scale.GetSize(), Entry.Rotation.GetSize()));
        const Real32 Rate  = Max(Entry.Position.GetRate(),
                             Max(Entry.Scale.GetRate(), Entry.Rotation.GetRate()));

        if (Count == 0 || Rate <= 0.0f)
        {
            return false;
        }

        return Agrees(Entry.Position, Rate, Count)
            && Agrees(Entry.Scale, Rate, Count)
            && Agrees(Entry.Rotation, Rate, Count);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Animation::Animation(AnyRef<Content::Uri> Key)
        : AbstractResource { Move(Key) },
          mLockstep        { false },
          mDuration        { 0.0 },
          mExtent          { Sphere::Invalid() }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animation::Arrange(AnyRef<Sequence<Lane>> Lanes)
    {
        mLockstep = !Lanes.IsEmpty();
        mLanes    = Move(Lanes);
        mDuration = 0.0;

        for (ConstRef<Lane> Entry : mLanes)
        {
            mDuration = Max(mDuration, Entry.Position.GetDuration());
            mDuration = Max(mDuration, Entry.Scale.GetDuration());
            mDuration = Max(mDuration, Entry.Rotation.GetDuration());

            mLockstep = mLockstep && IsLockstep(Entry);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animation::Sample(Real64 Time, ConstRef<Skeleton> Skeleton, ConstRef<Skeleton::Pose> Output) const
    {
        ZY_ASSERT(Output.GetSize() >= Skeleton.GetBones().GetSize(),
            "Sampling needs a pose slot for every bone the skeleton carries");

        if (mLockstep)
        {
            for (ConstRef<Lane> Entry : mLanes)
            {
                if (const SInt32 Bone = Skeleton.Find(Entry.Name); Bone != Skeleton::kMissing)
                {
                    const Cursor Cursor = Entry.Locate(Time);

                    Output.Position[Bone] = Entry.Position.Sample(Cursor, Output.Position[Bone]);
                    Output.Scale[Bone]    = Entry.Scale.Sample(Cursor, Output.Scale[Bone]);
                    Output.Rotation[Bone] = Entry.Rotation.Sample(Cursor, Output.Rotation[Bone]);
                }
            }
        }
        else
        {
            for (ConstRef<Lane> Entry : mLanes)
            {
                if (const SInt32 Bone = Skeleton.Find(Entry.Name); Bone != Skeleton::kMissing)
                {
                    Output.Position[Bone] = Entry.Position.Sample(Time, Output.Position[Bone]);
                    Output.Scale[Bone]    = Entry.Scale.Sample(Time, Output.Scale[Bone]);
                    Output.Rotation[Bone] = Entry.Rotation.Sample(Time, Output.Rotation[Bone]);
                }
            }
        }
    }
}
