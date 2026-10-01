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

#include "Chunk.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    class Directory;

    /// \brief Represents what one query asks for, the chunks that answer it and where each keeps every field.
    struct ZY_API Selection final
    {
        /// The source of a field an archetype lends.
        static constexpr UInt16 kLent     = 0xFFFF;

        /// The source of an optional field the chunk does not carry.
        static constexpr UInt16 kMissing  = 0xFFFE;

        /// The source of a field read from the world entity.
        static constexpr UInt16 kGlobal   = 0xFFFD;

        /// The source of a field read from an ancestor.
        static constexpr UInt16 kAncestor = 0xFFFC;

        /// \brief Specifies how a field is read.
        enum class Mode : UInt8
        {
            Read,     ///< Read from the entity's own column or its archetype's, or missing to a pointer.
            Write,    ///< Written in the entity's own column, or handed as missing to a pointer.
            Global,   ///< Read from the world entity.
            Ancestor, ///< Read from the nearest ancestor carrying it.
        };

        /// The components a match holds or inherits.
        Sequence<UInt32>           Required;

        /// The components a match holds itself.
        Sequence<UInt32>           Owned;

        /// The components a match neither holds nor inherits.
        Sequence<UInt32>           Excluded;

        /// The groups of components a match carries at least one of.
        Sequence<Sequence<UInt32>> Alternatives;

        /// The singletons the world entity must hold for a walk to happen.
        Sequence<UInt32>           Globals;

        /// The components pointer fields read from the nearest ancestor carrying them.
        Sequence<UInt32>           Ancestors;

        /// The component each field hands out, in order.
        Sequence<UInt32>           Fields;

        /// How each field is read.
        Sequence<Mode>             Modes;

        /// The matched chunks, in the order they were made.
        Sequence<Ptr<Chunk>>       Chunks;

        /// Where each matched chunk keeps each field, one run per match.
        Sequence<UInt16>           Sources;

        /// The position of each chunk among the matches plus one, indexed by chunk.
        Sequence<UInt32>           Positions;

        /// The first row of each matched chunk for a spread walk, or the row past each depth for a sorted one.
        Sequence<UInt32>           Offsets;

        /// How deep each row an ordered walk gathered stands.
        Sequence<UInt32>           Depths;

        /// The chunk and row of each row an ordered walk gathered, then the same rows sorted by depth behind them.
        Sequence<UInt64>           Gathered;

        /// The entity in every row of every match when the rows gathered were sorted, which spares finding them again.
        Sequence<Handle>           Snapshot;

        /// The list of fields the walks were declared for, so a callback asking for others declares them again.
        ConstPtr<void>             Declared;

        /// The seconds one row costs, from a walk alone or a cheaper spread walk, which decides whether to spread.
        Real64                     Cost;

        /// The seconds one row took on the share the caller timed on the last spread walk, which sizes the shares.
        Real64                     Pace;

        /// The links the directory had made when the rows gathered were sorted, which changes with the hierarchy.
        UInt32                     Links;

        /// `true` when matches are visited parents first.
        Bool                       Ordered;

        /// `true` when the last walk that could choose spread.
        Bool                       Spread;

        /// \brief Constructs a selection that asks for nothing and matches nothing.
        ZY_INLINE Selection()
            : Declared { nullptr },
              Cost     { 0 },
              Pace     { 0 },
              Links    { 0 },
              Ordered  { false },
              Spread   { false }
        {
        }

        /// \brief Selections are not copied, since a query points at its own.
        Selection(ConstRef<Selection> Other) = delete;

        /// \brief Checks whether a chunk answers the query.
        ///
        /// \param Target The chunk.
        /// \param Slots  The directory, which knows what archetypes lend.
        /// \return `true` if it does, `false` otherwise.
        Bool Matches(ConstRef<Chunk> Target, ConstRef<Directory> Slots) const;

        /// \brief Adds, refreshes or removes a chunk that is new or whose archetype changed.
        ///
        /// \param Target The chunk.
        /// \param Slots  The directory, which knows what archetypes lend.
        void Update(Ptr<Chunk> Target, ConstRef<Directory> Slots);

        /// \brief Takes a chunk out of the matches, if it is there, keeping the others in order.
        ///
        /// \param Target The chunk.
        void Remove(ConstPtr<Chunk> Target);

        /// \brief Gets the position of a chunk among the matches.
        ///
        /// \param Target The chunk.
        /// \return The position plus one, or zero when the chunk does not answer.
        ZY_INLINE UInt32 GetPosition(ConstRef<Chunk> Target) const
        {
            return Target.GetIndex() < Positions.GetSize() ? Positions[Target.GetIndex()] : 0;
        }

        /// \brief Gets where a matched chunk keeps a field.
        ///
        /// \param Match The position of the chunk among the matches.
        /// \param Field The field.
        /// \return The column, or one of the sources above.
        ZY_INLINE UInt16 GetSource(UInt32 Match, UInt32 Field) const
        {
            return Sources[Match * Fields.GetSize() + Field];
        }

        /// \brief Gets how many entities the matched chunks hold.
        ///
        /// \return The number of entities.
        UInt Count() const;
    };
}