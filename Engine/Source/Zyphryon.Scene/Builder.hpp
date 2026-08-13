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

#include "Component.hpp"
#include "Timer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::DSL::_
{
    /// \brief Represents a relationship between two types, matched as a single query term.
    ///
    /// \tparam T0 The first type of the pair, used as the relationship.
    /// \tparam T1 The second type of the pair, used as the target and as the data the term carries.
    template<typename T0, typename T1>
    struct Pair
    {
        using First  = T0;
        using Second = T1;
    };

    /// \brief Concept that is satisfied when \p Type describes a relationship pair.
    template<typename Type>
    concept IsPair = requires { typename Type::First; typename Type::Second; };

    /// \brief Specifies how a term rewrites the mutability of the type it was declared with.
    enum class Mutability : UInt8
    {
        Immutable, ///< Force read-only access regardless of how the type was spelled.
        Mutable,   ///< Force read-write access regardless of how the type was spelled.
        Declared,  ///< Keep the mutability the type was spelled with.
    };

    /// \brief Represents a list of types.
    template<typename... Types>
    struct TypeList
    {
        template<typename... Others>
        constexpr auto operator+(TypeList<Others...>) const
        {
            return TypeList<Types..., Others...>{ };
        }
    };

    /// \brief Splits a declared term into the shape its spelling implies.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    template<typename Declared>
    struct Decompose
    {
        using Reduced   = StripPtr<StripRef<Declared>>;
        using Component = StripAll<Reduced>;

        /// True when the term was spelled as a pointer, which matches optionally.
        static constexpr Bool Optional  = IsPointer<StripRef<Declared>>;

        /// True when the term cannot be written through.
        static constexpr Bool Immutable = IsImmutable<Reduced>;

        /// True when the term was spelled as a mutable reference or pointer.
        static constexpr Bool Writable  = !Immutable && !IsAnyOf<Declared, Component>;
    };

    /// \brief Resolves the storage a term maps to and adds it to a builder.
    ///
    /// \tparam Component The component the term was declared with.
    template<typename Component>
    struct Resolve
    {
        using Type = Component;

        template<typename Constructor>
        ZY_INLINE static void Match(Ref<Constructor> Builder)
        {
            Builder.template with<Component>();
        }
    };

    /// \brief Resolves a pair term to its target, which is the half that carries the data.
    template<IsPair Component>
    struct Resolve<Component>
    {
        using Type = typename Component::Second;

        template<typename Constructor>
        ZY_INLINE static void Match(Ref<Constructor> Builder)
        {
            Builder.template with<typename Component::First, typename Component::Second>();
        }
    };

    /// \brief Rewrites a declared term so that it carries the requested mutability.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    /// \tparam Access   The mutability the enclosing term imposes.
    template<typename Declared, Mutability Access>
    struct Qualify
    {
        using Shape = Decompose<Declared>;
        using Value = Select<Access == Mutability::Declared,
                             StripPtr<StripRef<Declared>>,
                             Select<Access == Mutability::Immutable,
                                    const typename Shape::Component,
                                    typename Shape::Component>>;
        using Type  = Select<Shape::Optional, Pointer<Value>, Value>;
    };

    /// \brief Describes the value a term hands to the iteration callback.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    template<typename Declared>
    struct Field
    {
        using Shape     = Decompose<Declared>;
        using Value     = typename Resolve<typename Shape::Component>::Type;
        using Qualified = Select<Shape::Immutable, const Value, Value>;
        using Type      = Select<Shape::Optional, Pointer<Qualified>, Qualified>;

        /// True when the term has storage to hand to the callback, which tags do not.
        static constexpr Bool Carries = !IsEmpty<StripAll<Value>>;
    };

    /// \brief Adds an accessed term, in the pass whose field ordering it belongs to.
    ///
    /// \note Tags carry no storage, so they are emitted in the filter pass to keep field indices packed.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    /// \tparam Access   The read/write mode the term declares.
    /// \tparam Data     `true` while emitting terms that feed the callback, `false` otherwise.
    /// \param  Builder  The builder to add the term to.
    template<typename Declared, flecs::inout_kind_t Access, Bool Data, typename Constructor>
    ZY_INLINE void EmitAccess(Ref<Constructor> Builder)
    {
        if constexpr (Field<Declared>::Carries == Data)
        {
            Resolve<typename Decompose<Declared>::Component>::Match(Builder);

            Builder.inout(Field<Declared>::Carries ? Access : flecs::InOutNone);

            if constexpr (Decompose<Declared>::Optional)
            {
                Builder.optional();
            }
        }
    }

    /// \brief Adds a term that constrains matching without feeding the callback.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    /// \tparam Operator The operator applied to the term.
    /// \param  Builder  The builder to add the term to.
    template<typename Declared, flecs::oper_kind_t Operator, typename Constructor>
    ZY_INLINE void EmitFilter(Ref<Constructor> Builder)
    {
        Resolve<typename Decompose<Declared>::Component>::Match(Builder);

        Builder.inout(flecs::InOutNone).oper(Operator);
    }

    /// \brief Adds a chain of alternatives, which flecs collapses onto a single shared field.
    ///
    /// \tparam Declared The alternatives as they were spelled by the caller.
    /// \param  Builder  The builder to add the terms to.
    template<typename... Declared, typename Constructor>
    ZY_INLINE void EmitAlternatives(Ref<Constructor> Builder)
    {
        UInt Remaining = sizeof...(Declared);

        ((Resolve<typename Decompose<Declared>::Component>::Match(Builder),
          Builder.inout(flecs::InOutNone).oper(--Remaining ? flecs::Or : flecs::And)), ...);
    }

    /// \brief Adds a term that declares an access the callback performs outside the query.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    /// \tparam Access   The read/write mode declared for the scheduler.
    /// \param  Builder  The builder to add the term to.
    template<typename Declared, flecs::inout_kind_t Access, typename Constructor>
    ZY_INLINE void EmitStage(Ref<Constructor> Builder)
    {
        Resolve<typename Decompose<Declared>::Component>::Match(Builder);

        Builder.inout_stage(Access);
    }

    /// \brief Adds a term sourced from an ancestor rather than the matched entity.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    /// \tparam Data     `true` while emitting terms that feed the callback, `false` otherwise.
    /// \tparam Descend  `true` to traverse breadth-first, `false` to substitute the nearest ancestor.
    /// \param  Builder  The builder to add the term to.
    template<typename Declared, Bool Data, Bool Descend, typename Constructor>
    ZY_INLINE void EmitTraversal(Ref<Constructor> Builder)
    {
        if constexpr (Field<Declared>::Carries == Data)
        {
            Resolve<typename Decompose<Declared>::Component>::Match(Builder);

            if constexpr (Descend)
            {
                Builder.cascade();
            }
            else
            {
                Builder.up();
            }

            if constexpr (Field<Declared>::Carries)
            {
                Builder.inout(Decompose<Declared>::Immutable ? flecs::In : flecs::InOut);
            }
            else
            {
                Builder.inout(flecs::InOutNone);
            }

            if constexpr (Decompose<Declared>::Optional)
            {
                Builder.optional();
            }
        }
    }

    /// \brief Collects the callback values contributed by a set of declared terms.
    ///
    /// \tparam Access   The mutability the enclosing term imposes.
    /// \tparam Declared The terms as they were spelled by the caller.
    template<Mutability Access, typename... Declared>
    struct Collect
    {
        template<typename Type>
        using Entry = Select<Field<typename Qualify<Type, Access>::Type>::Carries,
                             TypeList<typename Field<typename Qualify<Type, Access>::Type>::Type>,
                             TypeList<>>;

        using Type = decltype((TypeList<> { } + ... + Entry<Declared> { }));
    };
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::DSL
{
    /// \brief Represents terms matched with read-only access and handed to the callback.
    ///
    /// \note A type spelled as a pointer matches optionally; a tag contributes nothing to the callback.
    ///
    /// \tparam Types The components, pairs or tags to match.
    template<typename... Types>
    struct In
    {
        /// The values this term contributes to the callback, in order.
        using Fields = typename _::Collect<_::Mutability::Immutable, Types...>::Type;

        /// The entity matched when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime term matching an entity resolved during execution.
        ///
        /// \param Expression The entity the term matches.
        ZY_INLINE constexpr explicit In(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            (_::EmitAccess<typename _::Qualify<Types, _::Mutability::Immutable>::Type, flecs::In, Data>(Builder), ...);
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout(flecs::In);
        }
    };

    /// \brief Represents terms matched with read-write access and handed to the callback.
    ///
    /// \note A type spelled as a pointer matches optionally; a tag contributes nothing to the callback.
    ///
    /// \tparam Types The components, pairs or tags to match.
    template<typename... Types>
    struct InOut
    {
        /// The values this term contributes to the callback, in order.
        using Fields = typename _::Collect<_::Mutability::Mutable, Types...>::Type;

        /// The entity matched when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime term matching an entity resolved during execution.
        ///
        /// \param Expression The entity the term matches.
        ZY_INLINE constexpr explicit InOut(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            (_::EmitAccess<typename _::Qualify<Types, _::Mutability::Mutable>::Type, flecs::InOut, Data>(Builder), ...);
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout(flecs::InOut);
        }
    };

    /// \brief Represents terms matched with write-only access and handed to the callback.
    ///
    /// \note The scheduler treats these terms as written but never read, which removes a sync dependency.
    ///
    /// \tparam Types The components, pairs or tags to match.
    template<typename... Types>
    struct Out
    {
        /// The values this term contributes to the callback, in order.
        using Fields = typename _::Collect<_::Mutability::Mutable, Types...>::Type;

        /// The entity matched when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime term matching an entity resolved during execution.
        ///
        /// \param Expression The entity the term matches.
        ZY_INLINE constexpr explicit Out(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            (_::EmitAccess<typename _::Qualify<Types, _::Mutability::Mutable>::Type, flecs::Out, Data>(Builder), ...);
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout(flecs::Out);
        }
    };

    /// \brief Represents terms an entity must carry, without handing them to the callback.
    ///
    /// \tparam Types The components, pairs or tags to match.
    template<typename... Types>
    struct With
    {
        /// The values this term contributes to the callback, in order.
        using Fields = _::TypeList<>;

        /// The entity matched when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime term matching an entity resolved during execution.
        ///
        /// \param Expression The entity the term matches.
        ZY_INLINE constexpr explicit With(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitFilter<Types, flecs::And>(Builder), ...);
            }
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout(flecs::InOutNone);
        }
    };

    /// \brief Represents terms an entity may carry, without handing them to the callback.
    ///
    /// \note Use `In<Ptr<Type>>` instead when the optional value has to reach the callback.
    ///
    /// \tparam Types The components, pairs or tags to match optionally.
    template<typename... Types>
    struct Opt
    {
        /// The values this term contributes to the callback, in order.
        using Fields = _::TypeList<>;

        /// The entity matched when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime term matching an entity resolved during execution.
        ///
        /// \param Expression The entity the term matches.
        ZY_INLINE constexpr explicit Opt(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitFilter<Types, flecs::Optional>(Builder), ...);
            }
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout(flecs::InOutNone).optional();
        }
    };

    /// \brief Represents terms an entity must not carry.
    ///
    /// \tparam Types The components, pairs or tags to exclude.
    template<typename... Types>
    struct Not
    {
        /// The values this term contributes to the callback, in order.
        using Fields = _::TypeList<>;

        /// The entity excluded when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime term excluding an entity resolved during execution.
        ///
        /// \param Expression The entity the term excludes.
        ZY_INLINE constexpr explicit Not(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitFilter<Types, flecs::Not>(Builder), ...);
            }
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout(flecs::InOutNone).oper(flecs::Not);
        }
    };

    /// \brief Represents a term satisfied when an entity carries any one of several types.
    ///
    /// \note The alternatives share a single field, so the term hands nothing to the callback.
    ///
    /// \tparam Types The alternatives to match, of which at least two are required.
    template<typename... Types>
    struct Or
    {
        /// The values this term contributes to the callback, in order.
        using Fields = _::TypeList<>;

        /// The entity matched by the first alternative when the term is supplied at runtime.
        Entity Left;

        /// The entity matched by the second alternative when the term is supplied at runtime.
        Entity Right;

        /// \brief Constructs a runtime term matching either of two entities resolved during execution.
        ///
        /// \param Left  The entity matched by the first alternative.
        /// \param Right The entity matched by the second alternative.
        ZY_INLINE constexpr Or(Entity Left, Entity Right)
            : Left  { Left },
              Right { Right }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            static_assert(sizeof...(Types) > 1, "Or requires at least two alternatives");

            if constexpr (!Data)
            {
                _::EmitAlternatives<Types...>(Builder);
            }
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Left.GetHandle()).inout(flecs::InOutNone).oper(flecs::Or);
            Builder.with(Right.GetHandle()).inout(flecs::InOutNone);
        }
    };

    /// \brief Represents a term sourced from the nearest ancestor that carries it.
    ///
    /// \tparam Type The component, pair or tag to match on an ancestor.
    template<typename Type>
    struct Up
    {
        /// The values this term contributes to the callback, in order.
        using Fields = typename _::Collect<_::Mutability::Declared, Type>::Type;

        /// The entity matched when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime term matching an entity resolved during execution.
        ///
        /// \param Expression The entity the term matches.
        ZY_INLINE constexpr explicit Up(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            _::EmitTraversal<Type, Data, false>(Builder);
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout(flecs::InOutNone).up();
        }
    };

    /// \brief Represents a term sourced from an ancestor, ordering results breadth-first.
    ///
    /// \tparam Type The component, pair or tag to match on an ancestor.
    template<typename Type>
    struct Cascade
    {
        /// The values this term contributes to the callback, in order.
        using Fields = typename _::Collect<_::Mutability::Declared, Type>::Type;

        /// The entity matched when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime term matching an entity resolved during execution.
        ///
        /// \param Expression The entity the term matches.
        ZY_INLINE constexpr explicit Cascade(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            _::EmitTraversal<Type, Data, true>(Builder);
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout(flecs::InOutNone).cascade();
        }
    };

    /// \brief Represents components the callback read outside the query, for sync placement only.
    ///
    /// \note These terms match nothing and hand nothing to the callback; declare them when using `Entity::Get`.
    ///
    /// \tparam Types The components, pairs or tags read outside the query.
    template<typename... Types>
    struct Read
    {
        /// The values this term contributes to the callback, in order.
        using Fields = _::TypeList<>;

        /// The entity declared when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime declaration for an entity resolved during execution.
        ///
        /// \param Expression The entity the callback read.
        ZY_INLINE constexpr explicit Read(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitStage<Types, flecs::In>(Builder), ...);
            }
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout_stage(flecs::In);
        }
    };

    /// \brief Represents components the callback writes outside the query, for sync placement only.
    ///
    /// \note These terms match nothing and hand nothing to the callback; declare them when using `Entity::Set`.
    ///
    /// \tparam Types The components, pairs or tags written outside the query.
    template<typename... Types>
    struct Write
    {
        /// The values this term contributes to the callback, in order.
        using Fields = _::TypeList<>;

        /// The entity declared when the term is supplied at runtime instead of at compile time.
        Entity Expression;

        /// \brief Constructs a runtime declaration for an entity resolved during execution.
        ///
        /// \param Expression The entity the callback writes.
        ZY_INLINE constexpr explicit Write(Entity Expression)
            : Expression { Expression }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitStage<Types, flecs::Out>(Builder), ...);
            }
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.with(Expression.GetHandle()).inout_stage(flecs::Out);
        }
    };

    /// \brief Represents an ordering applied to the results of a query.
    ///
    /// \tparam Type       The component the results are sorted by.
    /// \tparam Comparison The comparison used to order two values of \p Type.
    template<typename Type, auto Comparison>
    struct OrderBy
    {
        /// The values this term contributes to the callback, in order.
        using Fields = _::TypeList<>;

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            if constexpr (!Data)
            {
                Builder.template order_by<Type>(Comparison);
            }
        }
    };

    /// \brief Represents the minimum time that must elapse between two runs of a system.
    ///
    /// \tparam Seconds The interval in seconds.
    template<auto Seconds>
    struct Interval
    {
        /// The values this term contributes to the callback, in order.
        using Fields = _::TypeList<>;

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            if constexpr (!Data)
            {
                Builder.interval(Seconds);
            }
        }
    };

    /// \brief Represents the number of ticks that must elapse between two runs of a system.
    ///
    /// \tparam Ticks The number of ticks to skip between runs.
    template<auto Ticks>
    struct Rate
    {
        /// The values this term contributes to the callback, in order.
        using Fields = _::TypeList<>;

        /// The timer counted when the term is supplied at runtime instead of at compile time.
        Timer Source;

        /// \brief Constructs a runtime term counting the ticks of an explicit timer.
        ///
        /// \param Source The timer whose ticks are counted.
        ZY_INLINE constexpr explicit Rate(Timer Source)
            : Source { Source }
        {
        }

        template<Bool Data, typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            if constexpr (!Data)
            {
                Builder.rate(Ticks);
            }
        }

        template<typename Constructor>
        ZY_INLINE void ApplyRuntime(Ref<Constructor> Builder) const
        {
            Builder.rate(Source.GetHandle(), Ticks);
        }
    };

    /// \brief Represents a set of traits granted together, so several may travel as one term.
    ///
    /// \tparam Values The traits to grant.
    template<Trait... Values>
    struct Traits final
    {
    };

    /// \brief Grants a component the ability to be written to and read from an archive.
    inline constexpr Traits<Trait::Serializable> Serializable { };

    /// \brief Grants a component the ability to be supplied by an archetype.
    inline constexpr Traits<Trait::Inheritable>  Inheritable  { };

    /// \brief Grants a component the ability to be switched off without being removed.
    inline constexpr Traits<Trait::Toggleable>   Toggleable   { };

    /// \brief Grants a component storage that keeps it out of the table its entity is filed under.
    inline constexpr Traits<Trait::Sparse>       Sparse       { };

    /// \brief Grants a component the behaviour of a key-value association.
    inline constexpr Traits<Trait::Associative>  Associative  { };

    /// \brief Grants a component a single instance for the whole world, rather than one per entity.
    inline constexpr Traits<Trait::Singleton>    Singleton    { };

    /// \brief Grants a component the refusal to be extended or overridden.
    inline constexpr Traits<Trait::Final>        Final        { };

    /// \brief Grants a relationship the same meaning read in either direction.
    inline constexpr Traits<Trait::Symmetric>    Symmetric    { };

    /// \brief Grants a relationship at most one target per entity.
    inline constexpr Traits<Trait::Exclusive>    Exclusive    { };

    /// \brief Grants what an archetype-authored component needs: an instance inherits it, and it persists.
    inline constexpr Traits<Trait::Serializable, Trait::Inheritable> Authored { };

    /// \brief Represents the components a component brings along whenever it is attached.
    ///
    /// \tparam Types The components attached alongside.
    template<typename... Types>
    struct Implication final
    {
    };

    /// \brief Names the components a declaration brings along with it.
    ///
    /// \tparam Types The components attached alongside.
    template<typename... Types>
    inline constexpr Implication<Types...> Implies { };

    /// \brief Concept satisfied when \p Type states the name it should register under.
    template<typename Type>
    concept IsNamed = requires { { Type::kName } -> IsCastable<Text>; };

    /// \brief Describes one or more components that share the same traits and implications.
    ///
    /// \tparam Types The components being described.
    template<typename... Types>
    struct Declaration final
    {
        /// \brief Carries the terms describing the components, so registration can apply them in two passes.
        ///
        /// \tparam Parts The traits and implications the components share.
        template<typename... Parts>
        struct Description final
        {
            /// \brief The identifier the component registers under, or null to take it from the type.
            ConstPtr<Char> Name;

            /// \brief Registers every component named, so its name resolves before anything refers to it.
            ///
            /// \param World The world the components belong to.
            ZY_INLINE void Reserve(Ref<flecs::world> World) const
            {
                (ReserveEach<Types>(World), ...);
            }

            /// \brief Applies every term to every component described.
            ///
            /// \param World The world the components belong to.
            ZY_INLINE void Apply(Ref<flecs::world> World) const
            {
                (ApplyEach<Types>(World), ...);
            }

        private:

            /// \brief Registers one component under the name it should answer to.
            ///
            /// \param World The world the component belongs to.
            template<typename Type>
            ZY_INLINE void ReserveEach(Ref<flecs::world> World) const
            {
                if (Name)
                {
                    World.template component<Type>(Name);
                }
                else if constexpr (IsNamed<Type>)
                {
                    World.template component<Type>(Text(Type::kName).GetData());
                }
                else
                {
                    const Text Path = StrConvert(flecs::_::type_name<Type>());
                    const Text Name = StrAfterLast(Path, "::");

                    World.template component<Type>(Name.IsEmpty() ? Path.GetData() : Name.GetData());
                }
            }

            /// \brief Applies every term to one component.
            ///
            /// \param World The world the component belongs to.
            template<typename Type>
            ZY_INLINE static void ApplyEach(Ref<flecs::world> World)
            {
                (ApplyTerm<Type>(World, Parts { }), ...);
            }

            /// \brief Grants one set of traits to one component.
            ///
            /// \param World The world the component belongs to.
            template<typename Type, Trait... Values>
            ZY_INLINE static void ApplyTerm(Ref<flecs::world> World, Traits<Values...>)
            {
                Component<Type>(World.template component<Type>()).Grant(Values...);
            }

            /// \brief Attaches one set of implications to one component.
            ///
            /// \param World The world the component belongs to.
            template<typename Type, typename... Targets>
            ZY_INLINE static void ApplyTerm(Ref<flecs::world> World, Implication<Targets...>)
            {
                const Component<Type> Handle(World.template component<Type>());

                (Handle.template With<Targets>(), ...);
            }
        };
    };

    /// \brief Describes one or more components, to be handed to \ref Service::Register.
    ///
    /// \tparam Types       The components being described, which share everything stated about them.
    /// \param  Description The traits and implications they share, in any order.
    /// \return The description, which registration applies in two passes.
    template<typename... Types, typename... Parts> requires (!(IsCastable<Parts, ConstPtr<Char>> || ...))
    ZY_INLINE constexpr auto Declare(Parts... Description)
    {
        return typename Declaration<Types...>::template Description<Parts...> { };
    }

    /// \brief Describes one component under a name of its own, rather than the one its type carries.
    ///
    /// \tparam Types       The component being described.
    /// \param  Name        The identifier the component registers under.
    /// \param  Description The traits and implications it carries, in any order.
    /// \return The description, which registration applies in two passes.
    template<typename... Types, typename... Parts>
    ZY_INLINE constexpr auto Declare(ConstPtr<Char> Name, Parts... Description)
    {
        static_assert(sizeof...(Types) == 1, "Only one component can be declared under a name");

        return typename Declaration<Types...>::template Description<Parts...> { Name };
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::DSL::_
{
    /// \brief Combines the callback values contributed by every expression of a description.
    ///
    /// \tparam Expressions The expressions to analyze.
    template<typename... Expressions>
    struct ExtractTypes
    {
        using Type = decltype((TypeList<>{ } + ... + typename Expressions::Fields{ }));
    };

    /// \brief Extracts the parameter list of a callable.
    ///
    /// \tparam Callable The callable to analyze.
    template<typename Callable>
    struct SignatureOf : SignatureOf<decltype(& StripAll<Callable>::operator())>
    {
    };

    /// \brief Extracts the parameter list of a free function.
    template<typename Return, typename... Arguments>
    struct SignatureOf<Return (*)(Arguments...)>
    {
        using Type = TypeList<Arguments...>;
    };

    /// \brief Extracts the parameter list of a mutable function object.
    template<typename Class, typename Return, typename... Arguments>
    struct SignatureOf<Return (Class::*)(Arguments...)>
    {
        using Type = TypeList<Arguments...>;
    };

    /// \brief Extracts the parameter list of an immutable function object.
    template<typename Class, typename Return, typename... Arguments>
    struct SignatureOf<Return (Class::*)(Arguments...) const>
    {
        using Type = TypeList<Arguments...>;
    };

    /// \brief Removes the leading iteration context a callback may accept before its components.
    ///
    /// \tparam List The parameter list to trim.
    template<typename List>
    struct StripContext
    {
        using Type = List;
    };

    /// \brief Removes a leading entity parameter, or keeps the list when the callback declares none.
    template<typename First, typename... Rest>
    struct StripContext<TypeList<First, Rest...>>
    {
        using Type = Select<IsAnyOf<StripAll<First>, Entity, flecs::entity>, TypeList<Rest...>, TypeList<First, Rest...>>;
    };

    /// \brief Removes a leading iterator and row pair.
    template<typename Row, typename... Rest>
    struct StripContext<TypeList<Ref<flecs::iter>, Row, Rest...>>
    {
        using Type = TypeList<Rest...>;
    };

    /// \brief Derives query terms from the parameters a callback declares.
    ///
    /// \tparam List The trimmed parameter list of the callback.
    template<typename List>
    struct Infer;

    /// \brief Derives one term per callback parameter, preserving their order.
    template<typename... Arguments>
    struct Infer<TypeList<Arguments...>>
    {
        using Fields = TypeList<StripRef<Arguments>...>;

        template<typename Constructor>
        ZY_INLINE static void Apply(Ref<Constructor> Builder)
        {
            (Emit<Arguments>(Builder), ...);
        }

    private:

        template<typename Argument, typename Constructor>
        ZY_INLINE static void Emit(Ref<Constructor> Builder)
        {
            constexpr flecs::inout_kind_t Access = Decompose<Argument>::Writable ? flecs::InOut : flecs::In;

            EmitAccess<Argument, Access, Field<Argument>::Carries>(Builder);
        }
    };

    /// \brief Applies every expression of a description to a builder and yields its callback values.
    ///
    /// Terms that feed the callback are emitted before every other term, because a callback receives
    /// fields by position and each term consumes a field index whether or not it carries storage.
    /// When no expression feeds the callback, the terms are derived from the callback's own parameters.
    ///
    /// \tparam Constructor       The builder being described.
    /// \tparam FEach             The callback the description is built for, or `void` when there is none.
    /// \tparam CompileExpression The expressions known at compile time.
    /// \param  Builder           The builder to describe.
    /// \param  Expressions       The expressions resolved at runtime.
    /// \return The list of values the callback receives, in field order.
    template<typename Constructor, typename FEach, typename... CompileExpression, typename... RuntimeExpression>
    ZY_INLINE auto Build(Ref<Constructor> Builder, AnyRef<RuntimeExpression>... Expressions)
    {
        using Declared = typename ExtractTypes<CompileExpression...>::Type;

        if constexpr (!IsVoid<FEach> && IsAnyOf<Declared, TypeList<>>)
        {
            using Signature = typename StripContext<typename SignatureOf<StripAll<FEach>>::Type>::Type;

            Infer<Signature>::Apply(Builder);

            (CompileExpression::template Apply<false>(Builder), ...);
            (Expressions.ApplyRuntime(Builder), ...);

            return typename Infer<Signature>::Fields { };
        }
        else
        {
            (CompileExpression::template Apply<true>(Builder), ...);
            (CompileExpression::template Apply<false>(Builder), ...);
            (Expressions.ApplyRuntime(Builder), ...);

            return Declared { };
        }
    }

    /// \brief Factory for creating query runner functions based on a list of component types and an iteration function.
    template<typename TypeList, typename FEach>
    struct RunnerFactory;

    /// \brief Factory specialization for creating a runner function that invokes a callback for each query result.
    template<typename... Types, typename FEach>
    struct RunnerFactory<TypeList<Types...>, FEach>
    {
        ZY_INLINE static auto Make(AnyRef<FEach> Each)
        {
            return [Each = Move(Each)](Ref<flecs::iter> Iterator)
            {
                while (Iterator.next())
                {
                    flecs::_::each_delegate<FEach, Types...>(Each).invoke(const_cast<Ptr<ecs_iter_t>>(Iterator.c_ptr()));
                }
            };
        }
    };

    /// \brief Factory specialization for creating a runner function that invokes callbacks for the beginning, each iteration, and the end of a query execution.
    template<typename TypeList, typename FBegin, typename FEach, typename FEnd>
    struct RunnerFactoryLifecycle;

    /// \brief Factory specialization for creating a runner function that invokes callbacks for the beginning, each iteration, and the end of a query execution.
    template<typename... Types, typename FBegin, typename FEach, typename FEnd>
    struct RunnerFactoryLifecycle<TypeList<Types...>, FBegin, FEach, FEnd>
    {
        ZY_INLINE static auto Make(AnyRef<FBegin> Begin, AnyRef<FEach> Each, AnyRef<FEnd> End)
        {
            return [Begin = Move(Begin), Each = Move(Each), End = Move(End)](Ref<flecs::iter> Iterator)
            {
                Begin();
                while (Iterator.next())
                {
                    flecs::_::each_delegate<FEach, Types...>(Each)
                        .invoke(const_cast<Ptr<ecs_iter_t>>(Iterator.c_ptr()));
                }
                End();
            };
        }
    };
}