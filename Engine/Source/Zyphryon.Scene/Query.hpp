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
        using Handle  = Ptr<ecs_query_t>;

    public:

        /// \brief Constructs an invalid query with no associated world or filters.
        ZY_INLINE Query()
            : mHandle { nullptr }
        {
        }

        /// \brief Constructs a query from an existing handle.
        ///
        /// \param Handle The handle of this query.
        ZY_INLINE Query(Handle Handle) noexcept
            : mHandle { Handle }
        {
        }

        /// \brief Move-constructs a query from another query instance.
        ///
        /// \param Other The query to move from.
        ZY_INLINE Query(AnyRef<Query> Other) noexcept
            : mHandle { Exchange(Other.mHandle, Handle()) }
        {
        }

        /// \brief Destroys the query and releases its underlying resources.
        ZY_INLINE ~Query()
        {
            // An anonymous query belongs to whoever built it, so nothing else would ever release it.
            if (mHandle && !mHandle->entity)
            {
                ecs_query_fini(mHandle);

                mHandle = Handle();
            }
        }

        /// \brief Explicitly destroys the query and resets its handle.
        ///
        /// \note The query becomes invalid after destruction.
        ZY_INLINE void Destruct() const
        {
            if (mHandle && mHandle->entity)
            {
                ecs_query_fini(mHandle);
                mHandle = Handle();
            }
        }

        /// \brief Gets the number of entities matching the query.
        ///
        /// \return The count of matching entities.
        ZY_INLINE UInt Matches() const
        {
            return mHandle ? static_cast<UInt>(ecs_query_count(mHandle).entities) : 0;
        }

        /// \brief Executes the query, invoking a callback for each matching entity.
        ///
        /// \note Omitting \p Types derives them from the parameters \p Each declares, in order.
        ///
        /// \param Each The function or functor to execute for every matching entity.
        template<typename... Types, typename FEach>
        ZY_INLINE void Run(AnyRef<FEach> Each) const
        {
            using Declared  = DSL::_::TypeList<Types...>;
            using Trimmed   = DSL::_::StripContext<typename DSL::_::SignatureOf<StripAll<FEach>>::Type>::Type;
            using Inferred  = typename DSL::_::Infer<Trimmed>::Fields;
            using Signature = Select<sizeof...(Types) == 0, Inferred, Declared>;
            using Runner    = DSL::_::RunnerFactory<Signature, StripAll<FEach>>;

            ecs_iter_t     Handle = ecs_query_iter(mHandle->world, mHandle);
            const Iterator Cursor(AddressOf(Handle));

            Cursor.Reset();

            Runner::Make(Move(Each))(Cursor);
        }

        /// \brief Move-assigns a query from another query instance, transferring ownership.
        ZY_INLINE Ref<Query> operator=(AnyRef<Query> Other) noexcept
        {
            if (this != AddressOf(Other))
            {
                mHandle = Exchange(Other.mHandle, Handle());
            }
            return (* this);
        }

        /// \brief Deleted copy assignment operator to prevent copying of queries.
        ZY_INLINE Ref<Query> operator=(ConstRef<Query>) = delete;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        mutable Handle mHandle;
    };
}