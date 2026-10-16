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

#include "Subscriber.hpp"
#include "Owner.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene::Protocol
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Subscriber::Subscriber(Ref<ZyEngine::Subsystem::Host> Host)
        : mScene    { * Host.GetService<Service>() },
          mLanded   { mScene },
          mIdentity { 0 },
          mSection  { kGlobal },
          mAccepted { false },
          mRejected { false }
    {
        mScene.Register<Replica>();

        mRoots = mScene.Query<const Locus, const Replica>();

        // A world loaded before this side followed it already holds persistent replicas.
        const ZyScene::Query Loaded = mScene.Query<const Replica>().Also<Asleep>();

        Loaded.Run([this](Entity Actor, ConstRef<Replica> Record)
        {
            Take(Actor, Record);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Subscriber::Receive(ConstSpan<Byte> Message)
    {
        Reader Input(Message);

        if (Input.GetAvailable() == 0 || Input.Peek<UInt8>() >= kMaxOpcode)
        {
            return false;
        }

        if (Input.Peek<UInt8>() == static_cast<UInt8>(Opcode::Packed))
        {
            Unpack(Input);
            return true;
        }

        // Every frame opens with the publisher's clock.
        if (static_cast<Opcode>(Input.Read<UInt8>()) != Opcode::Frame)
        {
            LOG_E("Scene: A protocol message arrived outside a frame");
            mRejected = true;
            return true;
        }

        const Real64 Time       = Input.Read<Real64>();
        const Real32 Multiplier = Input.Read<Real32>();

        // Nothing more is applied once refused, until a greeting starts a new session.
        if (mRejected)
        {
            if (Input.GetAvailable() == 0 || Input.Peek<UInt8>() != static_cast<UInt8>(Opcode::Hello))
            {
                return true;
            }
            mRejected = false;
            mAccepted = false;
        }

        Refresh();
        mScene.GetWorld().Get<Clock>()->Follow(Clock(Time, 0.0, Multiplier));

        mSection = kGlobal;

        while (Input.GetAvailable() > 0 && !mRejected)
        {
            Dispatch(Input);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Unpack(Ref<Reader> Input)
    {
        Input.Skip(sizeof(UInt8));

        const UInt32 Size = Input.Read<UInt32>();

        if (Size == 0 || Size > kMaxUnpacked)
        {
            LOG_E("Scene: A packed frame names a size no frame has, so the publisher is refused");
            mRejected = true;
            return;
        }

        if (mUnpacked.GetSize() < Size)
        {
            mUnpacked.Resize(Size);
        }

        const ConstSpan Packed(Input.GetData() + Input.GetOffset(), Input.GetAvailable());

        if (LZ4Decode(Packed, mUnpacked.GetData(), Size) != Size)
        {
            LOG_E("Scene: A packed frame did not unpack whole, so the publisher is refused");
            mRejected = true;
            return;
        }

        // Unpacked, it is a frame like any other, and a packed one inside it is refused as outside a frame.
        Receive(ConstSpan(mUnpacked.GetData(), Size));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Subscriber::Find(UInt64 Identifier)
    {
        const Ptr<Entity> Held = mReplicas.Find(Identifier);

        if (Held == nullptr)
        {
            return Entity();
        }

        // A replica the game destroyed, or one gone with its parent, is let go the first time it is looked for.
        const ConstPtr<Replica> Record = Held->IsAlive() ? Held->Get<const Replica>() : nullptr;

        if (Record && Record->GetIdentifier() == Identifier)
        {
            return * Held;
        }

        mReplicas.Erase(Identifier);
        return Entity();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Subscriber::Resolve(Ref<Link> Value)
    {
        return Value.Resolve([this](UInt64 Identifier)
        {
            return Find(Identifier);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Dispatch(Ref<Reader> Input)
    {
        // In the order of the opcodes, where a frame's own opens it and never appears inside it.
        static constexpr Array kHandlers
        {
            nullptr,
            & Subscriber::OnHello,
            & Subscriber::OnLeave,
            & Subscriber::OnSection,
            & Subscriber::OnEnter,
            & Subscriber::OnUpdate,
            & Subscriber::OnMove,
            & Subscriber::OnForget
        };

        // A new opcode without a handler added here fails to build instead of being refused silently.
        static_assert(kHandlers.GetSize() == static_cast<UInt8>(Opcode::Forget) + 1);

        const UInt8 Code = Input.Read<UInt8>();

        if (Code >= kHandlers.GetSize() || kHandlers[Code] == nullptr)
        {
            LOG_E("Scene: A message this build does not know arrived, so the publisher is refused");
            mRejected = true;
            return;
        }

        // Nothing but a greeting is read before one was accepted.
        if (!mAccepted && Code != static_cast<UInt8>(Opcode::Hello))
        {
            LOG_E("Scene: A message arrived before the greeting, so the publisher is refused");
            mRejected = true;
            return;
        }
        (this->* kHandlers[Code])(Input);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnHello(Ref<Reader> Input)
    {
        const UInt64 Hash     = Input.Read<UInt64>();
        const UInt64 Identity = Input.Read<UInt64>();

        if (Hash != Manifest::Get().GetHash())
        {
            LOG_E("Scene: The publisher replicates a different set of components than this build");
            mRejected = true;
            return;
        }

        // A greeting starts the session over, so whatever an earlier one left goes.
        Clear(kGlobal);

        mReplicas.EraseIf([](UInt64 Identifier, Ref<Entity> Actor)
        {
            return (Identifier & kPersistent) == 0 || !Actor.IsAlive();
        });
        mKeys.Clear();
        mPending.Clear();

        mAccepted = true;
        mIdentity = Identity;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnLeave(Ref<Reader> Input)
    {
        const UInt64 Key = Input.ReadUInt<UInt64>();

        mKeys.Erase(Key);
        Clear(Key);

        mPending.EraseIf([Key](UInt64, Ref<Sequence<Pending>> Sets)
        {
            return Discard(Key, Sets);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnSection(Ref<Reader> Input)
    {
        mSection = Input.ReadUInt<UInt64>();

        if (mSection != kGlobal && mSection != kPrivate)
        {
            mKeys.Insert(mSection);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnEnter(Ref<Reader> Input)
    {
        const UInt32 Start = Input.GetOffset();

        Arrival Message;
        Message.Identifier  = Input.ReadUInt<UInt64>();
        const UInt64 Parent = Input.ReadUInt<UInt64>();
        Message.Archetype   = Input.ReadUInt<UInt64>();
        Message.Peer        = Input.ReadUInt<UInt64>();
        Message.Set         = Extract(Input);

        if (mRejected)
        {
            return;
        }

        // A parent this side does not hold is one loaded from a cell not loaded yet, so the child waits for it.
        if (Parent != 0)
        {
            Message.Parent = Find(Parent);

            if (!Message.Parent.IsAlive())
            {
                const ConstSpan Bytes(Input.GetData() + Start, Input.GetOffset() - Start);
                mPending.FindOrInsert(Parent).Append(mSection, Opcode::Enter, Blob::Copy(Bytes));
                return;
            }
        }

        Entity Actor  = Find(Message.Identifier);
        Message.Fresh = !Actor.IsAlive();

        if (Message.Fresh)
        {
            Actor = mScene.CreateEntity();
            mReplicas.Assign(Message.Identifier, Actor);
        }

        // Everything the arrival names lands in one move.
        mScene.Defer([this, Actor, &Message]
        {
            Settle(Actor, Message);
        });

        // Children that came before it arrive now.
        Release(Message.Identifier, Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnUpdate(Ref<Reader> Input)
    {
        const UInt64          Identifier = Input.ReadUInt<UInt64>();
        const ConstSpan<Byte> Set        = Extract(Input);

        if (mRejected)
        {
            return;
        }

        if (Identifier == 0)
        {
            Apply(mScene.GetWorld(), Set, Mask());
        }
        else if (const Entity Actor = Find(Identifier); Actor.IsAlive())
        {
            Apply(Actor, Set, GetIgnored(GetOwner(Actor)));
        }
        else if (Identifier & kPersistent)
        {
            // Held under the section it came in, until the cell loads or that key is left.
            mPending.FindOrInsert(Identifier).Append(mSection, Opcode::Update, Blob::Copy(Set));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnMove(Ref<Reader> Input)
    {
        const UInt64 Identifier = Input.ReadUInt<UInt64>();
        const UInt64 Key        = Input.ReadUInt<UInt64>();
        const Entity Actor      = Find(Identifier);

        if (!Actor.IsAlive())
        {
            return;
        }

        if (!mKeys.Contains(Key))
        {
            Destroy(Actor);
            return;
        }

        // A child keeps key zero and follows its root.
        if (Actor.Get<const Locus>()->GetKey() != 0)
        {
            Actor.Set(Locus(Key));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnForget(Ref<Reader> Input)
    {
        const UInt64 Identifier = Input.ReadUInt<UInt64>();
        mPending.Erase(Identifier);

        if (const Entity Actor = Find(Identifier); Actor.IsAlive())
        {
            Destroy(Actor);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Refresh()
    {
        mLanded.Each([this](Entity Actor, ConstRef<Replica> Record)
        {
            Take(Actor, Record);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Take(Entity Actor, ConstRef<Replica> Record)
    {
        // A runtime replica is taken in by its arrival.
        if (!Record.IsPersistent())
        {
            return;
        }
        mReplicas.Assign(Record.GetIdentifier(), Actor);

        // Whatever is loaded from disk belongs to the publisher, save what is kept to this side.
        if (!Actor.Has<Client>())
        {
            Actor.Add<Remote>();
        }

        Release(Record.GetIdentifier(), Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Release(UInt64 Identifier, Entity Actor)
    {
        Sequence<Pending> Items;

        if (!mPending.Extract(Identifier, Items))
        {
            return;
        }

        // A replayed arrival reads the section it came in, so the current one is put back after.
        const UInt64 Section = mSection;

        for (ConstRef<Pending> Item : Items)
        {
            Replay(Actor, Item);
        }
        mSection = Section;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Replay(Entity Actor, ConstRef<Pending> Item)
    {
        const ConstSpan Bytes(Item.Data.GetData(), Item.Data.GetSize());

        if (Item.Code == Opcode::Update)
        {
            Apply(Actor, Bytes, GetIgnored(GetOwner(Actor)));
            return;
        }

        mSection = Item.Key;

        Reader Input(Bytes);
        OnEnter(Input);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Subscriber::Discard(UInt64 Key, Ref<Sequence<Pending>> Sets)
    {
        Sets.RemoveSomeIf([Key](ConstRef<Pending> Item)
        {
            return Item.Key == Key;
        });
        return Sets.IsEmpty();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Settle(Entity Actor, ConstRef<Arrival> Message)
    {
        if (Message.Fresh)
        {
            const Entity Base = Message.Archetype ? mScene.GetEntity(Message.Archetype) : Entity();

            if (Base.IsAlive() && Base.IsArchetype())
            {
                Actor.SetArchetype(Base);
            }
            else if (Message.Archetype)
            {
                LOG_W("Scene: Replica {0} names an archetype {1} this world does not hold",
                    Message.Identifier, Message.Archetype);
            }
            Actor.Set(Replica(Message.Identifier));
        }

        // A child stands under key zero beneath its parent; a root under the section it came in.
        if (Message.Parent.IsAlive())
        {
            if (Actor.GetParent().GetID() != Message.Parent.GetID())
            {
                Actor.Attach(Message.Parent);
            }
            Actor.Set(Locus());
        }
        else
        {
            if (Actor.GetParent().IsAlive())
            {
                Actor.Attach(Entity());
            }
            Actor.Set(Locus(mSection));
        }
        Claim(Actor, Message.Peer);
        Apply(Actor, Message.Set, GetIgnored(Message.Peer));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Claim(Entity Actor, UInt64 Peer)
    {
        if (Peer != 0)
        {
            Actor.Set(Owner(Peer));
        }
        else
        {
            Actor.Remove<Owner>();
        }

        if (IsMine(Peer))
        {
            Actor.Remove<Remote>().Add<Possessed>();
            return;
        }
        Actor.Remove<Possessed>().Add<Remote>();

        // What only an owner receives is not this side's to keep once another peer owns it.
        ConstRef<Manifest> Table = Manifest::Get();

        for (UInt8 Index = 0; Index < Table.GetCount(); ++Index)
        {
            if (Table.GetOwnerMask().Test(Index))
            {
                Actor.Remove(Component(Table.GetEntry(Index).Identifier));
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Clear(UInt64 Key)
    {
        mDoomed.Clear();

        mRoots.Run([this, Key](Entity Actor, ConstRef<Locus> Place, ConstRef<Replica> Record)
        {
            Collect(Actor, Place, Record, Key);
        });

        // A child gathered with its parent may have gone with it already.
        for (const Entity Actor : mDoomed)
        {
            if (Actor.IsAlive())
            {
                Destroy(Actor);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Collect(Entity Actor, ConstRef<Locus> Place, ConstRef<Replica> Record, UInt64 Key)
    {
        if (Record.IsPersistent())
        {
            return;
        }

        // A child goes with its parent, save beneath a persistent parent, which stays from disk without it.
        const ConstPtr<Replica> Above   = Actor.GetParent().Get<const Replica>();
        const Bool              Alone   = Place.GetKey() != 0 || (Above && Above->IsPersistent());
        const UInt64            Current = GetKey(Actor);

        if (Alone && Current != 0 && (Key == kGlobal || Current == Key))
        {
            mDoomed.Append(Actor);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Destroy(Entity Actor)
    {
        if (const ConstPtr<Replica> Record = Actor.Get<const Replica>())
        {
            mReplicas.Erase(Record->GetIdentifier());
        }
        Actor.Destruct();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstSpan<Byte> Subscriber::Extract(Ref<Reader> Input)
    {
        const UInt32 Start = Input.GetOffset();

        if (!Manifest::Get().Skip(Input))
        {
            LOG_E("Scene: A set this build cannot read arrived, so the publisher is refused");
            mRejected = true;
            return ConstSpan<Byte>();
        }
        return ConstSpan<Byte>(Input.GetData() + Start, Input.GetOffset() - Start);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Apply(Entity Actor, ConstSpan<Byte> Set, Mask Ignored)
    {
        Reader Input(Set);

        if (!Manifest::Get().Read(Input, Actor, Ignored))
        {
            LOG_E("Scene: A set this build cannot read arrived, so the publisher is refused");
            mRejected = true;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Mask Subscriber::GetIgnored(UInt64 Peer) const
    {
        return IsMine(Peer) ? Manifest::Get().GetOthersMask() : Mask();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Subscriber::GetOwner(Entity Actor)
    {
        const ConstPtr<Owner> Holder = Actor.Get<const Owner>();
        return Holder ? Holder->GetPeer() : 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Subscriber::GetKey(Entity Actor)
    {
        for (Entity Above = Actor; Above.IsAlive(); Above = Above.GetParent())
        {
            if (const ConstPtr<Locus> Place = Above.Get<const Locus>(); Place && Place->GetKey() != 0)
            {
                return Place->GetKey();
            }
        }
        return 0;
    }
}