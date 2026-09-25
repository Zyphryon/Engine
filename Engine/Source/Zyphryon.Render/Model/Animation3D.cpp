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

#include "Animation3D.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Animation3D::Animation3D(AnyRef<ZyContent::Uri> Key)
        : AbstractResource { Move(Key) },
          mLockstep        { false },
          mDuration        { 0.0 },
          mExtent          { Sphere::Invalid() }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animation3D::Arrange(AnyRef<Sequence<Lane>> Lanes)
    {
        mLockstep = !Lanes.IsEmpty();
        mLanes    = Move(Lanes);
        mDuration = 0.0;

        for (ConstRef<Lane> Entry : mLanes)
        {
            mDuration = Max(mDuration, Entry.Position.GetDuration());
            mDuration = Max(mDuration, Entry.Scale.GetDuration());
            mDuration = Max(mDuration, Entry.Rotation.GetDuration());

            mLockstep = mLockstep && Entry.IsLockstep();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Animation3D::Resolve(ConstRef<Skeleton3D> Skeleton, Ref<Sequence<SInt32>> Binding) const
    {
        Binding.Clear();
        Binding.Reserve(mLanes.GetSize());

        UInt Reached = 0;

        for (ConstRef<Lane> Entry : mLanes)
        {
            const SInt32 Bone = Skeleton.Find(Entry.Name);

            Binding.Append(Bone);

            if (Bone != Skeleton3D::kMissing && !Entry.Position.IsEmpty() && !Entry.Scale.IsEmpty() && !Entry.Rotation.IsEmpty())
            {
                ++Reached;
            }
        }
        return (Reached == Skeleton.GetBones().GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animation3D::Sample(Real64 Time, ConstSpan<SInt32> Binding, ConstRef<Skeleton3D::Pose> Output) const
    {
        ZY_ASSERT(Binding.GetSize() >= mLanes.GetSize(), "Sampling needs a resolution for every lane");

        if (mLockstep)
        {
            for (UInt Index = 0; Index < mLanes.GetSize(); ++Index)
            {
                if (const SInt32 Bone = Binding[Index]; Bone != Skeleton3D::kMissing)
                {
                    ConstRef<Lane> Entry  = mLanes[Index];
                    const Cursor   Cursor = Entry.Timing.Locate(Time);

                    Output.Position[Bone] = Entry.Position.Sample(Cursor, Output.Position[Bone]);
                    Output.Scale[Bone]    = Entry.Scale.Sample(Cursor, Output.Scale[Bone]);
                    Output.Rotation[Bone] = Entry.Rotation.Sample(Cursor, Output.Rotation[Bone]);
                }
            }
        }
        else
        {
            for (UInt Index = 0; Index < mLanes.GetSize(); ++Index)
            {
                if (const SInt32 Bone = Binding[Index]; Bone != Skeleton3D::kMissing)
                {
                    ConstRef<Lane> Entry = mLanes[Index];

                    Output.Position[Bone] = Entry.Position.Sample(Time, Output.Position[Bone]);
                    Output.Scale[Bone]    = Entry.Scale.Sample(Time, Output.Scale[Bone]);
                    Output.Rotation[Bone] = Entry.Rotation.Sample(Time, Output.Rotation[Bone]);
                }
            }
        }
    }
}