// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
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
    ///
    /// \tparam Types The component types this query will match against.
    template<typename... Types>
    class Query
    {
    public:

        /// \brief Underlying handle to the ECS query object.
        using Handle  = flecs::query<>;

        /// \brief Pattern type representing the query's component requirements.
        using Pattern = DSL::In<Types...>;

    public:

        /// \brief Constructs an empty query with no associated handle.
        ZYPHRYON_INLINE Query() = default;

        /// \brief Constructs a query from an existing handle.
        ///
        /// \param Handle The handle of this query.
        ZYPHRYON_INLINE Query(AnyRef<Handle> Handle)
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
        template<typename FEach>
        ZYPHRYON_INLINE void Run(AnyRef<FEach> Each) const
        {
            mHandle.run([Each](Ref<flecs::iter> Iterator)
            {
                Invoke(Iterator, Each);
            });
        }

        /// \brief Executes the query with optional begin and end callbacks.
        ///
        /// \param Begin The function or functor executed before processing entities.
        /// \param Each  The function or functor executed for each matching entity.
        /// \param End   The function or functor executed after processing entities.
        template<typename FBegin, typename FEach, typename FEnd>
        ZYPHRYON_INLINE void Run(AnyRef<FBegin> Begin, AnyRef<FEach> Each, AnyRef<FEnd> End) const
        {
            mHandle.run([Begin, Each, End](Ref<flecs::iter> Iterator)
            {
                Invoke(Iterator, Begin, Each, End);
            });
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

    public:

        /// \brief Creates a runner function for iterating over entities with a per-entity callback.
        ///
        /// \param Each The function or functor to execute for each entity.
        /// \return A callable that can be used to run the query.
        template<typename FEach>
        ZYPHRYON_INLINE static auto Runner(AnyRef<FEach> Each)
        {
            return [Each](Ref<flecs::iter> Iterator)
            {
                Invoke(Iterator, Each);
            };
        }

        /// \brief Creates a runner function for iterating over entities with begin, per-entity, and end callbacks.
        ///
        /// \param Begin The function or functor executed before iteration begins.
        /// \param Each  The function or functor executed for each entity.
        /// \param End   The function or functor executed after iteration completes.
        /// \return A callable that can be used to run the query.
        template<typename FBegin, typename FEach, typename FEnd>
        ZYPHRYON_INLINE static auto Runner(AnyRef<FBegin> Begin, AnyRef<FEach> Each, AnyRef<FEnd> End)
        {
            return [Begin, Each, End](Ref<flecs::iter> Iterator)
            {
                Query::Invoke(Iterator, Begin, Each, End);
            };
        }

        /// \brief Internal helper for invoking per-entity callbacks during iteration.
        ///
        /// \param Iterator The iterator over matching entities.
        /// \param Each     The function or functor to execute for each entity.
        template<typename FEach>
        ZYPHRYON_INLINE static void Invoke(Ref<flecs::iter> Iterator, AnyRef<FEach> Each)
        {
            while (Iterator.next())
            {
                flecs::_::each_delegate<FEach, Types...>(Each).invoke(const_cast<ecs_iter_t*>(Iterator.c_ptr()));
            }
        }

        /// \brief Internal helper for invoking begin, per-entity, and end callbacks during iteration.
        ///
        /// \param Iterator The iterator over matching entities.
        /// \param Begin    The callback executed before iteration begins.
        /// \param Each     The callback executed for each entity.
        /// \param End      The callback executed after iteration completes.
        template<typename FBegin, typename FEach, typename FEnd>
        ZYPHRYON_INLINE static void Invoke(Ref<flecs::iter> Iterator, AnyRef<FBegin> Begin, AnyRef<FEach> Each, AnyRef<FEnd> End)
        {
            Begin();

            while (Iterator.next())
            {
                flecs::_::each_delegate<FEach, Types...>(Each).invoke(const_cast<ecs_iter_t*>(Iterator.c_ptr()));
            }

            End();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        mutable Handle mHandle;
    };
}