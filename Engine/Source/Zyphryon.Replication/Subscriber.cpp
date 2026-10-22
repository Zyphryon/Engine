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

namespace ZyReplication
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Subscriber::Subscriber(Ref<ZyScene::Service> Scene, ConstRef<Manifest> Table, ConstRetainer<Listener> Listener)
        : mScene    { Scene },
          mManifest { Table },
          mListener { Listener },
          mHeld     { 0 },
          mLanded   { Scene },
          mIdentity { 0 },
          mSection  { kGlobal },
          mAccepted { false },
          mRejected { false }
    {
        mRoots = mScene.Query<const Locus, const Replica>();

        // A world loaded before this side followed it already holds persistent replicas.
        const ZyScene::Query Loaded = mScene.Query<const Replica>().Also<ZyScene::Asleep>();

        Loaded.Run([this](ZyScene::Entity Actor, ConstRef<Replica> Record)
        {
            Take(Actor, Record);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Receive(ConstSpan<Byte> Message)
    {
        Reader Input(Message);

        if (Input.GetAvailable() == 0)
        {
            return;
        }

        if (Input.Peek<UInt8>() == static_cast<UInt8>(Opcode::Packed))
        {
            Unpack(Input);
            return;
        }

        // Every frame opens with the publisher's clock.
        if (static_cast<Opcode>(Input.Read<UInt8>()) != Opcode::Frame)
        {
            LOG_E("Replication: A message arrived outside a frame, so the publisher is refused");
            mRejected = true;
            return;
        }

        if (Input.GetAvailable() < sizeof(Real64) + sizeof(Real32))
        {
            LOG_E("Replication: A frame arrived cut short of its clock, so the publisher is refused");
            mRejected = true;
            return;
        }

        const Real64 Time       = Input.Read<Real64>();
        const Real32 Multiplier = Input.Read<Real32>();

        // A clock that is not a number, or runs backwards, would break every timer that follows it.
        if (IsNaN(Time) || IsInf(Time) || IsNaN(Multiplier) || IsInf(Multiplier) || Multiplier < 0.0f)
        {
            LOG_E("Replication: A frame names a clock no publisher keeps, so the publisher is refused");
            mRejected = true;
            return;
        }

        // Nothing more is applied once refused, until a greeting starts a new session.
        if (mRejected)
        {
            if (Input.GetAvailable() == 0 || Input.Peek<UInt8>() != static_cast<UInt8>(Opcode::Hello))
            {
                return;
            }
            mRejected = false;
            mAccepted = false;
        }

        Refresh();
        mScene.GetWorld().Get<ZyScene::Clock>()->Follow(ZyScene::Clock(Time, 0.0, Multiplier));

        mSection = kGlobal;

        while (Input.GetAvailable() > 0 && !mRejected)
        {
            Dispatch(Input);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Refresh()
    {
        // Gathered first, since what waited for them is spawned as they are taken in, and a walk holds that back.
        Sequence<ZyScene::Entity> Landed;

        mLanded.Each([&Landed](ZyScene::Entity Actor, ConstRef<Replica>)
        {
            Landed.Append(Actor);
        });

        for (const ZyScene::Entity Actor : Landed)
        {
            if (const ConstPtr<Replica> Record = Actor.Get<const Replica>())
            {
                Take(Actor, * Record);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ZyScene::Entity Subscriber::Find(UInt64 Identifier)
    {
        const Ptr<ZyScene::Entity> Held = mReplicas.Find(Identifier);

        if (Held == nullptr)
        {
            return ZyScene::Entity();
        }

        // A replica the game destroyed, or one gone with its parent, is let go the first time it is looked for.
        const ConstPtr<Replica> Record = Held->IsAlive() ? Held->Get<const Replica>() : nullptr;

        if (Record && Record->GetIdentifier() == Identifier)
        {
            return * Held;
        }

        mReplicas.Erase(Identifier);
        return ZyScene::Entity();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Take(ZyScene::Entity Actor, ConstRef<Replica> Record)
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

        Release(Record.GetIdentifier());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Hold(UInt64 Under, UInt64 Identifier, Opcode Code, ConstSpan<Byte> Bytes)
    {
        mHeld += GetCost(Bytes.GetSize());

        if (mHeld > kMaxHeld)
        {
            LOG_E("Replication: More is held for replicas not loaded than a world needs, so the publisher is refused");
            mRejected = true;
            return;
        }
        mPending.FindOrInsert(Under).Append(mSection, Identifier, Code, Blob::Copy(Bytes));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Subscriber::IsHeld(UInt64 Identifier) const
    {
        Bool Found = false;

        mPending.ForEach([Identifier, &Found](UInt64, ConstRef<Sequence<Pending>> Items)
        {
            for (ConstRef<Pending> Item : Items)
            {
                Found = Found || (Item.Code == Opcode::Enter && Item.Identifier == Identifier);
            }
        });
        return Found;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Release(UInt64 Identifier)
    {
        // A replayed arrival releases its own children, which join the list instead of nesting another call.
        mReleases.Append(Identifier);

        if (mReleases.GetSize() > 1)
        {
            return;
        }

        // A replayed arrival reads the section it came in, so the current one is put back after.
        const UInt64 Section = mSection;

        for (UInt Index = 0; Index < mReleases.GetSize() && !mRejected; ++Index)
        {
            Sequence<Pending> Items;

            if (!mPending.Extract(mReleases[Index], Items))
            {
                continue;
            }

            const ZyScene::Entity Actor = Find(mReleases[Index]);

            for (ConstRef<Pending> Item : Items)
            {
                mHeld -= GetCost(Item.Data.GetSize());

                if (!Actor.IsAlive() || mRejected)
                {
                    continue;
                }

                const ConstSpan Bytes(Item.Data.GetData(), Item.Data.GetSize());

                if (Item.Code == Opcode::Update)
                {
                    Apply(Actor, Bytes, Actor.Has<Possessed>() ? mManifest.GetOthersMask() : Mask());
                }
                else
                {
                    mSection = Item.Key;

                    Reader Input(Bytes);
                    OnEnter(Input);
                }
            }
        }
        mReleases.Clear();
        mSection = Section;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Unpack(Ref<Reader> Input)
    {
        Input.Skip(sizeof(UInt8));

        const UInt32 Size = Input.Read<UInt32>();

        if (Size == 0 || Size > kMaxUnpacked)
        {
            LOG_E("Replication: A packed frame names a size no frame has, so the publisher is refused");
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
            LOG_E("Replication: A packed frame did not unpack whole, so the publisher is refused");
            mRejected = true;
            return;
        }

        // Unpacked, it must be a plain frame, since a packed one inside would unpack over the bytes it is read from.
        if (mUnpacked[0] != static_cast<Byte>(Opcode::Frame))
        {
            LOG_E("Replication: A packed frame did not unpack to a frame, so the publisher is refused");
            mRejected = true;
            return;
        }
        Receive(ConstSpan(mUnpacked.GetData(), Size));
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
            & Subscriber::OnForget,
            & Subscriber::OnMessage
        };

        // A new opcode without a handler added here fails to build instead of being refused silently.
        static_assert(kHandlers.GetSize() == static_cast<UInt8>(Opcode::Message) + 1);

        const UInt8 Code = Input.Read<UInt8>();

        if (Code >= kHandlers.GetSize() || kHandlers[Code] == nullptr)
        {
            LOG_E("Replication: A message this build does not know arrived, so the publisher is refused");
            mRejected = true;
            return;
        }

        // Nothing but a greeting is read before one was accepted.
        if (!mAccepted && Code != static_cast<UInt8>(Opcode::Hello))
        {
            LOG_E("Replication: A message arrived before the greeting, so the publisher is refused");
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

        if (Hash != mManifest.GetHash())
        {
            LOG_E("Replication: The publisher replicates a different set of components than this build");
            mRejected = true;
            return;
        }

        // A greeting starts the session over, so whatever an earlier one left goes.
        Clear(kGlobal);

        mReplicas.EraseIf([](UInt64 Identifier, Ref<ZyScene::Entity> Actor)
        {
            return (Identifier & kPersistent) == 0 || !Actor.IsAlive();
        });
        mKeys.Clear();
        mPending.Clear();
        mHeld = 0;

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

        Drop([Key](UInt64, ConstRef<Pending> Item)
        {
            return Item.Key == Key;
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
        const UInt32          Start      = Input.GetOffset();
        const UInt64          Identifier = Input.ReadUInt<UInt64>();
        const UInt64          Parent     = Input.ReadUInt<UInt64>();
        const UInt64          Archetype  = Input.ReadUInt<UInt64>();
        const UInt64          Peer       = Input.ReadUInt<UInt64>();
        const ConstSpan<Byte> Set        = Extract(Input);

        if (mRejected)
        {
            return;
        }

        // An arrival names a key it stands under, never the world's nor the private one, which nothing ever leaves.
        if (Identifier == 0 || mSection == kGlobal || mSection == kPrivate)
        {
            LOG_E("Replication: A replica arrived outside the section of a key, so the publisher is refused");
            mRejected = true;
            return;
        }

        // A parent this side does not hold is one loaded from a cell not loaded yet, so the child waits for it.
        const ZyScene::Entity Above = Parent != 0 ? Find(Parent) : ZyScene::Entity();

        if (Parent != 0 && !Above.IsAlive())
        {
            Hold(Parent, Identifier, Opcode::Enter, ConstSpan(Input.GetData() + Start, Input.GetOffset() - Start));

            // What this side plays would wait on a cell loaded around it, so the game is asked to load that one.
            if (IsMine(Peer) && (Parent & kPersistent))
            {
                mListener->OnAwait(Parent);
            }
            return;
        }

        ZyScene::Entity Actor = Find(Identifier);

        if (!Actor.IsAlive())
        {
            // A persistent replica comes from disk, never from an arrival, which would make a copy of it.
            if (Identifier & kPersistent)
            {
                LOG_E("Replication: Persistent replica {0} arrived, so the publisher is refused", Identifier);
                mRejected = true;
                return;
            }

            const ZyScene::Entity Base = Archetype ? mScene.GetEntity(Archetype) : ZyScene::Entity();

            if (Archetype && !Base.IsArchetype())
            {
                LOG_W("Replication: Replica {0} names an archetype {1} this world does not hold",
                    Identifier, Archetype);
            }

            // Made from its archetype under its parent in one move, as the publisher's was.
            Actor = mScene.Spawn(Base.IsArchetype() ? Base : ZyScene::Entity(), Above, Replica(Identifier));
            mReplicas.Assign(Identifier, Actor);
        }
        else
        {
            // A replica attached beneath itself would make the hierarchy a loop no walk up it ever leaves.
            for (ZyScene::Entity Up = Above; Up.IsAlive(); Up = Up.GetParent())
            {
                if (Up.GetID() == Actor.GetID())
                {
                    LOG_E("Replication: Replica {0} was attached beneath itself, so the publisher is refused",
                        Identifier);
                    mRejected = true;
                    return;
                }
            }
        }

        // Everything the arrival names lands in one move, a child under no key of its own, a root under the section.
        mScene.Defer([&]
        {
            if (Actor.GetParent().GetID() != Above.GetID())
            {
                Actor.Attach(Above);
            }
            Actor.Set(Above.IsAlive() ? Locus() : Locus(mSection));

            Claim(Actor, Peer);
            Apply(Actor, Set, IsMine(Peer) ? mManifest.GetOthersMask() : Mask());
        });

        // Children that came before it arrive now.
        Release(Identifier);
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
        else if (const ZyScene::Entity Actor = Find(Identifier); Actor.IsAlive())
        {
            Apply(Actor, Set, Actor.Has<Possessed>() ? mManifest.GetOthersMask() : Mask());
        }
        else if ((Identifier & kPersistent) || IsHeld(Identifier))
        {
            // Held under the section it came in, until the cell or the arrival lands or that key is left.
            Hold(Identifier, Identifier, Opcode::Update, Set);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnForget(Ref<Reader> Input)
    {
        const UInt64 Identifier = Input.ReadUInt<UInt64>();
        const UInt64 Key        = Input.ReadUInt<UInt64>();

        // Peers load a persistent replica from disk and keep it under the key it was placed under.
        if (Identifier & kPersistent)
        {
            LOG_E("Replication: Persistent replica {0} was forgotten, so the publisher is refused", Identifier);
            mRejected = true;
            return;
        }

        // Moved to a key this side sees, it arrives there too, which reattaches it, as does what this side plays.
        const ZyScene::Entity Actor = Find(Identifier);

        if (Key != 0 && (mKeys.Contains(Key) || (Actor.IsAlive() && Actor.Has<Possessed>())))
        {
            return;
        }

        // What waited for it goes, and so does its own arrival, held under a parent not loaded yet.
        Drop([Identifier](UInt64 Under, ConstRef<Pending> Item)
        {
            return Under == Identifier || Item.Identifier == Identifier;
        });

        if (Actor.IsAlive())
        {
            Destroy(Actor);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnMessage(Ref<Reader> Input)
    {
        const UInt64 Skipped = Input.ReadUInt<UInt64>();
        const UInt32 Size    = Input.ReadUInt<UInt32>();

        // A message never runs past the frame it came in.
        if (Size > Input.GetAvailable())
        {
            LOG_E("Replication: A message arrived cut short, so the publisher is refused");
            mRejected = true;
            return;
        }

        const ConstSpan<Byte> Message(Input.GetData() + Input.GetOffset(), Size);
        Input.Skip(Size);

        // What everyone but an entity's owner hears passes over the side that owns it.
        if (!IsMine(Skipped))
        {
            mListener->OnMessage(0, Message);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Claim(ZyScene::Entity Actor, UInt64 Peer)
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
        for (UInt8 Index = 0; Index < mManifest.GetCount(); ++Index)
        {
            if (mManifest.GetOwnerMask().Test(Index))
            {
                Actor.Remove(ZyScene::Component(mManifest.GetEntry(Index).Identifier));
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Clear(UInt64 Key)
    {
        Sequence<ZyScene::Entity> Doomed;

        mRoots.Run([&Doomed, Key](ZyScene::Entity Actor, ConstRef<Locus> Place, ConstRef<Replica> Record)
        {
            // A child goes with its parent, save beneath a persistent parent, which stays from disk without it.
            const ConstPtr<Replica> Above = Actor.GetParent().Get<const Replica>();

            if (Record.IsPersistent() || (Place.GetKey() == 0 && !(Above && Above->IsPersistent())))
            {
                return;
            }

            // What this side plays outlives the keys around it, since it arrives under the next key it stands under.
            if (Key != kGlobal && Actor.Has<Possessed>())
            {
                return;
            }

            // The key it stands under is its own, or its nearest ancestor's beneath a persistent parent.
            UInt64 Current = 0;

            for (ZyScene::Entity Up = Actor; Up.IsAlive() && Current == 0; Up = Up.GetParent())
            {
                const ConstPtr<Locus> Held = Up.Get<const Locus>();
                Current = Held ? Held->GetKey() : 0;
            }

            if (Current != 0 && (Key == kGlobal || Current == Key))
            {
                Doomed.Append(Actor);
            }
        });

        // A child gathered with its parent may have gone with it already.
        for (const ZyScene::Entity Actor : Doomed)
        {
            if (Actor.IsAlive())
            {
                Destroy(Actor);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Destroy(ZyScene::Entity Actor)
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

        if (!mManifest.Skip(Input))
        {
            LOG_E("Replication: A set this build cannot read arrived, so the publisher is refused");
            mRejected = true;
            return ConstSpan<Byte>();
        }
        return ConstSpan<Byte>(Input.GetData() + Start, Input.GetOffset() - Start);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Apply(ZyScene::Entity Actor, ConstSpan<Byte> Set, Mask Ignored)
    {
        Reader Input(Set);

        if (!mManifest.Read(Input, Actor, Ignored))
        {
            LOG_E("Replication: A set this build cannot read arrived, so the publisher is refused");
            mRejected = true;
        }
    }
}