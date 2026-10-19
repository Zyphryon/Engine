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

        /// \brief Gets the world entity, which holds every singleton.
        ///
        /// \return The world entity, always alive.
        Entity GetWorld() const;

        /// \brief Gets the entity a number names.
        ///
        /// \param Identifier The number, as \ref Entity::GetID gives it, or an archetype's slot with no generation.
        /// \return The entity, or one naming nothing when nothing lives under the number.
        Entity GetEntity(UInt64 Identifier);

        /// \brief Creates an archetype, an entity no walk sees that others are made from.
        ///
        /// \param Base   The archetype it reads from itself, or one naming nothing.
        /// \param Parent The archetype it is a part of, copied beneath each later instance, or one naming nothing.
        /// \return The archetype, placed at once even during a walk.
        Entity CreateArchetype(Entity Base, Entity Parent);

        /// \brief Copies an archetype, everything it holds and every part beneath it, into a new one.
        ///
        /// \param Source The archetype, which must be alive.
        /// \param Parent The archetype the copy becomes a part of, or one naming nothing to leave it alone.
        /// \return The copy, made from what the source was made from.
        Entity CloneArchetype(Entity Source, Entity Parent);

        /// \brief Takes a tag off every entity carrying it, which no walk may be running to ask for.
        template<typename Type>
        ZY_INLINE void Purge()
        {
            Purge(IdentifierOf<Type>());
        }

        /// \brief Runs a callable with every structural change it asks for held until it returns.
        ///
        /// \param Callback The callable, taking nothing.
        template<typename Callable>
        ZY_INLINE void Defer(AnyRef<Callable> Callback)
        {
            Enter();
            Callback();
            Leave();
        }

        /// \brief Runs a task once the running walk ends, in order with the changes it held, or at once outside one.
        ///
        /// \param Callback The task, moved in, which may make entities where a spread walk cannot.
        template<typename Callable>
        ZY_INLINE void Postpone(AnyRef<Callable> Callback)
        {
            if (!mDeferral.IsWalking())
            {
                Callback();
                return;
            }

            using Function = StripAll<Callable>;

            // The task's own call stands first and the task right after it, so no capture is too large for the room.
            constexpr UInt kOffset    = Align(sizeof(Deferral::Task), alignof(Function));
            constexpr UInt kAlignment = Max(alignof(Deferral::Task), alignof(Function));

            const Ptr<Byte> Payload = mDeferral.Allocate(kOffset + sizeof(Function), kAlignment);
            ::Construct(reinterpret_cast<Ptr<Deferral::Task>>(Payload), & Deferral::Run<Function, kOffset>);
            ::Construct(reinterpret_cast<Ptr<Function>>(Payload + kOffset), Forward<Callable>(Callback));
            Queue(Deferral::Operation::Invoke, Handle(), 0, Handle(), Payload, false);
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
        /// \param Mark       `true` to mark its entity, so every change of it waits behind this one, `false` otherwise.
        ZY_INLINE void Queue(
            Deferral::Operation Kind,
            Handle              Actor,
            UInt32              Identifier,
            Handle              Other,
            Ptr<Byte>           Payload,
            Bool                Mark = true)
        {
            mDeferral.Queue(Deferral::Command(Kind, Identifier, Actor, Other, Payload), Mark);
        }

        /// \brief Checks whether any reader watches a component for some changes.
        ///
        /// \param Identifier The identifier of the component to ask about.
        /// \param Kinds      The changes, any one of which counts.
        /// \return `true` if a reader watches any of them, `false` otherwise.
        ZY_INLINE Bool IsWatched(UInt32 Identifier, Pull Kinds) const
        {
            return mLedger.IsWatched(Identifier, Kinds);
        }

        /// \brief Takes the tick writes mark pages with now and moves on to the next, which a reader takes as its end.
        ///
        /// \return The tick taken, never \ref Chunk::kNever.
        ZY_INLINE UInt32 Tick()
        {
            const UInt32 Taken = mTick;
            mTick = (Taken + 1 == Chunk::kNever ? Taken + 2 : Taken + 1);
            return Taken;
        }

        /// \brief Records a change of what an entity sees, and what its heirs see through it, when a reader watches it.
        ///
        /// \param Identifier The identifier of the component that changed.
        /// \param Kind       The change, exactly one.
        /// \param Index      The slot of the entity.
        /// \param Value      The value leaving on a removal, or `nullptr`.
        ZY_INLINE void Record(UInt32 Identifier, Pull Kind, UInt32 Index, ConstPtr<Byte> Value)
        {
            if (mLedger.IsWatched(Identifier, Kind))
            {
                Record(Identifier, Kind, mDirectory.GetHandle(Index), Value);
            }
        }

        /// \brief Records a change like the one taking a slot, for a caller that holds the entity's handle already.
        ///
        /// \param Identifier The identifier of the component that changed.
        /// \param Kind       The change, exactly one.
        /// \param Actor      The entity, which must be alive.
        /// \param Value      The value leaving on a removal, or `nullptr`.
        ZY_INLINE void Record(UInt32 Identifier, Pull Kind, Handle Actor, ConstPtr<Byte> Value)
        {
            if (!mLedger.IsWatched(Identifier, Kind))
            {
                return;
            }

            // A spread walk only writes in place, and its record waits with its other changes.
            if (mDeferral.IsSpreading())
            {
                Queue(Deferral::Operation::Notify, Actor, Identifier, Handle(), nullptr, false);
                return;
            }

            // Only a removal keeps the chunk the entity left.
            const UInt32 Index  = Actor.GetIndex();
            const UInt32 Source = (Kind == Pull::Removed ? mDirectory[Index].Holder->GetIndex() : 0);
            mLedger.Record(Identifier, Kind, Actor, Value, Source);

            if (Directory::IsArchetype(Index))
            {
                RecordHeirs(Identifier, Kind, Index, Value);
            }
        }

        /// \brief Records a change of what an archetype lends for every heir reading it.
        ///
        /// \param Identifier The identifier of the component that changed.
        /// \param Kind       The change, exactly one.
        /// \param Archetype  The slot of the archetype.
        /// \param Value      The value leaving on a removal, or `nullptr`.
        void RecordHeirs(UInt32 Identifier, Pull Kind, UInt32 Archetype, ConstPtr<Byte> Value);

        /// \brief Records every watched component an entity sees, held or lent, that it did not see before.
        ///
        /// \param Index  The slot of the entity.
        /// \param Before The chunk it sat in before, or `nullptr` for an entity just made.
        void Gain(UInt32 Index, ConstPtr<Chunk> Before);

        /// \brief Records that a component written in place changed.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The identifier of the component written.
        void Notify(Handle Actor, UInt32 Identifier);

        /// \brief Marks every page a walk handed a field out writable in, for readers of those components.
        ///
        /// \param State The selection walked, whose chunks have not moved since the walk.
        void Mark(ConstRef<Selection> State);

        /// \brief Makes an entity holding nothing, which waits outside every walk until the running one ends.
        ///
        /// \return The handle of the entity.
        Handle Create();

        /// \brief Places an entity whose slot sits in no chunk at the end of one.
        ///
        /// \param Index  The slot of the entity.
        /// \param Target The chunk to place it in.
        ZY_INLINE void Place(UInt32 Index, Ref<Chunk> Target)
        {
            Ref<Directory::Slot> Entry = mDirectory[Index];
            Entry.Holder = AddressOf(Target);
            Entry.Row    = Target.Append(Handle(Index, Entry.Generation));
        }

        /// \brief Moves an entity to another chunk, destroying what it lacks there and leaving new columns unbuilt.
        ///
        /// \param Index  The slot of the entity.
        /// \param Target The chunk to move it to.
        void Transfer(UInt32 Index, Ref<Chunk> Target);

        /// \brief Gets the chunk reached by adding a component and everything it brings along.
        ///
        /// \param Source     The chunk an entity sits in.
        /// \param Identifier The identifier of the component added.
        /// \return The chunk, made from the same archetype.
        ZY_INLINE Ptr<Chunk> Widen(Ref<Chunk> Source, UInt32 Identifier)
        {
            const Ptr<Chunk> Known = Source.GetAddition(Identifier);
            return Known ? Known : CreateAddition(Source, Identifier);
        }

        /// \brief Gets the chunk reached by removing a component.
        ///
        /// \param Source     The chunk an entity sits in.
        /// \param Identifier The component, which the chunk must hold.
        /// \return The chunk, made from the same archetype.
        ZY_INLINE Ptr<Chunk> Narrow(Ref<Chunk> Source, UInt32 Identifier)
        {
            const Ptr<Chunk> Known = Source.GetRemoval(Identifier);
            return Known ? Known : CreateRemoval(Source, Identifier);
        }

        /// \brief Gets the chunk an entity lands in once it holds a component, its own when it holds one already.
        ///
        /// \param Source     The chunk an entity sits in.
        /// \param Identifier The identifier of the component, held already or added.
        /// \return The chunk, made from the same archetype.
        ZY_INLINE Ptr<Chunk> Extend(Ref<Chunk> Source, UInt32 Identifier)
        {
            return Source.Has(Identifier) ? AddressOf(Source) : Widen(Source, Identifier);
        }

        /// \brief Gets the chunk holding exactly a set of components for one archetype, making it the first time.
        ///
        /// \param Signature The components the chunk holds, sorted.
        /// \param Base      The slot of the archetype its entities are made from, or zero.
        /// \return The chunk, never `nullptr`.
        Ptr<Chunk> Obtain(ConstSpan<UInt32> Signature, UInt32 Base);

        /// \brief Gets the chunk an entity made from an archetype lands in, remembered for the next one.
        ///
        /// \param Source      The slot of the archetype.
        /// \param AsArchetype `true` for an archetype made from it, `false` for an instance.
        /// \return The chunk, never `nullptr`.
        Ptr<Chunk> FindHeir(UInt32 Source, Bool AsArchetype);

        /// \brief Finishes an entity placed in its final chunk, building what it was not given and copying the parts.
        ///
        /// \param Index     The slot of the entity, which sits in its final chunk.
        /// \param Archetype The slot of the archetype it is made from, or zero.
        /// \param Parent    The slot of the entity it hangs from, or zero.
        /// \param Given     The identifiers of the components the caller built in place.
        void Settle(UInt32 Index, UInt32 Archetype, UInt32 Parent, ConstSpan<UInt32> Given);

        /// \brief Builds every column an entity placed in a new chunk was not given, copying the archetype's values.
        ///
        /// \param Index     The slot of the entity, whose columns are unbuilt.
        /// \param Archetype The slot of the archetype the values are copied from, or zero to build them all.
        /// \param Given     The identifiers of the components the caller built in place.
        void Fill(UInt32 Index, UInt32 Archetype, ConstSpan<UInt32> Given);

        /// \brief Adds a component nobody watches by moving the entity and building the value in place.
        ///
        /// \param Index      The slot of the entity, which must not hold the component.
        /// \param Identifier The identifier of the component to add.
        /// \param Data       The value, moved into place.
        template<typename Type, typename Value>
        ZY_INLINE void Insert(UInt32 Index, UInt32 Identifier, AnyRef<Value> Data)
        {
            // Kept apart from the general move, since a first write of a component is hot and pays for a call.
            Ref<Chunk>       Source = * mDirectory[Index].Holder;
            const Ptr<Chunk> Target = Widen(Source, Identifier);
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
                Build(Index, Source, Identifier, true);
            }
        }

        /// \brief Destroys an entity and everything beneath it, or holds that until the running walk ends.
        ///
        /// \param Actor The entity, which must be alive and not the world.
        void Destroy(Handle Actor);

        /// \brief Gives an entity a default component unless it holds or inherits one, recording it.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The identifier of the component to give.
        void Add(Handle Actor, UInt32 Identifier);

        /// \brief Takes a component an entity holds off it, recording it.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The identifier of the component to take off.
        void Remove(Handle Actor, UInt32 Identifier);

        /// \brief Gives an entity a component or overwrites its own, recording it, or holds that during a walk.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The component, which must carry data.
        /// \param Fresh      The value, moved out of here whatever happens.
        void Commit(Handle Actor, UInt32 Identifier, Ptr<Byte> Fresh);

        /// \brief Gives an entity its own value of a component, copying the inherited one or building a default.
        ///
        /// \param Actor      The entity, which must be alive, outside any walk.
        /// \param Identifier The identifier of the component, never a tag.
        /// \return The value the entity holds.
        Ptr<Byte> Ensure(Handle Actor, UInt32 Identifier);

        /// \brief Reads a component over an entity from the bytes it was written as, adding it when absent.
        ///
        /// \param Actor      The entity, which must be alive.
        /// \param Identifier The identifier of the component to read.
        /// \param Input      The bytes, which a value that cannot read itself leaves at its default.
        void Read(Handle Actor, UInt32 Identifier, Ref<Reader> Input);

        /// \brief Makes an entity the last child of another, or takes it from its parent.
        ///
        /// \param Actor  The entity, which must be alive.
        /// \param Parent The new parent, which must not stand beneath it, or one naming nothing alive to detach.
        void Attach(Handle Actor, Handle Parent);

        /// \brief Finds a child of an entity by its name.
        ///
        /// \param Parent The slot of the parent.
        /// \param Name   The \ref Named value the child carries.
        /// \return The slot of the child, or zero.
        UInt32 FindChild(UInt32 Parent, Text Name) const;

        /// \brief Gets the entities made straight from an archetype.
        ///
        /// \param Archetype The slot of the archetype.
        /// \return The entities, copied out so the caller may change them.
        Sequence<Handle> GetHeirs(UInt32 Archetype) const;

        /// \brief Destroys an entity and everything beneath it right away, children first.
        ///
        /// \param Root The slot of the entity.
        void Prune(UInt32 Root);

        /// \brief Rebuilds every chunk keeping a component in a column its registration no longer describes.
        ///
        /// \param Identifier The component, just declared or about to lose its code.
        void Rebuild(UInt32 Identifier);

        /// \brief Moves every value a save kept as bytes onto the type that now answers to its name.
        ///
        /// \param Unresolved The type kept as bytes.
        /// \param Identifier The type that reads them.
        void Resolve(UInt32 Unresolved, UInt32 Identifier);

        /// \brief Takes a component off every entity holding it.
        ///
        /// \param Identifier The identifier of the component to take off.
        void Purge(UInt32 Identifier);

        /// \brief Builds the defaults an entity gained by a move, bar the caller's own, and records them when asked.
        ///
        /// \param Index     The slot of the entity, which sits in its new chunk.
        /// \param Source    The chunk it came from.
        /// \param Except    The component the caller builds, or zero.
        /// \param Recording `true` to record each component the entity came to hold, bar the caller's own.
        void Build(UInt32 Index, ConstRef<Chunk> Source, UInt32 Except, Bool Recording);

    private:

        /// \brief Applies every held change, those they cause included.
        void Flush();

        /// \brief Applies one held change.
        ///
        /// \param Change The change to apply.
        void Apply(ConstRef<Deferral::Command> Change);

        /// \brief Moves an entity along the edge adding a component, building what came along but not the component.
        ///
        /// \param Index      The slot of the entity, which must not hold the component.
        /// \param Identifier The identifier of the component to add.
        /// \return The unbuilt place of the component's value, or `nullptr` for a tag.
        Ptr<Byte> Grow(UInt32 Index, UInt32 Identifier);

        /// \brief Gives an entity a component it does not hold, recording it.
        ///
        /// \param Index      The slot of the entity.
        /// \param Identifier The identifier of the component to give.
        /// \param Fresh      The value, moved into place, or `nullptr` to build a default.
        void Give(UInt32 Index, UInt32 Identifier, Ptr<Byte> Fresh);

        /// \brief Makes a copy of every part of an archetype beneath an entity.
        ///
        /// \param Source The slot of the archetype.
        /// \param Index  The slot of the entity the copies hang from, which decides whether they are archetypes too.
        void Copy(UInt32 Source, UInt32 Index);

        /// \brief Lists what a walk marked written of the rows a row's leaving moves, before they leave their pages.
        ///
        /// \param Holder  The chunk.
        /// \param Row     The row about to be taken out, which the last row fills.
        /// \param Target  The chunk the row's entity moves to, or `nullptr` when it is destroyed.
        /// \param Landing The row it takes there, ignored without a chunk to move to.
        void Carry(ConstRef<Chunk> Holder, UInt32 Row, ConstPtr<Chunk> Target, UInt32 Landing);

        /// \brief Destroys one entity whose children are gone, its heirs first when it is an archetype.
        ///
        /// \param Index The slot of the entity.
        void Erase(UInt32 Index);

        /// \brief Makes an entity the last child of another right away, marking it a part under an archetype.
        ///
        /// \param Index  The slot of the entity.
        /// \param Parent The slot of the new parent.
        void SetParent(UInt32 Index, UInt32 Parent);

        /// \brief Takes an entity from its parent right away.
        ///
        /// \param Index The slot of the entity, which must have a parent.
        void ClearParent(UInt32 Index);

        /// \brief Checks whether an entity is a part, an archetype hanging under an archetype.
        ///
        /// \param Index  The slot of the entity.
        /// \param Parent The slot of the parent, or zero for none.
        /// \return `true` if both it and its parent are archetypes, `false` otherwise.
        ZY_INLINE Bool IsPart(UInt32 Index, UInt32 Parent) const
        {
            return Parent && Directory::IsArchetype(Index) && Directory::IsArchetype(Parent);
        }

        /// \brief Gets the chunk made the first time a component is added to another chunk's set.
        ///
        /// \param Source     The chunk an entity sits in.
        /// \param Identifier The identifier of the component added.
        /// \return The chunk, which the source remembers from now on.
        Ptr<Chunk> CreateAddition(Ref<Chunk> Source, UInt32 Identifier);

        /// \brief Gets the chunk made the first time a component is removed from another chunk's set.
        ///
        /// \param Source     The chunk an entity sits in.
        /// \param Identifier The identifier of the component removed.
        /// \return The chunk, which the source remembers from now on.
        Ptr<Chunk> CreateRemoval(Ref<Chunk> Source, UInt32 Identifier);

        /// \brief Hands a callback every watched component an entity in a chunk sees through its archetype alone.
        ///
        /// \param Holder   The chunk the entity sits in.
        /// \param Kinds    The changes, any one of which a reader must watch for.
        /// \param Callback The callable, taking the identifier and the value lent, \ref Directory::kPresent for a tag.
        template<typename Callable>
        ZY_INLINE void Lend(ConstRef<Chunk> Holder, Pull Kinds, AnyRef<Callable> Callback) const
        {
            const UInt32 First = Holder.GetBase();

            for (UInt32 Base = First; Base; Base = mDirectory[Base].Holder->GetBase())
            {
                for (const UInt32 Identifier : mDirectory[Base].Holder->GetSignature())
                {
                    if (Holder.Has(Identifier) || !IsWatched(Identifier, Kinds) || !IsNearest(First, Base, Identifier))
                    {
                        continue;
                    }

                    if (const ConstPtr<Byte> Lent = mDirectory.FindLent(First, Identifier))
                    {
                        Callback(Identifier, Lent);
                    }
                }
            }
        }

        /// \brief Checks whether an archetype is the nearest one up a chain holding a component.
        ///
        /// \param First      The slot the chain starts at.
        /// \param Base       The slot of the archetype, which stands on the chain.
        /// \param Identifier The identifier of the component looked for up the chain.
        /// \return `true` if no archetype before it on the chain holds the component, `false` otherwise.
        ZY_INLINE Bool IsNearest(UInt32 First, UInt32 Base, UInt32 Identifier) const
        {
            for (UInt32 Above = First; Above != Base; Above = mDirectory[Above].Holder->GetBase())
            {
                if (mDirectory[Above].Holder->Has(Identifier))
                {
                    return false;
                }
            }
            return true;
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Directory mDirectory;
        Layout    mLayout;
        Deferral  mDeferral;
        Ledger    mLedger;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32    mTick;
    };
}