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

#include "Directory.hpp"
#include "Selection.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the chunks of a world, found by components and archetype, and the selections kept current.
    class ZY_API Layout final
    {
    public:

        /// \brief Constructs a layout holding the empty root chunk and the pending chunk no query ever sees.
        Layout();

        /// \brief Layouts are not copied, since every slot points into their chunks.
        Layout(ConstRef<Layout> Other) = delete;

        /// \brief Gets the chunk of entities that hold nothing.
        ///
        /// \return The chunk.
        ZY_INLINE Ptr<Chunk> GetRoot() const
        {
            return AddressOf(* mChunks[0]);
        }

        /// \brief Gets the chunk entities made during a walk wait in until it ends.
        ///
        /// \return The chunk.
        ZY_INLINE Ptr<Chunk> GetPending() const
        {
            return AddressOf(* mPending);
        }

        /// \brief Finds the chunk holding exactly a set of components for one archetype.
        ///
        /// \param Signature The components, sorted.
        /// \param Base      The slot of the archetype, or zero.
        /// \return The chunk, or `nullptr` when none was made yet.
        Ptr<Chunk> Find(ConstSpan<UInt32> Signature, UInt32 Base) const;

        /// \brief Makes the chunk holding exactly a set of components for one archetype, for every selection to match.
        ///
        /// \param Signature The components, sorted.
        /// \param Base      The slot of the archetype, or zero.
        /// \param Marker    The component that makes archetypes.
        /// \param Slots     The directory, which knows what archetypes lend.
        /// \return The chunk.
        Ptr<Chunk> Create(ConstSpan<UInt32> Signature, UInt32 Base, UInt32 Marker, ConstRef<Directory> Slots);

        /// \brief Destroys an empty chunk whose archetype is gone, so the slot's next archetype makes its own.
        ///
        /// \param Target The chunk, which only chunks made from the same archetype point at.
        void Destroy(Ptr<Chunk> Target);

        /// \brief Gets a chunk by its position.
        ///
        /// \param Index The position of the chunk.
        /// \return The chunk, or `nullptr` when it was retired.
        ZY_INLINE Ptr<Chunk> GetChunk(UInt32 Index) const
        {
            return mChunks[Index] ? AddressOf(* mChunks[Index]) : nullptr;
        }

        /// \brief Gets every chunk.
        ///
        /// \return The chunks, indexed by their position, where a retired one is empty.
        ZY_INLINE ConstSpan<Unique<Chunk>> GetChunks() const
        {
            return mChunks;
        }

        /// \brief Updates what every selection matches of the chunks made from an archetype or its derived ones.
        ///
        /// \param Archetype The slot of the archetype.
        /// \param Slots     The directory, which knows what archetypes lend.
        void UpdateHeirs(UInt32 Archetype, ConstRef<Directory> Slots);

        /// \brief Takes the chunks made from an archetype being destroyed off its lineage.
        ///
        /// \param Archetype The slot of the archetype.
        /// \return The chunks, which the caller empties and retires.
        Sequence<Ptr<Chunk>> ExtractHeirs(UInt32 Archetype);

        /// \brief Gets the chunks made from an archetype.
        ///
        /// \param Archetype The slot of the archetype.
        /// \return The chunks, which change as chunks are made.
        ZY_INLINE ConstSpan<Ptr<Chunk>> GetHeirChunks(UInt32 Archetype) const
        {
            const UInt32 Kin = Archetype - kMinRangeArchetypes - 1;
            return Kin < mLineages.GetSize() ? ConstSpan<Ptr<Chunk>>(mLineages[Kin].Heirs) : ConstSpan<Ptr<Chunk>>();
        }

        /// \brief Remembers the chunk a copy of an archetype is made in.
        ///
        /// \param Archetype   The slot of the archetype.
        /// \param Holder      The chunk the archetype sits in now.
        /// \param AsArchetype `true` for a copy that is an archetype too, `false` for an instance.
        /// \param Target      The chunk.
        void SetCopy(UInt32 Archetype, Ptr<Chunk> Holder, Bool AsArchetype, Ptr<Chunk> Target);

        /// \brief Gets the chunk a copy of an archetype was last made in, while the archetype has not moved since.
        ///
        /// \param Archetype   The slot of the archetype.
        /// \param Holder      The chunk the archetype sits in now.
        /// \param AsArchetype `true` for a copy that is an archetype too, `false` for an instance.
        /// \return The chunk, or `nullptr` when it must be worked out again.
        ZY_INLINE Ptr<Chunk> GetCopy(UInt32 Archetype, ConstPtr<Chunk> Holder, Bool AsArchetype) const
        {
            const UInt32 Kin = Archetype - kMinRangeArchetypes - 1;

            if (Kin >= mLineages.GetSize() || mLineages[Kin].Shape != Holder)
            {
                return nullptr;
            }
            return mLineages[Kin].Copies[AsArchetype];
        }

        /// \brief Keeps a selection current from now on, offering it every chunk.
        ///
        /// \param State The selection, whose terms are all declared.
        /// \param Slots The directory, which knows what archetypes lend.
        /// \return The selection.
        Ptr<Selection> AddSelection(AnyRef<Unique<Selection>> State, ConstRef<Directory> Slots);

        /// \brief Destroys a selection.
        ///
        /// \param State The selection.
        void RemoveSelection(ConstPtr<Selection> State);

        /// \brief Gets the selections kept current.
        ///
        /// \return The selections.
        ZY_INLINE ConstSpan<Unique<Selection>> GetSelections() const
        {
            return mSelections;
        }

        /// \brief Fills a selection with every chunk it matches, once what it asks for changed.
        ///
        /// \param State The selection.
        /// \param Slots The directory, which knows what archetypes lend.
        void Populate(Ref<Selection> State, ConstRef<Directory> Slots) const;

        /// \brief Updates what every selection matches of a chunk.
        ///
        /// \param Target The chunk, which is new or whose columns or archetype changed.
        /// \param Slots  The directory, which knows what archetypes lend.
        void Update(Ptr<Chunk> Target, ConstRef<Directory> Slots);

    private:

        /// \brief Represents what the layout knows of one archetype.
        struct Lineage final
        {
            /// The chunks made from it.
            Sequence<Ptr<Chunk>> Heirs;

            /// The chunk it sat in when its copies' chunks were worked out.
            Ptr<Chunk>           Shape;

            /// The chunk its instances land in, then the one its archetype copies land in.
            Array<Ptr<Chunk>, 2> Copies;
        };

        /// \brief Gets the lineage of an archetype, making it the first time.
        ///
        /// \param Archetype The slot of the archetype.
        /// \return The lineage.
        Ref<Lineage> GetLineage(UInt32 Archetype);

        /// \brief Gets the key a chunk is filed under.
        ///
        /// \param Signature The components, sorted.
        /// \param Base      The slot of the archetype, or zero.
        /// \return The key, before any step past another set filed alike.
        static UInt64 GetKey(ConstSpan<UInt32> Signature, UInt32 Base);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Unique<Chunk>>     mChunks;
        Table<UInt64, Ptr<Chunk>>   mSignature;
        Unique<Chunk>               mPending;
        Sequence<Lineage>           mLineages;
        Sequence<Unique<Selection>> mSelections;
    };
}