// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the 2D skeleton of a \ref Sheet2D.
    class ZY_API Skeleton2D final
    {
    public:

        /// \brief The value returned for a bone the skeleton does not carry.
        static constexpr SInt32 kMissing = -1;

        /// \brief Represents one bone of the skeleton.
        struct Bone final
        {
            /// The hash of the bone's name.
            UInt64   Name = 0;

            /// The name the bone is shown by, which may be cut short; lookups go by \ref Name alone.
            Str32    Label;
        };

    public:

        /// \brief Replaces the bones.
        ///
        /// \param Bones The bones, in the order the sheet's lanes are indexed by.
        void SetBones(AnyRef<Sequence<Bone>> Bones);

        /// \brief Gets every bone.
        ///
        /// \return The bones, in authored order.
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
        /// \return The index of the bone, or \ref kMissing when the skeleton has no such bone.
        ZY_INLINE SInt32 Find(UInt64 Name) const
        {
            const ConstPtr<UInt16> Entry = mRegistry.Find(Name);
            return Entry ? static_cast<SInt32>(* Entry) : kMissing;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Bone>        mBones;
        Table<UInt64, UInt16> mRegistry;
    };
}