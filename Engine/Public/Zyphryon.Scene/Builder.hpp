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

#include "Entity.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::DSL
{
    /// \brief Resolves a query term to its underlying component type.
    template<class Term>
    using Component = UnPtr<UnRef<Term>>;

    /// \brief Represents a pair relationship between two types.
    ///
    /// Pairs are commonly used to express relationships such as (Parent, Child) or (Tag, Component).
    ///
    /// \tparam T0 The first type of the pair.
    /// \tparam T1 The second type of the pair.
    template<typename T0, typename T1>
    struct Pair
    {
        using First  = T0;
        using Second = T1;
    };

    /// \brief Encapsulates a query term requiring the presence of one or more components.
    template<typename Term>
    struct With
    {
        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr decltype(auto) ApplyRuntime(Ref<Constructor> Builder, flecs::entity Entity)
        {
            return Builder.with(Entity);
        }

        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr decltype(auto) Apply(Ref<Constructor> Builder)
        {
            if constexpr (requires { typename Term::First; typename Term::Second; })
            {
                return Builder.template with<typename Term::First, typename Term::Second>();
            }
            else
            {
                if constexpr (IsPointer<Term>)
                {
                    return Builder.template with<Component<Term>>().optional();
                }
                else
                {
                    return Builder.template with<Term>();
                }
            }
        }
    };

    /// \brief Marks one or more components as input dependencies for a query.
    template<typename... Types>
    struct In
    {
        Entity Expression;

        ZYPHRYON_INLINE constexpr explicit In(Entity Expression)
            : Expression { Expression }
        {
        }

        template<typename Constructor>
        ZYPHRYON_INLINE decltype(auto) ApplyRuntime(Ref<Constructor> Builder)
        {
            With<Entity>::ApplyRuntime(Builder, Expression.GetHandle());
            return Builder;
        }

        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr auto Apply(Ref<Constructor> Builder)
        {
            (With<Types>::Apply(Builder), ...);
            return Builder;
        }
    };

    /// \brief Marks a cascading input dependency for a query.
    template<typename Type>
    struct InCascade
    {
        Entity Expression;

        ZYPHRYON_INLINE constexpr explicit InCascade(Entity Expression)
            : Expression { Expression }
        {
        }

        template<typename Constructor>
        ZYPHRYON_INLINE decltype(auto) ApplyRuntime(Ref<Constructor> Builder)
        {
            With<Entity>::ApplyRuntime(Builder, Expression.GetHandle()).cascade();
            return Builder;
        }

        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr auto Apply(Ref<Constructor> Builder)
        {
            With<Type>::Apply(Builder).cascade();
            return Builder;
        }
    };

    /// \brief Marks one or more components as outputs of a query.
    template<typename... Types>
    struct Out
    {
        Entity Expression;

        ZYPHRYON_INLINE constexpr explicit Out(Entity Expression)
            : Expression { Expression }
        {
        }

        template<typename Constructor>
        ZYPHRYON_INLINE decltype(auto) ApplyRuntime(Ref<Constructor> Builder)
        {
            With<Entity>::ApplyRuntime(Builder, Expression.GetHandle()).inout(flecs::Out);
            return Builder;
        }

        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr auto Apply(Ref<Constructor> Builder)
        {
            (With<Types>::Apply(Builder).inout(flecs::Out), ...);
            return Builder;
        }
    };

    /// \brief Excludes one or more components from a query.
    template<typename... Types>
    struct Not
    {
        Entity Expression;

        ZYPHRYON_INLINE constexpr explicit Not(Entity Expression)
            : Expression { Expression }
        {
        }

        template<typename Constructor>
        ZYPHRYON_INLINE decltype(auto) ApplyRuntime(Ref<Constructor> Builder)
        {
            With<Entity>::ApplyRuntime(Builder, Expression.GetHandle()).oper(flecs::Not);
            return Builder;
        }

        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr auto Apply(Ref<Constructor> Builder)
        {
            (With<Types>::Apply(Builder).oper(flecs::Not), ...);
            return Builder;
        }
    };
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::DSL::_
{
    /// \brief Maps a DSL term to its resolved component type.
    template<typename Term, typename = void>
    struct MapTypeFromTerm
    {
        using Type = Component<Term>;
    };

    /// \brief Maps a pair term to the component type of its second element.
    template<typename Term>
    struct MapTypeFromTerm<Term, std::void_t<typename Term::Second>>
    {
        using Type = Component<typename Term::Second>;
    };

    /// \brief Convenience alias for the resolved component type of a DSL term.
    template<typename Term>
    using MapType = MapTypeFromTerm<Term>::Type;

    /// \brief Extracts component types from a list of DSL terms and removes empty marker types.
    ///
    /// \tparam Terms The list of DSL terms to process.
    template<typename... Terms>
    struct ExtractAndFilterTypes
    {
        using ExtractedTypes = Tuple<MapType<Terms>...>;

        template<typename Tuple, typename = void>
        struct FilterEmpty;

        template<typename... Types>
        struct FilterEmpty<Tuple<Types...>, std::void_t<std::enable_if_t<true>>>
        {
            using Type = decltype([]<typename... T0>(Tuple<T0...>)
            {
                return std::tuple_cat(Switch<!std::is_empty_v<T0>, Tuple<T0>, Tuple<> >{}...);
            }(Tuple<Types...>{}));
        };

        using Type = typename FilterEmpty<ExtractedTypes>::Type;
    };

    /// \brief Extracts component types from a DSL expression.
    ///
    /// \tparam Expression The query expression to analyze.
    template<typename Expression>
    struct ExtractTypesFromExpression;

    /// \brief Extracts component types from an \c In expression.
    template<typename... Types>
    struct ExtractTypesFromExpression<In<Types...>>
    {
        using Type = typename ExtractAndFilterTypes<Types...>::Type;
    };

    /// \brief Extracts component types from an \c InCascade expression.
    template<typename Types>
    struct ExtractTypesFromExpression<InCascade<Types>>
    {
        using Type = typename ExtractAndFilterTypes<Types>::Type;
    };

    /// \brief Extracts component types from an \c Out expression.
    template<typename... Types>
    struct ExtractTypesFromExpression<Out<Types...>>
    {
        using Type = typename ExtractAndFilterTypes<Types...>::Type;
    };

    /// \brief Extracts component types from an \c InCascade expression.
    template<typename... Types>
    struct ExtractTypesFromExpression<Not<Types...>>
    {
        using Type = Tuple<>;
    };

    /// \brief Combines the extracted component types from multiple DSL expressions into a tuple.
    ///
    /// \tparam Expressions The list of expressions to analyze.
    template<typename... Expressions>
    struct ExtractTypes
    {
        using Type = decltype(std::tuple_cat(std::declval<typename ExtractTypesFromExpression<Expressions>::Type>()...));
    };

    /// \brief Transforms a tuple of types into a new template instantiation.
    template <typename Tuple, template <typename...> class Target>
    struct Extract;

    /// \brief Specialization of Extract for tuples of types.
    template <typename... Types, template <typename...> class Target>
    struct Extract<Tuple<Types...>, Target>
    {
        using Type = Target<Types...>;
    };
}