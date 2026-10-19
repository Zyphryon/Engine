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

#include "Builder.hpp"
#include "Execution/Scheduler.hpp"
#include "Type/Declaration.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents a world, its entities, and the queries, systems, modules and saves over them.
    class ZY_API World : public Storage
    {
        friend class Draft;
        friend class Enrollment;

    public:

        /// \brief Constructs a world holding only the world entity and the built-in components.
        ///
        /// \param Host The host holding the job service spread walks run on.
        explicit World(Ref<ZyEngine::Subsystem::Host> Host);

        /// \brief Destroys every entity, dropping systems first so no callback runs meanwhile.
        ~World();

        /// \brief Worlds are not copied, since every entity points back into its own.
        World(ConstRef<World> Other) = delete;

        /// \brief Creates a plain root entity holding a set of components, in one move.
        ///
        /// \param Values The components, moved into place.
        /// \return The entity, which waits outside every query until the running walk ends when there is one.
        template<typename... Types>
        ZY_INLINE Entity Spawn(AnyRef<Types>... Values)
        {
            return Spawn(Entity(), Entity(), Forward<Types>(Values)...);
        }

        /// \brief Creates an entity from an archetype under a parent, holding its own components over the archetype's.
        ///
        /// \param Archetype The archetype it reads from, or one that names nothing for a plain entity.
        /// \param Parent    The entity it hangs from, or one that names nothing for a root.
        /// \param Values    The components it holds itself, moved into place.
        /// \return The entity, which waits outside every query until the running walk ends when there is one.
        template<typename... Types>
        Entity Spawn(Entity Archetype, Entity Parent, AnyRef<Types>... Values)
        {
            ZY_ASSERT(!Archetype.IsAlive() || Archetype.IsArchetype(), "An entity is made only from an archetype");

            const Handle From  = Archetype.IsAlive() ? Archetype.GetHandle() : Handle();
            const Handle Above = Parent.IsAlive() ? Parent.GetHandle() : Handle();

            if (!mDeferral.IsWalking())
            {
                const Handle Actor = mDirectory.Allocate(false);
                Land(Actor, From.GetIndex(), Above.GetIndex(), Forward<Types>(Values)...);
                return Entity(this, Actor);
            }

            // The slot is taken now, and the entity lands in order with the changes the walk held.
            const Handle Actor = Create();

            Postpone([this, Actor, From, Above, ...Values = Forward<Types>(Values)]() mutable
            {
                if (mDirectory.IsAlive(Actor))
                {
                    const UInt32 Base = mDirectory.IsAlive(From) ? From.GetIndex() : 0;
                    const UInt32 Home = mDirectory.IsAlive(Above) ? Above.GetIndex() : 0;
                    Land(Actor, Base, Home, Move(Values)...);
                }
            });
            return Entity(this, Actor);
        }

        /// \brief Starts a query over the entities holding some components, the writable ones as their own.
        ///
        /// \return The description, which becomes the query when it is converted to one.
        template<typename... Types>
        ZY_INLINE Builder<ZyScene::Query> Query()
        {
            Builder<ZyScene::Query> Outline(this, Text(), 0);
            (Outline.template Ask<Types>(), ...);
            return Outline;
        }

        /// \brief Starts a system, which a phase runs each progress over what its callback asks for or a list it reads.
        ///
        /// \param Name  The name profiles show.
        /// \param Phase The phase it runs in, in the order systems were made.
        /// \return The description, which becomes the system once it is handed its callback.
        ZY_INLINE Builder<ZyScene::System> System(Text Name, UInt32 Phase)
        {
            return Builder<ZyScene::System>(this, Name, Phase);
        }

        /// \brief Creates a phase, which runs after every phase made before it.
        ///
        /// \param Name The name profiles show.
        /// \return The phase, never zero.
        ZY_INLINE UInt32 Phase(Text Name)
        {
            return mScheduler.CreatePhase(Name, mImporting);
        }

        /// \brief Runs every phase once, in order, and every system in each, in the order they were made.
        void Progress();

        /// \brief Declares a component under the name saves know it by.
        ///
        /// \param Name The name, or empty for the type's `kName` or the one the compiler gives it.
        /// \return The declaration, which takes its traits by chained calls and tells the world when it ends.
        template<typename Type>
        ZY_INLINE Declaration<Type> Declare(Text Name = Text())
        {
            return Declaration<Type>(this, Name);
        }

        /// \brief Declares every component that declares itself.
        template<typename... Types>
        ZY_INLINE void Register()
            requires (requires (Ref<World> Scene) { Types::OnDeclare(Scene); } && ...)
        {
            (Types::OnDeclare(* this), ...);
        }

        /// \brief Imports a module, recording every component, phase and system the callback declares.
        ///
        /// \param Name     The name of the module, which logs show.
        /// \param Callback The code that declares the module's contents, run once.
        /// \return The handle of the module, never zero.
        template<typename Callable>
        ZY_INLINE UInt32 Import(Text Name, AnyRef<Callable> Callback)
        {
            ZY_ASSERT(mImporting == 0, "A module cannot be imported while another one is");

            mModules.Append(Name, Sequence<UInt32>(), true);
            mImporting = static_cast<UInt32>(mModules.GetSize());

            const UInt32 Package = mImporting;
            Callback();
            mImporting = 0;
            return Package;
        }

        /// \brief Discards a module, taking out its systems and keeping its values as the bytes they save to.
        ///
        /// \param Package The handle \ref Import returned for the module.
        void Discard(UInt32 Package);

        /// \brief Writes entities, each with everything beneath it, as one save.
        ///
        /// \param Output The writer to append the save to.
        /// \param Roots  The entities to write, the world entity among them for its singletons.
        void Save(Ref<Writer> Output, ConstSpan<Entity> Roots);

        /// \brief Reads a save, making each entity from the archetype it names, the world entity read over itself.
        ///
        /// \param Input The bytes of the save.
        /// \return The entities made at the root of the save.
        Sequence<Entity> Load(ConstSpan<Byte> Input);

        /// \brief Reads a save over an existing entity, which takes the first root's values and what stands beneath it.
        ///
        /// \param Input The bytes of the save.
        /// \param Root  The entity to read the first root over, alive, keeping its place, parent and archetype.
        /// \return `true` if the save was read, `false` if the bytes were not a save this version reads.
        Bool Merge(ConstSpan<Byte> Input, Entity Root);

        /// \brief Copies entities, each with everything beneath it, into another world as a save would carry them.
        ///
        /// \param Target  The world to copy into, never this one, holding the same archetypes under the same handles.
        /// \param Roots   The entities whose trees are copied, the world entity among them for its singletons.
        /// \param Exclude The components to leave out of the copy, none to keep them all.
        /// \return The copies of the roots, in the order a save writes them.
        Sequence<Entity> Migrate(Ref<World> Target, ConstSpan<Entity> Roots, ConstSpan<Component> Exclude);

    private:

        /// The characters every scene save opens with.
        static constexpr UInt32 kMagic    = 'Z' | ('S' << 8) | ('C' << 16) | ('N' << 24);

        /// The version of the layout saves are written and read in.
        static constexpr UInt16 kVersion  = 2;

        /// The chunk listing the type names a save uses.
        static constexpr UInt32 kTypes    = 'T' | ('Y' << 8) | ('P' << 16) | ('E' << 24);

        /// The chunk holding the records, each after its parent's.
        static constexpr UInt32 kEntities = 'E' | ('N' << 8) | ('T' << 16) | ('S' << 24);

        /// \brief Represents one module the world imported, and the components it registered.
        struct Module final
        {
            /// The name logs show.
            Str              Name;

            /// The components it registered.
            Sequence<UInt32> Types;

            /// `true` while the module is in the world.
            Bool             Alive;
        };

        /// \brief Places an entity where its archetype and components lead, building its values and parts there.
        ///
        /// \param Actor     The entity, sitting in no chunk or in the root.
        /// \param Archetype The slot of the archetype it reads from, or zero.
        /// \param Parent    The slot of the entity it hangs from, or zero.
        /// \param Values    The components it holds itself, moved into place.
        template<typename... Types>
        ZY_INLINE void Land(Handle Actor, UInt32 Archetype, UInt32 Parent, AnyRef<Types>... Values)
        {
            // From the archetype's heir chunk along the add edges, which also bring along what the components imply.
            Ptr<Chunk> Target = Archetype ? FindHeir(Archetype, false) : mLayout.GetRoot();
            ((Target = Extend(* Target, IdentifierOf<StripAll<Types>>())), ...);

            // Made during a walk, it waits in the root, otherwise it sits nowhere yet.
            const UInt32 Index = Actor.GetIndex();

            if (mDirectory[Index].Holder)
            {
                Transfer(Index, * Target);
            }
            else
            {
                Place(Index, * Target);
            }

            const UInt32 Row = mDirectory[Index].Row;
            (Chunk::Emplace<StripAll<Types>>(* Target, Row, Forward<Types>(Values)), ...);

            const UInt32 Given[] = { IdentifierOf<StripAll<Types>>()..., 0 };
            Settle(Index, Archetype, Parent, ConstSpan<UInt32>(Given, sizeof...(Types)));
        }

        /// \brief Records a component a declaration is done with, reading back the values kept as bytes for it.
        ///
        /// \param Identifier The component the declaration is done with.
        void Registered(UInt32 Identifier);

        /// \brief Writes one entity's record, then everything beneath it, naming each component it saves once.
        ///
        /// \param Output  The writer the record is appended to.
        /// \param Index   The slot of the entity.
        /// \param Parent  The position of the parent's record plus one, or zero for a root.
        /// \param Indices The index each component written so far is written under.
        /// \param Names   The name of each component written so far, by index.
        /// \param Count   The records written so far.
        void Emit(
            Ref<Writer>                Output,
            UInt32                     Index,
            UInt32                     Parent,
            Ref<Table<UInt32, UInt32>> Indices,
            Ref<Sequence<Text>>        Names,
            Ref<UInt32>                Count);

        /// \brief Reads a save, making every entity but the first root, which is read over one given when it is alive.
        ///
        /// \param Input The bytes of the save.
        /// \param Root  The entity to read the first root over, or one that names nothing.
        /// \return The entities at the root of the save.
        Sequence<Entity> Decode(ConstSpan<Byte> Input, Entity Root);

        /// \brief Makes one entity from its record.
        ///
        /// \param Input       The reader, at the record's identifier.
        /// \param Types       The component each type index of the save answers to.
        /// \param Parent      The slot to make it beneath, or zero.
        /// \param Target      The entity to read the record over, or an invalid handle to make one.
        /// \param Identifiers The room the record's components are gathered in, reused from record to record.
        /// \param Payloads    The room the record's values are gathered in, reused from record to record.
        /// \return The slot of the entity.
        UInt32 Make(
            Ref<Reader>                    Input,
            ConstRef<Sequence<UInt32>>     Types,
            UInt32                         Parent,
            Handle                         Target,
            Ref<Sequence<UInt32>>          Identifiers,
            Ref<Sequence<ConstSpan<Byte>>> Payloads);

        /// \brief Gets the root of the hierarchy holding the archetype an archetype reads from.
        ///
        /// \param Index The slot of the archetype.
        /// \return The slot of the root, or zero when it reads from none.
        UInt32 GetOrigin(UInt32 Index) const;

    public:

        /// \brief Copies a save's bytes, leaving out every component excluded and naming only what it kept.
        ///
        /// \param Input   The bytes of a save as \ref Save writes them, read without any world.
        /// \param Output  The writer the copy is appended to, left as it was when the bytes are refused.
        /// \param Exclude The components to leave out, a name this build does not know being always kept.
        /// \return `true` if the save was copied, `false` if the bytes were not a save this version reads whole.
        static Bool Strip(ConstSpan<Byte> Input, Ref<Writer> Output, ConstSpan<Component> Exclude);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Scheduler        mScheduler;
        Sequence<Module> mModules;
        UInt32           mImporting;
    };
}