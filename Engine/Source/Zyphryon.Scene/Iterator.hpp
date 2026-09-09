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

#include "Entity.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents a walk over the entities a query, system or observer matched.
    ///
    /// Results arrive one batch at a time, each batch holding the entities of a single table.
    class Iterator final
    {
    public:

        /// \brief Constructs an iterator over a result the ECS handed out.
        ///
        /// \param Handle The result to walk. Owned by whoever created it, never by the iterator.
        ZY_INLINE explicit Iterator(Ptr<ecs_iter_t> Handle)
            : mHandle { Handle }
        {
        }

        /// \brief Gets the internal handle representing this iteration.
        ///
        /// \return The iteration internal handle.
        ZY_INLINE Ptr<ecs_iter_t> GetHandle() const
        {
            return mHandle;
        }

        /// \brief Marks the current batch as spent, so the first advance starts a fresh walk.
        ///
        /// \note A result handed to a system arrives already marked valid, which the first advance would unlock.
        ZY_INLINE void Reset() const
        {
            mHandle->flags &= ~EcsIterIsValid;
        }

        /// \brief Advances to the next batch of matching entities.
        ///
        /// \return `true` while a batch was produced, `false` once the walk is exhausted.
        ZY_INLINE Bool Next() const
        {
            if ((mHandle->flags & EcsIterIsValid) && mHandle->table)
            {
                ecs_table_unlock(mHandle->world, mHandle->table);
            }

            const Bool Result = mHandle->next(mHandle);

            mHandle->flags |= EcsIterIsValid;

            if (Result && mHandle->table)
            {
                ecs_table_lock(mHandle->world, mHandle->table);
            }
            return Result;
        }

        /// \brief Gets the number of entities in the current batch.
        ///
        /// \return A count of entities the batch holds.
        ZY_INLINE UInt GetCount() const
        {
            return static_cast<UInt>(mHandle->count);
        }

        /// \brief Gets the entity occupying a row of the current batch.
        ///
        /// \param Row The row to read, which must be below \ref GetCount.
        /// \return The entity at that row.
        ZY_INLINE Entity GetEntity(UInt Row) const
        {
            ZY_ASSERT(Row < GetCount(), "Row is outside the current batch");

            return Entity(mHandle->world, mHandle->entities[Row]);
        }

        /// \brief Gets the target of the pair a field matched, which is what a wildcard term leaves open.
        ///
        /// \param Field The field to read, which is the first term unless the query names more.
        /// \return The entity the pair points at, or an invalid entity when the field matched no pair.
        ZY_INLINE Entity GetTarget(UInt8 Field = 0) const
        {
            const ecs_id_t Handle = ecs_field_id(mHandle, Field);

            if (ecs_id_is_pair(Handle))
            {
                return Entity(mHandle->world, ecs_get_alive(mHandle->world, ECS_PAIR_SECOND(Handle)));
            }
            return Entity();
        }

        /// \brief Gets the time elapsed since the previous frame.
        ///
        /// \return The frame delta in seconds.
        ZY_INLINE Real32 GetDelta() const
        {
            return mHandle->delta_time;
        }

        /// \brief Gets the time elapsed since this system last ran, which a rate or interval can stretch.
        ///
        /// \return The system delta in seconds.
        ZY_INLINE Real32 GetSystemDelta() const
        {
            return mHandle->delta_system_time;
        }

        /// \brief Gets the system this iteration is running for.
        ///
        /// \return The system entity, or an invalid entity outside a system.
        ZY_INLINE Entity GetSystem() const
        {
            return Entity(mHandle->world, mHandle->system);
        }

        /// \brief Gets the event this iteration was triggered by.
        ///
        /// \return The event entity, or an invalid entity outside an observer.
        ZY_INLINE Entity GetEvent() const
        {
            return Entity(mHandle->world, mHandle->event);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<ecs_iter_t> mHandle;
    };
}