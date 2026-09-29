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

#include "World.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    World::World(Ref<ZyEngine::Subsystem::Host> Host)
        : Storage    { Host },
          mImporting { 0 }
    {
        // The built-in components every world has.
        Declare<Prefab>();
        Declare<Asleep>().Local();
        Declare<Transient>();
        Declare<Named>().Local().Serializable();

        const UInt32 Update = CreatePhase("Update");
        ZY_ASSERT(Update == kUpdate, "The update phase must be the first one made");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    World::~World()
    {
        mScheduler.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void World::Progress()
    {
        mScheduler.Progress();
        mScheduler.Purge();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void World::Discard(UInt32 Package)
    {
        if (Package == 0 || Package > mModules.GetSize() || !mModules[Package - 1].Alive)
        {
            return;
        }

        Ref<Module> Target = mModules[Package - 1];

        LOG_I("Scene: Discarding module '{0}' and its {1} component(s)", Target.Name, Target.Types.GetSize());

        // Its code is going away, so its systems and phases go first, their cursors with them.
        mScheduler.Discard(Package);
        mScheduler.Purge();

        // Values stay on their entities as the bytes they save to, so importing it again reads them back.
        for (const UInt32 Identifier : Target.Types)
        {
            ConstRef<Metatype> Info = Registry::Get().GetMetatype(Identifier);

            // A value that cannot be saved is lost with its code.
            if (!Info.IsTag() && !Info.Save)
            {
                Purge(Component(Identifier));
            }
            Registry::Get().SetTrait(Identifier, Trait::Loaded, false);
            Rebuild(Identifier);

            // Kept values run the module's code when they go, so they go while it is still there.
            mLedger.Retire(Identifier);
            Registry::Get().Retire(Identifier);
        }
        Target.Alive = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void World::Save(Ref<Writer> Output, ConstSpan<Entity> Roots)
    {
        Writer                Body(4096);
        Table<UInt32, UInt32> Indices;
        Sequence<Text>        Names;
        UInt32                Count = 0;

        // Archetypes go first, each after what it reads from when the save holds that too, so a load finds every base.
        Sequence<UInt32> Pending;

        for (const Entity Root : Roots)
        {
            if (Root.IsArchetype())
            {
                Pending.Append(Root.GetHandle().GetIndex());
            }
        }

        while (!Pending.IsEmpty())
        {
            UInt Next = 0;

            while (Next + 1 < Pending.GetSize() && Pending.Contains(GetOrigin(Pending[Next])))
            {
                ++Next;
            }
            Emit(Body, Pending[Next], 0, Indices, Names, Count);
            Pending.Remove(Next);
        }

        for (const Entity Root : Roots)
        {
            if (!Root.IsArchetype())
            {
                Emit(Body, Root.GetHandle().GetIndex(), 0, Indices, Names, Count);
            }
        }

        // The names go ahead of the records, though only writing the records gathers them.
        Writer Header;
        Header.Write<UInt32>(static_cast<UInt32>(Names.GetSize()));

        for (const Text Name : Names)
        {
            Header.WriteText(Name);
        }

        Output.Write<UInt32>(kMagic);
        Output.Write<UInt16>(kVersion);
        Output.Write<UInt32>(kTypes);
        Output.WriteBlock<UInt32, Byte>(Header);
        Output.Write<UInt32>(kEntities);
        Output.Write<UInt32>(static_cast<UInt32>(sizeof(UInt32) + Body.GetSize()));
        Output.Write<UInt32>(Count);
        Output.Write(Body.GetData(), Body.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sequence<Entity> World::Load(ConstSpan<Byte> Input, Entity Parent)
    {
        return Decode(Input, Parent, Entity());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool World::Merge(ConstSpan<Byte> Input, Entity Root)
    {
        ZY_ASSERT(Root.IsAlive(), "A save can only be read over an entity that is alive");

        return !Decode(Input, Entity(), Root).IsEmpty();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sequence<Entity> World::Decode(ConstSpan<Byte> Input, Entity Parent, Entity Root)
    {
        ZY_ASSERT(!mDeferral.IsWalking(), "A save cannot be read while a query walks");

        Sequence<Entity> Roots;
        Reader           Stream(Input);

        if (Stream.GetAvailable() < sizeof(UInt32) + sizeof(UInt16)
            || Stream.Read<UInt32>() != kMagic
            || Stream.Read<UInt16>() != kVersion)
        {
            LOG_W("Scene: The bytes are not a save this version reads");
            return Roots;
        }

        const UInt32              Home   = Parent.IsAlive() ? Parent.GetHandle().GetIndex() : 0;
        Handle                    Target = Root.IsAlive() ? Root.GetHandle() : Handle();
        Sequence<UInt32>          Types;
        Sequence<UInt32>          Made;
        Sequence<UInt32>          Identifiers;
        Sequence<ConstSpan<Byte>> Payloads;

        while (Stream.GetAvailable() >= 2 * sizeof(UInt32))
        {
            const UInt32 Kind  = Stream.Read<UInt32>();
            Reader       Scope = Stream.Split(Stream.Read<UInt32>());

            // A name this build does not know comes back as an unresolved type, its values kept as bytes.
            for (UInt32 Count = Kind == kTypes ? Scope.Read<UInt32>() : 0; Count > 0; --Count)
            {
                Types.Append(Registry::Get().Acquire(Scope.ReadText()));
            }

            const UInt32 Records = Kind == kEntities ? Scope.Read<UInt32>() : 0;

            for (UInt32 Count = 0; Count < Records && Scope.GetAvailable() > 0; ++Count)
            {
                const UInt32 Owner = Scope.Read<UInt32>();

                if (Owner == 0)
                {
                    Made.Append(Make(Scope, Types, Home, Exchange(Target, Handle()), Identifiers, Payloads));
                    Roots.Append(this, mDirectory.GetHandle(Made.GetBack()));
                }
                else
                {
                    const UInt32 Above = Owner <= Made.GetSize() ? Made[Owner - 1] : Home;
                    Made.Append(Make(Scope, Types, Above, Handle(), Identifiers, Payloads));
                }
            }
        }
        return Roots;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void World::Registered(ConstSpan<UInt32> Identifiers)
    {
        for (const UInt32 Identifier : Identifiers)
        {
            if (mImporting && !mModules[mImporting - 1].Types.Contains(Identifier))
            {
                mModules[mImporting - 1].Types.Append(Identifier);
            }
            Rebuild(Identifier);

            // A save read before the type had its name left the values under a type of their own.
            if (const UInt32 Unresolved = Registry::Get().FindUnresolved(Identifier))
            {
                Resolve(Unresolved, Identifier);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void World::Emit(
        Ref<Writer>                Output,
        UInt32                     Index,
        UInt32                     Parent,
        Ref<Table<UInt32, UInt32>> Indices,
        Ref<Sequence<Text>>        Names,
        Ref<UInt32>                Count)
    {
        ConstRef<Chunk> Shape = * mDirectory[Index].Holder;

        if (Shape.Has(IdentifierOf<Transient>()))
        {
            return;
        }

        // Only the world and an archetype keep their identifier, since instances saved apart name archetypes by it.
        const Bool   Kept  = Shape.IsArchetype() || Index == kWorld.GetIndex();
        const UInt32 Local = ++Count;
        UInt32       Saved = 0;

        for (const UInt32 Identifier : Shape.GetSignature())
        {
            Saved += (Shape.GetFlags(Identifier) & Chunk::kSaves) ? 1 : 0;
        }

        Output.Write<UInt32>(Parent);
        Output.Write<UInt64>(Kept ? mDirectory.GetHandle(Index).GetValue() : 0);
        Output.Write<UInt64>(Shape.GetBase() ? mDirectory.GetHandle(Shape.GetBase()).GetValue() : 0);
        Output.Write<UInt32>(Saved);

        const Entity Actor(this, mDirectory.GetHandle(Index));

        for (const UInt32 Identifier : Shape.GetSignature())
        {
            if (!(Shape.GetFlags(Identifier) & Chunk::kSaves))
            {
                continue;
            }

            // Each component is named once, under the index it first came to.
            const ConstPtr<UInt32> Known = Indices.Find(Identifier);
            const UInt32           Slot  = Known ? * Known : static_cast<UInt32>(Names.GetSize());

            if (!Known)
            {
                Indices.Assign(Identifier, Slot);
                Names.Append(Registry::Get().GetMetatype(Identifier).Name);
            }
            Output.Write<UInt32>(Slot);
            Actor.Write(Component(Identifier), Output);
        }

        for (UInt32 Child = mDirectory[Index].First; Child; Child = mDirectory[Child].Next)
        {
            Emit(Output, Child, Local, Indices, Names, Count);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 World::Make(
        Ref<Reader>                    Input,
        ConstRef<Sequence<UInt32>>     Types,
        UInt32                         Parent,
        Handle                         Target,
        Ref<Sequence<UInt32>>          Identifiers,
        Ref<Sequence<ConstSpan<Byte>>> Payloads)
    {
        const Handle Identity(Input.Read<UInt64>());
        const Handle Base(Input.Read<UInt64>());
        const Bool   Known = Base.IsValid()
            && mDirectory.IsAlive(Base)
            && mDirectory[Base.GetIndex()].Holder->IsArchetype();

        if (Base.IsValid() && !Known && !Target.IsValid())
        {
            LOG_W("Scene: An entity reads from archetype {0}, which is not loaded", Base.GetValue());
        }

        Handle Actor = Target.IsValid() ? Target : Identity;

        if (!Target.IsValid() && Identity != kWorld)
        {
            if (Identity.IsValid() && mDirectory[Identity.GetIndex()].Holder)
            {
                LOG_W("Scene: Archetype slot {0} was taken, and goes to the one the save names", Identity.GetIndex());
                DestroyTree(Identity.GetIndex());
            }
            Actor = Identity.IsValid() ? mDirectory.Acquire(Identity) : mDirectory.Allocate(false);

            // Parts are records of their own, so the base hands over what it copies and nothing beneath it.
            if (Known)
            {
                Propagate(Base.GetIndex(), Identity.IsValid(), Actor, false);
            }

            if (Parent)
            {
                mDirectory.Link(Actor.GetIndex(), Parent);
            }
        }

        const UInt32     Index = Actor.GetIndex();
        const Ptr<Chunk> Held  = mDirectory[Index].Holder;
        Ptr<Chunk>       Shape = Held;

        // A new entity starts from the root, and an archetype from the chunk every archetype shares.
        if (Shape == nullptr)
        {
            Shape = Identity.IsValid() ? FindOrCreate(ConstSpan(mPrefab), 0) : mLayout.GetRoot();
        }

        // Every component is gathered first, so the entity lands once, and a duplicate record keeps the first.
        Identifiers.Clear();
        Payloads.Clear();

        for (UInt32 Count = Input.Read<UInt32>(); Count > 0; --Count)
        {
            const UInt32          Type = Input.Read<UInt32>();
            const ConstSpan<Byte> Data = Input.ReadBlock<UInt32, Byte>();

            if (Type < Types.GetSize() && !Identifiers.Contains(Types[Type]))
            {
                Identifiers.Append(Types[Type]);
                Payloads.Append(Data);
                Shape = Shape->Has(Types[Type]) ? Shape : FindOrCreateAddition(* Shape, Types[Type]);
            }
        }

        PlaceOrTransfer(Index, * Shape);
        BuildDefaults(Index, Held ? * Held : * mLayout.GetRoot(), 0, false);

        // Each value is read over the default just built, or over what the entity held.
        const UInt32 Row = mDirectory[Index].Row;

        for (UInt Item = 0; Item < Identifiers.GetSize(); ++Item)
        {
            const SInt16 Column = Shape->Find(Identifiers[Item]);

            if (Column >= 0 && Shape->GetColumns()[Column].Info->Load)
            {
                Reader Block(Payloads[Item]);
                Shape->GetColumns()[Column].Info->Load(Block, Shape->At(Column, Row));
            }
        }

        RecordGain(Index, Held);
        return Index;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 World::GetOrigin(UInt32 Index) const
    {
        UInt32 Root = mDirectory[Index].Holder->GetBase();

        while (Root && mDirectory[Root].Parent)
        {
            Root = mDirectory[Root].Parent;
        }
        return Root;
    }
}