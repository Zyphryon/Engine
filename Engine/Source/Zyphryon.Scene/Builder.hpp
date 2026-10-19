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

#include "Cursor.hpp"
#include "Query.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    class World;

    /// \brief Represents the system kind of a \ref Builder, which a phase runs every progress.
    struct System final
    {
    };

    /// \brief Represents what every description holds, whatever it makes, and the making of it in its world.
    class ZY_API Draft
    {
    protected:

        /// \brief Constructs a description with the terms asked for so far.
        ///
        /// \param Owner The world it is made in.
        /// \param State The terms asked for so far.
        /// \param Name  The name profiles show.
        /// \param Phase The phase a system runs in, zero for a query.
        ZY_INLINE Draft(Ptr<World> Owner, AnyRef<Unique<Selection>> State, Text Name, UInt32 Phase)
            : mWorld  { Owner },
              mState  { Move(State) },
              mName   { Name },
              mPhase  { Phase },
              mRate   { 1 },
              mSpread { false }
        {
        }

        /// \brief Takes over another description.
        ///
        /// \param Other The description to take over, which describes nothing afterwards.
        Draft(AnyRef<Draft> Other) = default;

        /// \brief Descriptions are not copied, since each makes one thing.
        Draft(ConstRef<Draft> Other) = delete;

        /// \brief Makes a system that walks what its callback asks for, or only calls it when it asks for nothing.
        ///
        /// \param Callback The callable, taking the entity first when it asks for it, then its fields.
        template<typename Callable>
        void Appoint(AnyRef<Callable> Callback)
        {
            using Function  = StripAll<Callable>;
            using Blueprint = Plan<Function>;

            // A callback taking nothing, asked for nothing, walks nothing and is simply called.
            if constexpr (Blueprint::kCount == 0 && !Blueprint::kEntity)
            {
                if (mState->Required.IsEmpty() && mState->Alternatives.IsEmpty())
                {
                    Install(Forward<Callable>(Callback));
                    return;
                }
            }

            Blueprint::Declare(* mState);

            // A callback reading only singletons, asked for nothing else, matches the world alone that holds them.
            if constexpr (!Blueprint::kEntity)
            {
                const Bool Unasked
                    = mState->Owned.IsEmpty() && mState->Required.IsEmpty() && mState->Alternatives.IsEmpty();

                if (Unasked && !mState->Globals.IsEmpty() && mState->Globals.GetSize() == mState->Fields.GetSize())
                {
                    mState->Required.Append(mState->Globals);
                }
            }

            // Whether the walk spreads never changes, so it is settled here rather than on every run.
            if (mSpread)
            {
                Install(Chore<Function, true>(Enlist(), Forward<Callable>(Callback)));
            }
            else
            {
                Install(Chore<Function, false>(Enlist(), Forward<Callable>(Callback)));
            }
        }

        /// \brief Makes a system that reads a pull list.
        ///
        /// \param Callback The callable, taking a query's fields, or for removals the entity and maybe its old value.
        template<typename Reader, typename Callable>
        void Subscribe(AnyRef<Callable> Callback)
        {
            Reader Source(GetStorage(), Move(mState));
            Install(Tap<Reader, StripAll<Callable>> { Move(Source), Forward<Callable>(Callback) });
        }

        /// \brief Makes the query this description asks for.
        ///
        /// \return The query, archetypes and sleepers left out unless asked.
        Query Enlist();

        /// \brief Hands a runner to the scheduler as the system this description names.
        ///
        /// \param Runner The runner, called when the system's turn comes.
        void Install(AnyRef<Delegate<void()>> Runner);

        /// \brief Gets the storage of the world the description is made in.
        ///
        /// \return The world itself, as its storage.
        Ref<Storage> GetStorage() const;

    private:

        /// \brief Represents what a phase runs for a system that walks the entities it matches.
        template<typename Callable, Bool Spread>
        struct Chore final
        {
            /// The query it walks, let go of with it.
            Query    Scope;

            /// The callback the system was handed.
            Callable Callback;

            /// \brief Walks what the system matches when its turn comes, spread over the workers when it may.
            ZY_INLINE void operator()()
            {
                if constexpr (Spread)
                {
                    Walk::Spread(* Scope.mOwner, * Scope.mState, Callback);
                }
                else
                {
                    Walk::Run(* Scope.mOwner, * Scope.mState, Callback);
                }
            }
        };

        /// \brief Represents what a phase runs for a system reading changes.
        template<typename Reader, typename Callable>
        struct Tap final
        {
            /// The cursor it reads with, filtering entries by the terms the system asked for.
            Reader   Source;

            /// The callback the system was handed.
            Callable Callback;

            /// \brief Reads what changed since the last run, when its turn comes.
            ZY_INLINE void operator()()
            {
                Source.Each(Callback);
            }
        };

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<World>        mWorld;
        Unique<Selection> mState;
        Str               mName;
        UInt32            mPhase;
        UInt16            mRate;
        Bool              mSpread;
    };

    /// \brief Represents a query or system described with chained calls and made from its callback.
    ///
    /// \tparam Kind The thing it makes, a \ref Query, a \ref System, or a \ref Cursor a system reads changes with.
    template<typename Kind>
    class Builder final : public Draft
    {
        template<typename> friend class Builder;
        friend class World;

    public:

        /// \brief Takes over another description.
        ///
        /// \param Other The description to take over, which describes nothing afterwards.
        Builder(AnyRef<Builder> Other) = default;

        /// \brief Descriptions are not copied, since each makes one thing.
        Builder(ConstRef<Builder> Other) = delete;

        /// \brief Asks for components a match carries without handing them over, archetypes or sleepers when named.
        ///
        /// \return This description.
        template<typename... Types>
        ZY_INLINE AnyRef<Builder> With() &&
        {
            (mState->Required.Append(IdentifierOf<Types>()), ...);
            (mState->Excluded.Erase(IdentifierOf<Types>()), ...);
            return Move(* this);
        }

        /// \brief Asks for components a match neither holds nor inherits.
        ///
        /// \return This description.
        template<typename... Types>
        ZY_INLINE AnyRef<Builder> Not() &&
        {
            (mState->Excluded.Append(IdentifierOf<Types>()), ...);
            return Move(* this);
        }

        /// \brief Asks for at least one of a set of components.
        ///
        /// \return This description.
        template<typename... Types>
        ZY_INLINE AnyRef<Builder> Or() &&
        {
            static_assert(sizeof...(Types) > 1, "Or asks for at least two alternatives");

            Ref<Sequence<UInt32>> Group = mState->Alternatives.Append();
            (Group.Append(IdentifierOf<Types>()), ...);
            return Move(* this);
        }

        /// \brief Lets archetypes or sleepers in alongside everything else, where \ref With lets in only them.
        ///
        /// \return This description.
        template<typename... Types>
        ZY_INLINE AnyRef<Builder> Also() &&
        {
            static_assert(((IsAnyOf<Types, Prefab, Asleep>) && ...), "Also lets in archetypes or sleepers only");

            (mState->Excluded.Erase(IdentifierOf<Types>()), ...);
            return Move(* this);
        }

        /// \brief Reads a component the callback takes as a pointer from the nearest ancestor carrying it.
        ///
        /// \return This description.
        template<typename Type>
        ZY_INLINE AnyRef<Builder> Up() &&
            requires IsAnyOf<Kind, System>
        {
            mState->Ancestors.Append(IdentifierOf<StripAll<Type>>());
            return Move(* this);
        }

        /// \brief Reads a component like \ref Up, every ancestor visited before what stands beneath it, spread.
        ///
        /// \return This description.
        template<typename Type>
        ZY_INLINE AnyRef<Builder> Cascade() &&
            requires IsAnyOf<Kind, System>
        {
            mState->Ancestors.Append(IdentifierOf<StripAll<Type>>());
            mState->Ordered = true;
            mSpread         = true;
            return Move(* this);
        }

        /// \brief Declares the components the walk writes, each marked written for every entity it visits.
        ///
        /// \return This description.
        template<typename... Types>
        ZY_INLINE AnyRef<Builder> Writes() &&
            requires IsAnyOf<Kind, System, Query>
        {
            (Selection::Insert(mState->Writes, IdentifierOf<StripAll<Types>>()), ...);
            return Move(* this);
        }

        /// \brief Runs a system once every few progresses.
        ///
        /// \param Ticks The progresses from one run to the next, one to run on every one.
        /// \return This description.
        ZY_INLINE AnyRef<Builder> Rate(UInt16 Ticks) &&
            requires (!IsAnyOf<Kind, Query>)
        {
            mRate = Max<UInt16>(Ticks, 1);
            return Move(* this);
        }

        /// \brief Lets a system spread its walks over the workers when that pays.
        ///
        /// \return This description.
        ZY_INLINE AnyRef<Builder> Spread() &&
            requires IsAnyOf<Kind, System>
        {
            mSpread = true;
            return Move(* this);
        }

        /// \brief Makes a system read changes of a component rather than walk every match.
        ///
        /// \return The description, which leaves archetypes and sleepers out unless it asks for them.
        template<typename List>
        ZY_INLINE Builder<Cursor<List>> Reads() &&
            requires IsAnyOf<Kind, System>
        {
            ZY_ASSERT(mState->Ancestors.IsEmpty(), "A system reading changes reads no ancestor");

            return Builder<Cursor<List>>(Move(static_cast<Ref<Draft>>(* this)));
        }

        /// \brief Makes the system, handing it the callback whose parameters are its fields.
        ///
        /// \param Callback The callable, taking the entity first when it asks for it, then its fields, or nothing.
        template<typename Callable>
        ZY_INLINE void Each(AnyRef<Callable> Callback) &&
            requires (!IsAnyOf<Kind, Query>)
        {
            if constexpr (IsAnyOf<Kind, System>)
            {
                Appoint(Forward<Callable>(Callback));
            }
            else
            {
                Subscribe<Kind>(Forward<Callable>(Callback));
            }
        }

        /// \brief Makes the query.
        ///
        /// \return The query, which keeps what it matches current until it is let go.
        ZY_INLINE operator Query() &&
            requires IsAnyOf<Kind, Query>
        {
            return Enlist();
        }

    private:

        /// \brief Constructs a description asking only for what the callback will, archetypes and sleepers left out.
        ///
        /// \param Owner The world it is made in.
        /// \param Name  The name profiles show, empty for a query.
        /// \param Phase The phase a system runs in, zero for a query.
        ZY_INLINE Builder(Ptr<World> Owner, Text Name, UInt32 Phase)
            : Draft(Owner, Unique<Selection>::Create(), Name, Phase)
        {
        }

        /// \brief Constructs a description taking over everything another one asked for, whatever it made.
        ///
        /// \param Other The description to take over, which describes nothing afterwards.
        ZY_INLINE explicit Builder(AnyRef<Draft> Other)
            : Draft(Move(Other))
        {
        }

        /// \brief Asks for a component a query names, as its own when written and held or inherited when read.
        template<typename Type>
        ZY_INLINE void Ask()
        {
            if constexpr (!IsPointer<Type>)
            {
                Ref<Sequence<UInt32>> Terms = Field<Type, false>::kOwned ? mState->Owned : mState->Required;
                Selection::Insert(Terms, IdentifierOf<StripAll<Type>>());
            }
        }
    };
}