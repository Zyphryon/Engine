// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents a compile-time unique tag for ECS entities or components.
    ///
    /// Tags are used to mark or categorize entities and components at compile time using a UTF-8 string literal.
    ///
    /// \tparam Symbol The compile-time UTF-8 string literal for this tag.
    template<Symbol Symbol>
    struct Tag final
    {
        /// \brief The unique 64-bit identifier for this tag, computed from the string literal.
        static constexpr UInt64    kID   = Symbol.Hash();

        /// \brief The compile-time string literal for this tag.
        static constexpr ConstStr8 kName = Symbol;
    };

    /// \brief Tag type for marking transient (non-persistent) objects.
    ///
    /// Objects with this tag are not intended to persist across serialization boundaries.
    using Transient = Tag<"Transient">;
}