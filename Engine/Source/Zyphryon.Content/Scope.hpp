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

#include "Resource.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Tracks dependencies for a resource loading operation.
    class Scope final
    {
    public:

        /// \brief The maximum number of dependencies that can be tracked.
        constexpr static UInt32 kMaxDependencies = 10;

    public:

        /// \brief Constructs an empty scope with no associated resource.
        ZY_INLINE Scope()
            : mResource { nullptr }
        {
        }

        /// \brief Constructs a scope associated with the given resource.
        ///
        /// \param Resource The resource this scope is tracking dependencies for.
        ZY_INLINE explicit Scope(ConstRetainer<Resource> Resource)
            : mResource { Resource }
        {
        }

        /// \brief Gets the resource associated with this scope.
        ///
        /// \return A shared handle to the associated resource, or an empty handle if none.
        ZY_INLINE ConstRetainer<Resource> GetResource() const
        {
            return mResource;
        }

        /// \brief Polls the dependencies, removing any that have finished loading.
        ///
        /// \return `true` if all dependencies have finished loading, otherwise `false`.
        ZY_INLINE Bool Poll()
        {
            mDependencies.RemoveFastSomeIf([](ConstRetainer<Resource> Dependency)
            {
                return Dependency->HasFinished();
            });
            return mDependencies.IsEmpty();
        }

        /// \brief Adds a dependency to track.
        ///
        /// \param Dependency The resource dependency to track.
        ZY_INLINE void DependsOn(ConstRetainer<Resource> Dependency)
        {
            mDependencies.Append(Dependency);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Resource>                             mResource;
        Sequence<Retainer<Resource>, kMaxDependencies> mDependencies;
    };
}