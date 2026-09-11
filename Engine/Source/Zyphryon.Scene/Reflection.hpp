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
    /// \brief Specifies where a tool may attach a component, given what the entity it goes on stands as.
    enum class Authoring : UInt8
    {
        Anywhere,   ///< On an archetype and on an instance alike.
        Instance,   ///< On a placed instance only, since an archetype has no use for it.
        Archetype,  ///< On an archetype only, since every instance shares it.
        Derived,    ///< Never by hand, since something else brings it along.
    };

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

        /// Where a tool may attach the component.
        Authoring Policy;
    };
}

namespace Scene::DSL
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

        /// \brief Restricts where a tool may attach the component.
        ///
        /// \param Policy Where the component may be attached.
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
            Entity(World, Scene::_::Identify<Type>(World)).Set(Value);
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
            static_assert(Reflection::IsDescribed<Type>,
                "A reflected component lays out its own fields with OnDescribe, or with ZY_REFLECT");

            Entity(World, Scene::_::Identify<Type>(World)).Set(Reflection::Schema::Of<Type>());
        }
    };

    /// \brief Shows the fields the component's own type laid out, so a tool may read and write them.
    inline constexpr Reflecting Reflect { };
}