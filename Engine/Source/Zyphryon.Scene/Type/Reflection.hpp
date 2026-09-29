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

namespace ZyScene
{
    /// \brief Specifies where a tool may put a component, as the set of places it is allowed to stand.
    enum class Authoring : UInt8
    {
        Derived   = 0,                     ///< Nowhere by hand, since something else brings it along.
        Archetype = 1 << 0,                ///< On an archetype, where every instance shares it.
        Instance  = 1 << 1,                ///< On a placed instance, which carries it alone.
        World     = 1 << 2,                ///< On the world as a whole, which stands in nothing.
        Anywhere  = Archetype | Instance,  ///< On an archetype and on an instance alike.
    };
    ZY_DEFINE_BITWISE_ENUM(Authoring)

    /// \brief Represents how a component presents itself to a tool, which nothing in the simulation reads.
    struct Description final
    {
        /// The name a tool shows for the component, empty when the component was never described.
        Text      Label;

        /// The heading the component is listed under, empty for none.
        Text      Group;

        /// The text saying what the component is for, shown on request.
        Text      Tooltip;

        /// The codepoint of the glyph a tool shows beside the label, or zero for none.
        UInt32    Icon    = 0;

        /// The places a tool may put the component.
        Authoring Policy  = Authoring::Anywhere;

        /// \brief Checks whether a tool may put the component in any of the places given.
        ///
        /// \param Where The places to ask after, which may be one or several taken together.
        /// \return `true` if it may stand in at least one of them, `false` otherwise.
        ZY_INLINE constexpr Bool Allows(Authoring Where) const
        {
            return (Policy & Where) != Authoring::Derived;
        }
    };
}