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

#include "Timer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::DSL
{
    /// \brief Resolves a query term to its underlying component type.
    ///
    /// \tparam Term The query term to resolve.
    template<class Term>
    using Component = UnRef<Term>;

    /// \brief Represents a pair relationship between two types.
    ///
    /// Used to express relationships such as (Parent, Child) or (Tag, Component) in ECS queries.
    ///
    /// \tparam T0 The first type of the pair.
    /// \tparam T1 The second type of the pair.
    template<typename T0, typename T1>
    struct Pair
    {
        using First  = T0; ///< The first type in the pair.
        using Second = T1; ///< The second type in the pair.
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

            if constexpr (IsPointer<Term>)
            {
                using Unwrapped = UnPtr<Component<Term>>;

                if constexpr (requires { typename Unwrapped::First; typename Unwrapped::Second; })
                {
                    return Builder.template with<typename Unwrapped::First, typename Unwrapped::Second>().optional();
                }
                else
                {
                    return Builder.template with<Unwrapped>().optional();
                }
            }
            else
            {
                if constexpr (requires { typename Term::First; typename Term::Second; })
                {
                    return Builder.template with<typename Term::First, typename Term::Second>();
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
        Entity Expression; ///< The entity expression for the input.

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
    struct Cascade
    {
        Entity Expression; ///< The entity expression for the cascade.

        ZYPHRYON_INLINE constexpr explicit Cascade(Entity Expression)
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
        Entity Expression; ///< The entity expression for the output.

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
            (With<Types>::Apply(Builder).inout_stage(flecs::Out), ...);
            return Builder;
        }
    };

    /// \brief Excludes one or more components from a query.
    template<typename... Types>
    struct Not
    {
        Entity Expression; ///< The entity expression for the exclusion.

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

    /// \brief Orders query results based on a comparison function.
    template<typename Type, auto Comparison>
    struct OrderBy
    {
        template<typename Constructor>
        ZYPHRYON_INLINE decltype(auto) ApplyRuntime(Ref<Constructor> Builder)
        {
            return Builder; // no-op at runtime
        }

        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr auto Apply(Ref<Constructor> Builder)
        {
            return Builder.template order_by<Type>(Comparison);
        }
    };

    /// \brief Defines a time interval for query execution.
    template<auto Time>
    struct Interval
    {
        Timer Entity;

        ZYPHRYON_INLINE constexpr explicit Interval(Timer Entity)
            : Entity { Entity }
        {
        }

        template<typename Constructor>
        ZYPHRYON_INLINE decltype(auto) ApplyRuntime(Ref<Constructor> Builder)
        {
            return Builder.tick_source(Entity.GetHandle()).rate(Time);
        }

        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr auto Apply(Ref<Constructor> Builder)
        {
            return Builder.interval(Time);
        }
    };

    /// \brief Combines two query terms with a logical OR operation.
    template<typename First, typename Second>
    struct Or
    {
        Entity Left;
        Entity Right;

        ZYPHRYON_INLINE constexpr explicit Or(Entity Left, Entity Right)
            : Left  { Left },
              Right { Right }
        {
        }

        template<typename Constructor>
        ZYPHRYON_INLINE decltype(auto) ApplyRuntime(Ref<Constructor> Builder)
        {
            With<Entity>::ApplyRuntime(With<Entity>::ApplyRuntime(Builder, Left.GetHandle()).or_(), Right.GetHandle());
            return Builder;
        }

        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr auto Apply(Ref<Constructor> Builder)
        {
            With<Second>::Apply(With<First>::Apply(Builder).or_());
            return Builder;
        }
    };

    /// \brief Combines two query terms with a logical AND operation.
    template<typename First, typename Second>
    struct And
    {
        Entity Left;
        Entity Right;

        ZYPHRYON_INLINE constexpr explicit And(Entity Left, Entity Right)
            : Left  { Left },
              Right { Right }
        {
        }

        template<typename Constructor>
        ZYPHRYON_INLINE decltype(auto) ApplyRuntime(Ref<Constructor> Builder)
        {
            With<Entity>::ApplyRuntime(With<Entity>::ApplyRuntime(Builder, Left.GetHandle()).and_(), Right.GetHandle());
            return Builder;
        }

        template<typename Constructor>
        ZYPHRYON_INLINE static constexpr auto Apply(Ref<Constructor> Builder)
        {
            With<Second>::Apply(With<First>::Apply(Builder).and_());
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
        using Type = Component<Switch<IsMutable<Term>, typename Term::Second, const typename Term::Second>>;
    };

    /// \brief Maps a pair term to the component type of its second element.
    template<typename Term>
    struct MapTypeFromTerm<Ptr<Term>, std::void_t<typename Term::Second>>
    {
        using Type = std::add_pointer_t<Component<Switch<IsMutable<Term>, typename Term::Second, const typename Term::Second>>>;
    };

    /// \brief Convenience alias for the resolved component type of a DSL term.
    template<typename Term>
    using MapType = typename MapTypeFromTerm<Term>::Type;

    /// \brief Extracts component types from a list of DSL terms and removes empty marker types.
    ///
    /// \tparam Terms The list of DSL terms to process.
    template<typename... Terms>
    struct ExtractAndFilterTypes
    {
        using ExtractedTypes = Tuple<MapType<Terms>...>;

        template<class T>
        using Underlying = std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

        template<typename Tuple, typename = void>
        struct FilterEmpty;

        template<typename... Types>
        struct FilterEmpty<Tuple<Types...>, std::void_t<std::enable_if_t<true>>>
        {
            using Type = decltype([]<typename... T0>(Tuple<T0...>)
            {
                return std::tuple_cat(Switch<std::is_empty_v<Underlying<T0>>, Tuple<>, Tuple<T0>>{}...);
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

    /// \brief Extracts component types from an \c Cascade expression.
    template<typename Types>
    struct ExtractTypesFromExpression<Cascade<Types>>
    {
        using Type = typename ExtractAndFilterTypes<Types>::Type;
    };

    /// \brief Extracts component types from an \c Out expression.
    template<typename... Types>
    struct ExtractTypesFromExpression<Out<Types...>>
    {
        using Type = typename ExtractAndFilterTypes<Types...>::Type;
    };

    /// \brief Extracts component types from an \c Out expression.
    template<typename... Types>
    struct ExtractTypesFromExpression<Not<Types...>>
    {
        using Type = Tuple<>;
    };

    /// \brief Extracts component types from an \c OrderBy expression.
    template<typename Types, auto Comparison>
    struct ExtractTypesFromExpression<OrderBy<Types, Comparison>>
    {
        using Type = Tuple<>;
    };

    /// \brief Extracts component types from an \c Interval expression.
    template<auto Time>
    struct ExtractTypesFromExpression<Interval<Time>>
    {
        using Type = Tuple<>;
    };

    /// \brief Extracts component types from an \c Or expression.
    template<typename... Types>
    struct ExtractTypesFromExpression<Or<Types...>>
    {
        using Type = typename ExtractAndFilterTypes<Types...>::Type;
    };

    /// \brief Extracts component types from an \c And expression.
    template<typename... Types>
    struct ExtractTypesFromExpression<And<Types...>>
    {
        using Type = typename ExtractAndFilterTypes<Types...>::Type;
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

    /// \brief Convenience alias for transforming a tuple of types into a new template instantiation.
    ///
    /// \param Builder     The builder object to apply expressions to.
    /// \param Expressions The list of runtime expressions to apply.
    template<typename Type, typename... CompileExpression, typename... RuntimeExpression>
    void ApplyExpressions(Ref<Type> Builder, AnyRef<RuntimeExpression>... Expressions)
    {
        (CompileExpression::Apply(Builder), ...);
        (Expressions.ApplyRuntime(Builder), ...);
    }
}