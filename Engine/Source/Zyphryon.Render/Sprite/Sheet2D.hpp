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

#include "Animation2D.hpp"
#include "Motion2D.hpp"
#include "Zyphryon.Content/Resource.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the layout of a sprite sheet.
    class ZY_API Sheet2D final : public ZyContent::AbstractResource<Sheet2D>
    {
    public:

        /// \brief Represents one clip of the sheet, the frames it shows and the bone keys laid on them.
        struct Clip final
        {
            /// The hash of the clip's name.
            UInt64      Name = 0;

            /// The name the clip is shown by, which may be cut short, while lookups go by \ref Name alone.
            Str32       Label;

            /// The frames the clip shows, and how it plays them.
            Animation2D Frames;

            /// The keys the clip moves the bones by, one lane per bone once the sheet is laid out.
            Motion2D    Motion;
        };

    public:

        /// \brief Constructs a sheet resource with the given content key.
        ///
        /// \param Key The unique content key identifying this sheet.
        explicit Sheet2D(AnyRef<ZyContent::Uri> Key);

        /// \brief Initializes the sheet with its skeleton and its clips, laying each clip's lanes out over the bones.
        ///
        /// \param Skeleton The bones attachments hang from, in the order the lanes are indexed by.
        /// \param Clips    The clips, in authored order.
        /// \return `true` when every keyed lane drives a bone of the skeleton, otherwise `false`.
        Bool Setup(AnyRef<Skeleton2D> Skeleton, AnyRef<Sequence<Clip>> Clips);

        /// \brief Gets the skeleton the sheet's bone keys were authored against.
        ///
        /// \return The skeleton, empty for a sheet whose clips move no bone.
        ZY_INLINE ConstRef<Skeleton2D> GetSkeleton() const
        {
            return mSkeleton;
        }

        /// \brief Gets every clip of the sheet.
        ///
        /// \return The clips, in authored order.
        ZY_INLINE ConstSpan<Clip> GetClips() const
        {
            return mClips;
        }

        /// \brief Finds a clip by name.
        ///
        /// \param Name The hash of the clip's name.
        /// \return The clip, or `nullptr` when the sheet has no clip by that name.
        ZY_INLINE ConstPtr<Clip> FindClip(UInt64 Name) const
        {
            const SInt Index = mClips.Find([Name](ConstRef<Clip> Entry)
            {
                return Entry.Name == Name;
            });
            return Index < 0 ? nullptr : AddressOf(mClips[Index]);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Skeleton2D     mSkeleton;
        Sequence<Clip> mClips;
    };
}