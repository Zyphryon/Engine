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

#include "Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::_
{
    /// \brief Holds the identifier a component type answers to.
    ///
    /// \note The slot is filled once by \ref Register and read by every operation spelled with a type.
    ///
    /// \tparam Type The component type the slot belongs to.
    template<typename Type>
    struct Identity final
    {
        /// The identifier the world assigned, or `0` while the type has not been registered yet.
        static inline ecs_entity_t Value = 0;
    };

    /// \brief Gets the name the compiler records for a type, reduced to its last qualifier.
    ///
    /// \note The view points into the compiler's own literal, so it carries no null terminator of its own.
    ///
    /// \return The unqualified name of \p Type.
    template<typename Type>
    ZY_INLINE constexpr Text GetTypeName()
    {
#if defined(ZY_COMPILER_MSVC)
        constexpr Text Signature = __FUNCSIG__;
        constexpr Text Prologue  = "GetTypeName<";
        constexpr Text Epilogue  = ">(void)";
#else
        constexpr Text Signature = __PRETTY_FUNCTION__;
        constexpr Text Prologue  = "Type = ";
        constexpr Text Epilogue  = "]";
#endif

        constexpr Text Qualified = StrBefore(StrAfter(Signature, Prologue), Epilogue);
        constexpr Text Scoped    = StrAfterLast(Qualified, "::");
        constexpr Text Named     = Scoped.IsEmpty() ? Qualified : Scoped;

        // A type outside every namespace keeps the keyword its compiler spells it with, which is dropped here.
        constexpr Text Keyword   = StrAfterLast(Named, ' ');
        return Keyword.IsEmpty() ? Named : Keyword;
    }

    /// \brief Builds the lifecycle hooks a component type needs to survive being moved between tables.
    ///
    /// \note A hook left null means the operation is a plain copy of the bytes, which only trivial types allow.
    ///
    /// \return The hooks describing how instances of \p Type are constructed, copied, moved and destroyed.
    template<typename Type>
    ZY_INLINE ecs_type_hooks_t GetTypeHooks()
    {
        ecs_type_hooks_t Hooks { };

        if constexpr (!IsTriviallyConstructible<Type>)
        {
            if constexpr (IsDefaultConstructible<Type>)
            {
                Hooks.ctor = [](Ptr<void> Data, SInt32 Count, ConstPtr<ecs_type_info_t>)
                {
                    for (Ptr<Type> Element = static_cast<Ptr<Type>>(Data); Count--; ++Element)
                    {
                        new (Element) Type();
                    }
                };
            }
            else
            {
                Hooks.flags |= ECS_TYPE_HOOK_CTOR_ILLEGAL;
            }
        }

        if constexpr (!IsTriviallyDestructible<Type>)
        {
            Hooks.dtor = [](Ptr<void> Data, SInt32 Count, ConstPtr<ecs_type_info_t>)
            {
                for (Ptr<Type> Element = static_cast<Ptr<Type>>(Data); Count--; ++Element)
                {
                    Element->~Type();
                }
            };
        }

        if constexpr (!IsTriviallyCopyable<Type>)
        {
            if constexpr (IsCopyAssignable<Type>)
            {
                Hooks.copy = [](Ptr<void> Destination, ConstPtr<void> Source, SInt32 Count, ConstPtr<ecs_type_info_t>)
                {
                    const Ptr<Type>      Output = static_cast<Ptr<Type>>(Destination);
                    const ConstPtr<Type> Input  = static_cast<ConstPtr<Type>>(Source);

                    for (SInt32 Element = 0; Element < Count; ++Element)
                    {
                        Output[Element] = Input[Element];
                    }
                };
            }
            else
            {
                Hooks.flags |= ECS_TYPE_HOOK_COPY_ILLEGAL;
            }

            if constexpr (IsCopyConstructible<Type>)
            {
                Hooks.copy_ctor = [](Ptr<void> Destination, ConstPtr<void> Source, SInt32 Count, ConstPtr<ecs_type_info_t>)
                {
                    const Ptr<Type>      Output = static_cast<Ptr<Type>>(Destination);
                    const ConstPtr<Type> Input  = static_cast<ConstPtr<Type>>(Source);

                    for (SInt32 Element = 0; Element < Count; ++Element)
                    {
                        new (AddressOf(Output[Element])) Type(Input[Element]);
                    }
                };
            }
            else
            {
                Hooks.flags |= ECS_TYPE_HOOK_COPY_CTOR_ILLEGAL;
            }
        }

        if constexpr (!IsTriviallyMoveAssignable<Type>)
        {
            if constexpr (IsMoveAssignable<Type>)
            {
                Hooks.move = [](Ptr<void> Destination, Ptr<void> Source, SInt32 Count, ConstPtr<ecs_type_info_t>)
                {
                    const Ptr<Type> Output = static_cast<Ptr<Type>>(Destination);
                    const Ptr<Type> Input  = static_cast<Ptr<Type>>(Source);

                    for (SInt32 Element = 0; Element < Count; ++Element)
                    {
                        Output[Element] = Move(Input[Element]);
                    }
                };
            }
            else
            {
                Hooks.flags |= ECS_TYPE_HOOK_MOVE_ILLEGAL;
            }
        }

        if constexpr (!IsTriviallyMoveConstructible<Type>)
        {
            if constexpr (IsMoveConstructible<Type>)
            {
                Hooks.move_ctor = [](Ptr<void> Destination, Ptr<void> Source, SInt32 Count, ConstPtr<ecs_type_info_t>)
                {
                    const Ptr<Type> Output = static_cast<Ptr<Type>>(Destination);
                    const Ptr<Type> Input  = static_cast<Ptr<Type>>(Source);

                    for (SInt32 Element = 0; Element < Count; ++Element)
                    {
                        new (AddressOf(Output[Element])) Type(Move(Input[Element]));
                    }
                };
            }
            else
            {
                Hooks.flags |= ECS_TYPE_HOOK_MOVE_CTOR_ILLEGAL;
            }
        }

        // Relocating a component builds it in the new table and tears the old instance down in the same pass.
        if constexpr (!IsTriviallyMoveConstructible<Type> || !IsTriviallyDestructible<Type>)
        {
            if constexpr (IsMoveConstructible<Type>)
            {
                Hooks.ctor_move_dtor = [](Ptr<void> Destination, Ptr<void> Source, SInt32 Count, ConstPtr<ecs_type_info_t>)
                {
                    const Ptr<Type> Output = static_cast<Ptr<Type>>(Destination);
                    const Ptr<Type> Input  = static_cast<Ptr<Type>>(Source);

                    for (SInt32 Element = 0; Element < Count; ++Element)
                    {
                        new (AddressOf(Output[Element])) Type(Move(Input[Element]));
                        Input[Element].~Type();
                    }
                };
            }
            else
            {
                Hooks.flags |= ECS_TYPE_HOOK_CTOR_MOVE_DTOR_ILLEGAL;
            }
        }

        // Overwriting a live component assigns onto it and tears the old instance down in the same pass.
        if constexpr (!IsTriviallyMoveAssignable<Type> || !IsTriviallyDestructible<Type>)
        {
            if constexpr (IsMoveAssignable<Type>)
            {
                Hooks.move_dtor = [](Ptr<void> Destination, Ptr<void> Source, SInt32 Count, ConstPtr<ecs_type_info_t>)
                {
                    const Ptr<Type> Output = static_cast<Ptr<Type>>(Destination);
                    const Ptr<Type> Input  = static_cast<Ptr<Type>>(Source);

                    for (SInt32 Element = 0; Element < Count; ++Element)
                    {
                        Output[Element] = Move(Input[Element]);
                        Input[Element].~Type();
                    }
                };
            }
            else
            {
                Hooks.flags |= ECS_TYPE_HOOK_MOVE_DTOR_ILLEGAL;
            }
        }
        return Hooks;
    }

    /// \brief Registers a component type in the world, or resolves the identifier it already holds.
    ///
    /// \note A type that carries no data registers as a zero-sized component, which is how flecs spells a tag.
    ///
    /// \param World The world the component belongs to.
    /// \param Name  The identifier the component registers under, or empty to derive it from the type.
    /// \return The identifier assigned to \p Type.
    template<typename Type>
    ZY_INLINE ecs_entity_t Register(Ptr<ecs_world_t> World, Text Name)
    {
        using Component = StripAll<Type>;

        if (Ref<ecs_entity_t> Slot = Identity<Component>::Value; !Slot)
        {
            const Str64 Label(Name.IsEmpty() ? GetTypeName<Component>() : Name);

            ecs_entity_desc_t Entity { };
            Entity.name       = Label.GetData();
            Entity.sep        = "::";
            Entity.root_sep   = "::";
            Entity.use_low_id = true;

            ecs_component_desc_t Description { };
            Description.entity = ecs_entity_init(World, AddressOf(Entity));

            if constexpr (!IsEmpty<Component>)
            {
                Description.type.size      = sizeof(Component);
                Description.type.alignment = alignof(Component);
                Description.type.hooks     = GetTypeHooks<Component>();
            }

            Slot = ecs_component_init(World, AddressOf(Description));

            ZY_ASSERT(Slot, "Failed to register a component in the world");
        }
        return Identity<Component>::Value;
    }

    /// \brief Gets the identifier a component type registered under.
    ///
    /// \return The identifier of \p Type.
    template<typename Type>
    ZY_INLINE ecs_entity_t Identify()
    {
        ZY_ASSERT(Identity<StripAll<Type>>::Value, "Component is used before it was registered in the world");

        return Identity<StripAll<Type>>::Value;
    }

    /// \brief Gets the identifier a relation pair formed by two component types resolves to.
    ///
    /// \return The identifier of the pair.
    template<typename Relation, typename Component>
    ZY_INLINE ecs_id_t Identify()
    {
        return ecs_pair(Identify<Relation>(), Identify<Component>());
    }

    /// \brief Gets the identifier a relation pair with a runtime target resolves to.
    ///
    /// \param Component The target of the relation.
    /// \return The identifier of the pair.
    template<typename Relation>
    ZY_INLINE ecs_id_t Identify(ecs_entity_t Component)
    {
        return ecs_pair(Identify<Relation>(), Component);
    }
}