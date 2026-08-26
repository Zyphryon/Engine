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
#include "Factory.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief What one world keeps that cannot be kept in the world itself.
    class Context final
    {
    public:

        /// \brief Constructs an empty context.
        ZY_INLINE Context() = default;

        /// \brief Remembers the serializer a component is read and written through.
        ///
        /// \param Component  The identifier of the component the serializer belongs to.
        /// \param Serializer The serializer the component is read and written through.
        ZY_INLINE void AddFactory(ecs_entity_t Component, Factory Serializer)
        {
            mFactories.FindOrInsert(Component) = Serializer;
        }

        /// \brief Gets the serializer a component is read and written through.
        ///
        /// \param Component The identifier of the component to look up.
        /// \return The serializer, or null when the component was never declared serializable.
        ZY_INLINE ConstPtr<Factory> GetFactory(ecs_entity_t Component) const
        {
            return mFactories.Find(Component);
        }

        /// \brief Forgets the serializer a component was read and written through.
        ///
        /// \param Component The identifier of the component to forget.
        ZY_INLINE void RemoveFactory(ecs_entity_t Component)
        {
            mFactories.Erase(Component);
        }

    public:

        /// \brief Gives a world a context of its own, which it owns until it closes.
        ///
        /// \param World The world to give a context to.
        ZY_INLINE static void Attach(Ptr<ecs_world_t> World)
        {
            ecs_set_binding_ctx(World, new Context(), [](Ptr<void> Instance)
            {
                delete static_cast<Ptr<Context>>(Instance);
            });
        }

        /// \brief Gets the context a world was given.
        ///
        /// \param World The world to read, which may be any of its stages.
        /// \return The context the world owns.
        ZY_INLINE static Ref<Context> Get(Ptr<ecs_world_t> World)
        {
            const Ptr<Context> Instance = static_cast<Ptr<Context>>(ecs_get_binding_ctx(ecs_get_world(World)));
            ZY_ASSERT(Instance, "World was never given a context");

            return (* Instance);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<ecs_entity_t, Factory> mFactories;
    };
}