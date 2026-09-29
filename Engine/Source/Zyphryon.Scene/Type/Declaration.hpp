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

#include "Registry.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    class World;

    /// \brief Represents what every declaration holds, whatever it declares, and the telling of its world.
    class ZY_API Enrollment
    {
    protected:

        /// \brief Constructs an enrollment in a world.
        ///
        /// \param Owner The world the components are declared in.
        ZY_INLINE explicit Enrollment(Ptr<World> Owner)
            : mWorld { Owner }
        {
        }

        /// \brief Tells the world the components are declared, reading back the values it kept as bytes for them.
        ///
        /// \param Identifiers The components.
        void Finish(ConstSpan<UInt32> Identifiers);

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<World>  mWorld;
        Description mPresentation;
    };

    /// \brief Represents components declared with chained calls, the world told once the declaring statement ends.
    ///
    /// \tparam Types The components, which share everything stated about them.
    template<typename... Types>
    class Declaration final : public Enrollment
    {
    public:

        /// \brief Constructs the declaration of components, naming them after their type or under the given name.
        ///
        /// \param Owner The world they are declared in.
        /// \param Name  The name saves know the component by, or empty for the one its type carries.
        ZY_INLINE Declaration(Ptr<World> Owner, Text Name)
            : Enrollment(Owner)
        {
            ZY_ASSERT(Name.IsEmpty() || sizeof...(Types) == 1, "Only one component can be declared under a name");

            (Enroll<Types>(Name), ...);
        }

        /// \brief Tells the world the components are declared, reading back the values it kept as bytes for them.
        ZY_INLINE ~Declaration()
        {
            const UInt32 Identifiers[] = { IdentifierOf<Types>()... };
            Finish(ConstSpan<UInt32>(Identifiers));
        }

        /// \brief Declarations are not copied, since each tells the world once.
        Declaration(ConstRef<Declaration> Other) = delete;

        /// \brief Makes saves carry the components.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Serializable() &&
        {
            return Apply(Trait::Serializable);
        }

        /// \brief Makes instances read the components from their archetype rather than hold a copy.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Inheritable() &&
        {
            return Apply(Trait::Inheritable);
        }

        /// \brief Keeps the components off every instance made from an archetype.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Local() &&
        {
            return Apply(Trait::Local);
        }

        /// \brief Makes the world hold the one instance of each component, which queries read from there.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Singleton() &&
        {
            return Apply(Trait::Singleton);
        }

        /// \brief Makes removals of the components keep the value that left, which readers of the removals are handed.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Keeps() &&
        {
            return Apply(Trait::Keeps);
        }

        /// \brief Makes the components what a tool places on archetypes, saved and inherited.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Authored() &&
        {
            Apply(Trait::Serializable);
            return Apply(Trait::Inheritable);
        }

        /// \brief Brings other components along wherever these are added, which may be only declared here.
        ///
        /// \return This declaration.
        template<typename... Implied>
        ZY_INLINE AnyRef<Declaration> Implies() &&
        {
            (Imply<Types, Implied...>(), ...);
            return Move(* this);
        }

        /// \brief Gives the components the label a tool shows.
        ///
        /// \param Label The label.
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Describe(Text Label) &&
        {
            mPresentation.Label = Label;
            return Move(* this);
        }

        /// \brief Gives the described components the heading they are listed under.
        ///
        /// \param Group The heading.
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Under(Text Group) &&
        {
            mPresentation.Group = Group;
            return Move(* this);
        }

        /// \brief Gives the described components the glyph shown beside their label.
        ///
        /// \param Icon The codepoint of the glyph.
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Iconed(UInt32 Icon) &&
        {
            mPresentation.Icon = Icon;
            return Move(* this);
        }

        /// \brief Gives the described components the text saying what they are for.
        ///
        /// \param Tooltip The text.
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Hinted(Text Tooltip) &&
        {
            mPresentation.Tooltip = Tooltip;
            return Move(* this);
        }

        /// \brief Sets the places a tool may put the described components.
        ///
        /// \param Policy The places.
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Placed(Authoring Policy) &&
        {
            mPresentation.Policy = Policy;
            return Move(* this);
        }

        /// \brief Lays the components' fields out for tools, as each type describes them.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Reflect() &&
        {
            (Registry::Get().SetSchema(IdentifierOf<Types>(), ZyReflection::Schema::Of<Types>()), ...);
            return Move(* this);
        }

    private:

        /// \brief Registers one component under the name it answers to.
        ///
        /// \param Name The name, or empty for the one its type carries.
        template<typename Type>
        ZY_INLINE static void Enroll(Text Name)
        {
            if constexpr (requires { { Type::kName } -> IsCastable<Text>; })
            {
                Name = Name.IsEmpty() ? Text(Type::kName) : Name;
            }
            Registry::Get().Declare(IdentifierOf<Type>(Name), Metatype::Of<Type>(), Name);
        }

        /// \brief Makes one component bring others along.
        template<typename Type, typename... Implied>
        ZY_INLINE static void Imply()
        {
            (Registry::Get().Imply(IdentifierOf<Type>(), Reserve<Implied>()), ...);
        }

        /// \brief Gets the identifier of a component, reserving it by the name the compiler gives it when only declared.
        ///
        /// \return The identifier, which a component only declared takes over once it is declared in turn.
        template<typename Type>
        ZY_INLINE static UInt32 Reserve()
        {
            if constexpr (requires { sizeof(Type); })
            {
                return IdentifierOf<Type>();
            }
            else
            {
                return Registry::Get().Acquire(Metatype::Qualify<Type>());
            }
        }

        /// \brief Gives every component a trait.
        ///
        /// \param Which The trait.
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Apply(Trait Which)
        {
            (Registry::Get().SetTrait(IdentifierOf<Types>(), Which, true), ...);
            return Move(* this);
        }
    };
}