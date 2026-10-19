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

#include "Types.hpp"
#include "Execution/Walk.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents a reader of a component's changes with a position of its own, which never outlives its world.
    ///
    /// \tparam List The changes it reads, \ref Added, \ref Set or \ref Removed of a component.
    template<typename List>
    class Cursor final
    {
        friend class Draft;

    public:

        /// \brief Constructs a cursor that reads nothing.
        ZY_INLINE Cursor()
            : mOwner   { nullptr },
              mState   { nullptr },
              mHandles { },
              mLast    { Chunk::kNever }
        {
        }

        /// \brief Constructs a cursor of a world, seeing every change from now on, archetypes and sleepers left out.
        ///
        /// \param Owner The world whose changes the cursor reads.
        ZY_INLINE explicit Cursor(Ref<Storage> Owner)
            : Cursor(Owner, Unique<Selection>::Create())
        {
        }

        /// \brief Takes over another cursor.
        ///
        /// \param Other The cursor to take over, which reads nothing afterwards.
        ZY_INLINE Cursor(AnyRef<Cursor> Other)
            : mOwner   { Exchange(Other.mOwner, nullptr) },
              mState   { Exchange(Other.mState, nullptr) },
              mHandles { Exchange(Other.mHandles, Array<UInt32, List::kSize>()) },
              mLast    { Other.mLast }
        {
        }

        /// \brief Lets go of the position, so the list drops what only this cursor had left to read.
        ZY_INLINE ~Cursor()
        {
            for (const UInt32 Handle : mHandles)
            {
                if (Handle)
                {
                    mOwner->mLedger.Unsubscribe(Handle);
                }
            }

            if (mState)
            {
                mOwner->mLayout.RemoveSelection(mState);
            }
        }

        /// \brief Cursors are not copied, since each owns its position.
        Cursor(ConstRef<Cursor> Other) = delete;

        /// \brief Cursors are not assigned, since each owns its position.
        Ref<Cursor> operator=(ConstRef<Cursor> Other) = delete;

        /// \brief Checks whether the cursor reads a list of a world.
        ///
        /// \return `true` if it is subscribed to a list, `false` for one made empty or moved from.
        ZY_INLINE Bool IsValid() const
        {
            return mHandles.GetFront() != 0;
        }

        /// \brief Hands every entry recorded since the last call to a callback.
        ///
        /// \param Callback The callable, taking a query's fields, or for removals the entity and maybe its old value.
        template<typename Callable>
        void Each(AnyRef<Callable> Callback)
        {
            ZY_ASSERT(IsValid(), "The cursor reads nothing");

            if constexpr (List::kKind == Pull::Removed)
            {
                ReadRemovals(Callback);
            }
            else
            {
                ReadChanges(Callback);
            }
        }

    private:

        /// The component the list is about.
        using Type = typename List::Value;

        /// \brief Constructs a cursor filtering entries by a selection it owns from here on.
        ///
        /// \param Owner The world whose changes the cursor reads.
        /// \param State The selection, whose terms are declared.
        ZY_INLINE Cursor(Ref<Storage> Owner, AnyRef<Unique<Selection>> State)
            : mOwner   { AddressOf(Owner) },
              mState   { Adopt(Owner, Move(State)) },
              mHandles { Subscribe(Owner) },
              mLast    { Owner.Tick() }
        {
        }

        /// \brief Subscribes to the list of each change the reader asks for, additions first.
        ///
        /// \param Owner The world keeping the lists.
        /// \return The handle of each subscription.
        ZY_INLINE static Array<UInt32, List::kSize> Subscribe(Ref<Storage> Owner)
        {
            Ref<Ledger> Lists = Owner.mLedger;

            if constexpr (List::kSize == 2)
            {
                const UInt32 Added = Lists.Subscribe(IdentifierOf<Type>(), Pull::Added);
                return Array<UInt32, List::kSize>(Added, Lists.Subscribe(IdentifierOf<Type>(), Pull::Changed));
            }
            else
            {
                return Array<UInt32, List::kSize>(Lists.Subscribe(IdentifierOf<Type>(), List::kKind));
            }
        }

        /// \brief Keeps the selection of a list, whose entries are matched against it.
        ///
        /// \param Owner The world that keeps the selection current.
        /// \param State The terms of the reader, handed over.
        /// \return The selection kept current by the world, or `nullptr` for removals asking for nothing more.
        ZY_INLINE static Ptr<Selection> Adopt(Ref<Storage> Owner, AnyRef<Unique<Selection>> State)
        {
            if constexpr (List::kKind == Pull::Removed)
            {
                // An archetype or a sleeper that goes is a removal like any other.
                State->Excluded.Erase(IdentifierOf<Prefab>());
                State->Excluded.Erase(IdentifierOf<Asleep>());

                if (State->Required.IsEmpty() && State->Excluded.IsEmpty() && State->Alternatives.IsEmpty())
                {
                    return nullptr;
                }
            }
            return Owner.mLayout.AddSelection(Move(State), Owner.mDirectory);
        }

        /// \brief Hands every removal since the last call to a callback taking the entity, and maybe its lost value.
        ///
        /// \param Callback The callable, taking `(Entity)` or `(Entity, ConstRef<Type>)`.
        template<typename Callable>
        void ReadRemovals(Ref<Callable> Callback)
        {
            constexpr Bool kValue = IsInvocable<Callable, Entity, ConstRef<Type>>;

            static_assert(
                kValue || IsInvocable<Callable, Entity>,
                "A removal hands (Entity) or (Entity, ConstRef<T>)");
            ZY_ASSERT(!kValue || Registry::Get().GetMetatype(IdentifierOf<Type>()).Has(Trait::Keeps),
                "A removal hands its value only when the component is declared to keep it");

            const UInt32 Feed  = mHandles.GetFront();
            Ref<Ledger>  Lists = mOwner->mLedger;
            const UInt64 End   = Lists.GetEnd(Feed);

            // What the callback changes waits like in any walk.
            mOwner->Enter();

            for (UInt64 Next = Lists.GetPosition(Feed); Next < End; ++Next)
            {
                const Handle Actor = Lists.GetActor(Feed, Next);

                if (Directory::IsArchetype(Actor.GetIndex()))
                {
                    continue;
                }

                // Terms are matched against the chunk the entity left, which a gone entity no longer has.
                if (mState)
                {
                    const UInt32 Source = Lists.GetSource(Feed, Next);

                    if (Source >= mState->Positions.GetSize() || mState->Positions[Source] == 0)
                    {
                        continue;
                    }
                }

                if constexpr (kValue)
                {
                    const ConstPtr<Byte> Bytes = Lists.GetValue(Feed, Next);

                    Callback(Entity(mOwner, Actor), * reinterpret_cast<ConstPtr<Type>>(Bytes));
                }
                else
                {
                    Callback(Entity(mOwner, Actor));
                }
            }

            Lists.SetPosition(Feed, End);
            mOwner->Leave();
        }

        /// \brief Hands every entry since the last call to a callback, with the fields it asks for, like a query.
        ///
        /// \param Callback The callable, taking the entity first when it asks for it, then its fields.
        template<typename Callable>
        void ReadChanges(Ref<Callable> Callback)
        {
            using Blueprint = Plan<StripAll<Callable>>;

            if (mState->Declared != Blueprint::GetKey())
            {
                Blueprint::Declare(* mState);
                mOwner->mLayout.Populate(* mState, mOwner->mDirectory);
            }

            // Every end is taken first, so what reading the additions causes waits for the next read like the rest.
            Ref<Ledger>                Lists = mOwner->mLedger;
            Array<UInt64, List::kSize> Ends;

            for (UInt Index = 0; Index < List::kSize; ++Index)
            {
                Ends[Index] = Lists.GetEnd(mHandles[Index]);
            }

            // Writes from here on mark pages with a later tick than every mark this read takes.
            const UInt32 Until = mOwner->Tick();

            // What the callback changes waits like in any walk.
            mOwner->Enter();

            ConstRef<Directory> Slots = mOwner->mDirectory;

            for (UInt Index = 0; Index < List::kSize; ++Index)
            {
                // Read by position, since the callback may record more entries into the same list as it goes.
                UInt64                     First  = 0;
                ConstRef<Sequence<Handle>> Actors = Lists.GetActors(mHandles[Index], First);

                for (UInt64 Next = Lists.GetPosition(mHandles[Index]); Next < Ends[Index]; ++Next)
                {
                    const Handle Actor = Actors[Next - First];

                    // A gone entity, or one whose chunk no longer answers the terms, is passed over.
                    const ConstPtr<Directory::Slot> Entry = Slots.FindSlot(Actor);

                    if (Entry == nullptr)
                    {
                        continue;
                    }

                    if (const UInt32 Place = mState->GetPosition(* Entry->Holder))
                    {
                        Blueprint::template Apply<Walk::Kernel>::Deliver(
                            * mOwner, * mState, Actor, * Entry->Holder, Entry->Row, Place - 1, Callback);
                    }
                }
                Lists.SetPosition(mHandles[Index], Ends[Index]);
            }

            if constexpr ((List::kKind & Pull::Changed) != Pull())
            {
                ReadMarks(Callback, Until);
            }
            mLast = Until;

            mOwner->Leave();
        }

        /// \brief Hands every row of every page a walk wrote whole since the last read to a callback, like a query.
        ///
        /// \param Callback The callable, taking the entity first when it asks for it, then its fields.
        /// \param Until    The tick of this read, past which writes wait for the next one.
        template<typename Callable>
        void ReadMarks(Ref<Callable> Callback, UInt32 Until)
        {
            const UInt32 Identifier = IdentifierOf<Type>();

            for (UInt32 Match = 0; Match < mState->Chunks.GetSize(); ++Match)
            {
                Ref<Chunk>   Target = * mState->Chunks[Match];
                const SInt16 Column = Target.Find(Identifier);

                if (Column < 0)
                {
                    continue;
                }

                for (UInt32 First = 0; First < Target.GetSize(); First += Target.GetRows())
                {
                    if (Chunk::IsBetween(Target.GetMark(Column, First), mLast, Until))
                    {
                        const UInt32 Last = Min(First + Target.GetRows(), Target.GetSize());
                        Walk::Scan(* mOwner, * mState, Match, First, Last, Callback);
                    }
                }
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Storage>               mOwner;
        Ptr<Selection>             mState;
        Array<UInt32, List::kSize> mHandles;
        UInt32                     mLast;
    };
}