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

#include "Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Engine
{
    /// \brief Base class for engine modules that can be attached to a host.
    class Module
    {
    public:

        /// \brief Destructs the module and releases any associated resources.
        virtual ~Module() = default;

        /// \brief Gets the name of the module.
        ///
        /// \return The module name.
        virtual Text GetName() const = 0;

        /// \brief Gets the version of the module.
        ///
        /// \return The module version.
        virtual Text GetVersion() const = 0;

        /// \brief Called when the module is attached to a host.
        ///
        /// \param Host The subsystem host managing this module's lifetime.
        virtual void OnAttach(Ref<Subsystem::Host> Host) = 0;

        /// \brief Called when the module is detached from a host.
        ///
        /// \param Host The subsystem host managing this module's lifetime.
        virtual void OnDetach(Ref<Subsystem::Host> Host) = 0;
    };

    /// \brief A sequence of uniquely owned modules.
    using Modules = Sequence<Unique<Module>>;
}