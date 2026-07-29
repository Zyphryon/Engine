// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Content/Resource.hpp"
#include "Zyphryon.Math/Matrix4x3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Represents the bone hierarchy a model's geometry is skinned to.
    class Skeleton final : public Content::AbstractResource<Skeleton>
    {
    public:

        /// \brief The value a bone uses to say it hangs from nothing.
        static constexpr SInt16 kRoot    = -1;

        /// \brief The value returned for a bone the skeleton does not carry.
        static constexpr SInt32 kMissing = -1;

        /// \brief A single bone, carrying its parent link, rest-pose transform, and inverse bind matrix.
        struct Bone final
        {
            /// The hash of the bone's name.
            UInt64     Name     = 0;

            /// The index of the bone this one hangs from, or \ref kRoot for a bone that hangs from nothing.
            SInt16     Parent   = kRoot;

            /// The bone's resting position, relative to its parent.
            Vector3    Position;

            /// The bone's resting scale, relative to its parent.
            Vector3    Scale    = Vector3(1.0f, 1.0f, 1.0f);

            /// The bone's resting rotation, relative to its parent.
            Quaternion Rotation;

            /// The matrix that takes a vertex from the bind pose into this bone's space.
            Matrix4x3  Inverse  = Matrix4x3::Identity();
        };

        /// \brief A view over the local transform of every bone, split by channel.
        struct Pose final
        {
            /// The local position of every bone, relative to its parent.
            Span<Vector3>    Position;

            /// The local scale of every bone, relative to its parent.
            Span<Vector3>    Scale;

            /// The local rotation of every bone, relative to its parent.
            Span<Quaternion> Rotation;

            /// \brief Gets the number of bones the pose has room for.
            ///
            /// \return The number of bones every channel covers.
            ZY_INLINE UInt GetSize() const
            {
                ZY_ASSERT(Position.GetSize() == Scale.GetSize() && Scale.GetSize() == Rotation.GetSize(),
                    "Pose channels must cover the same bones");

                return Position.GetSize();
            }
        };

    public:

        /// \brief Constructs a skeleton resource with the given content key.
        ///
        /// \param Key The unique content key identifying this skeleton.
        explicit Skeleton(AnyRef<Content::Uri> Key);

        /// \brief Replaces the bone hierarchy.
        ///
        /// \param Bones The bones, ordered so that every bone follows the parent it names.
        void SetBones(AnyRef<Sequence<Bone>> Bones);

        /// \brief Gets the bone hierarchy.
        ///
        /// \return The bones, in an order where a parent always precedes its children.
        ZY_INLINE ConstSpan<Bone> GetBones() const
        {
            return mBones;
        }

        /// \brief Checks whether the skeleton carries any bone at all.
        ///
        /// \return `true` when the skeleton is empty, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mBones.IsEmpty();
        }

        /// \brief Finds the bone a name belongs to.
        ///
        /// \param Name The hash of the bone's name.
        /// \return The bone's index, or \ref kMissing when the skeleton has no such bone.
        ZY_INLINE SInt32 Find(UInt64 Name) const
        {
            const ConstPtr<UInt16> Entry = mRegistry.Find(Name);

            return Entry ? static_cast<SInt32>(* Entry) : kMissing;
        }

        /// \brief Writes every bone's resting local transform into a pose.
        ///
        /// An animation is layered over this, so a bone no channel drives keeps resting where the hierarchy
        /// says it should instead of collapsing onto its parent.
        ///
        /// \param Output Receives each bone's resting position, scale and rotation.
        void Rest(ConstRef<Pose> Output) const;

        /// \brief Composes local bone transforms into skeleton space.
        ///
        /// Because a parent always precedes its children, this is one forward pass with no recursion and no
        /// scratch beyond the output itself.
        ///
        /// \param Local  The local transform of every bone.
        /// \param Output Receives each bone's transform in skeleton space.
        void Compose(ConstRef<Pose> Local, Span<Matrix4x3> Output) const;

        /// \brief Folds the bind pose out of the rest pose, producing the matrices a model wears undriven.
        ///
        /// Fused rather than composed then skinned through the other overload, because the palette can carry
        /// the composed transforms between the two passes and so the whole thing runs without scratch.
        ///
        /// \param Palette Receives the matrix each bone deforms its vertices by while at rest.
        void Skin(Span<Matrix4x3> Palette) const;

        /// \brief Folds the bind pose out of a composed pose to produce the matrices vertices are skinned by.
        ///
        /// Kept apart from \ref Compose because the composed pose is what a socket or an attachment reads,
        /// while this is only what the vertex shader wants.
        ///
        /// \param Composed The bone transforms in skeleton space, as produced by \ref Compose.
        /// \param Palette  Receives the matrix each bone deforms its vertices by; may alias \p Composed.
        void Skin(ConstSpan<Matrix4x3> Composed, Span<Matrix4x3> Palette) const;

    public:

        /// \brief Interpolates a pose toward another, bone by bone, in local space.
        ///
        /// \param Destination The pose to blend, receiving the result.
        /// \param Source      The pose to blend toward.
        /// \param Weight      The interpolation factor in the `[0, 1]` range.
        static void Blend(Ref<Pose> Destination, ConstRef<Pose> Source, Real32 Weight);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Bone>        mBones;
        Table<UInt64, UInt16> mRegistry;
    };
}