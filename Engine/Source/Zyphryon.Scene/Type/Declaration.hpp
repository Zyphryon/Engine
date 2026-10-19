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

    /// \brief Represents what every declaration holds, and tells its world once it is done.
    class ZY_API Enrollment
    {
    protected:

        /// \brief Constructs an enrollment in a world.
        ///
        /// \param Owner      The world the component is declared in.
        /// \param Identifier The identifier the component answers to.
        ZY_INLINE Enrollment(Ptr<World> Owner, UInt32 Identifier)
            : mWorld      { Owner },
              mIdentifier { Identifier }
        {
        }

        /// \brief Tells the world the component is declared, which reads back the values a save kept as bytes for it.
        void Finish();

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<World> mWorld;
        UInt32     mIdentifier;
    };

    /// \brief Represents one component declared with chained calls, the world told once the declaring statement ends.
    ///
    /// \tparam Type The component, named by its `kName` or the name the world was handed.
    template<typename Type>
    class Declaration final : public Enrollment
    {
    public:

        /// \brief Constructs the declaration of a component.
        ///
        /// \param Owner The world it is declared in.
        /// \param Name  The name saves know it by, or empty for its `kName`.
        ZY_INLINE Declaration(Ptr<World> Owner, Text Name)
            : Enrollment(Owner, Enroll(Name))
        {
        }

        /// \brief Tells the world the component is declared, which reads back the values a save kept as bytes for it.
        ZY_INLINE ~Declaration()
        {
            Finish();
        }

        /// \brief Declarations are not copied, since each tells the world once.
        Declaration(ConstRef<Declaration> Other) = delete;

        /// \brief Makes saves carry the component.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Serializable() &&
        {
            return Apply(Trait::Serializable);
        }

        /// \brief Makes instances read the component from their archetype rather than hold a copy.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Inheritable() &&
        {
            return Apply(Trait::Inheritable);
        }

        /// \brief Keeps the component off every instance made from an archetype.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Local() &&
        {
            return Apply(Trait::Local);
        }

        /// \brief Makes the component what a tool places on archetypes, saved and read by instances.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Authored() &&
        {
            return Apply(Trait::Serializable | Trait::Inheritable);
        }

        /// \brief Makes the world entity hold the one instance, which walks read as a field.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Singleton() &&
        {
            return Apply(Trait::Singleton);
        }

        /// \brief Makes removals of the component keep the value that left, which readers of the removals are handed.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Keeps() &&
        {
            return Apply(Trait::Keeps);
        }

        /// \brief Keeps the component only on the server side of a session.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Server() &&
        {
            ZY_ASSERT(!Registry::Get().GetMetatype(mIdentifier).Has(Trait::Client),
                "A component is kept by one side or by both");

            return Apply(Trait::Server);
        }

        /// \brief Keeps the component only on the client side of a session.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Client() &&
        {
            ZY_ASSERT(!Registry::Get().GetMetatype(mIdentifier).Has(Trait::Server),
                "A component is kept by one side or by both");

            return Apply(Trait::Client);
        }

        /// \brief Brings other components along wherever this one is added, which may be only declared here.
        ///
        /// \return This declaration.
        template<typename... Implied>
        ZY_INLINE AnyRef<Declaration> Implies() &&
        {
            (Imply(Reserve<Implied>()), ...);
            return Move(* this);
        }

        /// \brief Gives the component where a tool lists it and may put it.
        ///
        /// \param Group  The heading it is listed under, which a tool keys its text by, never empty.
        /// \param Icon   The codepoint of the glyph shown beside it, or zero for none.
        /// \param Policy The places a tool may put it.
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Describe(Text Group, UInt32 Icon = 0, Authoring Policy = Authoring::Anywhere) &&
        {
            ZY_ASSERT(!Group.IsEmpty(), "A described component is listed under a group");

            Registry::Get().Amend(mIdentifier, [&](Ref<Metatype> Info)
            {
                Info.Presentation = Description(Group, Icon, Policy);
            });
            return Move(* this);
        }

        /// \brief Lays the component's fields out for tools, as the type describes them.
        ///
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Reflect() &&
        {
            const ZyReflection::Schema Layout = ZyReflection::Schema::Of<Type>();

            Registry::Get().Amend(mIdentifier, [&Layout](Ref<Metatype> Info)
            {
                Info.Layout = ZyReflection::Schema(
                    Layout.GetFields(), Layout.GetName(), Info.GetSize(), Info.GetAlignment());
            });
            return Move(* this);
        }

    private:

        /// \brief Registers the component under the name it answers to.
        ///
        /// \param Name The name, or empty for its `kName`.
        /// \return The identifier the component answers to.
        ZY_INLINE static UInt32 Enroll(Text Name)
        {
            if constexpr (requires { { Type::kName } -> IsCastable<Text>; })
            {
                Name = Name.IsEmpty() ? Text(Type::kName) : Name;
            }

            const UInt32 Identifier = IdentifierOf<Type>(Name);
            Registry::Get().Declare(Identifier, Metatype::Of<Type>(), Name);
            return Identifier;
        }

        /// \brief Gets the identifier of a component, reserving it by its compiler name when it is only declared.
        ///
        /// \return The identifier, which a component only declared takes over once it is declared in turn.
        template<typename Other>
        ZY_INLINE static UInt32 Reserve()
        {
            if constexpr (requires { sizeof(Other); })
            {
                return IdentifierOf<Other>();
            }
            else
            {
                return Registry::Get().Acquire(Metatype::Qualify<Other>());
            }
        }

        /// \brief Makes the component bring another along.
        ///
        /// \param Implied The identifier of the component brought along, never this one.
        ZY_INLINE void Imply(UInt32 Implied)
        {
            ZY_ASSERT(Implied != mIdentifier, "A component cannot bring itself along");

            Registry::Get().Amend(mIdentifier, [Implied](Ref<Metatype> Info)
            {
                if (!Info.Implies.Contains(Implied))
                {
                    Info.Implies.Append(Implied);
                }
            });
        }

        /// \brief Gives the component a trait.
        ///
        /// \param Which The trait, or several taken together.
        /// \return This declaration.
        ZY_INLINE AnyRef<Declaration> Apply(Trait Which)
        {
            Registry::Get().Amend(mIdentifier, [Which](Ref<Metatype> Info)
            {
                Info.Set(Which, true);
            });
            return Move(* this);
        }
    };
}