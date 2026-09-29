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

#include "Handle.hpp"
#include "Zyphryon.Scene/Type/Registry.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the entities holding one set of components made from one archetype, in pages that never move.
    class ZY_API Chunk final
    {
    public:

        /// The column a component answers to when the chunk carries it as a tag.
        static constexpr SInt16 kTag    = -2;

        /// The column a component answers to when the chunk does not carry it.
        static constexpr SInt16 kAbsent = -1;

        /// The flag of a component an entity made from one stored here reads rather than holds.
        static constexpr UInt8  kLends  = 1;

        /// The flag of a component an entity made from one stored here copies when it is made.
        static constexpr UInt8  kCopies = 2;

        /// The flag of a component saves carry.
        static constexpr UInt8  kSaves  = 4;

        /// \brief Represents one component's run inside every page.
        struct Column final
        {
            /// The component the run holds.
            UInt32             Identifier;

            /// The bytes one instance takes.
            UInt32             Size;

            /// The first byte of the run inside a page.
            UInt32             Offset;

            /// The description of how instances are built, moved and destroyed.
            ConstPtr<Metatype> Info;
        };

    public:

        /// \brief Constructs an empty chunk.
        ///
        /// \param Index     The position of the chunk in its storage, which queries index their matches by.
        /// \param Base      The slot of the archetype every entity here was made from, or zero for none.
        /// \param Archetype `true` when the entities here are archetypes.
        /// \param Signature The components every entity here holds, sorted by identifier.
        Chunk(UInt32 Index, UInt32 Base, Bool Archetype, ConstSpan<UInt32> Signature);

        /// \brief Destroys every component still stored and gives the pages back.
        ~Chunk();

        /// \brief Chunks are not copied, since every entity's slot points at the one it sits in.
        Chunk(ConstRef<Chunk> Other) = delete;

        /// \brief Gets the position of the chunk in its storage.
        ///
        /// \return The position, fixed for the life of the chunk.
        ZY_INLINE UInt32 GetIndex() const
        {
            return mIndex;
        }

        /// \brief Gets the archetype every entity here was made from, which lends them what they do not hold.
        ///
        /// \return The slot of the archetype, or zero for none.
        ZY_INLINE UInt32 GetBase() const
        {
            return mBase;
        }

        /// \brief Checks whether the entities here are archetypes.
        ///
        /// \return `true` if they are, `false` otherwise.
        ZY_INLINE Bool IsArchetype() const
        {
            return mArchetype;
        }

        /// \brief Gets the components every entity here holds.
        ///
        /// \return The identifiers, sorted.
        ZY_INLINE ConstSpan<UInt32> GetSignature() const
        {
            return mSignature;
        }

        /// \brief Gets how many entities sit here.
        ///
        /// \return The number of rows in use.
        ZY_INLINE UInt32 GetSize() const
        {
            return mSize;
        }

        /// \brief Gets the column a component is stored in.
        ///
        /// \param Identifier The component.
        /// \return The column, or \ref kTag when it is carried without data, or \ref kAbsent when it is not carried.
        ZY_INLINE SInt16 Find(UInt32 Identifier) const
        {
            return Identifier < mColumnOf.GetSize() ? mColumnOf[Identifier] : kAbsent;
        }

        /// \brief Checks whether every entity here carries a component.
        ///
        /// \param Identifier The component.
        /// \return `true` if it is carried, with data or as a tag, `false` otherwise.
        ZY_INLINE Bool Has(UInt32 Identifier) const
        {
            return Find(Identifier) != kAbsent;
        }

        /// \brief Gets how a component the chunk carries reaches entities made from one stored here, and saves.
        ///
        /// \param Identifier The component, which the chunk must carry.
        /// \return The flags, \ref kLends, \ref kCopies and \ref kSaves taken together.
        ZY_INLINE UInt8 GetFlags(UInt32 Identifier) const
        {
            return mFlags[Identifier];
        }

        /// \brief Gets the columns that hold data, tags left out.
        ///
        /// \return The columns.
        ZY_INLINE ConstSpan<Column> GetColumns() const
        {
            return mColumns;
        }

        /// \brief Gets where one row keeps one column's instance.
        ///
        /// \param Index The column.
        /// \param Row   The row.
        /// \return The first byte of the instance.
        ZY_INLINE Ptr<Byte> At(UInt32 Index, UInt32 Row) const
        {
            ConstRef<Column> Target = mColumns[Index];
            return GetPage(Row) + Target.Offset + static_cast<UInt>(GetPlace(Row)) * Target.Size;
        }

        /// \brief Gets the entity sitting at a row.
        ///
        /// \param Row The row.
        /// \return The entity.
        ZY_INLINE Handle GetHandle(UInt32 Row) const
        {
            return GetHandles(Row)[GetPlace(Row)];
        }

        /// \brief Gets the handles that open the page a row sits in.
        ///
        /// \param Row The row.
        /// \return The first handle of the page, which the place of each row in it indexes.
        ZY_INLINE Ptr<Handle> GetHandles(UInt32 Row) const
        {
            return reinterpret_cast<Ptr<Handle>>(GetPage(Row));
        }

        /// \brief Gets the page a row sits in.
        ///
        /// \param Row The row.
        /// \return The first byte of the page.
        ZY_INLINE Ptr<Byte> GetPage(UInt32 Row) const
        {
            return mPages[Row >> mShift];
        }

        /// \brief Gets how many rows a full page holds.
        ///
        /// \return The rows, a power of two.
        ZY_INLINE UInt32 GetRows() const
        {
            return mMask + 1;
        }

        /// \brief Gets where a row sits inside its page.
        ///
        /// \param Row The row.
        /// \return The row within the page.
        ZY_INLINE UInt32 GetPlace(UInt32 Row) const
        {
            return Row & mMask;
        }

        /// \brief Makes room for one more entity at the end, leaving its components unconstructed.
        ///
        /// \param Actor The entity the row belongs to.
        /// \return The row made.
        ZY_INLINE UInt32 Append(Handle Actor)
        {
            if (mSize == mCapacity)
            {
                Grow();
            }

            const UInt32 Row = mSize++;
            GetHandles(Row)[GetPlace(Row)] = Actor;
            return Row;
        }

        /// \brief Takes a row out whose components were already moved away or destroyed, filling it with the last row.
        ///
        /// \param Row The row.
        /// \return The entity moved into the row, which names nothing when the row was the last one.
        ZY_INLINE Handle Release(UInt32 Row)
        {
            const UInt32 Last = --mSize;

            if (Row == Last)
            {
                return Handle();
            }

            // Every column of a row sits in the same page, at the same place in its run.
            const Ptr<Byte> Into  = GetPage(Row);
            const Ptr<Byte> From  = GetPage(Last);
            const UInt      Hole  = GetPlace(Row);
            const UInt      Taken = GetPlace(Last);

            for (ConstRef<Column> Target : mColumns)
            {
                const Ptr<Byte> Destination = Into + Target.Offset + Hole * Target.Size;
                const Ptr<Byte> Source      = From + Target.Offset + Taken * Target.Size;

                Relocate(* Target.Info, Destination, Source, 1);
            }

            const Handle Moved = GetHandle(Last);
            GetHandles(Row)[Hole] = Moved;
            return Moved;
        }

        /// \brief Takes a row out, destroying what it still holds and filling it with the last row.
        ///
        /// \param Row The row.
        /// \return The entity moved into the row, which names nothing when the row was the last one.
        ZY_INLINE Handle Erase(UInt32 Row)
        {
            for (UInt32 Index = 0; Index < mColumns.GetSize(); ++Index)
            {
                Destruct(* mColumns[Index].Info, At(Index, Row), 1);
            }
            return Release(Row);
        }

        /// \brief Lays the chunk out again from the registry, converting each value of a type resolved or unresolved.
        void Rebuild();

        /// \brief Remembers the chunk reached by adding a component.
        ///
        /// \param Identifier The component.
        /// \param Target     The chunk.
        void SetAddition(UInt32 Identifier, Ptr<Chunk> Target);

        /// \brief Gets the chunk reached by adding a component, once that edge was found.
        ///
        /// \param Identifier The component.
        /// \return The chunk, or `nullptr` until the edge is known.
        ZY_INLINE Ptr<Chunk> GetAddition(UInt32 Identifier) const
        {
            return Identifier < mAdditions.GetSize() ? mAdditions[Identifier] : nullptr;
        }

        /// \brief Remembers the chunk reached by removing a component.
        ///
        /// \param Identifier The component.
        /// \param Target     The chunk.
        void SetRemoval(UInt32 Identifier, Ptr<Chunk> Target);

        /// \brief Gets the chunk reached by removing a component, once that edge was found.
        ///
        /// \param Identifier The component.
        /// \return The chunk, or `nullptr` until the edge is known.
        ZY_INLINE Ptr<Chunk> GetRemoval(UInt32 Identifier) const
        {
            return Identifier < mRemovals.GetSize() ? mRemovals[Identifier] : nullptr;
        }

        /// \brief Sets the chunk an entity lands in once it is given everything this chunk's components bring along.
        ///
        /// \param Target The chunk.
        ZY_INLINE void SetClosure(Ptr<Chunk> Target)
        {
            mClosure = Target;
        }

        /// \brief Gets the chunk an entity lands in once it is given everything this chunk's components bring along.
        ///
        /// \return The chunk, this one when nothing is brought along.
        ZY_INLINE Ptr<Chunk> GetClosure() const
        {
            return mClosure;
        }

    public:

        /// \brief Moves instances into unconstructed storage, ending them where they were.
        ///
        /// \param Info        The type of the instances.
        /// \param Destination The storage to move them into.
        /// \param Source      The instances.
        /// \param Count       The number of instances.
        ZY_INLINE static void Relocate(ConstRef<Metatype> Info, Ptr<Byte> Destination, Ptr<Byte> Source, UInt Count)
        {
            if (Info.Relocate)
            {
                Info.Relocate(Destination, Source, Count);
            }
            else
            {
                Blit(Destination, Info.GetSize() * Count, Source);
            }
        }

        /// \brief Copies instances into unconstructed storage.
        ///
        /// \param Info        The type of the instances.
        /// \param Destination The storage to copy them into.
        /// \param Source      The instances.
        /// \param Count       The number of instances.
        ZY_INLINE static void Copy(ConstRef<Metatype> Info, Ptr<Byte> Destination, ConstPtr<Byte> Source, UInt Count)
        {
            if (Info.Copy)
            {
                Info.Copy(Destination, Source, Count);
            }
            else
            {
                Blit(Destination, Info.GetSize() * Count, Source);
            }
        }

        /// \brief Builds a component of a row from a value, doing nothing for a tag.
        ///
        /// \param Target The chunk, which carries the component.
        /// \param Row    The row, whose instance is unconstructed.
        /// \param Data   The value, moved or copied into place.
        template<typename Type, typename Value>
        ZY_INLINE static void Emplace(Ref<Chunk> Target, UInt32 Row, AnyRef<Value> Data)
        {
            if constexpr (!IsEmpty<Type>)
            {
                const Ptr<Type> Object = reinterpret_cast<Ptr<Type>>(Target.At(Target.Find(IdentifierOf<Type>()), Row));

                ::Construct(Object, Forward<Value>(Data));
            }
        }

        /// \brief Destroys instances, leaving their storage to be reused.
        ///
        /// \param Info   The type of the instances.
        /// \param Target The first instance.
        /// \param Count  The number of instances.
        ZY_INLINE static void Destruct(ConstRef<Metatype> Info, Ptr<Byte> Target, UInt Count)
        {
            if (Info.Destruct)
            {
                Info.Destruct(Target, Count);
            }
        }

        /// \brief Builds default instances in unconstructed storage.
        ///
        /// \param Info   The type of the instances.
        /// \param Target The first instance.
        /// \param Count  The number of instances.
        ZY_INLINE static void Construct(ConstRef<Metatype> Info, Ptr<Byte> Target, UInt Count)
        {
            if (Info.Construct)
            {
                Info.Construct(Target, Count);
            }
            else
            {
                Zero(Target, Info.GetSize() * Count);
            }
        }

    private:

        /// \brief The rows the first page starts with, doubled until a page is full.
        static constexpr UInt32 kFirstRows     = 16;

        /// \brief The bytes a full page holds about, so a walk leaves its loop rarely whatever the size of a row.
        static constexpr UInt32 kPageBytes     = 262'144;

        /// \brief The alignment of every page, and the gap between runs, so runs of equal rows never share cache sets.
        static constexpr UInt32 kPageAlignment = 64;

        /// \brief Checks whether a column sits before another, wider alignment first.
        ///
        /// \param Left  The first column.
        /// \param Right The second column.
        /// \return `true` if the first is aligned wider, `false` otherwise.
        static Bool IsWider(ConstRef<Column> Left, ConstRef<Column> Right);

        /// \brief Adds room for more rows, doubling the first page until full and adding pages after that.
        void Grow();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32               mSize;
        UInt32               mCapacity;
        UInt32               mShift;
        UInt32               mMask;
        Sequence<Ptr<Byte>>  mPages;
        Sequence<Column>     mColumns;
        Sequence<SInt16>     mColumnOf;
        UInt32               mStride;
        UInt32               mIndex;
        UInt32               mBase;
        Bool                 mArchetype;
        Sequence<UInt8>      mFlags;
        Sequence<Ptr<Chunk>> mAdditions;
        Sequence<Ptr<Chunk>> mRemovals;
        Sequence<UInt32>     mSignature;
        Ptr<Chunk>           mClosure;
    };
}