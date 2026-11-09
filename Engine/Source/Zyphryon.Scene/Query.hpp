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
    /// \brief Represents a query walked on demand, which keeps what it matches current until it is let go.
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

        /// \brief Queries are not assigned, since each owns what it matches.
        Ref<Query> operator=(ConstRef<Query> Other) = delete;

        /// \brief Gets the number of entities the query matches right now.
        ///
        /// \return The number, zero for a query that matches nothing.
        ZY_INLINE UInt Matches() const
        {
            return mState ? mState->Count() : 0;
        }

        /// \brief Hands every match to a callback, holding every structural change it asks for until the walk ends.
        ///
        /// \param Callback The callable, taking the entity first when it asks for it, then its fields.
        template<typename Callable>
        ZY_INLINE void Run(AnyRef<Callable> Callback) const
        {
            ZY_ASSERT(mState, "The query matches nothing");

            Walk::Declare<StripAll<Callable>>(* mOwner, * mState);
            Walk::Run(* mOwner, * mState, Callback);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Storage>   mOwner;
        Ptr<Selection> mState;
    };
}