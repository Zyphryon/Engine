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

#include "Uri.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Represents a thread-safe cache and factory for managing loadable assets for \p Type.
    template<typename Type>
    class Cache final
    {
    public:

        /// \brief Constructs an empty cache with no memory budget or usage.
        ZY_INLINE Cache()
            : mLimit { 0 }
        {
        }

        /// \brief Sets the maximum memory budget for stored assets.
        ///
        /// \param Limit The asset memory budget in bytes.
        ZY_INLINE void SetMemoryLimit(UInt64 Limit)
        {
            mLimit = Limit;
        }

        /// \brief Gets the configured memory budget for stored assets.
        ///
        /// \return The memory budget in bytes.
        ZY_INLINE UInt64 GetMemoryLimit() const
        {
            return mLimit;
        }

        /// \brief Computes the total memory footprint of all assets currently in the cache.
        ///
        /// Thread-safe. Acquires a lock on the internal registry.
        ///
        /// \return The sum of memory footprint for every asset in the registry.
        ZY_INLINE UInt64 GetMemoryUsage() const
        {
            Guard Lock(mMutex);

            UInt64 Total = 0;

            for (const auto [_, Asset] : mRegistry)
            {
                Total += Asset->GetFootprint();
            }
            return Total;
        }

        /// \brief Retrieves an existing asset or optionally creates it if not found.
        ///
        /// Thread-safe. Acquires a lock on the internal registry.
        ///
        /// \param Key            The URI of the asset.
        /// \param CreateIfNeeded If `true`, a new asset is created when not found.
        /// \return A Retainer to the asset, or `nullptr` if not found and not created.
        ZY_INLINE Retainer<Type> GetOrCreate(AnyRef<Uri> Key, Bool CreateIfNeeded)
        {
            Guard Lock(mMutex);

            const UInt64 ID = Hash(Key);

            if (const Ptr<Retainer<Type>> Asset = mRegistry.Find(ID))
            {
                return (* Asset);
            }

            if (CreateIfNeeded)
            {
                Retainer<Type> Result = Retainer<Type>::Create(Move(Key));
                mRegistry.Assign(ID, Result);
                return Result;
            }
            return nullptr;
        }

        /// \brief Removes an asset if it has finished loading.
        ///
        /// Thread-safe. Acquires a lock on the internal registry.
        ///
        /// \param Key The URI of the asset to remove.
        /// \return `true` if the asset was found and had finished loading, otherwise `false`.
        ZY_INLINE Bool Remove(ConstRef<Uri> Key)
        {
            Guard Lock(mMutex);

            return mRegistry.EraseIf(Hash(Key), [](ConstRetainer<Type> Asset)
            {
                return Asset->HasFinished();
            });
        }

        /// \brief Removes finished assets that are solely owned by the cache, or all finished assets if forced.
        ///
        /// Thread-safe. Acquires a lock on the internal registry.
        ///
        /// \param Force      If `true`, all finished assets are removed regardless of tracking state.
        /// \param Dispatcher The callback to invoke for each asset before it is removed.
        template<typename Function>
        ZY_INLINE void Prune(Bool Force, AnyRef<Function> Dispatcher)
        {
            Guard Lock(mMutex);

            mRegistry.EraseIf([&](ConstRef<Uri>, ConstRetainer<Type> Asset)
            {
                if (Force || (Asset->IsUnique() && Asset->HasFinished()))
                {
                    Dispatcher(* Asset);
                    return true;
                }
                return false;
            });
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<UInt64, Retainer<Type>> mRegistry;
        mutable Mutex                 mMutex;
        UInt64                        mLimit;
    };
}