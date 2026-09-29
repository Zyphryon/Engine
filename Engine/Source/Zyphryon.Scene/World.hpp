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
#include "Type/Declaration.hpp"
#include "Execution/Scheduler.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents a world, its entities, and the queries, systems, pull lists, modules and saves over them.
    class ZY_API World : public Storage
    {
        friend class Draft;
        friend class Enrollment;
        friend class System;

    public:

        /// The phase every world makes first, which systems run in unless they ask for another.
        static constexpr UInt32 kUpdate = 1;

    public:

        /// \brief Constructs a world holding only the world entity, the built-in components and the update phase.
        ///
        /// \param Host The host holding the job service spread walks run on.
        explicit World(Ref<ZyEngine::Subsystem::Host> Host);

        /// \brief Destroys every entity, dropping systems first so no callback runs meanwhile.
        ~World();

        /// \brief Worlds are not copied, since every entity points back into its own.
        World(ConstRef<World> Other) = delete;

        /// \brief Creates an entity holding a set of components, placed straight where it belongs.
        ///
        /// \param Values The components, moved into place.
        /// \return The entity, which waits outside every query until the running walk ends when there is one.
        template<typename... Types>
        Entity Spawn(AnyRef<Types>... Values)
        {
            if (mDeferral.IsWalking())
            {
                const Entity Actor = CreateEntity();
                (Actor.Set(Forward<Types>(Values)), ...);
                return Actor;
            }

            // Along the add edges from the root, which also bring along what the components imply.
            Ptr<Chunk> Target = mLayout.GetRoot();
            ((Target = FindOrCreateAddition(* Target, IdentifierOf<StripAll<Types>>())), ...);

            const Handle Actor = mDirectory.Allocate(false);
            Place(Actor.GetIndex(), * Target);

            const UInt32 Row = mDirectory[Actor.GetIndex()].Row;
            (Chunk::Emplace<StripAll<Types>>(* Target, Row, Forward<Types>(Values)), ...);

            if (Target->GetSignature().GetSize() != sizeof...(Types))
            {
                const UInt32 Given[] = { IdentifierOf<StripAll<Types>>()... };

                for (UInt32 Column = 0; Column < Target->GetColumns().GetSize(); ++Column)
                {
                    if (!ZyBase::Find(Given, sizeof...(Types), Target->GetColumns()[Column].Identifier))
                    {
                        Chunk::Construct(* Target->GetColumns()[Column].Info, Target->At(Column, Row), 1);
                    }
                }
            }

            RecordGain(Actor.GetIndex(), nullptr);
            return Entity(this, Actor);
        }

        /// \brief Hands every archetype to a callback, in slot order.
        ///
        /// \param Callback The callable, taking each archetype.
        template<typename Callable>
        void QueryArchetypes(AnyRef<Callable> Callback)
        {
            mDirectory.ForEachArchetype([this, &Callback](Handle Archetype)
            {
                Callback(Entity(this, Archetype));
            });
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
        /// \param Name  The name profiles and logs show.
        /// \param Phase The phase it runs in.
        /// \return The description, which becomes the system once it is handed its callback.
        ZY_INLINE Builder<ZyScene::System> System(Text Name, UInt32 Phase = kUpdate)
        {
            return Builder<ZyScene::System>(this, Name, Phase);
        }

        /// \brief Creates a phase, which runs every system in it, in the order they were created, when its turn comes.
        ///
        /// \param Name  The name of the phase, which profiles and logs show.
        /// \param After The phase it runs right after, or zero to run after every phase made so far.
        /// \return The handle of the phase, never zero.
        ZY_INLINE UInt32 CreatePhase(Text Name, UInt32 After = 0)
        {
            return mScheduler.CreatePhase(Name, After, mImporting);
        }

        /// \brief Runs every phase once, in order, and every system in each, in the order they were created.
        void Progress();

        /// \brief Declares components, each named after its type.
        ///
        /// \return The declaration, which takes their traits by chained calls and tells the world when it ends.
        template<typename... Types>
        ZY_INLINE Declaration<Types...> Declare()
        {
            return Declaration<Types...>(this, Text());
        }

        /// \brief Declares one component under a name of its own.
        ///
        /// \param Name The name saves know it by.
        /// \return The declaration, which takes its traits by chained calls and tells the world when it ends.
        template<typename Type>
        ZY_INLINE Declaration<Type> Declare(Text Name)
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
        /// \param Package The module.
        void Discard(UInt32 Package);

        /// \brief Writes entities, each with everything beneath it, as one save.
        ///
        /// \param Output The writer to append the save to.
        /// \param Roots  The entities to write, the world entity among them for its singletons.
        void Save(Ref<Writer> Output, ConstSpan<Entity> Roots);

        /// \brief Reads a save, making each entity from the archetype it names, the world entity read over itself.
        ///
        /// \param Input  The bytes of the save.
        /// \param Parent The entity to attach every root to, or one that names nothing.
        /// \return The entities made at the root of the save.
        Sequence<Entity> Load(ConstSpan<Byte> Input, Entity Parent = Entity());

        /// \brief Reads a save over an existing entity, which takes the first root's values and what stands beneath it.
        ///
        /// \param Input The bytes of the save.
        /// \param Root  The entity to read the first root over, alive, keeping its place, parent and archetype.
        /// \return `true` if the save was read, `false` if the bytes were not a save this version reads.
        Bool Merge(ConstSpan<Byte> Input, Entity Root);

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

        /// \brief Records components a declaration is done with, reading back the values kept as bytes for them.
        ///
        /// \param Identifiers The components.
        void Registered(ConstSpan<UInt32> Identifiers);

        /// \brief Writes one entity's record, then everything beneath it, naming each component it saves once.
        ///
        /// \param Output  The writer.
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
        /// \param Input  The bytes of the save.
        /// \param Parent The entity to attach every root made to, or one that names nothing.
        /// \param Root   The entity to read the first root over, or one that names nothing.
        /// \return The entities at the root of the save.
        Sequence<Entity> Decode(ConstSpan<Byte> Input, Entity Parent, Entity Root);

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

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Scheduler        mScheduler;
        Sequence<Module> mModules;
        UInt32           mImporting;
    };
}