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
    /// \brief Represents a reader of a pull list with a position of its own, which must not outlive its world.
    ///
    /// \tparam List The list, as \ref Pulled names it, or \ref Added, \ref Changed and \ref Removed for one change.
    template<typename List>
    class Cursor final
    {
        friend class Draft;

    public:

        /// \brief Constructs a cursor that reads nothing.
        ZY_INLINE Cursor()
            : mOwner      { nullptr },
              mArchetypes { false },
              mState      { nullptr },
              mHandles    { }
        {
        }

        /// \brief Constructs a cursor of a world, seeing every change from now on, archetypes and sleepers left out.
        ///
        /// \param Owner The world.
        ZY_INLINE explicit Cursor(Ref<Storage> Owner)
            : Cursor(Owner, WithoutHidden(Owner))
        {
        }

        /// \brief Takes over another cursor.
        ///
        /// \param Other The cursor to take over, which reads nothing afterwards.
        ZY_INLINE Cursor(AnyRef<Cursor> Other)
            : mOwner      { Exchange(Other.mOwner, nullptr) },
              mArchetypes { Other.mArchetypes },
              mState      { Exchange(Other.mState, nullptr) },
              mHandles    { Exchange(Other.mHandles, Array<UInt32, List::kSize>()) }
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

        /// \brief Checks whether the cursor reads a list of a world.
        ///
        /// \return `true` if it does, `false` if it reads nothing.
        ZY_INLINE Bool IsValid() const
        {
            return mHandles.GetFront() != 0;
        }

        /// \brief Hands every entry recorded since the last call to a callback.
        ///
        /// \param Callback The callable, taking `(Entity)` or `(Entity, ConstRef<T>)` for removals, else fields like a query.
        template<typename Callable>
        void Each(AnyRef<Callable> Callback)
        {
            ZY_ASSERT(mHandles.GetFront(), "The cursor reads nothing");

            if constexpr (List::kKind == Pull::Removed)
            {
                ReadRemovals(Callback);
            }
            else
            {
                ReadChanges(Callback);
            }
        }

        /// \brief Cursors are not assigned, since each owns its position.
        Ref<Cursor> operator=(ConstRef<Cursor> Other) = delete;

    private:

        /// The component the list is about.
        using Type = typename List::Value;

        /// \brief Constructs a cursor filtering entries by a selection it owns from here on.
        ///
        /// \param Owner The world.
        /// \param State The selection, whose terms are declared.
        ZY_INLINE Cursor(Ref<Storage> Owner, AnyRef<Unique<Selection>> State)
            : mOwner      { AddressOf(Owner) },
              mArchetypes { WantsArchetypes(Owner, * State) },
              mState      { Adopt(Owner, Move(State)) },
              mHandles    { Subscribe(Owner) }
        {
        }

        /// \brief Subscribes to the list of each change the reader asks for, additions first.
        ///
        /// \param Owner The world.
        /// \return The handle of each subscription.
        ZY_INLINE static Array<UInt32, List::kSize> Subscribe(Ref<Storage> Owner)
        {
            if constexpr (List::kSize == 2)
            {
                const UInt32 Added = Owner.mLedger.Subscribe(IdentifierOf<Type>(), Pull::Added);
                return Array<UInt32, List::kSize>(Added, Owner.mLedger.Subscribe(IdentifierOf<Type>(), Pull::Changed));
            }
            else
            {
                return Array<UInt32, List::kSize>(Owner.mLedger.Subscribe(IdentifierOf<Type>(), List::kKind));
            }
        }

        /// \brief Makes a selection that leaves archetypes and sleepers out.
        ///
        /// \param Owner The world.
        /// \return The selection.
        ZY_INLINE static Unique<Selection> WithoutHidden(Ref<Storage> Owner)
        {
            Unique<Selection> State = Unique<Selection>::Create();
            State->Excluded.Append(Owner.mPrefab);
            State->Excluded.Append(Owner.mAsleep);
            return State;
        }

        /// \brief Checks whether a reader of removals asked for archetypes.
        ///
        /// \param Owner The world.
        /// \param State The selection the reader was described with.
        /// \return `true` if it reads archetypes alone, `false` if it reads everything else.
        ZY_INLINE static Bool WantsArchetypes(Ref<Storage> Owner, ConstRef<Selection> State)
        {
            return List::kKind == Pull::Removed && State.Required.Contains(Owner.mPrefab);
        }

        /// \brief Keeps the selection of a list, whose entries are matched against it.
        ///
        /// \param Owner The world.
        /// \param State The selection.
        /// \return The selection kept current by the world, or `nullptr` for removals asking for nothing more.
        ZY_INLINE static Ptr<Selection> Adopt(Ref<Storage> Owner, AnyRef<Unique<Selection>> State)
        {
            if constexpr (List::kKind == Pull::Removed)
            {
                // Sleepers were always seen, and the archetype marker alone is settled by the handle.
                State->Required.Erase(Owner.mPrefab);
                State->Excluded.Erase(Owner.mPrefab);
                State->Excluded.Erase(Owner.mAsleep);

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

            Ref<Ledger>  Lists = mOwner->mLedger;
            const UInt64 End   = Lists.GetEnd(mHandles.GetFront());

            // What the callback changes waits like in any walk.
            mOwner->Enter();

            for (UInt64 Next = Lists.GetPosition(mHandles.GetFront()); Next < End; ++Next)
            {
                const Handle Actor = Lists.GetActor(mHandles.GetFront(), Next);

                // Archetypes go only to a reader that asked for them, and then nothing else does.
                if (Directory::IsArchetype(Actor.GetIndex()) != mArchetypes)
                {
                    continue;
                }

                // Terms are matched against the chunk the entity left, which a gone entity no longer has.
                if (mState)
                {
                    const UInt32 Source = Lists.GetSource(mHandles.GetFront(), Next);

                    if (Source >= mState->Positions.GetSize() || mState->Positions[Source] == 0)
                    {
                        continue;
                    }
                }

                if constexpr (kValue)
                {
                    const ConstPtr<Byte> Bytes = Lists.GetValue(mHandles.GetFront(), Next);
                    Callback(Entity(mOwner, Actor), * reinterpret_cast<ConstPtr<Type>>(Bytes));
                }
                else
                {
                    Callback(Entity(mOwner, Actor));
                }
            }

            Lists.SetPosition(mHandles.GetFront(), End);
            mOwner->Leave();
        }

        /// \brief Hands every entry since the last call to a callback, with the fields it asks for, like a query.
        ///
        /// \param Callback The callable, taking the entity first when it asks for it, then its fields.
        template<typename Callable>
        void ReadChanges(Ref<Callable> Callback)
        {
            using Blueprint = Plan<StripAll<Callable>>;

            static_assert(!Blueprint::kBatched, "A cursor is handed one entry at a time");

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

            // What the callback changes waits like in any walk, and entries it causes wait for the next read.
            mOwner->Enter();

            for (UInt Index = 0; Index < List::kSize; ++Index)
            {
                for (UInt64 Next = Lists.GetPosition(mHandles[Index]); Next < Ends[Index]; ++Next)
                {
                    const Handle Actor = Lists.GetActor(mHandles[Index], Next);

                    if (Matches(Actor))
                    {
                        Blueprint::template Apply<Walk::Kernel>::RunEntry(* mOwner, * mState, Actor, Callback);
                    }
                }
                Lists.SetPosition(mHandles[Index], Ends[Index]);
            }

            mOwner->Leave();
        }

        /// \brief Checks whether an entity is still alive and still matches the terms the reader asked for.
        ///
        /// \param Actor The entity.
        /// \return `true` if it does, `false` otherwise.
        ZY_INLINE Bool Matches(Handle Actor) const
        {
            ConstRef<Directory> Directory = mOwner->mDirectory;
            return Directory.IsAlive(Actor) && mState->GetPosition(* Directory[Actor.GetIndex()].Holder) != 0;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Storage>               mOwner;
        Bool                       mArchetypes;
        Ptr<Selection>             mState;
        Array<UInt32, List::kSize> mHandles;
    };
}