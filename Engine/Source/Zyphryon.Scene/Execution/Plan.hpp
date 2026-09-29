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

#include "Zyphryon.Scene/Storage/Selection.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    class Entity;

    /// \brief Represents one value a walk hands its callback.
    ///
    /// \tparam Type    The component, `const` when only read.
    /// \tparam Pointer `true` when handed as a pointer, which may be missing or read from an ancestor.
    template<typename Type, Bool Pointer>
    struct Field final
    {
        /// The component, `const` when only read.
        using Value = Type;

        /// `true` when handed as a pointer.
        static constexpr Bool kPointer = Pointer;

        /// `true` when the component carries no data, so a fresh one is handed.
        static constexpr Bool kTag     = IsEmpty<StripAll<Type>>;

        /// `true` when the callback writes it, so it lives in the entity's own column.
        static constexpr Bool kWritten = !IsImmutable<Type>;
    };

    /// \brief Represents a parameter reduced to the field it stands for.
    ///
    /// \tparam Parameter The parameter, as the callback spells it.
    template<typename Parameter>
    struct FieldOf final
    {
        /// The parameter without its reference.
        using Plain = StripRef<Parameter>;

        /// The field, read through a pointer when the parameter is one, and `const` unless taken by reference.
        using Type  = Select<IsPointer<Plain>,
                        Field<StripPtr<Plain>, true>,
                        Field<Select<IsAnyOf<Parameter, Plain>,
                                        const Plain,
                                              Plain>, false>>;

        /// `true` when the parameter is a span.
        static constexpr Bool kSpan = false;
    };

    /// \brief Represents a span parameter reduced to the field of the element it spans.
    ///
    /// \tparam Element The element.
    template<typename Element>
    struct FieldOf<Span<Element>> final
    {
        /// The field of the element.
        using Type = Field<Element, false>;

        /// `true` when the parameter is a span.
        static constexpr Bool kSpan = true;
    };

    /// \brief Represents the fields a callback asks a walk for, in the order it takes them.
    ///
    /// \tparam Parts The fields.
    template<typename... Parts>
    struct Fieldset
    {
        /// The number of fields.
        static constexpr UInt kCount = sizeof...(Parts);

        /// The loops of a walk made for these fields, handed their positions in the callback alongside.
        template<template<typename, typename...> class Target>
        using Apply = Target<MakeIntegerSequence<UInt, kCount>, Parts...>;

        /// \brief Gets what stands for these fields, the same for every callback asking for them.
        ///
        /// \return The key.
        ZY_INLINE static ConstPtr<void> GetKey()
        {
            return AddressOf(kKey);
        }

        /// \brief Writes the fields into a selection, adding what a match must carry for them.
        ///
        /// \param State The selection, whose ancestor components are declared already.
        ZY_INLINE static void Declare(Ref<Selection> State)
        {
            State.Fields.Clear();
            State.Modes.Clear();
            State.Globals.Clear();
            State.Chunks.Clear();
            State.Sources.Clear();
            State.Positions.Clear();
            State.Gathered.Clear();
            State.Snapshot.Clear();

            (AddField<Parts>(State), ...);
            State.Declared = GetKey();
        }

    private:

        /// The byte whose address stands for these fields.
        static constexpr Byte kKey = 0;

        /// \brief Writes one field into a selection, as walks read it.
        ///
        /// \param State The selection.
        template<typename Field>
        static void AddField(Ref<Selection> State)
        {
            using Mode = Selection::Mode;

            // A written field lives in the entity's own column, unless it is a tag with nothing to write.
            constexpr Bool kOwned  = Field::kWritten && !Field::kTag;
            constexpr Mode kAccess = Field::kWritten ? Mode::Write : Mode::Read;

            const UInt32 Identifier = IdentifierOf<StripAll<typename Field::Value>>();
            const Bool   Singleton  = Registry::Get().GetMetatype(Identifier).Has(Trait::Singleton);

            State.Fields.Append(Identifier);

            if constexpr (Field::kPointer)
            {
                if (State.Ancestors.Contains(Identifier))
                {
                    State.Modes.Append(Mode::Ancestor);
                }
                else
                {
                    State.Modes.Append(Singleton ? Mode::Global : kAccess);
                }
            }
            else if (Singleton)
            {
                State.Modes.Append(Mode::Global);
                State.Globals.Append(Identifier);
            }
            else
            {
                if constexpr (kOwned)
                {
                    AddTerm(State.Owned, Identifier);
                }
                else
                {
                    AddTerm(State.Required, Identifier);
                }
                State.Modes.Append(kOwned ? Mode::Write : Mode::Read);
            }
        }

        /// \brief Adds a component to a list of terms unless it is there already.
        ///
        /// \param Terms      The list.
        /// \param Identifier The component.
        ZY_INLINE static void AddTerm(Ref<Sequence<UInt32>> Terms, UInt32 Identifier)
        {
            if (!Terms.Contains(Identifier))
            {
                Terms.Append(Identifier);
            }
        }
    };

    /// \brief Represents the parameters of a callback, each one a field.
    ///
    /// \tparam Arguments The parameters, in order.
    template<typename... Arguments>
    struct Parameters : Fieldset<typename FieldOf<Arguments>::Type...>
    {
        /// `true` when the callback takes the entity ahead of its fields.
        static constexpr Bool kEntity  = false;

        /// `true` when the callback takes a span per field, and is handed a run of rows at a time.
        static constexpr Bool kBatched = sizeof...(Arguments) > 0 && (FieldOf<Arguments>::kSpan && ...);
    };

    /// \brief Represents the parameters of a callback taking the entity ahead of its fields.
    ///
    /// \tparam First The entity.
    /// \tparam Rest  The fields, in order.
    template<typename First, typename... Rest>
        requires IsAnyOf<StripAll<First>, Entity>
    struct Parameters<First, Rest...> : Parameters<Rest...>
    {
        /// `true` when the callback takes the entity ahead of its fields.
        static constexpr Bool kEntity = true;
    };

    /// \brief Represents the fields a callback asks a walk for, as the types of its parameters spell them.
    ///
    /// \tparam Callable The callback, a function object.
    template<typename Callable>
    struct Plan : Plan<decltype(& StripAll<Callable>::operator())>
    {
    };

    /// \brief Represents the fields a free function asks a walk for.
    template<typename Return, typename... Arguments>
    struct Plan<Return (Arguments...)> : Parameters<Arguments...>
    {
    };

    /// \brief Represents the fields a pointer to a free function asks a walk for.
    template<typename Return, typename... Arguments>
    struct Plan<Return (*)(Arguments...)> : Parameters<Arguments...>
    {
    };

    /// \brief Represents the fields a mutable function object asks a walk for.
    template<typename Class, typename Return, typename... Arguments>
    struct Plan<Return (Class::*)(Arguments...)> : Parameters<Arguments...>
    {
    };

    /// \brief Represents the fields an immutable function object asks a walk for.
    template<typename Class, typename Return, typename... Arguments>
    struct Plan<Return (Class::*)(Arguments...) const> : Parameters<Arguments...>
    {
    };
}