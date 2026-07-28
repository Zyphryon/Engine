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

#include "Skeleton.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Skeleton::Skeleton(AnyRef<Content::Uri> Key)
        : AbstractResource { Move(Key) }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Skeleton::SetBones(AnyRef<Sequence<Bone>> Bones)
    {
        mBones = Move(Bones);

        mRegistry.Clear();
        mRegistry.Reserve(mBones.GetSize());

        for (UInt Index = 0; Index < mBones.GetSize(); ++Index)
        {
            ZY_ASSERT(mBones[Index].Parent < static_cast<SInt16>(Index),
                "Skeleton bones must be ordered so that a parent precedes every child naming it");

            mRegistry.Assign(mBones[Index].Name, static_cast<UInt16>(Index));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Skeleton::Rest(ConstRef<Pose> Output) const
    {
        ZY_ASSERT(Output.GetSize() >= mBones.GetSize(), "Rest pose output must have room for every bone");

        for (UInt Index = 0; Index < mBones.GetSize(); ++Index)
        {
            ConstRef<Bone> Entry = mBones[Index];

            Output.Position[Index] = Entry.Position;
            Output.Scale[Index]    = Entry.Scale;
            Output.Rotation[Index] = Entry.Rotation;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Skeleton::Compose(ConstRef<Pose> Local, Span<Matrix4x3> Output) const
    {
        ZY_ASSERT(Local.GetSize()  >= mBones.GetSize() && Output.GetSize() >= mBones.GetSize(),
            "Pose composition needs a local transform and an output slot for every bone");

        for (UInt Index = 0; Index < mBones.GetSize(); ++Index)
        {
            const Matrix4x3 Transform = Matrix4x3::FromTransform(Local.Position[Index], Local.Scale[Index], Local.Rotation[Index]);

            // The parent is already final, since the ordering guarantees it was reached first, so a single
            // multiply carries this bone the whole way into skeleton space.
            if (const SInt16 Owner = mBones[Index].Parent; Owner == kRoot)
            {
                Output[Index] = Transform;
            }
            else
            {
                Output[Index] = Output[Owner] * Transform;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Skeleton::Skin(Span<Matrix4x3> Palette) const
    {
        ZY_ASSERT(Palette.GetSize() >= mBones.GetSize(), "Bind palette needs an output slot for every bone");

        // Bones are sorted parents-first, so each world transform can compose in place from one already written.
        for (UInt Index = 0; Index < mBones.GetSize(); ++Index)
        {
            ConstRef<Bone> Entry = mBones[Index];

            const Matrix4x3 Local = Matrix4x3::FromTransform(Entry.Position, Entry.Scale, Entry.Rotation);

            if (Entry.Parent == kRoot)
            {
                Palette[Index] = Local;
            }
            else
            {
                Palette[Index] = Palette[Entry.Parent] * Local;
            }
        }

        // Folding in each bone's inverse bind pose turns the world transforms into skinning matrices.
        for (UInt Index = 0; Index < mBones.GetSize(); ++Index)
        {
            Palette[Index] *= mBones[Index].Inverse;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Skeleton::Skin(ConstSpan<Matrix4x3> Composed, Span<Matrix4x3> Palette) const
    {
        ZY_ASSERT(Composed.GetSize() >= mBones.GetSize() && Palette.GetSize() >= mBones.GetSize(),
            "Skinning needs a composed transform and an output slot for every bone");

        // Each slot reads only its own composed transform, so the palette may alias the pose it folds.
        for (UInt Index = 0; Index < mBones.GetSize(); ++Index)
        {
            Palette[Index] = Composed[Index] * mBones[Index].Inverse;
        }
    }
}