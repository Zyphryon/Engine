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
#include "Iterator.hpp"
#include "Timer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::DSL::_
{
    /// \brief Holds the callable a system or observer runs, so flecs can invoke and release it by plain pointer.
    ///
    /// \tparam Callable The callable invoked once per iteration result.
    template<typename Callable>
    class Runner final
    {
    public:

        /// \brief Constructs a runner taking ownership of a callable.
        ///
        /// \param Callback The callable the system or observer runs.
        ZY_INLINE explicit Runner(AnyRef<Callable> Callback)
            : mCallback { Move(Callback) }
        {
        }

        /// \brief Invokes the callable the result was created for.
        ///
        /// \param Handle The result flecs hands to the system or observer.
        ZY_INLINE static void OnInvoke(Ptr<ecs_iter_t> Handle)
        {
            ConstRef<Runner> Self = (* static_cast<ConstPtr<Runner>>(Handle->run_ctx));

            const Iterator Cursor(Handle);
            Cursor.Reset();

            Self.mCallback(Cursor);
        }

        /// \brief Releases the runner once flecs is done with the system or observer that owns it.
        ///
        /// \param Context The runner to release.
        ZY_INLINE static void OnRelease(Ptr<void> Context)
        {
            delete static_cast<Ptr<Runner>>(Context);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Callable mCallback;
    };

    /// \brief Represents the description a query, system, observer or pipeline is assembled into.
    ///
    /// Terms accumulate one at a time, and every modifier applies to the term that was added last.
    class Descriptor final
    {
    public:

        /// \brief Constructs an empty description bound to a world.
        ///
        /// \param World The world the description is built against.
        ZY_INLINE explicit Descriptor(Ptr<ecs_world_t> World)
            : mWorld { World }
        {
        }

        /// \brief Adds a term matching a component type.
        ///
        /// \return This description, allowing for method chaining.
        template<typename Component>
        ZY_INLINE Ref<Descriptor> With()
        {
            return With(Scene::_::Identify<Component>());
        }

        /// \brief Adds a term matching a relation pair formed by two component types.
        ///
        /// \return This description, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE Ref<Descriptor> With()
        {
            return With(Scene::_::Identify<Relation>(), Scene::_::Identify<Component>());
        }

        /// \brief Adds a term matching an identifier resolved at runtime.
        ///
        /// \param Identifier The identifier the term matches.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> With(ecs_id_t Identifier)
        {
            Ref<ecs_term_t> Term = Advance();

            // A pair arrives with its flags already set, while a plain identifier names the first element.
            if (Identifier & ECS_ID_FLAGS_MASK)
            {
                Term.id = Identifier;
            }
            else
            {
                Term.first.id = Identifier;
            }
            return (* this);
        }

        /// \brief Adds a term matching a relation pair formed by two identifiers resolved at runtime.
        ///
        /// \param Relation  The relation the term matches.
        /// \param Component The target the term matches.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> With(ecs_entity_t Relation, ecs_entity_t Component)
        {
            Ref<ecs_term_t> Term = Advance();
            Term.first.id  = Relation;
            Term.second.id = Component;
            return (* this);
        }

        /// \brief Declares the read/write access the last term performs on the data it matches.
        ///
        /// \param Access The access mode to declare.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> InOut(ecs_inout_kind_t Access)
        {
            Current().inout = static_cast<SInt16>(Access);
            return (* this);
        }

        /// \brief Declares an access the callback performs outside the query, for sync placement only.
        ///
        /// \param Access The access mode to declare.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> InOutStage(ecs_inout_kind_t Access)
        {
            Ref<ecs_term_t> Term = Current();
            Term.inout = static_cast<SInt16>(Access);

            // The term states an intent rather than a match, so it is detached from the entity being iterated.
            if (Term.oper != EcsNot)
            {
                Term.src.id = EcsIsEntity;
            }
            return (* this);
        }

        /// \brief Applies an operator to the last term.
        ///
        /// \param Operator The operator to apply.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Oper(ecs_oper_kind_t Operator)
        {
            Current().oper = static_cast<SInt16>(Operator);
            return (* this);
        }

        /// \brief Makes the last term match even when the entity does not carry it.
        ///
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Optional()
        {
            return Oper(EcsOptional);
        }

        /// \brief Sources the last term from the nearest ancestor that carries it.
        ///
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Up()
        {
            Current().src.id |= EcsUp;
            return (* this);
        }

        /// \brief Sources the last term from an ancestor, ordering results breadth-first.
        ///
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Cascade()
        {
            Up();
            Current().src.id |= EcsCascade;
            return (* this);
        }

        /// \brief Orders the results by the value of a component.
        ///
        /// \param Comparison The comparison used to order two values of \p Type.
        /// \return This description, allowing for method chaining.
        template<typename Type, typename Comparator>
        ZY_INLINE Ref<Descriptor> OrderBy(Comparator Comparison)
        {
            mQuery.order_by          = Scene::_::Identify<Type>();
            mQuery.order_by_callback = reinterpret_cast<ecs_order_by_action_t>(Comparison);
            return (* this);
        }

        /// \brief Sets the minimum time that must elapse between two runs of a system.
        ///
        /// \param Seconds The interval in seconds.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Interval(Real32 Seconds)
        {
            mInterval = Seconds;
            return (* this);
        }

        /// \brief Sets the number of ticks that must elapse between two runs of a system.
        ///
        /// \param Ticks The number of ticks to skip between runs.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Rate(SInt32 Ticks)
        {
            mRate = Ticks;
            return (* this);
        }

        /// \brief Sets the number of ticks of an explicit timer that must elapse between two runs of a system.
        ///
        /// \param Source The timer whose ticks are counted.
        /// \param Ticks  The number of ticks to skip between runs.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Rate(ecs_entity_t Source, SInt32 Ticks)
        {
            mSource = Source;
            mRate   = Ticks;
            return (* this);
        }

        /// \brief Sets the phase that determines when a system runs.
        ///
        /// \param Phase The phase entity the system belongs to.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Phase(ecs_entity_t Phase)
        {
            mPhase = Phase;
            return (* this);
        }

        /// \brief Sets the event that triggers an observer.
        ///
        /// \param Event The event entity the observer listens for.
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Event(ecs_entity_t Event)
        {
            mEvent = Event;
            return (* this);
        }

        /// \brief Runs the system outside the staged context, so its mutations take effect at once.
        ///
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Immediate()
        {
            mImmediate = true;
            return (* this);
        }

        /// \brief Spreads the system across the worker threads the world was given.
        ///
        /// \return This description, allowing for method chaining.
        ZY_INLINE Ref<Descriptor> Concurrent()
        {
            mConcurrent = true;
            return (* this);
        }

        /// \brief Builds the query this description assembled.
        ///
        /// \param Name  The optional name the query registers under.
        /// \param Cache The caching strategy to apply.
        /// \return The query, which the caller owns.
        ZY_INLINE Ptr<ecs_query_t> BuildQuery(Text Name, ecs_query_cache_kind_t Cache)
        {
            mQuery.cache_kind = Cache;
            mQuery.entity     = Reserve(Name);

            return ecs_query_init(mWorld, AddressOf(mQuery));
        }

        /// \brief Builds the system this description assembled.
        ///
        /// \param Name     The optional name the system registers under.
        /// \param Callback The callable invoked once per iteration result.
        /// \return The entity representing the system.
        template<typename Callable>
        ZY_INLINE ecs_entity_t BuildSystem(Text Name, AnyRef<Callable> Callback)
        {
            using Delegate = Runner<StripAll<Callable>>;

            ecs_system_desc_t Description { };
            Description.entity         = Reserve(Name);
            Description.query          = mQuery;
            Description.phase          = mPhase;
            Description.interval       = mInterval;
            Description.rate           = mRate;
            Description.tick_source    = mSource;
            Description.multi_threaded = mConcurrent;
            Description.immediate      = mImmediate;
            Description.run            = Delegate::OnInvoke;
            Description.run_ctx        = new Delegate(Forward<Callable>(Callback));
            Description.run_ctx_free   = Delegate::OnRelease;

            return ecs_system_init(mWorld, AddressOf(Description));
        }

        /// \brief Builds the observer this description assembled.
        ///
        /// \param Name     The optional name the observer registers under.
        /// \param Callback The callable invoked once per iteration result.
        /// \return The entity representing the observer.
        template<typename Callable>
        ZY_INLINE ecs_entity_t BuildObserver(Text Name, AnyRef<Callable> Callback)
        {
            using Delegate = Runner<StripAll<Callable>>;

            ecs_observer_desc_t Description { };
            Description.entity       = Reserve(Name);
            Description.query        = mQuery;
            Description.events[0]    = mEvent;
            Description.run          = Delegate::OnInvoke;
            Description.run_ctx      = new Delegate(Forward<Callable>(Callback));
            Description.run_ctx_free = Delegate::OnRelease;

            return ecs_observer_init(mWorld, AddressOf(Description));
        }

        /// \brief Builds the pipeline this description assembled.
        ///
        /// \return The entity representing the pipeline.
        ZY_INLINE ecs_entity_t BuildPipeline()
        {
            ecs_pipeline_desc_t Description { };
            Description.query = mQuery;

            return ecs_pipeline_init(mWorld, AddressOf(Description));
        }

    private:

        /// \brief Opens the next term and makes it the one every modifier applies to.
        ///
        /// \return The term that was opened.
        ZY_INLINE Ref<ecs_term_t> Advance()
        {
            ZY_ASSERT(mCount < FLECS_TERM_COUNT_MAX, "Exceeded the number of terms a query can carry");

            mTerm = AddressOf(mQuery.terms[mCount++]);
            return (* mTerm);
        }

        /// \brief Gets the term every modifier applies to.
        ///
        /// \return The term that was opened last.
        ZY_INLINE Ref<ecs_term_t> Current()
        {
            ZY_ASSERT(mTerm, "No term is open, so there is nothing to describe");

            return (* mTerm);
        }

        /// \brief Reserves the entity a named query, system or observer answers to.
        ///
        /// \param Name The name to register under, or empty to leave the result anonymous.
        /// \return The reserved entity, or `0` when there is no name to register.
        ZY_INLINE ecs_entity_t Reserve(Text Name) const
        {
            if (Name.IsEmpty())
            {
                return 0;
            }

            ecs_entity_desc_t Description { };
            Description.name     = Name.GetData();
            Description.sep      = "::";
            Description.root_sep = "::";

            return ecs_entity_init(mWorld, AddressOf(Description));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<ecs_world_t> mWorld;
        ecs_query_desc_t mQuery      { };
        Ptr<ecs_term_t>  mTerm       { nullptr };
        SInt8            mCount      { 0 };
        ecs_entity_t     mPhase      { 0 };
        ecs_entity_t     mEvent      { 0 };
        ecs_entity_t     mSource     { 0 };
        Real32           mInterval   { 0.0f };
        SInt32           mRate       { 0 };
        Bool             mImmediate  { false };
        Bool             mConcurrent { false };
    };
}

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

    /// \brief Reduces a callback parameter to the term it stands for.
    ///
    /// A parameter spelled as a view asks for the whole batch at once, and names the same term a parameter
    /// spelled as a single value would.
    ///
    /// \tparam Declared The parameter as it was spelled by the caller.
    template<typename Declared>
    struct Unbatch
    {
        using Type = Declared;
    };

    /// \brief Reduces a view parameter to the element it is a view over.
    template<typename Declared>
    struct Unbatch<Span<Declared>>
    {
        using Type = Declared;
    };

    /// \brief Resolves the storage a term maps to and adds it to a description.
    ///
    /// \tparam Component The component the term was declared with.
    template<typename Component>
    struct Resolve
    {
        using Type = Component;

        ZY_INLINE static void Match(Ref<Descriptor> Builder)
        {
            Builder.template With<Component>();
        }
    };

    /// \brief Resolves a pair term to its target, which is the half that carries the data.
    template<IsPair Component>
    struct Resolve<Component>
    {
        using Type = typename Component::Second;

        ZY_INLINE static void Match(Ref<Descriptor> Builder)
        {
            Builder.template With<typename Component::First, typename Component::Second>();
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
    /// \param  Builder  The description to add the term to.
    template<typename Declared, ecs_inout_kind_t Access, Bool Data>
    ZY_INLINE void EmitAccess(Ref<Descriptor> Builder)
    {
        if constexpr (Field<Declared>::Carries == Data)
        {
            Resolve<typename Decompose<Declared>::Component>::Match(Builder);

            Builder.InOut(Field<Declared>::Carries ? Access : EcsInOutNone);

            if constexpr (Decompose<Declared>::Optional)
            {
                Builder.Optional();
            }
        }
    }

    /// \brief Adds a term that constrains matching without feeding the callback.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    /// \tparam Operator The operator applied to the term.
    /// \param  Builder  The description to add the term to.
    template<typename Declared, ecs_oper_kind_t Operator>
    ZY_INLINE void EmitFilter(Ref<Descriptor> Builder)
    {
        Resolve<typename Decompose<Declared>::Component>::Match(Builder);

        Builder.InOut(EcsInOutNone).Oper(Operator);
    }

    /// \brief Adds a chain of alternatives, which flecs collapses onto a single shared field.
    ///
    /// \tparam Declared The alternatives as they were spelled by the caller.
    /// \param  Builder  The description to add the terms to.
    template<typename... Declared>
    ZY_INLINE void EmitAlternatives(Ref<Descriptor> Builder)
    {
        UInt Remaining = sizeof...(Declared);

        ((Resolve<typename Decompose<Declared>::Component>::Match(Builder),
          Builder.InOut(EcsInOutNone).Oper(--Remaining ? EcsOr : EcsAnd)), ...);
    }

    /// \brief Adds a term that declares an access the callback performs outside the query.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    /// \tparam Access   The read/write mode declared for the scheduler.
    /// \param  Builder  The description to add the term to.
    template<typename Declared, ecs_inout_kind_t Access>
    ZY_INLINE void EmitStage(Ref<Descriptor> Builder)
    {
        Resolve<typename Decompose<Declared>::Component>::Match(Builder);

        Builder.InOutStage(Access);
    }

    /// \brief Adds a term sourced from an ancestor rather than the matched entity.
    ///
    /// \tparam Declared The term as it was spelled by the caller.
    /// \tparam Data     `true` while emitting terms that feed the callback, `false` otherwise.
    /// \tparam Descend  `true` to traverse breadth-first, `false` to substitute the nearest ancestor.
    /// \param  Builder  The description to add the term to.
    template<typename Declared, Bool Data, Bool Descend>
    ZY_INLINE void EmitTraversal(Ref<Descriptor> Builder)
    {
        if constexpr (Field<Declared>::Carries == Data)
        {
            Resolve<typename Decompose<Declared>::Component>::Match(Builder);

            if constexpr (Descend)
            {
                Builder.Cascade();
            }
            else
            {
                Builder.Up();
            }

            if constexpr (Field<Declared>::Carries)
            {
                Builder.InOut(Decompose<Declared>::Immutable ? EcsIn : EcsInOut);
            }
            else
            {
                Builder.InOut(EcsInOutNone);
            }

            if constexpr (Decompose<Declared>::Optional)
            {
                Builder.Optional();
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            (_::EmitAccess<typename _::Qualify<Types, _::Mutability::Immutable>::Type, EcsIn, Data>(Builder), ...);
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOut(EcsIn);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            (_::EmitAccess<typename _::Qualify<Types, _::Mutability::Mutable>::Type, EcsInOut, Data>(Builder), ...);
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOut(EcsInOut);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            (_::EmitAccess<typename _::Qualify<Types, _::Mutability::Mutable>::Type, EcsOut, Data>(Builder), ...);
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOut(EcsOut);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitFilter<Types, EcsAnd>(Builder), ...);
            }
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOut(EcsInOutNone);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitFilter<Types, EcsOptional>(Builder), ...);
            }
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOut(EcsInOutNone).Optional();
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitFilter<Types, EcsNot>(Builder), ...);
            }
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOut(EcsInOutNone).Oper(EcsNot);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            static_assert(sizeof...(Types) > 1, "Or requires at least two alternatives");

            if constexpr (!Data)
            {
                _::EmitAlternatives<Types...>(Builder);
            }
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Left.GetHandle()).InOut(EcsInOutNone).Oper(EcsOr);
            Builder.With(Right.GetHandle()).InOut(EcsInOutNone);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            _::EmitTraversal<Type, Data, false>(Builder);
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOut(EcsInOutNone).Up();
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            _::EmitTraversal<Type, Data, true>(Builder);
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOut(EcsInOutNone).Cascade();
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitStage<Types, EcsIn>(Builder), ...);
            }
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOutStage(EcsIn);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            if constexpr (!Data)
            {
                (_::EmitStage<Types, EcsOut>(Builder), ...);
            }
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.With(Expression.GetHandle()).InOutStage(EcsOut);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            if constexpr (!Data)
            {
                Builder.template OrderBy<Type>(Comparison);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            if constexpr (!Data)
            {
                Builder.Interval(Seconds);
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

        template<Bool Data>
        ZY_INLINE static void Apply(Ref<_::Descriptor> Builder)
        {
            if constexpr (!Data)
            {
                Builder.Rate(Ticks);
            }
        }

        ZY_INLINE void ApplyRuntime(Ref<_::Descriptor> Builder) const
        {
            Builder.Rate(Source.GetHandle(), Ticks);
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

    /// \brief Grants a component the refusal to travel to an instance spawned from an archetype.
    inline constexpr Traits<Trait::Local>        Local        { };

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
            ZY_INLINE void Reserve(Ptr<ecs_world_t> World) const
            {
                (ReserveEach<Types>(World), ...);
            }

            /// \brief Applies every term to every component described.
            ///
            /// \param World The world the components belong to.
            ZY_INLINE void Apply(Ptr<ecs_world_t> World) const
            {
                (ApplyEach<Types>(World), ...);
            }

        private:

            /// \brief Registers one component under the name it should answer to.
            ///
            /// \param World The world the component belongs to.
            template<typename Type>
            ZY_INLINE void ReserveEach(Ptr<ecs_world_t> World) const
            {
                if (Name)
                {
                    Scene::_::Register<Type>(World, StrConvert(Name));
                }
                else if constexpr (IsNamed<Type>)
                {
                    Scene::_::Register<Type>(World, Text(Type::kName));
                }
                else
                {
                    // An empty name asks the registry to take the one the compiler records for the type.
                    Scene::_::Register<Type>(World, Text::Empty());
                }
            }

            /// \brief Applies every term to one component.
            ///
            /// \param World The world the component belongs to.
            template<typename Type>
            ZY_INLINE static void ApplyEach(Ptr<ecs_world_t> World)
            {
                (ApplyTerm<Type>(World, Parts { }), ...);
            }

            /// \brief Grants one set of traits to one component.
            ///
            /// \param World The world the component belongs to.
            template<typename Type, Trait... Values>
            ZY_INLINE static void ApplyTerm(Ptr<ecs_world_t> World, Traits<Values...>)
            {
                Component<Type>(World, Scene::_::Identify<Type>()).Grant(Values...);
            }

            /// \brief Attaches one set of implications to one component.
            ///
            /// \param World The world the component belongs to.
            template<typename Type, typename... Targets>
            ZY_INLINE static void ApplyTerm(Ptr<ecs_world_t> World, Implication<Targets...>)
            {
                const Component<Type> Handle(World, Scene::_::Identify<Type>());

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
        using Type = Select<IsAnyOf<StripAll<First>, Entity>, TypeList<Rest...>, TypeList<First, Rest...>>;
    };

    /// \brief Removes a leading iterator and row pair.
    template<typename Row, typename... Rest>
    struct StripContext<TypeList<ConstRef<Iterator>, Row, Rest...>>
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
        using Fields = TypeList<typename Unbatch<StripRef<Arguments>>::Type...>;

        ZY_INLINE static void Apply(Ref<Descriptor> Builder)
        {
            (Emit<typename Unbatch<StripRef<Arguments>>::Type>(Builder), ...);
        }

    private:

        template<typename Argument>
        ZY_INLINE static void Emit(Ref<Descriptor> Builder)
        {
            constexpr ecs_inout_kind_t Access = Decompose<Argument>::Writable ? EcsInOut : EcsIn;

            EmitAccess<Argument, Access, Field<Argument>::Carries>(Builder);
        }
    };

    /// \brief Applies every expression of a description to a builder and yields its callback values.
    ///
    /// \tparam FEach             The callback the description is built for, or `void` when there is none.
    /// \tparam CompileExpression The expressions known at compile time.
    /// \param  Builder           The description to describe.
    /// \param  Expressions       The expressions resolved at runtime.
    /// \return The list of values the callback receives, in field order.
    template<typename FEach, typename... CompileExpression, typename... RuntimeExpression>
    ZY_INLINE auto Build(Ref<Descriptor> Builder, AnyRef<RuntimeExpression>... Expressions)
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

    /// \brief Holds where one term's data lives for the span of a single iteration result.
    struct Column final
    {
        /// The base address of the term's data, or null when the term matched nothing.
        Ptr<void> Data   = nullptr;

        /// The number of elements a row advances by, which is zero for a value the whole batch shares.
        SInt32    Stride = 0;

        /// The position of the term within the result, needed when its value is fetched one row at a time.
        SInt8     Index  = 0;

        /// True when the term's value lives outside the table and has to be fetched one row at a time.
        Bool      Sparse = false;
    };

    /// \brief Unpacks the fields of an iteration result and hands them to a callback, one entity at a time.
    ///
    /// \tparam List  The values the callback receives, in field order.
    /// \tparam FEach The callback invoked for each matching entity.
    template<typename List, typename FEach>
    struct Dispatcher;

    /// \brief Unpacks a known set of fields and hands them to a callback, one entity at a time.
    template<typename... Types, typename FEach>
    struct Dispatcher<TypeList<Types...>, FEach>
    {
        /// \brief Invokes the callback for every entity of the current result.
        ///
        /// \param Cursor The result being walked.
        /// \param Each   The callback invoked for each matching entity.
        ZY_INLINE static void Invoke(ConstRef<Iterator> Cursor, ConstRef<FEach> Each)
        {
            const Ptr<ecs_iter_t> Handle = Cursor.GetHandle();

            Column Columns[sizeof...(Types) + 1] { };
            Bind(Handle, Columns, MakeIntegerSequence<UInt, sizeof...(Types)> { });

            // A query matching no entity of its own still reports one result, whose fields all come from elsewhere.
            const SInt32 Count = (Handle->count == 0 && !Handle->table) ? 1 : Handle->count;

            // A callback spelled with views is handed the whole result at once, so it decides for itself what
            // holds for the batch and then runs a loop of its own with nothing left to test per row.
            if constexpr (IsBatched(MakeIntegerSequence<UInt, sizeof...(Types)> { }))
            {
                ZY_ASSERT(!Handle->row_fields,
                    "A batched callback cannot take a component that is stored outside the table");

                Sweep(Each, Columns, Count, MakeIntegerSequence<UInt, sizeof...(Types)> { });
            }
            else if (Handle->row_fields)
            {
                Walk(Cursor, Each, Columns, Count);
            }
            else
            {
                Select(Measure(Columns, MakeIntegerSequence<UInt, sizeof...(Types)> { }),
                    Cursor, Each, Columns, Count, MakeIntegerSequence<UInt, kPatterns> { });
            }
        }

    private:

        /// The value the term at a given position contributes to the callback, padded so an empty set still names one.
        ///
        /// A list inferred from a callback keeps the references the parameters were spelled with, which are
        /// dropped here so a field is described by its value type alone.
        template<UInt Index>
        using Value = StripRef<typename Identify<Index, Types..., Empty>::Type>;

        /// The view a term hands a batched callback, which is read-only whenever the term is.
        template<typename Type>
        using Batch = Select<IsImmutable<StripPtr<Type>>,
                             ConstSpan<StripAll<StripPtr<Type>>>,
                             Span<StripAll<StripPtr<Type>>>>;

        /// \brief Checks whether the callback takes the whole batch as views rather than one row at a time.
        ///
        /// \return `true` when the callback is spelled with a view per term.
        template<UInt... Indices>
        ZY_INLINE static consteval Bool IsBatched(IntegerSequence<UInt, Indices...>)
        {
            return requires (ConstRef<FEach> Callback) { Callback(Batch<Value<Indices>> { }...); };
        }

        /// \brief Hands the whole result to a batched callback, one view per term.
        ///
        /// \param Each    The callback invoked for the batch.
        /// \param Columns The slots describing where each term's data lives.
        /// \param Count   The number of rows the result holds.
        template<UInt... Indices>
        ZY_INLINE static void Sweep(ConstRef<FEach> Each, Ptr<Column> Columns, SInt32 Count, IntegerSequence<UInt, Indices...>)
        {
            Each(Wrap<Value<Indices>>(Columns[Indices], Count)...);
        }

        /// \brief Builds the view one term hands a batched callback.
        ///
        /// \note The length carries what the row loop would otherwise have to test: a term that matched
        ///       nothing is empty, one the batch shares holds a single element, and an owned one holds a row
        ///       per entity.
        ///
        /// \param Slot  The slot describing where the term's data lives.
        /// \param Count The number of rows the result holds.
        /// \return The view over the term's data.
        template<typename Type>
        ZY_INLINE static Batch<Type> Wrap(ConstRef<Column> Slot, SInt32 Count)
        {
            using Storage = StripAll<StripPtr<Type>>;

            const UInt Length = Slot.Data ? (Slot.Stride ? static_cast<UInt>(Count) : 1u) : 0u;

            return Batch<Type>(static_cast<Ptr<Storage>>(Slot.Data), Length);
        }

        /// True for each term spelled as optional, which are the only ones a result can be missing.
        static constexpr Bool kOptional[sizeof...(Types) + 1] { IsPointer<StripRef<Types>>..., false };

        /// \brief Gets the bit a term takes in the absence pattern, which only optional terms hold one of.
        ///
        /// \param Index The position of the term within the result.
        /// \return The bit the term is described by.
        static constexpr UInt BitOf(UInt Index)
        {
            UInt Bit = 0;

            for (UInt Position = 0; Position < Index; ++Position)
            {
                Bit += (kOptional[Position] ? 1 : 0);
            }
            return Bit;
        }

        /// \brief Checks whether a term is missing from the result an absence pattern describes.
        ///
        /// \param Index   The position of the term within the result.
        /// \param Pattern The absence pattern to read.
        /// \return `true` while the term matched nothing.
        static constexpr Bool IsAbsent(UInt Index, UInt Pattern)
        {
            return kOptional[Index] && ((Pattern >> BitOf(Index)) & 1u);
        }

        /// The number of terms whose absence a loop can be specialized on.
        static constexpr UInt kOptionals = BitOf(sizeof...(Types));

        /// The number of absence patterns a loop is written for, capped so a query with many optional terms
        /// does not multiply the code its callback compiles to. The unspecialized loop is correct for all of
        /// them, and only leaves the null checks to run per row.
        static constexpr UInt kPatterns  = (kOptionals <= 4 ? (1u << kOptionals) : 1u);

        /// \brief Holds the address one term is read at, and what a row advances it by.
        struct Track final
        {
            /// The address the term is read at, or null when the term matched nothing.
            Ptr<UInt8> Data;

            /// The number of bytes a row advances by, which is zero for a value the whole result shares.
            SInt32     Step;
        };

        /// The number of bytes one row of a term occupies, which a tag never advances by.
        template<typename Type>
        static constexpr SInt32 kWidth = IsEmpty<StripAll<StripPtr<Type>>> ? 0 : static_cast<SInt32>(sizeof(StripAll<StripPtr<Type>>));

        /// \brief Gets the absence pattern of the current result.
        ///
        /// \param Columns The slots describing where each term's data lives.
        /// \return The pattern, or zero when the result holds more optional terms than are specialized for.
        template<UInt... Indices>
        ZY_INLINE static UInt Measure(Ptr<Column> Columns, IntegerSequence<UInt, Indices...>)
        {
            UInt Pattern = 0;

            ((kOptional[Indices] && !Columns[Indices].Data ? (Pattern |= (1u << BitOf(Indices))) : 0u), ...);

            return (Pattern < kPatterns ? Pattern : 0u);
        }

        /// \brief Walks the result with the loop its absence pattern selects.
        ///
        /// \param Pattern The pattern the result matched.
        /// \param Cursor  The result being walked.
        /// \param Each    The callback invoked for each row.
        /// \param Columns The slots describing where each term's data lives.
        /// \param Count   The number of rows the result holds.
        template<UInt... Patterns>
        ZY_INLINE static void Select(UInt Pattern, ConstRef<Iterator> Cursor, ConstRef<FEach> Each,
            Ptr<Column> Columns, SInt32 Count, IntegerSequence<UInt, Patterns...>)
        {
            ((Pattern == Patterns
                ? Stream<Patterns>(Cursor, Each, Columns, Count, MakeIntegerSequence<UInt, sizeof...(Types)> { })
                : void()), ...);
        }

        /// \brief Walks every row of the current result, each term through a cursor of its own.
        ///
        /// \note A cursor advances by one row for a term the entity owns and by nothing for one the whole
        ///       result shares, so a row costs an add rather than a multiply and the loop carries no
        ///       dependency on the index it is at.
        ///
        /// \tparam Pattern The absence pattern the result matched.
        /// \param  Cursor  The result being walked.
        /// \param  Each    The callback invoked for each row.
        /// \param  Columns The slots describing where each term's data lives.
        /// \param  Count   The number of rows the result holds.
        template<UInt Pattern, UInt... Indices>
        ZY_INLINE static void Stream(ConstRef<Iterator> Cursor, ConstRef<FEach> Each, Ptr<Column> Columns,
            SInt32 Count, IntegerSequence<UInt, Indices...> Order)
        {
            Chase<Pattern>(Cursor, Each, Count, Order,
                Track { static_cast<Ptr<UInt8>>(Columns[Indices].Data),
                        Columns[Indices].Stride * kWidth<Value<Indices>> }...);
        }

        /// \brief Walks every row of the current result, each term through a cursor of its own.
        ///
        /// \note The cursors arrive as parameters rather than as a table so that each one is a local the loop
        ///       can hold in a register.
        ///
        /// \tparam Pattern The absence pattern the result matched.
        /// \param  Cursor  The result being walked.
        /// \param  Each    The callback invoked for each row.
        /// \param  Count   The number of rows the result holds.
        /// \param  Heads   The cursor of every term, in field order.
        template<UInt Pattern, UInt... Indices, typename... Cursors>
        ZY_INLINE static void Chase(ConstRef<Iterator> Cursor, ConstRef<FEach> Each, SInt32 Count,
            IntegerSequence<UInt, Indices...>, Cursors... Heads)
        {
            for (SInt32 Row = 0; Row < Count; ++Row)
            {
                Apply(Cursor, Each, Row, Fetch<Value<Indices>, IsAbsent(Indices, Pattern)>(Heads.Data)...);

                ((Heads.Data += Heads.Step), ...);
            }
        }

        /// \brief Reads the value one term contributes at the row its cursor sits on.
        ///
        /// \tparam Absent `true` when the term matched nothing, which the callback is handed null for.
        /// \param  Head   The cursor of the term.
        /// \return The value the term contributes at that row.
        template<typename Type, Bool Absent>
        ZY_INLINE static decltype(auto) Fetch(Ptr<UInt8> Head)
        {
            using Storage = StripAll<StripPtr<Type>>;

            if constexpr (IsEmpty<Storage>)
            {
                return Storage();
            }
            else if constexpr (Absent)
            {
                return static_cast<Type>(nullptr);
            }
            else if constexpr (IsPointer<Type>)
            {
                return static_cast<Type>(reinterpret_cast<Ptr<Storage>>(Head));
            }
            else
            {
                return static_cast<Ref<Type>>(* reinterpret_cast<Ptr<Storage>>(Head));
            }
        }

        /// \brief Resolves where every term's data lives for the current result.
        ///
        /// \param Handle  The result being walked.
        /// \param Columns The slots to fill, one per term.
        template<UInt... Indices>
        ZY_INLINE static void Bind(Ptr<ecs_iter_t> Handle, Ptr<Column> Columns, IntegerSequence<UInt, Indices...>)
        {
            (BindOne<Value<Indices>>(Handle, Columns[Indices], static_cast<SInt8>(Indices)), ...);
        }

        /// \brief Resolves where one term's data lives for the current result.
        ///
        /// \param Handle The result being walked.
        /// \param Slot   The slot to fill.
        /// \param Index  The position of the term within the result.
        template<typename Type>
        ZY_INLINE static void BindOne(Ptr<ecs_iter_t> Handle, Ref<Column> Slot, SInt8 Index)
        {
            using Storage = StripAll<StripPtr<Type>>;

            if constexpr (!IsEmpty<Storage>)
            {
                Slot.Index = Index;

                if (Handle->row_fields & (1ull << Index))
                {
                    Slot.Sparse = true;
                }
                else
                {
                    Slot.Data   = ecs_field_w_size(Handle, sizeof(Storage), Index);
                    Slot.Stride = (Slot.Data && !Handle->sources[Index]) ? 1 : 0;
                }
            }
        }

        /// \brief Walks every row of the current result, fetching each term one row at a time.
        ///
        /// \param Cursor The result being walked.
        /// \param Each   The callback invoked for each row.
        /// \param Slots  The slots describing where each term's data lives.
        /// \param Count  The number of rows the result holds.
        ZY_INLINE static void Walk(ConstRef<Iterator> Cursor, ConstRef<FEach> Each, Ptr<Column> Slots, SInt32 Count)
        {
            for (SInt32 Row = 0; Row < Count; ++Row)
            {
                Emit(Cursor, Each, Slots, Row, MakeIntegerSequence<UInt, sizeof...(Types)> { });
            }
        }

        /// \brief Reads every term at one row and hands the values to the callback.
        ///
        /// \param Cursor  The result being walked.
        /// \param Each    The callback invoked for the row.
        /// \param Columns The slots describing where each term's data lives.
        /// \param Row     The row to read.
        template<UInt... Indices>
        ZY_INLINE static void Emit(ConstRef<Iterator> Cursor, ConstRef<FEach> Each,
            Ptr<Column> Columns, SInt32 Row, IntegerSequence<UInt, Indices...>)
        {
            Apply(Cursor, Each, Row, Read<Value<Indices>>(Cursor.GetHandle(), Columns[Indices], Row)...);
        }

        /// \brief Reads one term at one row.
        ///
        /// \param Handle The result being walked.
        /// \param Slot   The slot describing where the term's data lives.
        /// \param Row    The row to read.
        /// \return The value the term contributes at that row.
        template<typename Type>
        ZY_INLINE static decltype(auto) Read(Ptr<ecs_iter_t> Handle, Ref<Column> Slot, SInt32 Row)
        {
            using Storage = StripAll<StripPtr<Type>>;

            if constexpr (IsEmpty<Storage>)
            {
                // A tag carries no storage, so the callback receives a value of its own.
                return Storage();
            }
            else
            {
                // A term outside the table holds one row at a time, which leaves its stride at zero.
                if (Slot.Sparse)
                {
                    Slot.Data = ecs_field_at_w_size(Handle, sizeof(Storage), Slot.Index, Row);
                }

                Ptr<Storage> Data = static_cast<Ptr<Storage>>(Slot.Data);
                Data += Row * Slot.Stride;

                if constexpr (IsPointer<Type>)
                {
                    return static_cast<Type>(Data);
                }
                else
                {
                    return static_cast<Ref<Type>>(* Data);
                }
            }
        }

        /// \brief Hands the values of one row to the callback, in the shape the callback declares.
        ///
        /// \param Cursor The result being walked.
        /// \param Each   The callback invoked for the row.
        /// \param Row    The row being handed over.
        /// \param Data   The values the terms contributed at that row.
        template<typename... Values>
        ZY_INLINE static void Apply(ConstRef<Iterator> Cursor, ConstRef<FEach> Each, SInt32 Row, AnyRef<Values>... Data)
        {
            if constexpr (requires { Each(Entity(), Forward<Values>(Data)...); })
            {
                const Ptr<ecs_iter_t> Handle = Cursor.GetHandle();

                ZY_ASSERT(Handle->entities, "Callback asks for an entity the query does not produce");

                ZY_INLINE_CALL Each(Entity(Handle->world, Handle->entities[Row]), Forward<Values>(Data)...);
            }
            else if constexpr (requires { Each(Cursor, UInt(), Forward<Values>(Data)...); })
            {
                ZY_INLINE_CALL Each(Cursor, static_cast<UInt>(Row), Forward<Values>(Data)...);
            }
            else
            {
                ZY_INLINE_CALL Each(Forward<Values>(Data)...);
            }
        }
    };

    /// \brief Factory for creating query runner functions based on a list of component types and an iteration function.
    template<typename List, typename FEach>
    struct RunnerFactory;

    /// \brief Factory specialization for creating a runner function that invokes a callback for each query result.
    template<typename... Types, typename FEach>
    struct RunnerFactory<TypeList<Types...>, FEach>
    {
        ZY_INLINE static auto Make(AnyRef<FEach> Each)
        {
            return [Each = Move(Each)](ConstRef<Iterator> Cursor)
            {
                while (Cursor.Next())
                {
                    Dispatcher<TypeList<Types...>, FEach>::Invoke(Cursor, Each);
                }
            };
        }
    };
}