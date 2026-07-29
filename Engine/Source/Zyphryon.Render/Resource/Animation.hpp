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

#include "Skeleton.hpp"
#include "Zyphryon.Math/Animation/Cadence.hpp"
#include "Zyphryon.Math/Geometry/Sphere.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Represents a clip of keyframed motion, played over a skeleton.
    class Animation final : public Content::AbstractResource<Animation>
    {
    public:

        /// \brief The motion of one bone: what it is called, and where it goes over time.
        struct Lane final
        {
            /// The hash of the driven bone's name, matching \ref Skeleton::Bone::Name.
            UInt64            Name = 0;

            /// The cadence every component that varies is sampled on.
            Cadence           Timing;

            /// The bone's position over time, or empty when the clip does not move it.
            Track<Vector3>    Position;

            /// The bone's scale over time, or empty when the clip does not resize it.
            Track<Vector3>    Scale;

            /// The bone's rotation over time, or empty when the clip does not turn it.
            Track<Quaternion> Rotation;

            /// \brief Checks whether one cursor can serve every component the lane drives.
            ///
            /// \return `true` when \ref Cadence::Locate's cursor is valid for every track, otherwise `false`.
            ZY_INLINE Bool IsLockstep() const
            {
                return Timing.IsValid() && Timing.Agrees(Position) && Timing.Agrees(Scale) && Timing.Agrees(Rotation);
            }
        };

    public:

        /// \brief Constructs an animation resource with the given content key.
        ///
        /// \param Key The unique content key identifying this animation.
        explicit Animation(AnyRef<Content::Uri> Key);

        /// \brief Checks whether the clip drives anything at all.
        ///
        /// \return `true` when the clip has no lanes, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mLanes.IsEmpty();
        }

        /// \brief Replaces the clip's lanes, adopting the longest track's end as the duration.
        ///
        /// \param Lanes The lanes, one per driven bone.
        void Arrange(AnyRef<Sequence<Lane>> Lanes);

        /// \brief Gets the clip's lanes.
        ///
        /// \return The lanes, one per driven bone.
        ZY_INLINE ConstSpan<Lane> GetLanes() const
        {
            return mLanes;
        }

        /// \brief Overrides how long the clip runs, for a clip that holds past its last keyframe.
        ///
        /// \param Duration The clip's length, in seconds.
        ZY_INLINE void SetDuration(Real64 Duration)
        {
            mDuration = Duration;
        }

        /// \brief Gets how long the clip runs.
        ///
        /// \return The clip's length, in seconds.
        ZY_INLINE Real64 GetDuration() const
        {
            return mDuration;
        }

        /// \brief Sets the extent the skinned geometry sweeps over the whole clip.
        ///
        /// \param Extent The local-space sphere containing every pose the clip reaches.
        ZY_INLINE void SetExtent(Sphere Extent)
        {
            mExtent = Extent;
        }

        /// \brief Gets the extent the skinned geometry sweeps over the whole clip.
        ///
        /// \return The local-space sphere containing every pose the clip reaches, or an invalid sphere when unset.
        ZY_INLINE Sphere GetExtent() const
        {
            return mExtent;
        }

        /// \brief Pairs the clip with a skeleton, resolving everything a frame would otherwise rediscover.
        ///
        /// \param Skeleton The skeleton to resolve against.
        /// \param Binding  Receives one bone index per lane, \ref Skeleton::kMissing where there is no such bone.
        /// \return `true` when the clip writes every component of every bone, so the caller may skip the rest pose.
        Bool Resolve(ConstRef<Skeleton> Skeleton, Ref<Sequence<SInt32>> Binding) const;

        /// \brief Samples every lane at the given time, layering the result over a pose.
        ///
        /// \param Time    The time to sample at, in seconds, already wrapped into the clip's range.
        /// \param Binding The lane-to-bone resolution from \ref Resolve.
        /// \param Output  Receives the sampled local transforms.
        void Sample(Real64 Time, ConstSpan<SInt32> Binding, ConstRef<Skeleton::Pose> Output) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bool           mLockstep;
        Sequence<Lane> mLanes;
        Real64         mDuration;
        Sphere         mExtent;
    };
}