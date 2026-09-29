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

#include "Execution/Walk.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents a cached query, the entities carrying some components, kept current until it is let go.
    class ZY_API Query final
    {
        friend class Draft;

    public:

        /// \brief Constructs a query that matches nothing.
        Query();

        /// \brief Constructs a query over a selection the storage keeps current.
        ///
        /// \param Owner The storage the query belongs to.
        /// \param State The selection, which the query owns from here on.
        Query(Ptr<Storage> Owner, Ptr<Selection> State);

        /// \brief Takes over another query.
        ///
        /// \param Other The query to take over, which matches nothing afterwards.
        Query(AnyRef<Query> Other);

        /// \brief Lets go of what the query matches.
        ~Query();

        /// \brief Queries are not copied, since each owns what it matches.
        Query(ConstRef<Query> Other) = delete;

        /// \brief Takes over another query, letting go of what this one matched.
        ///
        /// \param Other The query to take over, which matches nothing afterwards.
        /// \return This query.
        Ref<Query> operator=(AnyRef<Query> Other);

        /// \brief Queries are not copied, since each owns what it matches.
        Ref<Query> operator=(ConstRef<Query> Other) = delete;

        /// \brief Checks whether the query was made by a world.
        ///
        /// \return `true` if it matches entities of a world, `false` if it matches nothing.
        ZY_INLINE Bool IsValid() const
        {
            return mState != nullptr;
        }

        /// \brief Gets the number of entities the query matches right now.
        ///
        /// \return The number of matching entities.
        ZY_INLINE UInt Matches() const
        {
            return mState ? mState->Count() : 0;
        }

        /// \brief Hands every match to a callback, holding every structural change it asks for until the walk ends.
        ///
        /// \param Each The callable, taking the entity first when it asks for it, then its fields.
        template<typename Callable>
        ZY_INLINE void Run(AnyRef<Callable> Each) const
        {
            Walk::Run(* mOwner, Prepare<StripAll<Callable>>(), Each);
        }

        /// \brief Hands every matching entity to a callback, spread over the compute workers when the work is worth it.
        ///
        /// \param Each The callable, safe to call from many threads at once and writing only in place.
        template<typename Callable>
        ZY_INLINE void Spread(AnyRef<Callable> Each) const
        {
            Walk::Spread(* mOwner, Prepare<StripAll<Callable>>(), Each);
        }

        /// \brief Checks whether the last walk that could spread did.
        ///
        /// \return `true` if it was spread over the workers, `false` if it stayed on one thread.
        ZY_INLINE Bool IsSpreading() const
        {
            return mState && mState->Spread;
        }

    private:

        /// \brief Gets the selection, declared for a callback's fields when it was declared for others.
        ///
        /// \return The selection.
        template<typename Callable>
        ZY_INLINE Ref<Selection> Prepare() const
        {
            ZY_ASSERT(mState, "The query matches nothing");

            if (mState->Declared != Plan<Callable>::GetKey())
            {
                Plan<Callable>::Declare(* mState);

                mOwner->mLayout.Populate(* mState, mOwner->mDirectory);
            }
            return * mState;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Storage>   mOwner;
        Ptr<Selection> mState;
    };
}