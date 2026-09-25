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

#include "Zyphryon.Math/Vector2.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the bone keys of one 2D clip, keyed per sheet row and frame.
    class ZY_API Motion2D final
    {
    public:

        /// \brief Represents a bone's placement on one frame of one row.
        struct Key final
        {
            /// The sheet row the key belongs to, one per drawn direction.
            UInt8   Row      = 0;

            /// The frame the key sits on; frames between keys are interpolated.
            UInt8   Keyframe = 0;

            /// The position of the bone, in the frame's own units from its bottom-left corner, x right and y up.
            Vector2 Offset   = Vector2::Zero();

            /// The rotation of the bone's attachment, counter-clockwise from upright.
            Angle   Roll     = Angle::FromDegrees(0.0f);

            /// The rotation of the bone's attachment out of the sprite's plane, where a quarter turn is edge-on.
            Angle   Turn     = Angle::FromDegrees(0.0f);

            /// The scale of the bone's attachment, where one is as authored.
            Real32  Scale    = 1.0f;

            /// The draw order of the bone's attachment against the sprite, positive in front and negative behind.
            SInt8   Layer    = 1;
        };

        /// \brief Represents the keys of one bone over the clip.
        struct Lane final
        {
            /// The hash of the bone's name, matching \ref Skeleton2D::Bone::Name, or zero on a slot no lane was given.
            UInt64        Bone = 0;

            /// The keys, in authored order.
            Sequence<Key> Keys;
        };

        /// \brief Represents a bone's placement on one frame, interpolated between the keys around it.
        struct Placement final
        {
            /// The position of the bone, in the frame's own units from its bottom-left corner.
            Vector2 Offset = Vector2::Zero();

            /// The rotation of the bone's attachment, counter-clockwise from upright.
            Angle   Roll   = Angle::FromDegrees(0.0f);

            /// The rotation of the bone's attachment out of the sprite's plane.
            Angle   Turn   = Angle::FromDegrees(0.0f);

            /// The scale of the bone's attachment.
            Real32  Scale  = 1.0f;

            /// The draw order of the bone's attachment against the sprite.
            SInt8   Layer  = 1;
        };

    public:

        /// \brief Constructs a motion with no lanes, holding its end keys rather than wrapping between them.
        ZY_INLINE Motion2D()
            : mCycle { 0 }
        {
        }

        /// \brief Sets how many frames one lap of the clip spans, so its last key blends back into its first.
        ///
        /// \param Cycle The number of frames in a lap, where `0` holds the end keys of a clip that does not loop.
        ZY_INLINE void SetCycle(UInt32 Cycle)
        {
            mCycle = Cycle;
        }

        /// \brief Gets how many frames one lap of the clip spans.
        ///
        /// \return The number of frames in a lap, or `0` when the end keys hold.
        ZY_INLINE UInt32 GetCycle() const
        {
            return mCycle;
        }

        /// \brief Adds a lane with no keys for a bone.
        ///
        /// \param Bone The hash of the name of the bone the lane drives.
        /// \return The new lane, for its keys to be written into.
        ZY_INLINE Ref<Lane> AddLane(UInt64 Bone)
        {
            Ref<Lane> Added = mLanes.Append();
            Added.Bone = Bone;
            return Added;
        }

        /// \brief Removes a lane, shifting the ones after it down by one.
        ///
        /// \param Index The index of the lane.
        ZY_INLINE void RemoveLane(UInt32 Index)
        {
            mLanes.Remove(Index);
        }

        /// \brief Gets a lane for writing.
        ///
        /// \param Index The index of the lane.
        /// \return The lane.
        ZY_INLINE Ref<Lane> GetLane(UInt32 Index)
        {
            return mLanes[Index];
        }

        /// \brief Gets every lane.
        ///
        /// \return The lanes, in authored order.
        ZY_INLINE ConstSpan<Lane> GetLanes() const
        {
            return mLanes;
        }

        /// \brief Checks whether the clip drives no bone at all.
        ///
        /// \return `true` when there is no lane, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mLanes.IsEmpty();
        }

        /// \brief Finds the lane driving a bone.
        ///
        /// \param Bone The hash of the bone's name.
        /// \return The lane, or `nullptr` when no lane drives it.
        ConstPtr<Lane> Find(UInt64 Bone) const;

        /// \brief Lays the lanes out so the lane at each index drives the bone at that index.
        ///
        /// \note A keyed lane with no bone of its own is kept after the last bone, so a later layout can place it.
        ///
        /// \param Binding One bone index per lane, as resolved by \ref Skeleton2D::Find.
        /// \param Bones   The number of bones the skeleton carries.
        /// \return `true` when every keyed lane found a bone of its own, otherwise `false`.
        Bool Arrange(ConstSpan<SInt32> Binding, UInt32 Bones);

        /// \brief Computes a bone's placement on a frame from the keys around it on the same row.
        ///
        /// \note The lanes must have been laid out by \ref Arrange, which \ref Sheet2D does on load.
        ///
        /// \param Bone     The index of the bone.
        /// \param Row      The sheet row being drawn.
        /// \param Keyframe The frame being drawn.
        /// \param Output   Receives the placement, left untouched when there is none.
        /// \return `true` when the placement was written, `false` when the bone has no keys on that row.
        Bool Sample(UInt32 Bone, UInt8 Row, UInt8 Keyframe, Ref<Placement> Output) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Lane> mLanes;
        UInt32         mCycle;
    };
}