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

    /// \brief Describes how a component presents itself to a tool, which nothing in the simulation reads.
    struct Description final
    {
        /// The name a tool shows for the component.
        Text      Label;

        /// The heading the component is listed under, empty for none.
        Text      Group;

        /// The glyph a tool shows beside the label, in whatever font the tool draws with.
        Text      Icon;

        /// What the component is for, shown on request.
        Text      Tooltip;

        /// The places a tool may put the component.
        Authoring Policy;

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

namespace ZyScene::DSL
{
    /// \brief Represents the declaration of how a component presents itself to a tool.
    struct Describing final
    {
        /// The description the component is given.
        Description Value;

        /// \brief Lists the component under a heading.
        ///
        /// \param Group The heading to list it under.
        /// \return The term, grouped.
        ZY_INLINE constexpr Describing Under(Text Group) const
        {
            Describing Result = (* this);
            Result.Value.Group = Group;
            return Result;
        }

        /// \brief Gives the component the glyph a tool shows beside its label.
        ///
        /// \param Icon The glyph, in whatever font the tool draws with.
        /// \return The term, iconed.
        ZY_INLINE constexpr Describing Iconed(Text Icon) const
        {
            Describing Result = (* this);
            Result.Value.Icon = Icon;
            return Result;
        }

        /// \brief Gives the component the explanation a tool shows on request.
        ///
        /// \param Tooltip What the component is for.
        /// \return The term, explained.
        ZY_INLINE constexpr Describing Hinted(Text Tooltip) const
        {
            Describing Result = (* this);
            Result.Value.Tooltip = Tooltip;
            return Result;
        }

        /// \brief Restricts the places a tool may put the component to the ones given.
        ///
        /// \param Policy The places the component may stand, which replace the ones it stood in before.
        /// \return The term, restricted.
        ZY_INLINE constexpr Describing Placed(Authoring Policy) const
        {
            Describing Result = (* this);
            Result.Value.Policy = Policy;
            return Result;
        }

        /// \brief Puts the description on one component.
        ///
        /// \tparam Type  The component described.
        /// \param  World The world the component belongs to.
        template<typename Type>
        ZY_INLINE void Apply(Ptr<ecs_world_t> World) const
        {
            Entity(World, ZyScene::_::Identify<Type>()).Set(Value);
        }
    };

    /// \brief Describes a component to tools, beside the traits it carries.
    ///
    /// \param Label The name a tool shows for the component.
    /// \return The term, to be handed to \ref Declare, or refined first.
    ZY_INLINE constexpr Describing Describe(Text Label)
    {
        return Describing { .Value = Description { .Label = Label, .Policy = Authoring::Anywhere } };
    }

    /// \brief Describes a component to tools under a heading and a glyph, which is how most are described.
    ///
    /// \param Label The name a tool shows for the component.
    /// \param Group The heading the component is listed under.
    /// \param Icon  The glyph a tool shows beside the label, in whatever font the tool draws with.
    /// \return The term, to be handed to \ref Declare, or refined first.
    ZY_INLINE constexpr Describing Describe(Text Label, Text Group, Text Icon)
    {
        return Describe(Label).Under(Group).Iconed(Icon);
    }

    /// \brief Represents the declaration that a component shows the fields its type laid out.
    struct Reflecting final
    {
        /// \brief Puts the fields of the type on the component.
        ///
        /// \tparam Type  The component reflected.
        /// \param  World The world the component belongs to.
        template<typename Type>
        ZY_INLINE void Apply(Ptr<ecs_world_t> World) const
        {
            static_assert(ZyReflection::IsDescribed<Type>,
                "A reflected component lays out its own fields with OnDescribe, or with ZY_REFLECT");

            Entity(World, ZyScene::_::Identify<Type>()).Set(ZyReflection::Schema::Of<Type>());
        }
    };

    /// \brief Shows the fields the component's own type laid out, so a tool may read and write them.
    inline constexpr Reflecting Reflect { };
}