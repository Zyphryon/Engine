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

#include "Builder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents a query over entities within the ECS (Entity-Component System).
    class Query
    {
    public:

        /// \brief Underlying handle to the ECS query object.
        using Handle  = flecs::query<>;

    public:

        /// \brief Constructs an empty query with no associated handle.
        ZYPHRYON_INLINE Query() = default;

        /// \brief Constructs a query from an existing handle.
        ///
        /// \param Handle The handle of this query.
        ZYPHRYON_INLINE Query(AnyRef<Handle> Handle) noexcept
            : mHandle { Move(Handle) }
        {
        }

        /// \brief Move-constructs a query from another query instance.
        ///
        /// \param Other The query to move from.
        ZYPHRYON_INLINE Query(AnyRef<Query> Other) noexcept
            : mHandle { Exchange(Other.mHandle, Handle()) }
        {
        }

        /// \brief Destroys the query and releases its underlying resources.
        ZYPHRYON_INLINE ~Query()
        {
            Destruct();
        }

        /// \brief Explicitly destroys the query and resets its handle.
        ///
        /// \note The query becomes invalid after destruction.
        ZYPHRYON_INLINE void Destruct() const
        {
            if (mHandle && mHandle.entity())
            {
                mHandle.destruct();
                mHandle = Handle();
            }
        }

        /// \brief Retrieves the number of entities matching the query.
        ///
        /// \return The count of matching entities.
        ZYPHRYON_INLINE UInt64 Matches() const
        {
            return mHandle.count();
        }

        /// \brief Executes the query, invoking a callback for each matching entity.
        ///
        /// \param Each The function or functor to execute for every matching entity.
        template<typename... Types, typename FEach>
        ZYPHRYON_INLINE void Run(AnyRef<FEach> Each) const
        {
            using Signature = DSL::_::TypeList<Types...>;
            using Runner    = DSL::_::RunnerFactory<Signature, Decay<FEach>>;

            mHandle.run(Runner::Make(Move(Each)));
        }

        /// \brief Executes the query with optional begin and end callbacks.
        ///
        /// \param Begin The function or functor executed before processing entities.
        /// \param Each  The function or functor executed for each matching entity.
        /// \param End   The function or functor executed after processing entities.
        template<typename... Types, typename FBegin, typename FEach, typename FEnd>
        ZYPHRYON_INLINE void Run(AnyRef<FBegin> Begin, AnyRef<FEach> Each, AnyRef<FEnd> End) const
        {
            using Signature = DSL::_::TypeList<Types...>;
            using Runner    = DSL::_::RunnerFactoryLifecycle<Signature, Decay<FBegin>, Decay<FEach>, Decay<FEnd>>;

            mHandle.run(Runner::Make(Move(Begin), Move(Each), Move(End)));
        }

        /// \brief Move-assigns a query from another query instance.
        ///
        /// \param Other The query to move from.
        /// \return Reference to this query.
        ZYPHRYON_INLINE Ref<Query> operator=(AnyRef<Query> Other)
        {
            if (this != &Other)
            {
                mHandle = Exchange(Other.mHandle, Handle());
            }
            return (* this);
        }

        /// \brief Copy assignment is explicitly disabled.
        ZYPHRYON_INLINE Ref<Query> operator=(ConstRef<Query>) = delete;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        mutable Handle mHandle;
    };
}