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
#include "World.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Manages the world lifecycle, entity allocation, and scene serialization.
    class Service final : public AbstractService<Service>
    {
    public:

        /// \brief Constructs the scene service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \copydoc Service::OnTick(Time)
        void OnTick(Time Time) override;

        /// \brief Gets a world handle that provides access to singleton components.
        ///
        /// \return The world wrapper for the world.
        ZYPHRYON_INLINE World GetWorld()
        {
            return World(mWorld);
        }

        /// \brief Sets the global timescale applied to all system delta times each tick.
        ///
        /// \param Scale The multiplier to apply to elapsed time. Must be non-negative.
        ZYPHRYON_INLINE void SetTimeScale(Real32 Scale)
        {
            LOG_ASSERT(Scale >= 0.0f, "Timescale cannot be negative");

            mMultiplier = Scale;
        }

        /// \brief Gets the current global timescale.
        ///
        /// \return The timescale multiplier currently in effect.
        ZYPHRYON_INLINE Real32 GetTimeScale() const
        {
            return mMultiplier;
        }

        /// \brief Creates a new runtime entity and marks it as non-inheritable.
        ///
        /// \return The newly created entity.
        ZYPHRYON_INLINE Entity CreateEntity()
        {
            const Entity Actor = Allocate<false>();
            Actor.Add(flecs::Final);
            return Actor;
        }

        /// \brief Creates a new archetype entity, optionally cloning an existing one.
        ///
        /// \param Source An optional archetype to clone from. Must be a valid archetype if provided.
        /// \return The newly created archetype entity.
        ZYPHRYON_INLINE Entity CreateArchetype(Entity Source = Entity())
        {
            const Entity Actor = Allocate<true>();

            if (Source.IsValid())
            {
                LOG_ASSERT(Source.IsArchetype(), "Source entity is not an archetype");

                Source.Clone(Actor);
            }
            else
            {
                Actor.Add(flecs::Prefab);
            }
            return Actor;
        }

        /// \brief Looks up a live entity by its unique numeric identifier.
        ///
        /// \param ID The numeric entity identifier to resolve.
        /// \return The entity corresponding to the given ID, or an invalid entity if not found.
        ZYPHRYON_INLINE Entity GetEntity(UInt64 ID) const
        {
            const Entity::Handle Handle = mWorld.entity(ID);
            return Entity(Handle);
        }

        /// \brief Looks up a live entity by its registered name.
        ///
        /// \param Name The name to search for in the world.
        /// \return The entity with the given name, or an invalid entity if not found.
        ZYPHRYON_INLINE Entity GetEntity(ConstStr8 Name) const
        {
            const Entity::Handle Handle = mWorld.lookup(Name.data());
            return Entity(Handle);
        }

        /// \brief Registers or retrieves a typed component in the world under the given identifier.
        ///
        /// \param ID The string identifier used to register or look up the component.
        /// \return A typed component handle for \p Type.
        template<typename Type>
        ZYPHRYON_INLINE Component<Type> GetComponent(ConstStr8 ID) const
        {
            return Component<Type>(mWorld.component<Type>(ID.data()));
        }

        /// \brief Creates a named pipeline phase tag and optionally chains it after a dependency phase.
        ///
        /// \tparam Name       The compile-time symbol used as the tag name and identifier.
        /// \param  Type       The phase kind entity this phase belongs to.
        /// \param  Dependency An optional phase entity that this phase depends on and must run after.
        /// \return The entity representing the newly created phase.
        template<Symbol Name>
        ZYPHRYON_INLINE Entity CreatePhase(Entity Type, Entity Dependency = Entity()) const
        {
            const auto Component = GetComponent<Tag<Name>>();
            Component.Add(Type);

            if (Dependency.IsValid())
            {
                Component.DependsOn(Dependency);
            }
            return Component;
        }

        /// \brief Creates a pipeline that controls which systems run and in what order.
        ///
        /// Compile-time DSL expressions are applied to the pipeline builder to filter the systems
        /// it will execute. Optional runtime expressions can extend the filter dynamically.
        ///
        /// \tparam CompileExpression Zero or more compile-time DSL filter expressions.
        /// \tparam RuntimeExpression Zero or more runtime expression types.
        /// \param  Runtime           The runtime expression values to append to the pipeline filter.
        /// \return The constructed pipeline.
        template<typename... CompileExpression, typename... RuntimeExpression>
        ZYPHRYON_INLINE Pipeline CreatePipeline(AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::pipeline_builder<> Builder = mWorld.pipeline().with(flecs::System);
            DSL::_::ApplyExpressions<decltype(Builder), CompileExpression...>(Builder, Runtime...);

            return Pipeline(Builder.build());
        }

        /// \brief Creates a reactive observer that fires a callback when entities match an event.
        ///
        /// \tparam CompileExpression Zero or more compile-time DSL filter expressions.
        /// \tparam FEach             The callable type invoked for each matching entity.
        /// \tparam RuntimeExpression Zero or more runtime expression types.
        /// \param  Name              The optional display name for the observer entity.
        /// \param  Event             The event entity that triggers the observer.
        /// \param  Each              The callback invoked for each entity that matches the event.
        /// \param  Runtime           The runtime expression values to append to the observer filter.
        /// \return The entity representing the created observer.
        template<typename... CompileExpression, typename FEach, typename... RuntimeExpression>
        ZYPHRYON_INLINE Entity CreateObserver(ConstStr8 Name, Entity Event, AnyRef<FEach> Each, AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::observer_builder<> Builder = mWorld.observer<>(Name.empty() ? nullptr : Name.data());
            DSL::_::ApplyExpressions<decltype(Builder), CompileExpression...>(Builder, Runtime...);

            Builder.event(Event.GetHandle());

            using Types = typename DSL::_::ExtractTypes<CompileExpression...>::Type;
            return Entity(Builder.run(DSL::_::RunnerFactory<Types, FEach>::Make(Move(Each))));
        }

        /// \brief Creates a cached or uncached query over entities matching the given expressions.
        ///
        /// \tparam CompileExpression Zero or more compile-time DSL filter expressions.
        /// \tparam RuntimeExpression Zero or more runtime expression types.
        /// \param  Name              The optional display name for the query.
        /// \param  Policy            The caching strategy to apply to this query.
        /// \param  Runtime           The runtime expression values to append to the query filter.
        /// \return The constructed query.
        template<typename... CompileExpression, typename... RuntimeExpression>
        ZYPHRYON_INLINE Query CreateQuery(ConstStr8 Name, Cache Policy, AnyRef<RuntimeExpression>... Runtime) const
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
            return Query(Builder.build());
        }

        /// \brief Creates a timer entity that can be used to rate-limit or schedule systems.
        ///
        /// \return The newly created timer.
        ZYPHRYON_INLINE Timer CreateTimer()
        {
            const Timer::Handle Handle = mWorld.timer();
            return Timer(Handle);
        }

        /// \brief Creates a system that runs a callback each tick for all matching entities.
        ///
        /// \tparam CompileExpression Zero or more compile-time DSL filter expressions.
        /// \tparam RuntimeExpression Zero or more runtime expression types.
        /// \param  Name              The optional display name for the system entity.
        /// \param  Phase             The phase entity that determines when this system runs.
        /// \param  Execution         The threading mode for this system.
        /// \param  Each              The callback invoked for each matching entity.
        /// \param  Runtime           The runtime expression values to append to the system filter.
        /// \return The created system.
        template<typename... CompileExpression, typename FEach, typename... RuntimeExpression>
        ZYPHRYON_INLINE System CreateSystem(ConstStr8 Name, Entity Phase, Execution Execution, AnyRef<FEach> Each, AnyRef<RuntimeExpression>... Runtime) const
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

            using Types = typename DSL::_::ExtractTypes<CompileExpression...>::Type;
            return System(Builder.run(DSL::_::RunnerFactory<Types, FEach>::Make(Move(Each))));
        }

        /// \brief Creates a system with explicit begin, per-entity, and end lifecycle callbacks.
        ///
        /// \tparam CompileExpression Zero or more compile-time DSL filter expressions.
        /// \tparam RuntimeExpression Zero or more runtime expression types.
        /// \param  Name              The optional display name for the system entity.
        /// \param  Phase             The phase entity that determines when this system runs.
        /// \param  Execution         The threading mode for this system.
        /// \param  Begin             The callback invoked before the per-entity loop.
        /// \param  Each              The callback invoked for each matching entity.
        /// \param  End               The callback invoked after the per-entity loop.
        /// \param  Runtime           The runtime expression values to append to the system filter.
        /// \return The created system.
        template<typename... CompileExpression, typename FBegin, typename FEach, typename FEnd, typename... RuntimeExpression>
        ZYPHRYON_INLINE System CreateSystemWithLifecycle(ConstStr8 Name, Entity Phase, Execution Execution, AnyRef<FBegin> Begin, AnyRef<FEach> Each, AnyRef<FEnd> End, AnyRef<RuntimeExpression>... Runtime) const
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

            using Types = typename DSL::_::ExtractTypes<CompileExpression...>::Type;
            return System(Builder.run(DSL::_::RunnerFactoryLifecycle<Types, FBegin, FEach, FEnd>::Make(Move(Begin), Move(Each), Move(End))));
        }

        /// \brief Iterates over all allocated archetype entities and invokes a callback for each one.
        ///
        /// \param Callback The function to call for each archetype entity.
        template<typename Function>
        ZYPHRYON_INLINE void QueryArchetypes(AnyRef<Function> Callback) const
        {
            for (UInt32 Handle = 1; Handle <= mArchetypes.GetHead(); ++Handle)
            {
                if (mArchetypes.IsAllocated(Handle))
                {
                    Callback(GetEntity(kMinRangeArchetypes + Handle));
                }
            }
        }

        /// \brief Iterates over all entities that carry a specific tag and invokes a callback for each one.
        ///
        /// \tparam Tag      The tag type to filter entities by.
        /// \param  Callback The function to call for each matching entity.
        template<typename Tag, typename Function>
        ZYPHRYON_INLINE void QueryTag(AnyRef<Function> Callback) const
        {
            CreateQuery<DSL::In<Tag>>(Format("QueryTag<{}>", Tag::kName), Cache::Default).Run(Callback);
        }

        /// \brief Loads all singleton components from a binary archive into the world.
        ///
        /// \param Archive The binary data reader to read the world state from.
        void LoadWorld(Ref<Reader> Archive);

        /// \brief Saves all singleton components of the world to a binary archive.
        ///
        /// \param Archive The binary data writer to write the world state to.
        void SaveWorld(Ref<Writer> Archive);

        /// \brief Loads a full entity hierarchy from a binary archive and returns the root entity.
        ///
        /// \param Archive The binary data reader to read the hierarchy from.
        /// \return The root entity of the loaded hierarchy.
        Entity LoadHierarchy(Ref<Reader> Archive);

        /// \brief Saves a full entity hierarchy rooted at the given actor to a binary archive.
        ///
        /// \param Archive The binary data writer to write the hierarchy to.
        /// \param Actor   The root entity of the hierarchy to save.
        void SaveHierarchy(Ref<Writer> Archive, Entity Actor);

        /// \brief Loads all archetypes from a binary archive and registers them in the world.
        ///
        /// \param Archive The binary data reader to read the archetypes from.
        void LoadArchetypes(Ref<Reader> Archive);

        /// \brief Saves all currently registered archetypes to a binary archive.
        ///
        /// \param Archive The binary data writer to write the archetypes to.
        void SaveArchetypes(Ref<Writer> Archive);

    private:

        /// \brief Registers built-in components and default system hooks into the world.
        void RegisterDefaultComponentsAndSystems();

        /// \brief Allocates a new entity or archetype handle, reusing an existing ID if provided.
        ///
        /// If \p Archetype is `true`, the entity is allocated from the archetype slot pool using a
        /// reserved ID range. If an ID is supplied and the entity is already known to the world, its
        /// generation is bumped; otherwise the entity is made alive for the first time.
        ///
        /// \tparam Archetype `true` to allocate from the archetype pool, `false` for a regular entity.
        /// \param  ID        An optional existing numeric entity ID to reuse. Pass `0` to auto-assign.
        /// \return The allocated entity handle.
        template<Bool Archetype>
        ZYPHRYON_INLINE Entity Allocate(UInt64 ID = 0)
        {
            flecs::entity Handle;

            if constexpr (Archetype)
            {
                Handle = mWorld.entity(ID ? ID : kMinRangeArchetypes + mArchetypes.Allocate());
            }
            else
            {
                Handle = (ID ? mWorld.entity(ID) : mWorld.entity());
            }

            if (const Entity::Handle Generation = mWorld.get_alive(Handle); ID > 0 && Generation)
            {
                mWorld.set_version(Handle);
            }
            else
            {
                mWorld.make_alive(Handle);
            }
            return Handle;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        flecs::world              mWorld;       // TODO: Support for multiple world
        Time                      mTime;
        Real32                    mMultiplier;
        Slot<kMaxCountArchetypes> mArchetypes;
    };
}

