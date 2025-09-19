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

#include "Component.hpp"
#include "Query.hpp"
#include "System.hpp"
#include "Tag.hpp"

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

        /// \brief Advances the scene once per frame.
        ///
        /// This updates systems, processes deferred commands, and executes scheduled tasks.
        ///
        /// \param Time The time step data for the current frame.
        void OnTick(ConstRef<Time> Time) override;

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

        /// \brief Provides a singleton tag of the given type to the world.
        ///
        /// \tparam Component The tag type to register.
        template<typename Component>
        ZYPHRYON_INLINE void Provide()
        {
            mWorld.add<Component>();
        }

        /// \brief Provides a singleton component of the given type in the world.
        ///
        /// \param Data The instance of the component to associate with the singleton.
        template<typename Component>
        ZYPHRYON_INLINE void Provide(AnyRef<Component> Data)
        {
            mWorld.set<Component>(Move(Data));
        }

        /// \brief Fetches the singleton component of the given type from the world.
        ///
        /// \tparam Component The component type to fetch.
        /// \return A reference to the singleton instance.
        template<typename Component>
        ZYPHRYON_INLINE Ref<Component> Fetch() const
        {
            if constexpr (IsMutable<Component>)
            {
                return mWorld.get_mut<Component>();
            }
            else
            {
                return mWorld.get<Component>();
            }
        }

        /// \brief Checks whether the world currently contains a singleton of the given type.
        ///
        /// \tparam Component The tag or component type to check.
        /// \return `true` if the singleton exists, `false` otherwise.
        template<typename Component>
        ZYPHRYON_INLINE bool Contains() const
        {
            return mWorld.has<Component>();
        }

        /// \brief Revokes a singleton of the given type from the world.
        ///
        /// \tparam Component The tag or component type to remove.
        template<typename Component>
        ZYPHRYON_INLINE void Revoke()
        {
            mWorld.remove<Component>();
        }

        /// \brief Clears all instances of a component type from the world.
        ///
        /// This removes the specified component from every entity that currently has it.
        ///
        /// \tparam Component The component type to clear from all entities.
        template<typename Component>
        ZYPHRYON_INLINE void Clear()
        {
            mWorld.remove_all<Component>();
        }

        /// \brief Creates a new entity.
        ///
        /// \tparam Archetype If `true`, creates an archetype entity; otherwise a regular entity.
        /// \return The newly created entity object.
        template<Bool Archetype>
        ZYPHRYON_INLINE Entity CreateEntity()
        {
            return Allocate<Archetype>();
        }

        /// \brief Retrieves an entity by its identifier.
        ///
        /// \param ID The unique entity identifier.
        /// \return A valid entity if alive, otherwise an empty entity.
        ZYPHRYON_INLINE Entity GetEntity(UInt64 ID) const
        {
            const Entity::Handle Handle = mWorld.entity(ID);
            return Handle.is_valid() && mWorld.is_alive(Handle) ? Entity(Handle) : Entity();
        }

        /// \brief Retrieves an entity by its name.
        ///
        /// \param Name The name of the entity.
        /// \return A valid entity if alive, otherwise an empty entity.
        ZYPHRYON_INLINE Entity GetEntity(ConstStr8 Name) const
        {
            const Entity::Handle Handle = mWorld.lookup(Name.data());
            return Handle.is_valid() && mWorld.is_alive(Handle) ? Entity(Handle) : Entity();
        }

        /// \brief Retrieves a relation entity by tag and target.
        ///
        /// \tparam Tag    The relation tag component.
        /// \tparam Target The relation target component.
        /// \return The entity associated with the relation.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE Entity GetEntity() const
        {
            return Entity(mWorld.entity(mWorld.pair<Tag, Target>()));
        }

        /// \brief Retrieves a component entity definition by type.
        ///
        /// \param ID      Optional identifier override. Defaults to the symbol name of the type.
        /// \tparam Target The component type.
        /// \return The component entity object.
        template<typename Target>
        ZYPHRYON_INLINE Component<Target> GetComponent(ConstStr8 ID = flecs::_::symbol_name<Target>()) const
        {
            return Component<Target>(mWorld.component<Target>(ID.data()));
        }

        /// \brief Creates an observer for reacting to ECS events.
        ///
        /// \param Name    The name of the observer.
        /// \param Event   The event entity to subscribe to (e.g. OnAdd).
        /// \param Each    The callback to invoke for each matching entity.
        /// \param Runtime Optional runtime expressions to refine the query.
        template<typename... CompileExpression, typename FEach, typename... RuntimeExpression>
        ZYPHRYON_INLINE void CreateObserver(ConstStr8 Name, Entity Event,
            AnyRef<FEach> Each, AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::observer_builder<> Builder = mWorld.observer<>(Name.empty() ? nullptr : Name.data());

            (CompileExpression::Apply(Builder), ...);

            (Runtime.ApplyRuntime(Builder), ...);

            Builder.event(Event.GetHandle());
            Builder.each(Forward<FEach>(Each));
        }

        /// \brief Creates a query with optional compile-time and runtime expressions.
        ///
        /// \param Name    The name of the query.
        /// \param Runtime Optional runtime expressions to refine the query.
        /// \return The constructed query object.
        template<typename... CompileExpression, typename... RuntimeExpression>
        ZYPHRYON_INLINE auto CreateQuery(ConstStr8 Name, AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::query_builder<> Builder = mWorld.query_builder<>(Name.empty() ? nullptr : Name.data());
            (CompileExpression::Apply(Builder), ...);

            (Runtime.ApplyRuntime(Builder), ...);

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
        ///
        /// \return The constructed system object.
        template<typename... CompileExpression, typename FEach, typename... RuntimeExpression>
        ZYPHRYON_INLINE auto CreateSystem(ConstStr8 Name, Entity Phase, Execution Execution,
            AnyRef<FEach> Each, AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::system_builder<> Builder = mWorld.system<>(Name.empty() ? nullptr : Name.data());

            (CompileExpression::Apply(Builder), ...);

            (Runtime.ApplyRuntime(Builder), ...);

            switch (Execution)
            {
            case Execution::Default:
                break;
            case Execution::Immediate:
                Builder.immediate();
                break;
            case Execution::Asynchronous:
                Builder.multi_threaded();
                break;
            }

            Builder.kind(Phase.GetHandle());
            return System(Builder.run([Each](Ref<flecs::iter> Iterator)
            {
                using Query = DSL::_::Extract<typename DSL::_::ExtractTypes<CompileExpression...>::Type, Query>::Type;
                Query::Invoke(Iterator, Each);
            }));
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
        ///
        /// \return The constructed system object.
        template<typename... CompileExpression, typename FBegin, typename FEach, typename FEnd, typename... RuntimeExpression>
        ZYPHRYON_INLINE auto CreateSystem(ConstStr8 Name, Entity Phase, Execution Execution,
            AnyRef<FBegin> Begin, AnyRef<FEach> Each, AnyRef<FEnd> End, AnyRef<RuntimeExpression>... Runtime) const
        {
            flecs::system_builder<> Builder = mWorld.system<>(Name.empty() ? nullptr : Name.data());

            (CompileExpression::Apply(Builder), ...);

            (Runtime.ApplyRuntime(Builder), ...);

            switch (Execution)
            {
            case Execution::Default:
                break;
            case Execution::Immediate:
                Builder.immediate();
                break;
            case Execution::Asynchronous:
                Builder.multi_threaded();
                break;
            }

            Builder.kind(Phase.GetHandle());
            return System(Builder.run([Begin, Each, End](Ref<flecs::iter> Iterator)
            {
                using Query = DSL::_::Extract<typename DSL::_::ExtractTypes<CompileExpression...>::Type, Query>::Type;
                Query::Invoke(Iterator, Begin, Each, End);
            }));
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

            if (const Entity::Handle Generation = mWorld.get_alive(Actor); Generation)
            {
                mWorld.set_version(Actor);
            }
            else
            {
                Actor = mWorld.make_alive(Actor);
            }

            if constexpr(Archetype)
            {
                Actor.add(flecs::Prefab);
            }
            else
            {
                Actor.add(flecs::Final);
            }
            return Actor;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        flecs::world                mWorld;
        Handle<kMaxCountArchetypes> mArchetypes;
    };
}