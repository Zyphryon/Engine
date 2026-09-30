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

#include "Deferral.hpp"
#include "Layout.hpp"
#include "Ledger.hpp"
#include "Zyphryon.Engine/Locator.hpp"
#include "Zyphryon.Job/Service.hpp"
#include "Zyphryon.Scene/Type/Component.hpp"
#include "Zyphryon.Scene/Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    class Entity;

    template<typename>
    class Cursor;

    /// \brief Represents the entities of a world and every change that spans more than one of its parts.
    class ZY_API Storage : public ZyEngine::Locator<ZyJob::Service>
    {
        template<typename> friend class Cursor;
        friend class Entity;
        friend class Query;
        friend class Walk;

    public:

        /// The handle of the entity standing for the world itself, made first and never destroyed.
        static constexpr Handle kWorld = Handle(1, 0);

    public:

        /// \brief Storage is not copied, since every entity's slot points into its chunks.
        Storage(ConstRef<Storage> Other) = delete;

        /// \brief Gets the entity standing for the world, whose components are the singletons.
        ///
        /// \return The world entity.
        Entity GetWorld() const;

        /// \brief Gets the entity a number names, as \ref Entity::GetID hands it out.
        ///
        /// \param Identifier The number, where an archetype slot without a generation finds the archetype there.
        /// \return The entity, which is not alive when the number names nothing living.
        Entity GetEntity(UInt64 Identifier);

        /// \brief Creates an entity that carries nothing.
        ///
        /// \return The entity, which waits outside every query until the running walk ends when there is one.
        Entity CreateEntity();

        /// \brief Creates an archetype, an entity that queries never match and that other entities are made from.
        ///
        /// \return The archetype.
        Entity CreateArchetype();

        /// \brief Creates an entity from an archetype, with every part beneath it.
        ///
        /// \param Archetype The archetype, which must be alive.
        /// \return The instance, which carries nothing until the running walk ends when there is one.
        Entity Instantiate(Entity Archetype);

        /// \brief Copies an archetype, everything it holds and every part beneath it, into a new one.
        ///
        /// \param Source The archetype, which must be alive.
        /// \param Parent The archetype the copy becomes a part of, or one naming nothing to leave it alone.
        /// \return The copy, made from what the source was made from.
        Entity CloneArchetype(Entity Source, Entity Parent);

        /// \brief Takes a component off every entity holding it.
        ///
        /// \param Type The component.
        void Purge(Component Type);

        /// \brief Takes a component off every entity holding it.
        template<typename Type>
        ZY_INLINE void Purge()
        {
            Purge(Component(IdentifierOf<Type>()));
        }

        /// \brief Runs a callable with every change that would move an entity between chunks held until it returns.
        ///
        /// \param Callback The callable.
        template<typename Callable>
        ZY_INLINE void Defer(AnyRef<Callable> Callback)
        {
            Enter();
            Callback();
            Leave();
        }

    protected:

        /// \brief Constructs storage holding nothing but the world entity, as part of a world.
        ///
        /// \param Host The host holding the job service spread walks run on.
        explicit Storage(Ref<ZyEngine::Subsystem::Host> Host);

        /// \brief Marks a walk as started.
        ZY_INLINE void Enter()
        {
            mDeferral.Enter();
        }

        /// \brief Marks a walk as ended, applying what waited once the outermost one is.
        ZY_INLINE void Leave()
        {
            if (mDeferral.Leave() && (!mDeferral.GetCommands().IsEmpty() || mLayout.GetPending()->GetSize()))
            {
                Flush();
            }
        }

        /// \brief Holds a change until the running walk ends.
        ///
        /// \param Kind       The kind of change.
        /// \param Actor      The entity it changes.
        /// \param Identifier The component it is about, or zero.
        /// \param Other      The other entity it involves, or one naming nothing.
        /// \param Payload    The value it carries, taken from \ref Deferral::Allocate, or `nullptr`.
        ZY_INLINE void Queue(Deferral::Operation Kind, Handle Actor, UInt32 Identifier, Handle Other, Ptr<Byte> Payload)
        {
            mDeferral.Queue(Deferral::Command(Kind, Identifier, Actor, Other, Payload));
        }

        /// \brief Checks whether any reader watches a component for some changes.
        ///
        /// \param Identifier The component.
        /// \param Kinds      The changes.
        /// \return `true` if one does, `false` otherwise.
        ZY_INLINE Bool IsWatched(UInt32 Identifier, Pull Kinds) const
        {
            return mLedger.IsWatched(Identifier, Kinds);
        }

        /// \brief Records a change of what an entity sees, and of what its heirs see through it, when a reader watches it.
        ///
        /// \param Identifier The component.
        /// \param Kind       The change, exactly one.
        /// \param Index      The slot of the entity.
        /// \param Value      The value leaving on a removal, or `nullptr`.
        ZY_INLINE void Record(UInt32 Identifier, Pull Kind, UInt32 Index, ConstPtr<Byte> Value)
        {
            if (mLedger.IsWatched(Identifier, Kind))
            {
                ZY_ASSERT(!mDeferral.IsSpreading(), "A spread walk writes nothing a reader watches");
                mLedger.Record(Identifier, Kind, mDirectory.GetHandle(Index), Value, mDirectory[Index].Holder->GetIndex());

                if (Directory::IsArchetype(Index))
                {
                    RecordHeirs(Identifier, Kind, Index, Value);
                }
            }
        }

        /// \brief Records every watched component an entity sees, held or lent, that it did not see before.
        ///
        /// \param Index  The slot of the entity.
        /// \param Before The chunk it sat in before, or `nullptr` for an entity just made.
        ZY_INLINE void RecordGain(UInt32 Index, ConstPtr<Chunk> Before)
        {
            if (mLedger.IsActive())
            {
                AppendGain(Index, Before);
            }
        }

        /// \brief Records that a component written in place changed.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The component.
        void MarkChanged(Handle Actor, UInt32 Identifier);

        /// \brief Places an entity whose slot sits in no chunk at the end of one.
        ///
        /// \param Index  The slot of the entity.
        /// \param Target The chunk.
        ZY_INLINE void Place(UInt32 Index, Ref<Chunk> Target)
        {
            Ref<Directory::Slot> Entry = mDirectory[Index];
            Entry.Holder = AddressOf(Target);
            Entry.Row    = Target.Append(Handle(Index, Entry.Generation));
        }

        /// \brief Places an entity in a chunk, moving it there when it sits in one already.
        ///
        /// \param Index  The slot of the entity.
        /// \param Target The chunk.
        ZY_INLINE void PlaceOrTransfer(UInt32 Index, Ref<Chunk> Target)
        {
            if (mDirectory[Index].Holder)
            {
                Transfer(Index, Target);
            }
            else
            {
                Place(Index, Target);
            }
        }

        /// \brief Moves an entity to another chunk, destroying what it lacks there and leaving new columns unbuilt.
        ///
        /// \param Index  The slot of the entity.
        /// \param Target The chunk.
        void Transfer(UInt32 Index, Ref<Chunk> Target);

        /// \brief Gets the chunk reached by adding a component and everything it brings along.
        ///
        /// \param Source     The chunk an entity sits in.
        /// \param Identifier The component.
        /// \return The chunk, made from the same archetype.
        ZY_INLINE Ptr<Chunk> FindOrCreateAddition(Ref<Chunk> Source, UInt32 Identifier)
        {
            const Ptr<Chunk> Known = Source.GetAddition(Identifier);
            return Known ? Known : CreateAddition(Source, Identifier);
        }

        /// \brief Gets the chunk reached by removing a component.
        ///
        /// \param Source     The chunk an entity sits in.
        /// \param Identifier The component, which the chunk must hold.
        /// \return The chunk, made from the same archetype.
        ZY_INLINE Ptr<Chunk> FindOrCreateRemoval(Ref<Chunk> Source, UInt32 Identifier)
        {
            const Ptr<Chunk> Known = Source.GetRemoval(Identifier);
            return Known ? Known : CreateRemoval(Source, Identifier);
        }

        /// \brief Gets the chunk holding exactly a set of components for one archetype, making it the first time.
        ///
        /// \param Signature The components, sorted.
        /// \param Base      The slot of the archetype its entities are made from, or zero.
        /// \return The chunk.
        Ptr<Chunk> FindOrCreate(ConstSpan<UInt32> Signature, UInt32 Base);

        /// \brief Builds the defaults an entity gained by a move, bar the caller's own, and records them when asked.
        ///
        /// \param Index     The slot of the entity, which sits in its new chunk.
        /// \param Source    The chunk it came from.
        /// \param Except    The component the caller builds, or zero.
        /// \param Recording `true` to record each component the entity came to hold, bar the caller's own.
        void BuildDefaults(UInt32 Index, ConstRef<Chunk> Source, UInt32 Except, Bool Recording);

        /// \brief Adds a component nobody watches by moving the entity and building the value in place.
        ///
        /// \param Index      The slot of the entity, which must not hold the component.
        /// \param Identifier The component.
        /// \param Data       The value, moved into place.
        template<typename Type, typename Value>
        ZY_INLINE void Insert(UInt32 Index, UInt32 Identifier, AnyRef<Value> Data)
        {
            Ref<Chunk>       Source = * mDirectory[Index].Holder;
            const Ptr<Chunk> Target = FindOrCreateAddition(Source, Identifier);
            Transfer(Index, * Target);

            if constexpr (!IsEmpty<Type>)
            {
                const Ptr<Type> Instance
                    = reinterpret_cast<Ptr<Type>>(Target->At(Target->Find(Identifier), mDirectory[Index].Row));
                ::Construct(Instance, Forward<Value>(Data));
            }

            // Most edges bring nothing along.
            if (Target->GetSignature().GetSize() != Source.GetSignature().GetSize() + 1)
            {
                BuildDefaults(Index, Source, Identifier, true);
            }
        }

        /// \brief Destroys an entity and everything beneath it, or holds that until the running walk ends.
        ///
        /// \param Actor The entity, which must be alive and not the world.
        void Destroy(Handle Actor);

        /// \brief Gives an entity a default component unless it holds or inherits one, recording it.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The component.
        void Add(Handle Actor, UInt32 Identifier);

        /// \brief Takes a component an entity holds off it, recording it.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The component.
        void Remove(Handle Actor, UInt32 Identifier);

        /// \brief Gives an entity a component or overwrites the one it holds, recording it, or holds that during a walk.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The component, which must carry data.
        /// \param Fresh      The value, moved out of here whatever happens.
        void Commit(Handle Actor, UInt32 Identifier, Ptr<Byte> Fresh);

        /// \brief Gives an entity its own copy of a component its archetype lends, recording it, or holds that mid-walk.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The component, left alone when the entity holds it or nothing lends it.
        void Override(Handle Actor, UInt32 Identifier);

        /// \brief Reads a component over an entity from the bytes it was written as, adding it when absent.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The component.
        /// \param Input      The bytes, which a value that cannot read itself leaves at its default.
        void Read(Handle Actor, UInt32 Identifier, Ref<Reader> Input);

        /// \brief Wakes an entity or puts it to sleep.
        ///
        /// \param Actor     The entity, which must be alive.
        /// \param Awake     `true` to wake it, `false` to put it to sleep.
        /// \param Recursive `true` to do the same to everything beneath it, all at once when the call ends.
        void SetAwake(Handle Actor, Bool Awake, Bool Recursive);

        /// \brief Makes an entity the last child of another, or takes it from its parent.
        ///
        /// \param Actor  The entity, which must be alive.
        /// \param Parent The new parent, which must not stand beneath it, or one naming nothing alive to detach.
        void Attach(Handle Actor, Handle Parent);

        /// \brief Makes an entity read from another archetype, or from none, keeping what it holds.
        ///
        /// \param Actor     The entity, which must be alive.
        /// \param Archetype The archetype, or a handle naming nothing.
        void SetArchetype(Handle Actor, Handle Archetype);

        /// \brief Finds a child of an entity by its name.
        ///
        /// \param Parent The slot of the entity.
        /// \param Name   The name.
        /// \return The slot of the child, or zero.
        UInt32 FindChild(UInt32 Parent, Text Name) const;

        /// \brief Makes an entity an heir of an archetype, keeping what it holds, with its parts when asked.
        ///
        /// \param Source      The slot of the archetype.
        /// \param AsArchetype `true` to make an archetype too, `false` for an instance.
        /// \param Target      The entity to fill, sitting in no chunk, in the pending one or anywhere else.
        /// \param Parts       `true` to make the archetype's parts beneath it.
        void Propagate(UInt32 Source, Bool AsArchetype, Handle Target, Bool Parts);

        /// \brief Destroys an entity and everything beneath it right away, children first.
        ///
        /// \param Root The slot of the entity.
        void DestroyTree(UInt32 Root);

        /// \brief Rebuilds every chunk keeping a component in a column its registration no longer describes.
        ///
        /// \param Identifier The component, just resolved or about to be unresolved.
        void Rebuild(UInt32 Identifier);

        /// \brief Moves every value of an unresolved type onto the type that now answers to its name.
        ///
        /// \param Unresolved The unresolved type, whose values are the bytes they were saved as.
        /// \param Identifier The type that reads them.
        void Resolve(UInt32 Unresolved, UInt32 Identifier);

    private:

        /// \brief Applies every held change, those they cause included.
        void Flush();

        /// \brief Applies one held change.
        ///
        /// \param Change The change.
        void Apply(ConstRef<Deferral::Command> Change);

        /// \brief Gives an entity a component it does not hold, recording it.
        ///
        /// \param Index      The slot of the entity.
        /// \param Identifier The component.
        /// \param Fresh      The value, moved into place, or `nullptr` to build a default.
        void InsertRecorded(UInt32 Index, UInt32 Identifier, Ptr<Byte> Fresh);

        /// \brief Records every watched component an entity sees that it did not see before, once a reader exists.
        ///
        /// \param Index  The slot of the entity.
        /// \param Before The chunk it sat in before, or `nullptr` for an entity just made.
        void AppendGain(UInt32 Index, ConstPtr<Chunk> Before);

        /// \brief Records a change of what an archetype lends for every heir reading it, heirs of heirs included.
        ///
        /// \param Identifier The component.
        /// \param Kind       The change, exactly one.
        /// \param Archetype  The slot of the archetype.
        /// \param Value      The value leaving on a removal, or `nullptr`.
        void RecordHeirs(UInt32 Identifier, Pull Kind, UInt32 Archetype, ConstPtr<Byte> Value);

        /// \brief Destroys one entity whose children are gone, and gives its slot back.
        ///
        /// \param Index The slot of the entity.
        void DestroyOne(UInt32 Index);

        /// \brief Makes an entity the last child of another right away, copying a part under every heir.
        ///
        /// \param Index  The slot of the entity.
        /// \param Parent The slot of the new parent.
        void SetParent(UInt32 Index, UInt32 Parent);

        /// \brief Takes an entity from its parent right away, destroying its copies when it is a part.
        ///
        /// \param Index The slot of the entity, which must have a parent.
        void ClearParent(UInt32 Index);

        /// \brief Destroys the copies of a part beneath every heir of the archetype it leaves.
        ///
        /// \param Part      The slot of the part.
        /// \param Archetype The slot of the archetype.
        void DestroyPartCopies(UInt32 Part, UInt32 Archetype);

        /// \brief Gives the heirs of a dying archetype what it lent them, and makes them read from its own archetype.
        ///
        /// \param Archetype The slot of the archetype.
        void FlattenHeirs(UInt32 Archetype);

        /// \brief Gets the entities made straight from an archetype.
        ///
        /// \param Archetype The slot of the archetype.
        /// \return The entities, copied out so the caller may change them.
        Sequence<Handle> GetHeirs(UInt32 Archetype) const;

        /// \brief Gets the chunk made the first time a component is added to another chunk's set.
        ///
        /// \param Source     The chunk.
        /// \param Identifier The component.
        /// \return The chunk.
        Ptr<Chunk> CreateAddition(Ref<Chunk> Source, UInt32 Identifier);

        /// \brief Gets the chunk made the first time a component is removed from another chunk's set.
        ///
        /// \param Source     The chunk.
        /// \param Identifier The component.
        /// \return The chunk.
        Ptr<Chunk> CreateRemoval(Ref<Chunk> Source, UInt32 Identifier);

        /// \brief Gets every component an entity sees, held or lent, that a reader watches for a change.
        ///
        /// \param Index The slot of the entity.
        /// \param Kind  The change.
        /// \return The components.
        Sequence<UInt32> GetWatched(UInt32 Index, Pull Kind) const;

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Directory mDirectory;
        Layout    mLayout;
        Deferral  mDeferral;
        Ledger    mLedger;
        
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32    mPrefab;
        UInt32    mAsleep;
        UInt32    mPart;
    };
}