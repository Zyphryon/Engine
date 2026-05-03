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
#include "Pipeline.hpp"
#include "Query.hpp"
#include "System.hpp"
#include "Tag.hpp"
#include "Timer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Provides high-level management of the scene subsystem.
    class Service final : public AbstractService<Service>
    {
    public:

        /// \brief Constructs the scene service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \copydoc Service::OnTick(Time)
        void OnTick(Time Time) override;

        /// \brief Sets the global time scale multiplier for the scene.
        ///
        /// \param Scale The new time scale multiplier. Must be non-negative.
        ZYPHRYON_INLINE void SetTimeScale(Real32 Scale)
        {
            LOG_ASSERT(Scale >= 0.0f, "Time scale cannot be negative");

            mMultiplier = Scale;
        }

        /// \brief Gets the current global time scale multiplier for the scene.
        ///
        /// \return The current time scale multiplier.
        ZYPHRYON_INLINE Real32 GetTimeScale() const
        {
            return mMultiplier;
        }

        /// \brief Compacts memory by removing unused entities and components.
        ///
        /// This helps reduce fragmentation and optimize memory usage.
        ZYPHRYON_INLINE void Shrink()
        {
            mWorld.shrink();
        }

        /// \brief Purges all instances of a specific component type from the world.
        ///
        /// This removes the specified component from every entity that currently has it.
        ///
        /// \tparam Component The component type to clear from all entities.
        template<typename Component>
        ZYPHRYON_INLINE void Purge()
        {
            mWorld.remove_all<Component>();
        }

        /// \brief Defers execution of a callback until the end of the current frame.
        ///
        /// If called during iteration, the callback is queued and executed safely once iteration ends.
        /// Otherwise, it executes immediately.
        ///
        /// \param Callback   The function to execute.
        /// \param Parameters The arguments to forward to the callback.
        template<typename Function, typename ...Arguments>
        ZYPHRYON_INLINE void Schedule(AnyRef<Function> Callback, AnyRef<Arguments>... Parameters)
        {
            const Bool Dirty = mWorld.defer_begin();

            Callback(Forward<Arguments>(Parameters)...);

            if (Dirty)
            {
                mWorld.defer_end();
            }
        }

        /// \brief Creates a new entity.
        ///
        /// \return The newly created entity object.
        ZYPHRYON_INLINE Entity CreateEntity()
        {
            const Entity Actor = Allocate<false>();
            Actor.Add(EcsFinal);
            return Actor;
        }

        /// \brief Creates a new archetype entity.
        ///
        /// \return The newly created archetype entity object.
        ZYPHRYON_INLINE Entity CreateArchetype()
        {
            const Entity Actor = Allocate<true>();
            Actor.Add(EcsPrefab);
            return Actor;
        }

        /// \brief Clones an existing archetype entity.
        ///
        /// \param Source The archetype entity to clone from.
        /// \return The newly created archetype entity object.
        ZYPHRYON_INLINE Entity CloneArchetype(Entity Source)
        {
            LOG_ASSERT(Source.IsArchetype(), "Source entity is not an archetype");

            const Entity Actor = Allocate<true>();
            Source.Clone(Actor);
            return Actor;
        }

        /// \brief Retrieves the entity representing the world itself.
        ///
        /// \return The entity representing the world.
        ZYPHRYON_INLINE Entity GetWorld() const
        {
            return Scene::Entity(flecs::World);
        }

        /// \brief Retrieves an entity by its unique identifier.
        ///
        /// \param ID The unique identifier of the entity.
        /// \return A valid entity if alive, otherwise an empty entity.
        ZYPHRYON_INLINE Entity GetEntity(UInt64 ID) const
        {
            const Entity::Handle Handle = mWorld.entity(ID);
            return Handle.is_valid() && mWorld.is_alive(Handle) ? Entity(Handle) : Entity();
        }

        /// \brief Retrieves an entity by its unique name.
        ///
        /// \param Name The unique name of the entity.
        /// \return A valid entity if alive, otherwise an empty entity.
        ZYPHRYON_INLINE Entity GetEntity(ConstStr8 Name) const
        {
            const Entity::Handle Handle = mWorld.lookup(Name.data());
            return Handle.is_valid() && mWorld.is_alive(Handle) ? Entity(Handle) : Entity();
        }

        /// \brief Retrieves the entity associated with a specific relation pair.
        ///
        /// \tparam Tag    The relation tag component.
        /// \tparam Target The relation target component.
        /// \return The entity representing the pair if it exists, otherwise an empty entity.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE Entity GetEntity() const
        {
            return Entity(mWorld.entity(mWorld.pair<Tag, Target>()));
        }

        /// \brief Retrieves or creates a component type in the world.
        ///
        /// \tparam Target The component type to retrieve or create.
        /// \param ID      The unique name of the component type. If empty, the type's default name is used.
        /// \return The component object representing the type.
        template<typename Target>
        ZYPHRYON_INLINE Component<Target> GetComponent(ConstStr8 ID = flecs::_::type_name<Target>()) const
        {
            return Component<Target>(mWorld.component<Target>(ID.data()));
        }

        /// \brief Creates a new phase entity with an optional dependency.
        ///
        /// \tparam Name      The tag type to use as the phase identifier.
        /// \tparam Type      Optional component type to add to the phase entity.
        /// \param Dependency An optional entity that this phase should depend on.
        /// \return The newly created phase entity.
        template<Symbol Name, typename Type = Phase>
        ZYPHRYON_INLINE Entity CreatePhase(Entity Dependency = Entity()) const
        {
            const auto Component = GetComponent<Tag<Name>>();

            if constexpr (IsEqual<Type, Phase>)
            {
                Component.Add(flecs::Phase);
            }
            else
            {
                if constexpr (!IsEqual<Type, Empty>)
                {
                    Component.template Add<Type>();
                }
            }

            if (Dependency.IsValid())
            {
                Component.DependsOn(Dependency);
            }
            return Component;
        }

        /// \brief Creates a new pipeline with specified compile-time and runtime expressions.
        ///
        /// \param Runtime Optional runtime expressions to refine the query.
        /// \return The constructed pipeline object.
        template<typename... CompileExpression, typename... RuntimeExpression>
        ZYPHRYON_INLINE Pipeline CreatePipeline(AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::pipeline_builder<> Builder = mWorld.pipeline().with(flecs::System);
            DSL::_::ApplyExpressions<decltype(Builder), CompileExpression...>(Builder, Runtime...);

            return Pipeline(Builder.build());
        }

        /// \brief Creates a timer as a tick source in the world.
        ///
        /// \return The newly created timer object.
        ZYPHRYON_INLINE Timer CreateTimer()
        {
            const Timer::Handle Handle = mWorld.timer();
            return Timer(Handle);
        }

        /// \brief Creates an observer for reacting to ECS events.
        ///
        /// \param Name    The name of the observer.
        /// \param Event   The event entity to subscribe to (e.g. OnAdd).
        /// \param Each    The callback to invoke for each matching entity.
        /// \param Runtime Optional runtime expressions to refine the query.
        /// \return The entity representing the observer.
        template<typename... CompileExpression, typename FEach, typename... RuntimeExpression>
        ZYPHRYON_INLINE Entity CreateObserver(ConstStr8 Name, Entity Event, AnyRef<FEach> Each, AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::observer_builder<> Builder = mWorld.observer<>(Name.empty() ? nullptr : Name.data());
            DSL::_::ApplyExpressions<decltype(Builder), CompileExpression...>(Builder, Runtime...);

            Builder.event(Event.GetHandle());

            using Query = DSL::_::Extract<typename DSL::_::ExtractTypes<CompileExpression...>::Type, Query>::Type;
            return Entity(Builder.run(Query::template Runner<FEach>(Move(Each))));
        }

        /// \brief Creates a query with optional compile-time and runtime expressions.
        ///
        /// \param Name    The name of the query.
        /// \param Policy  The caching policy for the query results.
        /// \param Runtime Optional runtime expressions to refine the query.
        /// \return The constructed query object.
        template<typename... CompileExpression, typename... RuntimeExpression>
        ZYPHRYON_INLINE auto CreateQuery(ConstStr8 Name, Cache Policy, AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::query_builder<> Builder = mWorld.query_builder<>(Name.empty() ? nullptr : Name.data());
            DSL::_::ApplyExpressions<decltype(Builder), CompileExpression...>(Builder, Runtime...);

            switch (Policy)
            {
            case Cache::Default:
                Builder.cache_kind(flecs::QueryCacheDefault);
                break;
            case Cache::Auto:
                Builder.cache_kind(flecs::QueryCacheAuto);
                break;
            case Cache::None:
                Builder.cache_kind(flecs::QueryCacheNone);
                break;
            }

            using Query = DSL::_::Extract<typename DSL::_::ExtractTypes<CompileExpression...>::Type, Query>::Type;
            return Query(Builder.build());
        }

        /// \brief Creates a system with a single execution callback.
        ///
        /// \param Name      The name of the system.
        /// \param Phase     The phase entity (e.g. PreUpdate, OnUpdate).
        /// \param Execution Execution mode for the system.
        /// \param Each      The callback to run for each entity.
        /// \param Runtime   Optional runtime expressions to refine the query.
        /// \return The constructed system object.
        template<typename... CompileExpression, typename FEach, typename... RuntimeExpression>
        ZYPHRYON_INLINE auto CreateSystem(ConstStr8 Name, Entity Phase, Execution Execution, AnyRef<FEach> Each, AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::system_builder<> Builder = mWorld.system<>(Name.empty() ? nullptr : Name.data());
            DSL::_::ApplyExpressions<decltype(Builder), CompileExpression...>(Builder, Runtime...);

            switch (Execution)
            {
            case Execution::Default:
                break;
            case Execution::Immediate:
                Builder.immediate();
                break;
            case Execution::Concurrent:
                Builder.multi_threaded();
                break;
            }
            Builder.kind(Phase.GetHandle());

            using Query = DSL::_::Extract<typename DSL::_::ExtractTypes<CompileExpression...>::Type, Query>::Type;
            return System(Builder.run(Query::template Runner<FEach>(Move(Each))));
        }

        /// \brief Creates a system with begin, each, and end callbacks.
        ///
        /// \param Name      The name of the system.
        /// \param Phase     The phase entity (e.g. PreUpdate, OnUpdate).
        /// \param Execution Execution mode for the system.
        /// \param Begin     Callback invoked before iteration.
        /// \param Each      Callback invoked for each entity.
        /// \param End       Callback invoked after iteration.
        /// \param Runtime   Optional runtime expressions to refine the query.
        /// \return The constructed system object.
        template<typename... CompileExpression, typename FBegin, typename FEach, typename FEnd, typename... RuntimeExpression>
        ZYPHRYON_INLINE auto CreateSystemWithLifecycle(ConstStr8 Name, Entity Phase, Execution Execution, AnyRef<FBegin> Begin, AnyRef<FEach> Each, AnyRef<FEnd> End, AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::system_builder<> Builder = mWorld.system<>(Name.empty() ? nullptr : Name.data());
            DSL::_::ApplyExpressions<decltype(Builder), CompileExpression...>(Builder, Runtime...);

            switch (Execution)
            {
            case Execution::Default:
                break;
            case Execution::Immediate:
                Builder.immediate();
                break;
            case Execution::Concurrent:
                Builder.multi_threaded();
                break;
            }
            Builder.kind(Phase.GetHandle());

            using Query = DSL::_::Extract<typename DSL::_::ExtractTypes<CompileExpression...>::Type, Query>::Type;
            return System(Builder.run(Query::template Runner<FBegin, FEach, FEnd>(Move(Begin), Move(Each), Move(End))));
        }

        /// \brief Iterates over all archetype entities.
        ///
        /// \param Callback The function invoked once per archetype entity.
        template<typename Function>
        ZYPHRYON_INLINE void QueryArchetypes(AnyRef<Function> Callback) const
        {
            mArchetypesQueryAll.Run(Callback);
        }

        /// \brief Iterates over all entities with a given tag.
        ///
        /// \param Callback The function invoked once per matching entity.
        template<typename Tag, typename Function>
        ZYPHRYON_INLINE void QueryTag(AnyRef<Function> Callback) const
        {
            CreateQuery<DSL::In<Tag>>(Format("QueryTag<{}>", Tag::kName), Cache::Default).Run(Callback);
        }

        /// \brief Loads all archetypes from a stream.
        ///
        /// \param Reader The stream containing serialized archetypes.
        void LoadArchetypes(Ref<Reader> Reader);

        /// \brief Saves all archetypes to a stream.
        ///
        /// \param Writer The stream to write serialized archetypes to.
        void SaveArchetypes(Ref<Writer> Writer);

        /// \brief Loads a single entity from a stream.
        ///
        /// \param Reader The stream containing the serialized entity.
        /// \return The deserialized entity.
        Entity LoadEntity(Ref<Reader> Reader);

        /// \brief Loads an entity hierarchy from a stream.
        ///
        /// \param Reader The stream containing the serialized hierarchy.
        /// \return The root entity of the deserialized hierarchy.
        Entity LoadEntityHierarchy(Ref<Reader> Reader);

        /// \brief Saves a single entity to a stream.
        ///
        /// \param Writer The stream to write the entity to.
        /// \param Actor  The entity to serialize.
        void SaveEntity(Ref<Writer> Writer, Entity Actor);

        /// \brief Saves an entity hierarchy to a stream.
        ///
        /// \param Writer The stream to write the hierarchy to.
        /// \param Actor  The root entity to serialize.
        void SaveEntityHierarchy(Ref<Writer> Writer, Entity Actor);

        /// \brief Loads all components of an entity.
        ///
        /// \param Reader The stream containing the serialized components.
        /// \param Actor  The entity to apply the components to.
        void LoadComponents(Ref<Reader> Reader, Entity Actor);

        /// \brief Saves all components of an entity.
        ///
        /// \param Writer The stream to write the components to.
        /// \param Actor  The entity whose components to serialize.
        void SaveComponents(Ref<Writer> Writer, Entity Actor);

    private:

        /// \brief Registers the engine’s built-in components and systems.
        void RegisterDefaultComponentsAndSystems();

        /// \brief Allocates a new entity.
        ///
        /// \tparam Archetype If `true`, creates an archetype entity; otherwise a regular entity.
        /// \param ID         Optional explicit identifier. If `0`, a new unique ID is generated.
        /// \return The newly allocated entity object.
        template<Bool Archetype>
        ZYPHRYON_INLINE Entity Allocate(UInt64 ID = 0)
        {
            flecs::entity Actor;

            if constexpr(Archetype)
            {
                Actor = mWorld.entity(ID ? ID : kMinRangeArchetypes + mArchetypes.Allocate());
            }
            else
            {
                Actor = (ID ? mWorld.entity(ID) : mWorld.entity());
            }

            if (const Entity::Handle Generation = mWorld.get_alive(Actor); ID > 0 && Generation)
            {
                mWorld.set_version(Actor);
            }
            else
            {
                Actor = mWorld.make_alive(Actor);
            }
            return Actor;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        flecs::world              mWorld;
        Time                      mTime;
        Real32                    mMultiplier;
        Slot<kMaxCountArchetypes> mArchetypes;
        Query<>                   mArchetypesQueryAll;
    };
}