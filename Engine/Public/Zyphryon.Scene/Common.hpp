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

#include "Zyphryon.Base/Base.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Constants used across the scene module.
    enum
    {
        /// \brief Minimum ID for component identifiers.
        kMinRangeComponents = 0,

        /// \brief Maximum ID for component identifiers.
        kMaxRangeComponents = 1'023,

        /// \brief Maximum number of supported component types.
        kMaxCountComponents = kMaxRangeComponents - kMinRangeComponents,

        /// \brief Minimum ID for archetype identifiers.
        kMinRangeArchetypes = 1'024,

        /// \brief Maximum ID for archetype identifiers.
        kMaxRangeArchetypes = 65'534,

        /// \brief Maximum number of supported archetypes.
        kMaxCountArchetypes = kMaxRangeArchetypes - kMinRangeArchetypes,

        /// \brief Minimum ID for entity identifiers.
        kMinRangeEntities   = 65'535,

        /// \brief Maximum ID for entity identifiers.
        kMaxRangeEntities   = 4'294'967'295,

        /// \brief Maximum number of supported entities.
        kMaxCountEntities   = kMaxRangeEntities - kMinRangeEntities,
    };

    /// \brief Enumerates behavioral traits that define component capabilities.
    enum class Trait : UInt8
    {
        Serializable, ///< Component can be serialized to or from an archive.
        Inheritable,  ///< Component values can be inherited across entities.
        Toggleable,   ///< Component can be toggled on/off without removal.
        Sparse,       ///< Component is stored sparsely for memory efficiency.
        Associative,  ///< Component behaves like a key-value association.
        Singleton,    ///< Component exists only once globally (per world).
        Final,        ///< Component cannot be extended or overridden.
        Symmetric,    ///< Component has symmetric behavior in relationships.
        Phase,        ///< Component is a phase.
    };

    /// \brief Defines execution modes for system scheduling.
    enum class Execution : UInt8
    {
        Default,    ///< Normal scheduled execution (deferred to the pipeline).
        Immediate,  ///< Executes immediately when triggered.
        Concurrent, ///< Executes concurrently in worker threads.
    };

    /// \brief Defines caching strategies for system queries.
    enum class Cache : UInt8
    {
        Default,    ///< Determined by query creation context.
        Auto,       ///< Automatically manages caching based on query terms.
        None,       ///< Disables caching; query is evaluated every execution.
    };
}