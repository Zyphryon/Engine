// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Storage.hpp"
#include "Zyphryon.Scene/Entity.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Storage::Storage(Ref<ZyEngine::Subsystem::Host> Host)
        : Locator { Host },
          mPrefab { IdentifierOf<Prefab>() },
          mAsleep { IdentifierOf<Asleep>() }
    {
        // Neither marker reaches an entity made from an archetype.
        Registry::Get().SetTrait(mPrefab, Trait::Local, true);
        Registry::Get().SetTrait(mAsleep, Trait::Local, true);

        Place(kWorld.GetIndex(), * mLayout.GetRoot());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Storage::GetWorld() const
    {
        // An entity names its world the way a handle does, so a constant world still hands out its own.
        return Entity(const_cast<Ptr<Storage>>(this), kWorld);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Storage::GetEntity(UInt64 Identifier)
    {
        const Handle Actor(Identifier);
        const UInt32 Index = Actor.GetIndex();

        // A save may name an archetype by its slot alone.
        if (Actor.GetGeneration() == 0 && Directory::IsArchetype(Index) && mDirectory[Index].Holder)
        {
            return Entity(this, mDirectory.GetHandle(Index));
        }

        // A number from outside names an entity only while one lives under it, so reads never look past the slots.
        return mDirectory.IsAlive(Actor) ? Entity(this, Actor) : Entity();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Storage::CreateEntity()
    {
        ZY_ASSERT(!mDeferral.IsSpreading(), "An entity cannot be made while a walk is spread");

        // Made during a walk, it waits where no query sees it until the walk ends.
        const Handle Actor = mDirectory.Allocate(false);
        Place(Actor.GetIndex(), mDeferral.IsWalking() ? * mLayout.GetPending() : * mLayout.GetRoot());
        return Entity(this, Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Storage::CreateArchetype()
    {
        ZY_ASSERT(!mDeferral.IsSpreading(), "An archetype cannot be made while a walk is spread");

        // Placed at once even mid-walk, since walks skip archetypes unless they ask for them.
        const Handle Actor = mDirectory.Allocate(true);
        Place(Actor.GetIndex(), * FindOrCreate(ConstSpan(mPrefab), 0));
        return Entity(this, Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Storage::Instantiate(Entity Archetype)
    {
        ZY_ASSERT(Archetype.IsArchetype(), "Only an archetype can be instantiated");
        ZY_ASSERT(!mDeferral.IsSpreading(), "An entity cannot be made while a walk is spread");

        if (mDeferral.IsWalking())
        {
            const Entity Actor = CreateEntity();
            Queue(Deferral::Operation::Instantiate, Actor.GetHandle(), 0, Archetype.GetHandle(), nullptr);
            return Actor;
        }

        const Handle Actor = mDirectory.Allocate(false);
        Propagate(Archetype.GetHandle().GetIndex(), false, Actor, true);
        return Entity(this, Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Storage::CloneArchetype(Entity Source, Entity Parent)
    {
        ZY_ASSERT(Source.IsArchetype(), "Only an archetype can be cloned");
        ZY_ASSERT(!mDeferral.IsSpreading(), "An archetype cannot be made while a walk is spread");

        // The copy sits in the source's own chunk, every column copied, its parts copied beneath it.
        Sequence<UInt32> Pending;
        Sequence<UInt32> Made;
        Pending.Append(Source.GetHandle().GetIndex());

        for (UInt32 Next = 0; Next < Pending.GetSize(); ++Next)
        {
            const UInt32 Original = Pending[Next];
            const Handle Copy     = mDirectory.Allocate(true);
            Ref<Chunk>   Shape    = * mDirectory[Original].Holder;

            Place(Copy.GetIndex(), Shape);

            // Read after the append, which may have moved the rows of a growing first page.
            const UInt32 From = mDirectory[Original].Row;
            const UInt32 To   = mDirectory[Copy.GetIndex()].Row;

            for (UInt32 Index = 0; Index < Shape.GetColumns().GetSize(); ++Index)
            {
                Chunk::Copy(* Shape.GetColumns()[Index].Info, Shape.At(Index, To), Shape.At(Index, From), 1);
            }

            if (Next > 0)
            {
                mDirectory.Link(Copy.GetIndex(), Made[Pending.Find(mDirectory[Original].Parent)]);
            }
            Made.Append(Copy.GetIndex());

            for (UInt32 Child = mDirectory[Original].First; Child; Child = mDirectory[Child].Next)
            {
                if (mDirectory[Child].Holder->IsArchetype())
                {
                    Pending.Append(Child);
                }
            }
        }

        for (UInt32 Index = static_cast<UInt32>(Made.GetSize()); Index > 0; --Index)
        {
            RecordGain(Made[Index - 1], nullptr);
        }

        if (Parent.IsAlive())
        {
            SetParent(Made[0], Parent.GetHandle().GetIndex());
        }
        return Entity(this, mDirectory.GetHandle(Made[0]));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Purge(Component Type)
    {
        if (mDeferral.IsWalking())
        {
            Queue(Deferral::Operation::Purge, kWorld, Type.GetID(), Handle(), nullptr);
            return;
        }

        // By position, since taking a component off may make chunks.
        for (UInt32 Index = 0; Index < mLayout.GetChunks().GetSize(); ++Index)
        {
            const Ptr<Chunk> Target = mLayout.GetChunk(Index);

            while (Target && Target->Has(Type.GetID()) && Target->GetSize() > 0)
            {
                Remove(Target->GetHandle(Target->GetSize() - 1), Type.GetID());
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::MarkChanged(Handle Actor, UInt32 Identifier)
    {
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");

        ConstRef<Directory::Slot> Entry = mDirectory[Actor.GetIndex()];

        // Only what the entity sees can have changed.
        if (IsWatched(Identifier, Pull::Changed) && mDirectory.FindValue(* Entry.Holder, Entry.Row, Identifier))
        {
            Record(Identifier, Pull::Changed, Actor.GetIndex(), nullptr);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Transfer(UInt32 Index, Ref<Chunk> Target)
    {
        Ref<Chunk> Source = * mDirectory[Index].Holder;

        if (AddressOf(Source) == AddressOf(Target))
        {
            return;
        }

        Ref<Directory::Slot> Entry = mDirectory[Index];

        const UInt32 From = Entry.Row;
        const UInt32 To   = Target.Append(Handle(Index, Entry.Generation));

        // Every column of a row sits in the same page, at the same place in its run.
        const Ptr<Byte> Leaving  = Source.GetPage(From);
        const Ptr<Byte> Arriving = Target.GetPage(To);
        const UInt      Old      = Source.GetPlace(From);
        const UInt      New      = Target.GetPlace(To);

        for (ConstRef<Chunk::Column> Kept : Source.GetColumns())
        {
            const Ptr<Byte> Value = Leaving + Kept.Offset + Old * Kept.Size;

            if (const SInt16 Into = Target.Find(Kept.Identifier); Into >= 0)
            {
                ConstRef<Chunk::Column> Place = Target.GetColumns()[Into];
                Chunk::Relocate(* Kept.Info, Arriving + Place.Offset + New * Place.Size, Value, 1);
            }
            else
            {
                Chunk::Destruct(* Kept.Info, Value, 1);
            }
        }

        if (const Handle Moved = Source.Release(From); Moved.IsValid())
        {
            mDirectory[Moved.GetIndex()].Row = From;
        }

        Entry.Holder = AddressOf(Target);
        Entry.Row    = To;

        // An archetype that moves may lend something else, so what its heirs match changes now.
        if (Source.IsArchetype() || Target.IsArchetype())
        {
            mLayout.UpdateHeirs(Index, mDirectory);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Chunk> Storage::FindOrCreate(ConstSpan<UInt32> Signature, UInt32 Base)
    {
        if (const Ptr<Chunk> Known = mLayout.Find(Signature, Base))
        {
            return Known;
        }

        const Ptr<Chunk> Made = mLayout.Create(Signature, Base, mPrefab, mDirectory);

        // What the set brings along, except what the archetype lends already.
        Sequence<UInt32> Closure(Signature);

        for (UInt32 Next = 0; Next < Closure.GetSize(); ++Next)
        {
            for (const UInt32 Implied : Registry::Get().GetMetatype(Closure[Next]).Implies)
            {
                if (!Closure.Contains(Implied) && !mDirectory.FindLent(Base, Implied))
                {
                    Closure.Append(Implied);
                }
            }
        }

        if (Closure.GetSize() > Signature.GetSize())
        {
            Closure.Sort();
            Made->SetClosure(FindOrCreate(Closure, Base)->GetClosure());
        }
        return Made;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::BuildDefaults(UInt32 Index, ConstRef<Chunk> Source, UInt32 Except, Bool Recording)
    {
        ConstRef<Chunk> Target = * mDirectory[Index].Holder;
        const UInt32    Row    = mDirectory[Index].Row;

        for (UInt32 Column = 0; Column < Target.GetColumns().GetSize(); ++Column)
        {
            ConstRef<Chunk::Column> Gained = Target.GetColumns()[Column];

            if (Gained.Identifier != Except && !Source.Has(Gained.Identifier))
            {
                Chunk::Construct(* Gained.Info, Target.At(Column, Row), 1);
            }
        }

        if (!Recording || !mLedger.IsActive())
        {
            return;
        }

        // Holding what it read from its archetype changes the value it sees rather than adding one.
        for (const UInt32 Identifier : Target.GetSignature())
        {
            if (Identifier != Except && !Source.Has(Identifier))
            {
                const Bool Lent = mDirectory.FindLent(Source.GetBase(), Identifier) != nullptr;
                Record(Identifier, Lent ? Pull::Changed : Pull::Added, Index, nullptr);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Destroy(Handle Actor)
    {
        ZY_ASSERT(Actor != kWorld, "The world entity is never destroyed");
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");

        if (mDeferral.IsWalking())
        {
            Queue(Deferral::Operation::Destroy, Actor, 0, Handle(), nullptr);
            return;
        }
        DestroyTree(Actor.GetIndex());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Add(Handle Actor, UInt32 Identifier)
    {
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");

        if (mDeferral.IsWalking())
        {
            Queue(Deferral::Operation::Add, Actor, Identifier, Handle(), nullptr);
            return;
        }

        ConstRef<Chunk> Source = * mDirectory[Actor.GetIndex()].Holder;

        if (!Source.Has(Identifier) && !mDirectory.FindLent(Source.GetBase(), Identifier))
        {
            InsertRecorded(Actor.GetIndex(), Identifier, nullptr);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Remove(Handle Actor, UInt32 Identifier)
    {
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");

        if (mDeferral.IsWalking())
        {
            Queue(Deferral::Operation::Remove, Actor, Identifier, Handle(), nullptr);
            return;
        }

        const UInt32 Index  = Actor.GetIndex();
        Ref<Chunk>   Source = * mDirectory[Index].Holder;

        if (!Source.Has(Identifier))
        {
            return;
        }

        // An entity dropping its own copy reads its archetype's again, so what it sees changed rather than left.
        if (IsWatched(Identifier, Pull::Changed | Pull::Removed))
        {
            if (mDirectory.FindLent(Source.GetBase(), Identifier))
            {
                Record(Identifier, Pull::Changed, Index, nullptr);
            }
            else
            {
                const ConstPtr<Byte> Value = mDirectory.FindValue(Source, mDirectory[Index].Row, Identifier);
                Record(Identifier, Pull::Removed, Index, Value);
            }
        }
        Transfer(Index, * FindOrCreateRemoval(Source, Identifier));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Commit(Handle Actor, UInt32 Identifier, Ptr<Byte> Fresh)
    {
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");

        const UInt32 Index  = Actor.GetIndex();
        Ref<Chunk>   Source = * mDirectory[Index].Holder;
        const SInt16 Column = Source.Find(Identifier);

        // A move waits for the walk to end, and so does every change after one already waiting.
        if (mDeferral.IsWalking() && (Column < 0 || mDeferral.IsMarked(Index)))
        {
            ConstRef<Metatype> Info    = Registry::Get().GetColumn(Identifier);
            const Ptr<Byte>    Payload = mDeferral.Allocate(Info.GetSize(), Info.GetAlignment());
            Chunk::Relocate(Info, Payload, Fresh, 1);
            Queue(Deferral::Operation::Set, Actor, Identifier, Handle(), Payload);
        }
        else if (Column >= 0)
        {
            const Ptr<Byte> Place = Source.At(Column, mDirectory[Index].Row);
            Chunk::Destruct(* Source.GetColumns()[Column].Info, Place, 1);
            Chunk::Relocate(* Source.GetColumns()[Column].Info, Place, Fresh, 1);

            Record(Identifier, Pull::Changed, Index, nullptr);
        }
        else
        {
            InsertRecorded(Index, Identifier, Fresh);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Override(Handle Actor, UInt32 Identifier)
    {
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");

        // Held until the walk ends, where it looks again at what the archetype lends by then.
        if (mDeferral.IsWalking())
        {
            Queue(Deferral::Operation::Override, Actor, Identifier, Handle(), nullptr);
            return;
        }

        const UInt32    Index  = Actor.GetIndex();
        ConstRef<Chunk> Source = * mDirectory[Index].Holder;

        if (Source.Has(Identifier))
        {
            return;
        }

        const ConstPtr<Byte> Lent = mDirectory.FindLent(Source.GetBase(), Identifier);

        if (!Lent)
        {
            return;
        }

        ConstRef<Metatype> Info = Registry::Get().GetColumn(Identifier);

        if (Info.IsTag())
        {
            InsertRecorded(Index, Identifier, nullptr);
            return;
        }

        // Copied apart first, since the insertion moves the value in from a place of its own.
        const Ptr<Byte> Fresh = Allocate<Byte>(Info.GetSize(), Info.GetAlignment());
        Chunk::Copy(Info, Fresh, Lent, 1);
        InsertRecorded(Index, Identifier, Fresh);
        Free(Fresh, Info.GetAlignment());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Read(Handle Actor, UInt32 Identifier, Ref<Reader> Input)
    {
        ConstRef<Metatype> Info = Registry::Get().GetColumn(Identifier);

        if (Info.IsTag())
        {
            Add(Actor, Identifier);
            return;
        }

        // Built apart and moved in by the commit, which holds it when a walk runs.
        const Ptr<Byte> Fresh = Allocate<Byte>(Info.GetSize(), Info.GetAlignment());
        Chunk::Construct(Info, Fresh, 1);

        if (Info.Load)
        {
            Info.Load(Input, Fresh);
        }
        Commit(Actor, Identifier, Fresh);
        Free(Fresh, Info.GetAlignment());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::SetAwake(Handle Actor, Bool Awake, Bool Recursive)
    {
        if (!Recursive)
        {
            Awake ? Remove(Actor, mAsleep) : Add(Actor, mAsleep);
            return;
        }

        // Held until the end, so the subtree is walked parents first with no stack while nothing moves.
        const UInt32 Root = Actor.GetIndex();
        Enter();

        for (UInt32 Cursor = Root;;)
        {
            const Handle Current = mDirectory.GetHandle(Cursor);

            if (Awake)
            {
                Remove(Current, mAsleep);
            }
            else
            {
                Add(Current, mAsleep);
            }

            if (mDirectory[Cursor].First)
            {
                Cursor = mDirectory[Cursor].First;
                continue;
            }

            while (Cursor != Root && !mDirectory[Cursor].Next)
            {
                Cursor = mDirectory[Cursor].Parent;
            }

            if (Cursor == Root)
            {
                break;
            }
            Cursor = mDirectory[Cursor].Next;
        }
        Leave();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Attach(Handle Actor, Handle Parent)
    {
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");
        ZY_ASSERT(!mDeferral.IsSpreading(), "A spread walk only writes components in place");

        const UInt32 Index    = Actor.GetIndex();
        const UInt32 Above    = mDirectory.IsAlive(Parent) ? Parent.GetIndex() : 0;
        const UInt32 Previous = mDirectory[Index].Parent;

        if (Above == 0 && Previous == 0)
        {
            return;
        }

        for (UInt32 Cursor = Above; Cursor; Cursor = mDirectory[Cursor].Parent)
        {
            ZY_ASSERT(Cursor != Index, "An entity cannot be attached beneath itself");
        }

        // Moving a part copies or destroys entities under every heir, which waits like any structural change.
        const Bool Part = mDirectory[Index].Holder->IsArchetype()
            && ((Above && mDirectory[Above].Holder->IsArchetype())
                || (Previous && mDirectory[Previous].Holder->IsArchetype()));

        if (mDeferral.IsWalking() && Part)
        {
            Queue(Deferral::Operation::Attach, Actor, 0, Above ? Parent : Handle(), nullptr);
        }
        else if (Above)
        {
            SetParent(Index, Above);
        }
        else
        {
            ClearParent(Index);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::SetArchetype(Handle Actor, Handle Archetype)
    {
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");
        ZY_ASSERT(!mDirectory.IsAlive(Archetype) || mDirectory[Archetype.GetIndex()].Holder->IsArchetype(),
            "Only an archetype can be read from");

        if (mDeferral.IsWalking())
        {
            Queue(Deferral::Operation::Rebase, Actor, 0, Archetype, nullptr);
            return;
        }

        const UInt32 Index = Actor.GetIndex();
        const UInt32 Base  = mDirectory.IsAlive(Archetype) ? Archetype.GetIndex() : 0;

        if (mDirectory[Index].Holder->GetBase() == Base)
        {
            return;
        }

        for (UInt32 Cursor = Base; Cursor; Cursor = mDirectory[Cursor].Holder->GetBase())
        {
            ZY_ASSERT(Cursor != Index, "An archetype cannot read from itself");
        }

        // It keeps all it holds and gains what the new archetype copies and its parts, archetypes under an archetype.
        if (Base)
        {
            Propagate(Base, mDirectory[Index].Holder->IsArchetype(), Actor, true);
            return;
        }

        Ref<Chunk> Source = * mDirectory[Index].Holder;
        Transfer(Index, * FindOrCreate(Source.GetSignature(), 0)->GetClosure());
        BuildDefaults(Index, Source, 0, false);
        RecordGain(Index, AddressOf(Source));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Storage::FindChild(UInt32 Parent, Text Name) const
    {
        const UInt32 Identifier = IdentifierOf<Named>();

        for (UInt32 Child = mDirectory[Parent].First; Child; Child = mDirectory[Child].Next)
        {
            ConstRef<Chunk> Holder = * mDirectory[Child].Holder;

            if (const SInt16 Column = Holder.Find(Identifier); Column >= 0)
            {
                const ConstPtr<Byte> Component = Holder.At(Column, mDirectory[Child].Row);

                if (reinterpret_cast<ConstPtr<Named>>(Component)->Value == Name)
                {
                    return Child;
                }
            }
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Propagate(UInt32 Source, Bool AsArchetype, Handle Target, Bool Parts)
    {
        const UInt32     Index = Target.GetIndex();
        const Ptr<Chunk> Shape = mDirectory[Source].Holder;
        const Ptr<Chunk> Held  = mDirectory[Index].Holder;
        const Bool       Bare  = !Held || Held->GetSignature().IsEmpty();

        // A bare copy lands where the last one did while the archetype has not moved since.
        Ptr<Chunk> Destination = Bare ? mLayout.GetCopy(Source, Shape, AsArchetype) : nullptr;

        if (!Destination)
        {
            // What it holds already, plus what the archetype copies, plus what that brings along.
            Sequence<UInt32> Signature;

            if (Held)
            {
                Signature.Append(Held->GetSignature());
            }

            for (const UInt32 Identifier : Shape->GetSignature())
            {
                if ((Shape->GetFlags(Identifier) & Chunk::kCopies) && (!Held || !Held->Has(Identifier)))
                {
                    Signature.Append(Identifier);
                }
            }

            if (AsArchetype && (!Held || !Held->Has(mPrefab)))
            {
                Signature.Append(mPrefab);
            }
            Signature.Sort();

            Destination = FindOrCreate(Signature, Source)->GetClosure();

            if (Bare)
            {
                mLayout.SetCopy(Source, Shape, AsArchetype, Destination);
            }
        }

        PlaceOrTransfer(Index, * Destination);

        // What it held stays, the rest is copied from the archetype or built.
        const UInt32 Row  = mDirectory[Index].Row;
        const UInt32 From = mDirectory[Source].Row;

        for (UInt32 Column = 0; Column < Destination->GetColumns().GetSize(); ++Column)
        {
            ConstRef<Chunk::Column> Made = Destination->GetColumns()[Column];

            if (Held && Held->Has(Made.Identifier))
            {
                continue;
            }

            if (const SInt16 Origin = Shape->Find(Made.Identifier); Origin >= 0)
            {
                Chunk::Copy(* Made.Info, Destination->At(Column, Row), Shape->At(Origin, From), 1);
            }
            else
            {
                Chunk::Construct(* Made.Info, Destination->At(Column, Row), 1);
            }
        }

        for (UInt32 Part = Parts ? mDirectory[Source].First : 0; Part; Part = mDirectory[Part].Next)
        {
            if (mDirectory[Part].Holder->IsArchetype())
            {
                const Handle Child = mDirectory.Allocate(AsArchetype);
                mDirectory.Link(Child.GetIndex(), Index);
                Propagate(Part, AsArchetype, Child, true);
            }
        }

        // Recorded once its parts are beneath it, children first.
        RecordGain(Index, Held);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::DestroyTree(UInt32 Root)
    {
        const UInt32 Ancestor = mDirectory[Root].Parent;

        if (Ancestor && mDirectory[Root].Holder->IsArchetype() && mDirectory[Ancestor].Holder->IsArchetype())
        {
            DestroyPartCopies(Root, Ancestor);
        }

        // Children first, sinking to a leaf, burying it and climbing back, with no stack.
        for (UInt32 Cursor = Root;;)
        {
            while (mDirectory[Cursor].First)
            {
                Cursor = mDirectory[Cursor].First;
            }

            const UInt32 Parent = mDirectory[Cursor].Parent;
            const Bool   Last   = Cursor == Root;

            if (Parent)
            {
                mDirectory.Unlink(Cursor);
            }
            DestroyOne(Cursor);

            if (Last)
            {
                break;
            }
            Cursor = Parent;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Rebuild(UInt32 Identifier)
    {
        ZY_ASSERT(!mDeferral.IsWalking(), "A type cannot be resolved or unresolved while a query walks");

        const ConstPtr<Metatype> Stored = AddressOf(Registry::Get().GetColumn(Identifier));

        // The values are the same data, so no pull list records them.
        for (ConstRef<Unique<Chunk>> Target : mLayout.GetChunks())
        {
            const SInt16 Column = Target ? Target->Find(Identifier) : Chunk::kAbsent;

            const Bool Fitting = Column >= 0 ? Target->GetColumns()[Column].Info == Stored : Stored->IsTag();

            if (Column == Chunk::kAbsent || Fitting)
            {
                continue;
            }
            Target->Rebuild();

            // Columns moved, and an archetype may lend differently now, so every query looks at it again.
            mLayout.Update(AddressOf(* Target), mDirectory);

            for (UInt32 Row = 0; Target->IsArchetype() && Row < Target->GetSize(); ++Row)
            {
                mLayout.UpdateHeirs(Target->GetHandle(Row).GetIndex(), mDirectory);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Resolve(UInt32 Unresolved, UInt32 Identifier)
    {
        // By position, since moving the values over makes chunks, and each entity leaves the chunk it is found in.
        for (UInt32 Index = 0; Index < mLayout.GetChunks().GetSize(); ++Index)
        {
            const Ptr<Chunk> Target = mLayout.GetChunk(Index);
            const SInt16     Column = Target ? Target->Find(Unresolved) : Chunk::kAbsent;

            while (Column >= 0 && Target->GetSize() > 0)
            {
                const UInt32             Row   = Target->GetSize() - 1;
                const Handle             Actor = Target->GetHandle(Row);
                ConstRef<Sequence<Byte>> Bytes = * reinterpret_cast<ConstPtr<Sequence<Byte>>>(Target->At(Column, Row));
                Reader                   Input(Bytes);

                Read(Actor, Identifier, Input);
                Remove(Actor, Unresolved);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Flush()
    {
        mDeferral.SetFlushing(true);

        for (UInt32 Next = 0;;)
        {
            // Entities made meanwhile come out first, before any change naming them.
            for (Ref<Chunk> Pending = * mLayout.GetPending(); Pending.GetSize() > 0;)
            {
                Transfer(Pending.GetHandle(Pending.GetSize() - 1).GetIndex(), * mLayout.GetRoot());
            }

            if (Next == mDeferral.GetCommands().GetSize())
            {
                break;
            }

            // Copied, since applying it may hold more changes and move the list.
            const Deferral::Command Change = mDeferral.GetCommands()[Next++];
            Apply(Change);
        }

        mDeferral.Clear();
        mDeferral.SetFlushing(false);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Apply(ConstRef<Deferral::Command> Change)
    {
        const Bool Alive = mDirectory.IsAlive(Change.Actor);

        switch (Change.Kind)
        {
        case Deferral::Operation::Instantiate:
            if (Alive && mDirectory.IsAlive(Change.Other))
            {
                Propagate(Change.Other.GetIndex(), false, Change.Actor, true);
            }
            break;
        case Deferral::Operation::Add:
            if (Alive)
            {
                Add(Change.Actor, Change.Identifier);
            }
            break;
        case Deferral::Operation::Set:
            if (Alive)
            {
                Commit(Change.Actor, Change.Identifier, Change.Payload);
            }
            else
            {
                Chunk::Destruct(Registry::Get().GetColumn(Change.Identifier), Change.Payload, 1);
            }
            break;
        case Deferral::Operation::Remove:
            if (Alive)
            {
                Remove(Change.Actor, Change.Identifier);
            }
            break;
        case Deferral::Operation::Destroy:
            if (Alive)
            {
                Destroy(Change.Actor);
            }
            break;
        case Deferral::Operation::Attach:
            if (Alive && (!Change.Other.IsValid() || mDirectory.IsAlive(Change.Other)))
            {
                Attach(Change.Actor, Change.Other);
            }
            break;
        case Deferral::Operation::Rebase:
            if (Alive)
            {
                SetArchetype(Change.Actor, Change.Other);
            }
            break;
        case Deferral::Operation::Purge:
            Purge(Component(Change.Identifier));
            break;
        case Deferral::Operation::Override:
            if (Alive)
            {
                Override(Change.Actor, Change.Identifier);
            }
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::InsertRecorded(UInt32 Index, UInt32 Identifier, Ptr<Byte> Fresh)
    {
        Ref<Chunk>       Source = * mDirectory[Index].Holder;
        const Bool       Seen   = mDirectory.FindLent(Source.GetBase(), Identifier) != nullptr;
        const Ptr<Chunk> Target = FindOrCreateAddition(Source, Identifier);
        Transfer(Index, * Target);

        if (const SInt16 Column = Target->Find(Identifier); Column >= 0)
        {
            ConstRef<Metatype> Info  = * Target->GetColumns()[Column].Info;
            const Ptr<Byte>    Place = Target->At(Column, mDirectory[Index].Row);

            if (Fresh)
            {
                Chunk::Relocate(Info, Place, Fresh, 1);
            }
            else
            {
                Chunk::Construct(Info, Place, 1);
            }
        }

        // Most edges bring nothing along.
        if (Target->GetSignature().GetSize() != Source.GetSignature().GetSize() + 1)
        {
            BuildDefaults(Index, Source, Identifier, true);
        }
        Record(Identifier, Seen ? Pull::Changed : Pull::Added, Index, nullptr);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::AppendGain(UInt32 Index, ConstPtr<Chunk> Before)
    {
        for (const UInt32 Identifier : GetWatched(Index, Pull::Added))
        {
            if (!Before || (!Before->Has(Identifier) && !mDirectory.FindLent(Before->GetBase(), Identifier)))
            {
                Record(Identifier, Pull::Added, Index, nullptr);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::RecordHeirs(UInt32 Identifier, Pull Kind, UInt32 Archetype, ConstPtr<Byte> Value)
    {
        ConstRef<Metatype> Info = Registry::Get().GetMetatype(Identifier);

        if (!Info.Has(Trait::Inheritable) || Info.Has(Trait::Local))
        {
            return;
        }

        // An heir holding its own value sees nothing of the archetype's, and neither do its own heirs.
        for (const Ptr<Chunk> Target : mLayout.GetHeirChunks(Archetype))
        {
            for (UInt32 Row = 0; !Target->Has(Identifier) && Row < Target->GetSize(); ++Row)
            {
                Record(Identifier, Kind, Target->GetHandle(Row).GetIndex(), Value);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::DestroyOne(UInt32 Index)
    {
        if (mLedger.IsActive())
        {
            ConstRef<Directory::Slot> Entry = mDirectory[Index];

            // Straight into the lists, since the heirs of a dying archetype keep what it lent them.
            for (const UInt32 Identifier : GetWatched(Index, Pull::Removed))
            {
                const ConstPtr<Byte> Value = mDirectory.FindValue(* Entry.Holder, Entry.Row, Identifier);

                mLedger.Record(Identifier, Pull::Removed, mDirectory.GetHandle(Index), Value);
            }
        }

        if (Directory::IsArchetype(Index))
        {
            FlattenHeirs(Index);
        }

        if (const Handle Moved = mDirectory[Index].Holder->Erase(mDirectory[Index].Row); Moved.IsValid())
        {
            mDirectory[Moved.GetIndex()].Row = mDirectory[Index].Row;
        }
        mDirectory.Release(Index);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::SetParent(UInt32 Index, UInt32 Parent)
    {
        if (mDirectory[Index].Parent)
        {
            ClearParent(Index);
        }

        mDirectory.Link(Index, Parent);

        // A part attached to an archetype reaches every entity already made from it.
        if (mDirectory[Index].Holder->IsArchetype() && mDirectory[Parent].Holder->IsArchetype())
        {
            for (const Handle Heir : GetHeirs(Parent))
            {
                const Bool   Keep = mDirectory[Heir.GetIndex()].Holder->IsArchetype();
                const Handle Copy = mDirectory.Allocate(Keep);
                Propagate(Index, Keep, Copy, true);
                SetParent(Copy.GetIndex(), Heir.GetIndex());
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::ClearParent(UInt32 Index)
    {
        const UInt32 Parent = mDirectory[Index].Parent;

        if (mDirectory[Index].Holder->IsArchetype() && mDirectory[Parent].Holder->IsArchetype())
        {
            DestroyPartCopies(Index, Parent);
        }
        mDirectory.Unlink(Index);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::DestroyPartCopies(UInt32 Part, UInt32 Archetype)
    {
        for (const Handle Heir : GetHeirs(Archetype))
        {
            Sequence<UInt32> Copies;

            for (UInt32 Child = mDirectory[Heir.GetIndex()].First; Child; Child = mDirectory[Child].Next)
            {
                if (mDirectory[Child].Holder->GetBase() == Part)
                {
                    Copies.Append(Child);
                }
            }

            for (const UInt32 Copy : Copies)
            {
                DestroyTree(Copy);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::FlattenHeirs(UInt32 Archetype)
    {
        const Sequence<Ptr<Chunk>> Chunks = mLayout.ExtractHeirs(Archetype);
        ConstRef<Chunk>            Origin = * mDirectory[Archetype].Holder;
        const UInt32               Base   = Origin.GetBase();

        for (const Ptr<Chunk> Source : Chunks)
        {
            // The heirs take as their own what they read from here.
            Sequence<UInt32> Signature(Source->GetSignature());
            Sequence<UInt32> Taken;

            for (const UInt32 Identifier : Origin.GetSignature())
            {
                if ((Origin.GetFlags(Identifier) & Chunk::kLends) && !Source->Has(Identifier))
                {
                    Signature.Append(Identifier);
                    Taken.Append(Identifier);
                }
            }
            Signature.Sort();

            const Ptr<Chunk> Target = FindOrCreate(Signature, Base)->GetClosure();

            while (Source->GetSize() > 0)
            {
                const UInt32 Heir = Source->GetHandle(Source->GetSize() - 1).GetIndex();
                Transfer(Heir, * Target);
                BuildDefaults(Heir, * Source, 0, false);

                for (const UInt32 Identifier : Taken)
                {
                    if (const SInt16 Column = Target->Find(Identifier); Column >= 0)
                    {
                        const Ptr<Byte> Place = Target->At(Column, mDirectory[Heir].Row);
                        const Ptr<Byte> At    = Origin.At(Origin.Find(Identifier), mDirectory[Archetype].Row);

                        Chunk::Destruct(* Target->GetColumns()[Column].Info, Place, 1);
                        Chunk::Copy(* Target->GetColumns()[Column].Info, Place, At, 1);
                    }
                }
            }
            mLayout.Destroy(Source);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sequence<Handle> Storage::GetHeirs(UInt32 Archetype) const
    {
        Sequence<Handle> Heirs;

        for (const Ptr<Chunk> Target : mLayout.GetHeirChunks(Archetype))
        {
            for (UInt32 Row = 0; Row < Target->GetSize(); ++Row)
            {
                Heirs.Append(Target->GetHandle(Row));
            }
        }
        return Heirs;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Chunk> Storage::CreateAddition(Ref<Chunk> Source, UInt32 Identifier)
    {
        ZY_ASSERT(AddressOf(Source) != mLayout.GetPending(), "A pending entity is never changed in place");

        Sequence<UInt32> Signature(Source.GetSignature());
        Signature.Append(Identifier);
        Signature.Sort();

        const Ptr<Chunk> Target = FindOrCreate(Signature, Source.GetBase())->GetClosure();
        Source.SetAddition(Identifier, Target);

        // The way back is the same edge only when nothing came along.
        if (Target->GetSignature().GetSize() == Signature.GetSize())
        {
            Target->SetRemoval(Identifier, AddressOf(Source));
        }
        return Target;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Chunk> Storage::CreateRemoval(Ref<Chunk> Source, UInt32 Identifier)
    {
        Sequence<UInt32> Signature(Source.GetSignature());
        Signature.Erase(Identifier);

        const Ptr<Chunk> Target = FindOrCreate(Signature, Source.GetBase());
        Source.SetRemoval(Identifier, Target);

        if (Source.GetClosure() == AddressOf(Source))
        {
            Target->SetAddition(Identifier, AddressOf(Source));
        }
        return Target;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sequence<UInt32> Storage::GetWatched(UInt32 Index, Pull Kind) const
    {
        Sequence<UInt32> Seen;
        ConstRef<Chunk>  Holder = * mDirectory[Index].Holder;

        for (const UInt32 Identifier : Holder.GetSignature())
        {
            if (IsWatched(Identifier, Kind))
            {
                Seen.Append(Identifier);
            }
        }

        for (UInt32 Base = Holder.GetBase(); Base; Base = mDirectory[Base].Holder->GetBase())
        {
            for (const UInt32 Identifier : mDirectory[Base].Holder->GetSignature())
            {
                const Bool IsWatch = IsWatched(Identifier, Kind);

                if (IsWatch && !Seen.Contains(Identifier) && mDirectory.FindLent(Holder.GetBase(), Identifier))
                {
                    Seen.Append(Identifier);
                }
            }
        }
        return Seen;
    }
}