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
          mTick   { 1 }
    {
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
        if (Actor.GetGeneration() == 0 && Directory::IsArchetype(Index) && mDirectory.IsHeld(Index))
        {
            return Entity(this, mDirectory.GetHandle(Index));
        }

        // A number from outside names an entity only while one lives under it, so reads never look past the slots.
        return mDirectory.IsAlive(Actor) ? Entity(this, Actor) : Entity();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Storage::CreateArchetype(Entity Base, Entity Parent)
    {
        ZY_ASSERT(!mDeferral.IsSpreading(), "An archetype cannot be made while a walk is spread; Postpone the making");
        ZY_ASSERT(!Base.IsAlive() || Base.IsArchetype(), "An archetype reads only from another archetype");

        // Placed at once even mid-walk, since walks pass archetypes over unless they ask for them.
        const UInt32 From  = Base.IsAlive() ? Handle(Base.GetID()).GetIndex() : 0;
        const UInt32 Above = Parent.IsAlive() ? Handle(Parent.GetID()).GetIndex() : 0;
        const Handle Actor = mDirectory.Allocate(true);
        const UInt32 Marker = IdentifierOf<Prefab>();

        Place(Actor.GetIndex(), From ? * FindHeir(From, true) : * Obtain(ConstSpan(Marker), 0));
        Settle(Actor.GetIndex(), From, Above, ConstSpan<UInt32>());
        return Entity(this, Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Storage::CloneArchetype(Entity Source, Entity Parent)
    {
        ZY_ASSERT(Source.IsArchetype(), "Only an archetype can be cloned");
        ZY_ASSERT(!mDeferral.IsSpreading(), "An archetype cannot be made while a walk is spread; Postpone the making");

        // Each copy sits in its source's own chunk with every value copied, parents before what stands beneath.
        Sequence<UInt32> Pending;
        Sequence<UInt32> Made;
        Pending.Append(Handle(Source.GetID()).GetIndex());

        for (UInt32 Next = 0; Next < Pending.GetSize(); ++Next)
        {
            const UInt32 Original = Pending[Next];
            const Handle Copy     = mDirectory.Allocate(true);

            Place(Copy.GetIndex(), * mDirectory[Original].Holder);
            Fill(Copy.GetIndex(), Original, ConstSpan<UInt32>());

            if (Next > 0)
            {
                mDirectory.Link(Copy.GetIndex(), Made[Pending.Find(mDirectory.GetParent(Original))]);
            }
            Made.Append(Copy.GetIndex());

            for (UInt32 Child = mDirectory.GetFirst(Original); Child; Child = mDirectory.GetNext(Child))
            {
                if (Directory::IsArchetype(Child))
                {
                    Pending.Append(Child);
                }
            }
        }

        for (UInt32 Index = static_cast<UInt32>(Made.GetSize()); Index > 0; --Index)
        {
            Gain(Made[Index - 1], nullptr);
        }

        // The copies beneath are parts like their sources, while the copy itself is one only where it is put.
        const Handle Root = mDirectory.GetHandle(Made[0]);

        if (Parent.IsAlive())
        {
            SetParent(Root.GetIndex(), Handle(Parent.GetID()).GetIndex());
        }
        else if (mDirectory[Root.GetIndex()].Holder->Has(IdentifierOf<Part>()))
        {
            Remove(Root, IdentifierOf<Part>());
        }
        return Entity(this, Root);
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
        for (const Ptr<Chunk> Target : mLayout.GetHeirs(Archetype))
        {
            for (UInt32 Row = 0; !Target->Has(Identifier) && Row < Target->GetSize(); ++Row)
            {
                Record(Identifier, Kind, Target->GetHandle(Row).GetIndex(), Value);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Gain(UInt32 Index, ConstPtr<Chunk> Before)
    {
        if (!mLedger.IsActive())
        {
            return;
        }

        ConstRef<Chunk> Holder = * mDirectory[Index].Holder;

        const auto Note = [this, Index, Before](UInt32 Identifier)
        {
            if (!Before || !mDirectory.Sees(* Before, Identifier))
            {
                Record(Identifier, Pull::Added, Index, nullptr);
            }
        };

        for (const UInt32 Identifier : Holder.GetSignature())
        {
            if (IsWatched(Identifier, Pull::Added))
            {
                Note(Identifier);
            }
        }

        Lend(Holder, Pull::Added, [&Note](UInt32 Identifier, ConstPtr<Byte>)
        {
            Note(Identifier);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Notify(Handle Actor, UInt32 Identifier)
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

    void Storage::Mark(ConstRef<Selection> State)
    {
        for (const UInt32 Identifier : State.Writes)
        {
            if (!IsWatched(Identifier, Pull::Changed))
            {
                continue;
            }

            // An entity reading its archetype's value was not written, so only chunks holding their own are marked.
            for (const Ptr<Chunk> Target : State.Chunks)
            {
                if (const SInt16 Column = Target->Find(Identifier); Column >= 0)
                {
                    Target->Mark(Column, mTick);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Handle Storage::Create()
    {
        ZY_ASSERT(!mDeferral.IsSpreading(), "An entity cannot be made while a walk is spread; Postpone the making");

        // Made during a walk, it waits where no query sees it until the walk ends.
        const Handle Actor = mDirectory.Allocate(false);
        Place(Actor.GetIndex(), mDeferral.IsWalking() ? * mLayout.GetPending() : * mLayout.GetRoot());
        return Actor;
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

        if (mLedger.IsActive())
        {
            Carry(Source, From, AddressOf(Target), To);
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

    Ptr<Chunk> Storage::Obtain(ConstSpan<UInt32> Signature, UInt32 Base)
    {
        if (const Ptr<Chunk> Known = mLayout.Find(Signature, Base))
        {
            return Known;
        }

        const Ptr<Chunk> Made = mLayout.Create(Signature, Base, mDirectory);

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
            Made->SetClosure(Obtain(Closure, Base)->GetClosure());
        }
        return Made;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Chunk> Storage::FindHeir(UInt32 Source, Bool AsArchetype)
    {
        const Ptr<Chunk> Shape = mDirectory[Source].Holder;

        // Made where the last one landed while the archetype has not moved since.
        if (const Ptr<Chunk> Known = mLayout.GetCopy(Source, Shape, AsArchetype))
        {
            return Known;
        }

        // What the archetype copies, plus the marker of an archetype, plus what that brings along.
        Sequence<UInt32> Signature;

        for (const UInt32 Identifier : Shape->GetSignature())
        {
            if (Shape->GetFlags(Identifier) & Chunk::kCopies)
            {
                Signature.Append(Identifier);
            }
        }

        if (AsArchetype)
        {
            Signature.Append(IdentifierOf<Prefab>());
        }
        Signature.Sort();

        const Ptr<Chunk> Destination = Obtain(Signature, Source)->GetClosure();
        mLayout.SetCopy(Source, Shape, AsArchetype, Destination);
        return Destination;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Settle(UInt32 Index, UInt32 Archetype, UInt32 Parent, ConstSpan<UInt32> Given)
    {
        // A plain entity given every component it holds has nothing left to build.
        if (Archetype || mDirectory[Index].Holder->GetSignature().GetSize() != Given.GetSize())
        {
            Fill(Index, Archetype, Given);
        }

        if (Archetype)
        {
            Copy(Archetype, Index);
        }
        Gain(Index, nullptr);

        if (Parent)
        {
            SetParent(Index, Parent);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Fill(UInt32 Index, UInt32 Archetype, ConstSpan<UInt32> Given)
    {
        Ref<Chunk>       Target = * mDirectory[Index].Holder;
        const UInt32     Row    = mDirectory[Index].Row;
        const Ptr<Chunk> Shape  = Archetype ? mDirectory[Archetype].Holder : nullptr;

        // What it was not given is copied from the archetype or built.
        for (UInt32 Column = 0; Column < Target.GetColumns().GetSize(); ++Column)
        {
            ConstRef<Chunk::Column> Made = Target.GetColumns()[Column];

            if (ZyBase::Find(Given.GetData(), Given.GetSize(), Made.Identifier))
            {
                continue;
            }

            const SInt16 Origin = Shape ? Shape->Find(Made.Identifier) : Chunk::kAbsent;

            if (Origin >= 0)
            {
                Chunk::Copy(* Made.Info, Target.At(Column, Row), Shape->At(Origin, mDirectory[Archetype].Row), 1);
            }
            else
            {
                Chunk::Construct(* Made.Info, Target.At(Column, Row), 1);
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
        Prune(Actor.GetIndex());
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

        if (!mDirectory.Sees(* mDirectory[Actor.GetIndex()].Holder, Identifier))
        {
            Give(Actor.GetIndex(), Identifier, nullptr);
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
        Transfer(Index, * Narrow(Source, Identifier));
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
            Give(Index, Identifier, Fresh);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> Storage::Ensure(Handle Actor, UInt32 Identifier)
    {
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");
        ZY_ASSERT(!mDeferral.IsWalking(), "A component cannot be ensured while a query walks");

        const UInt32       Index = Actor.GetIndex();
        ConstRef<Metatype> Info  = Registry::Get().GetColumn(Identifier);

        ZY_ASSERT(!Info.IsTag(), "A tag holds no value to ensure");

        if (!mDirectory[Index].Holder->Has(Identifier))
        {
            const ConstPtr<Byte> Lent = mDirectory.FindLent(mDirectory[Index].Holder->GetBase(), Identifier);

            if (Lent)
            {
                // Copied apart first, since the insertion moves the value in from a place of its own.
                const Ptr<Byte> Fresh = Allocate<Byte>(Info.GetSize(), Info.GetAlignment());
                Chunk::Copy(Info, Fresh, Lent, 1);
                Give(Index, Identifier, Fresh);
                Free(Fresh, Info.GetAlignment());
            }
            else
            {
                Give(Index, Identifier, nullptr);
            }
        }

        ConstRef<Directory::Slot> Entry = mDirectory[Index];
        return Entry.Holder->At(Entry.Holder->Find(Identifier), Entry.Row);
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

        const UInt32 Index  = Actor.GetIndex();
        Ref<Chunk>   Source = * mDirectory[Index].Holder;
        const SInt16 Column = Source.Find(Identifier);

        if (mDeferral.IsWalking() && (Column < 0 || mDeferral.IsMarked(Index)))
        {
            const Ptr<Byte> Payload = mDeferral.Allocate(Info.GetSize(), Info.GetAlignment());
            Chunk::Construct(Info, Payload, 1);

            if (Info.Load)
            {
                Info.Load(Input, Payload);
            }
            Queue(Deferral::Operation::Set, Actor, Identifier, Handle(), Payload);
            return;
        }

        if (Column < 0)
        {
            Give(Index, Identifier, nullptr);
        }
        else
        {
            Chunk::Destruct(Info, Source.At(Column, mDirectory[Index].Row), 1);
            Chunk::Construct(Info, Source.At(Column, mDirectory[Index].Row), 1);
            Record(Identifier, Pull::Changed, Index, nullptr);
        }

        if (Info.Load)
        {
            Ref<Chunk> Target = * mDirectory[Index].Holder;
            Info.Load(Input, Target.At(Target.Find(Identifier), mDirectory[Index].Row));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Attach(Handle Actor, Handle Parent)
    {
        ZY_ASSERT(mDirectory.IsAlive(Actor), "The entity is not alive");

        const UInt32 Index    = Actor.GetIndex();
        const UInt32 Above    = mDirectory.IsAlive(Parent) ? Parent.GetIndex() : 0;
        const UInt32 Previous = mDirectory.GetParent(Index);

        if (Above == 0 && Previous == 0)
        {
            return;
        }

        for (UInt32 Cursor = Above; Cursor; Cursor = mDirectory.GetParent(Cursor))
        {
            ZY_ASSERT(Cursor != Index, "An entity cannot be attached beneath itself");
        }

        // The hierarchy is no chunk, so only a spread walk, whose workers read it, holds the move.
        if (mDeferral.IsSpreading())
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

    UInt32 Storage::FindChild(UInt32 Parent, Text Name) const
    {
        const UInt32 Identifier = IdentifierOf<Named>();

        for (UInt32 Child = mDirectory.GetFirst(Parent); Child; Child = mDirectory.GetNext(Child))
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

    Sequence<Handle> Storage::GetHeirs(UInt32 Archetype) const
    {
        Sequence<Handle> Heirs;

        for (const Ptr<Chunk> Target : mLayout.GetHeirs(Archetype))
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

    void Storage::Prune(UInt32 Root)
    {
        // Children first, sinking to a leaf, burying it and climbing back, with no stack.
        for (UInt32 Cursor = Root;;)
        {
            while (mDirectory.GetFirst(Cursor))
            {
                Cursor = mDirectory.GetFirst(Cursor);
            }

            const UInt32 Parent = mDirectory.GetParent(Cursor);
            const Bool   Last   = Cursor == Root;

            if (Parent)
            {
                mDirectory.Unlink(Cursor);
            }
            Erase(Cursor);

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

        // The values are the same data, so no reader is told.
        for (ConstRef<Unique<Chunk>> Target : mLayout.GetChunks())
        {
            const SInt16 Column  = Target ? Target->Find(Identifier) : Chunk::kAbsent;
            const Bool   Fitting = Column >= 0 ? Target->GetColumns()[Column].Info == Stored : Stored->IsTag();

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

    void Storage::Purge(UInt32 Identifier)
    {
        ZY_ASSERT(!mDeferral.IsWalking(), "A component cannot be purged while a query walks");

        // By position, since taking a component off may make chunks.
        for (UInt32 Index = 0; Index < mLayout.GetChunks().GetSize(); ++Index)
        {
            const Ptr<Chunk> Target = mLayout.GetChunk(Index);

            while (Target && Target->Has(Identifier) && Target->GetSize() > 0)
            {
                Remove(Target->GetHandle(Target->GetSize() - 1), Identifier);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Build(UInt32 Index, ConstRef<Chunk> Source, UInt32 Except, Bool Recording)
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
        case Deferral::Operation::Notify:
            if (Alive)
            {
                Notify(Change.Actor, Change.Identifier);
            }
            break;
        case Deferral::Operation::Invoke:
            (* reinterpret_cast<Ptr<Deferral::Task>>(Change.Payload))(Change.Payload);
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> Storage::Grow(UInt32 Index, UInt32 Identifier)
    {
        Ref<Chunk>       Source = * mDirectory[Index].Holder;
        const Ptr<Chunk> Target = Widen(Source, Identifier);
        Transfer(Index, * Target);

        if (Target->GetSignature().GetSize() != Source.GetSignature().GetSize() + 1)
        {
            Build(Index, Source, Identifier, true);
        }

        const SInt16 Column = Target->Find(Identifier);
        return Column >= 0 ? Target->At(Column, mDirectory[Index].Row) : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Give(UInt32 Index, UInt32 Identifier, Ptr<Byte> Fresh)
    {
        const UInt32 Base = mDirectory[Index].Holder->GetBase();
        const Bool   Seen = mDirectory.FindLent(Base, Identifier) != nullptr;

        if (const Ptr<Byte> Place = Grow(Index, Identifier))
        {
            ConstRef<Metatype> Info = Registry::Get().GetColumn(Identifier);

            if (Fresh)
            {
                Chunk::Relocate(Info, Place, Fresh, 1);
            }
            else
            {
                Chunk::Construct(Info, Place, 1);
            }
        }
        Record(Identifier, Seen ? Pull::Changed : Pull::Added, Index, nullptr);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Copy(UInt32 Source, UInt32 Index)
    {
        // A part's copy is an archetype beneath an archetype, an instance beneath anything else.
        const Bool AsArchetype = Directory::IsArchetype(Index);

        for (UInt32 Part = mDirectory.GetFirst(Source); Part; Part = mDirectory.GetNext(Part))
        {
            if (Directory::IsArchetype(Part))
            {
                const Handle Twin = mDirectory.Allocate(AsArchetype);
                Place(Twin.GetIndex(), * FindHeir(Part, AsArchetype));
                mDirectory.Link(Twin.GetIndex(), Index);
                Settle(Twin.GetIndex(), Part, 0, ConstSpan<UInt32>());
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Carry(ConstRef<Chunk> Holder, UInt32 Row, ConstPtr<Chunk> Target, UInt32 Landing)
    {
        const UInt32 Last   = Holder.GetSize() - 1;
        const UInt32 Leaver = Holder.GetHandle(Row).GetIndex();
        const UInt32 Filler = Holder.GetHandle(Last).GetIndex();
        const Bool   Moving = Holder.GetPage(Row) != Holder.GetPage(Last);

        for (UInt32 Column = 0; Column < Holder.GetColumns().GetSize(); ++Column)
        {
            const UInt32 Identifier = Holder.GetColumns()[Column].Identifier;

            if (!IsWatched(Identifier, Pull::Changed))
            {
                continue;
            }

            // A value a walk wrote in place leaves the page that says so, unless the page it lands in says so too.
            const SInt16 Into = Target ? Target->Find(Identifier) : Chunk::kAbsent;

            if (Into >= 0 && Chunk::IsLater(Holder.GetMark(Column, Row), Target->GetMark(Into, Landing)))
            {
                Record(Identifier, Pull::Changed, Leaver, nullptr);
            }

            // So does the last row's, filling the hole in a page a walk wrote earlier.
            if (Moving && Chunk::IsLater(Holder.GetMark(Column, Last), Holder.GetMark(Column, Row)))
            {
                Record(Identifier, Pull::Changed, Filler, nullptr);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::Erase(UInt32 Index)
    {
        // Nothing reads from an archetype that is gone, so whatever was made from it goes first.
        if (Directory::IsArchetype(Index))
        {
            for (const Handle Heir : GetHeirs(Index))
            {
                if (mDirectory.IsAlive(Heir))
                {
                    Prune(Heir.GetIndex());
                }
            }

            for (const Ptr<Chunk> Target : mLayout.ExtractHeirs(Index))
            {
                mLayout.Destroy(Target);
            }
        }

        ConstRef<Directory::Slot> Entry  = mDirectory[Index];
        ConstRef<Chunk>           Holder = * Entry.Holder;

        if (mLedger.IsActive())
        {
            const Handle Actor = mDirectory.GetHandle(Index);

            for (const UInt32 Identifier : Holder.GetSignature())
            {
                if (IsWatched(Identifier, Pull::Removed))
                {
                    const ConstPtr<Byte> Value = mDirectory.FindValue(Holder, Entry.Row, Identifier);
                    mLedger.Record(Identifier, Pull::Removed, Actor, Value, Holder.GetIndex());
                }
            }

            Lend(Holder, Pull::Removed, [this, Actor, &Holder](UInt32 Identifier, ConstPtr<Byte> Value)
            {
                mLedger.Record(Identifier, Pull::Removed, Actor, Value, Holder.GetIndex());
            });

            Carry(Holder, Entry.Row, nullptr, 0);
        }

        if (const Handle Moved = Entry.Holder->Erase(Entry.Row); Moved.IsValid())
        {
            mDirectory[Moved.GetIndex()].Row = Entry.Row;
        }
        mDirectory.Release(Index);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::SetParent(UInt32 Index, UInt32 Parent)
    {
        if (mDirectory.GetParent(Index))
        {
            ClearParent(Index);
        }

        mDirectory.Link(Index, Parent);

        // An archetype under an archetype is a part, copied beneath every instance made from then on.
        if (IsPart(Index, Parent))
        {
            Add(mDirectory.GetHandle(Index), IdentifierOf<Part>());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Storage::ClearParent(UInt32 Index)
    {
        const Bool Lifted = IsPart(Index, mDirectory.GetParent(Index));

        mDirectory.Unlink(Index);

        if (Lifted)
        {
            Remove(mDirectory.GetHandle(Index), IdentifierOf<Part>());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Chunk> Storage::CreateAddition(Ref<Chunk> Source, UInt32 Identifier)
    {
        ZY_ASSERT(AddressOf(Source) != mLayout.GetPending(), "A pending entity is never changed in place");

        Sequence<UInt32> Signature(Source.GetSignature());
        Signature.Append(Identifier);
        Signature.Sort();

        const Ptr<Chunk> Target = Obtain(Signature, Source.GetBase())->GetClosure();
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

        const Ptr<Chunk> Target = Obtain(Signature, Source.GetBase());
        Source.SetRemoval(Identifier, Target);

        if (Source.GetClosure() == AddressOf(Source))
        {
            Target->SetAddition(Identifier, AddressOf(Source));
        }
        return Target;
    }
}