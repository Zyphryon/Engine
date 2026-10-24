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

#include "Publisher.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Publisher::Publisher(
        Ref<ZyScene::Service>   Scene,
        Ref<ZyJob::Service>     Jobs,
        Ref<ZyNetwork::Service> Network,
        ConstRef<Manifest>      Table)
        : mScene    { Scene },
          mJobs     { Jobs },
          mNetwork  { Network },
          mManifest { Table },
          mLoci     { Manifest::Watch<Locus>(Scene) },
          mOwners   { Manifest::Watch<Owner>(Scene) },
          mNext     { 1 },
          mTick     { 0 }
    {
        // The table is sealed, so every component's readers are made once.
        for (UInt8 Index = 0; Index < Table.GetCount(); ++Index)
        {
            mFeeds.Append(Table.GetEntry(Index).Adapter.Watch(Scene));
        }

        // The global channel always stands, since every greeting reads its catch-up.
        GetChannel(kGlobal);

        // The world holds the singletons, whose changes its own tracker marks.
        Enlist(Scene.GetWorld());

        // A world loaded before the publisher already holds replicas its readers never saw land.
        const ZyScene::Query Placed = Scene.Query<const Locus>().Also<ZyScene::Asleep>();

        Placed.Run([this](ZyScene::Entity Actor)
        {
            Place(Actor);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Publisher::Admit(ZyNetwork::Connection Link)
    {
        const UInt64 Peer = Link.GetValue();
        Expel(Peer);

        mSessions.Assign(Peer, Unique<Session>::Create(Link));
        return Peer;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ZyNetwork::Connection Publisher::Expel(UInt64 Peer)
    {
        const Ptr<Session> Target = FindSession(Peer);

        if (Target == nullptr)
        {
            return ZyNetwork::Connection();
        }

        // Seeing nothing lets go of every channel the peer saw.
        const ZyNetwork::Connection Link = Target->Link;
        Subscribe(Peer, ConstSpan<UInt64>());
        mSessions.Erase(Peer);
        return Link;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Subscribe(UInt64 Peer, ConstSpan<UInt64> Keys)
    {
        const Ptr<Session> Target = FindSession(Peer);

        if (Target == nullptr)
        {
            return;
        }

        // What the peer stops seeing is gathered first, since the bag cannot be walked while it is emptied.
        const UInt First = Target->Departed.GetSize();

        for (const UInt64 Seen : Target->Keys)
        {
            if (!Keys.Contains(Seen))
            {
                Target->Departed.Append(Seen);
            }
        }

        for (UInt Index = First; Index < Target->Departed.GetSize(); ++Index)
        {
            const UInt64 Gone = Target->Departed[Index];
            Target->Keys.Erase(Gone);

            ZY_ASSERT(GetChannel(Gone).Subscribers > 0, "A channel lost a subscriber it never had");
            --GetChannel(Gone).Subscribers;
        }

        // Every peer reads the global and private keys anyway, so neither is subscribed to.
        for (const UInt64 Asked : Keys)
        {
            if (Asked != kGlobal && Asked != kPrivate && Target->Keys.Insert(Asked))
            {
                Target->Fresh.Append(Asked);
                ++GetChannel(Asked).Subscribers;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Publish()
    {
        ZY_PROFILE_SCOPE("Replication::Publish");

        Read(false, false);

        // Walked by index, since publishing one replica may put another in line.
        for (UInt Index = 0; Index < mQueue.GetSize(); ++Index)
        {
            Process(mQueue[Index]);
        }
        mQueue.Clear();

        // The world's singletons travel in the global channel.
        const ZyScene::Entity World   = mScene.GetWorld();
        Ref<Tracker>          Marks   = * Track(World);
        const Mask            Shared  = mManifest.GetSharedMask();
        const Mask            Wanted  = Exchange(Marks.Dirty, Mask()) & Shared;
        const Mask            Dropped = Exchange(Marks.Dropped, Mask()) & Shared;

        // The world never arrives, so a singleton set mid-session travels whole like every other.
        if ((Wanted | Dropped).Any())
        {
            WriteUpdate(GetChannel(kGlobal).Stream, 0, World, Wanted, Dropped, Mask());
        }

        Deliver(* World.Get<const ZyScene::Clock>());

        // A channel nobody sees and nothing stands under goes, save the global one every greeting reads.
        mChannels.EraseIf([](UInt64 Key, Ref<Unique<Channel>> Holder)
        {
            Holder->Stream.Bytes.Clear();
            Holder->Stream.Cuts.Clear();
            Holder->Catchup.Bytes.Clear();
            Holder->Catchup.Cuts.Clear();
            return Key != kGlobal && Holder->Members.IsEmpty() && Holder->Subscribers == 0;
        });
        ++mTick;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Post(UInt64 Peer, ConstSpan<Byte> Message)
    {
        // It rides in the peer's private section, which only that peer reads.
        if (const Ptr<Session> Target = FindSession(Peer))
        {
            WriteMessage(Target->Private, 0, Message);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Post(ZyScene::Entity Actor, Audience Audience, ConstSpan<Byte> Message)
    {
        const Bool         World    = (Actor == mScene.GetWorld());
        const Ptr<Tracker> Tracking = Track(Actor);

        // A replica published under no key yet is seen by nobody, while the world is seen by every peer.
        if (Tracking == nullptr || (!World && Tracking->Key == kGlobal))
        {
            return;
        }

        if (Audience == Audience::Owner)
        {
            Post(GetOwner(Actor), Message);
            return;
        }

        // It rides in the channel the entity stands under, which every peer that sees it reads.
        const UInt64 Skipped = (Audience == Audience::Others ? GetOwner(Actor) : 0);
        WriteMessage(GetChannel(World ? kGlobal : Tracking->Key).Stream, Skipped, Message);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Read(Bool Restoring, Bool Whole)
    {
        // Departures first, so a replica given its locus again within the tick keeps its identity.
        mTouched.Clear();
        mLoci.Removals(mTouched);

        for (const ZyScene::Entity Actor : mTouched)
        {
            if (const Ptr<Tracker> Tracking = Track(Actor); Tracking && !(Actor.IsAlive() && Actor.Owns<Locus>()))
            {
                Retire(Actor, * Tracking);
            }
        }

        mTouched.Clear();
        mLoci.Changes(mTouched);

        for (const ZyScene::Entity Actor : mTouched)
        {
            Place(Actor);
        }

        // A published replica whose owner changed arrives whole again, which names the owner, while one not yet
        // published names it in its first arrival anyway.
        mTouched.Clear();
        mOwners.Changes(mTouched);
        mOwners.Removals(mTouched);

        for (const ZyScene::Entity Actor : mTouched)
        {
            if (const Ptr<Tracker> Tracking = Track(Actor); Tracking && Tracking->Key != 0)
            {
                ZY_ASSERT((Tracking->Identifier & kPersistent) == 0, "A persistent replica cannot have an owner");

                Tracking->Reenter = true;
                Enqueue(Actor, * Tracking);
            }
        }

        for (UInt8 Index = 0; Index < mFeeds.GetSize(); ++Index)
        {
            mTouched.Clear();
            mFeeds[Index].Removals(mTouched);

            const UInt Lost = mTouched.GetSize();

            // A rated component's changes wait in its list until its turn, merging there, unless every list is read.
            if (Whole || mManifest.IsDue(Index, mTick))
            {
                mFeeds[Index].Changes(mTouched);
            }

            // What a load wrote is the baseline, so it marks nothing.
            for (UInt Entry = 0; Entry < mTouched.GetSize() && !Restoring; ++Entry)
            {
                if (const Ptr<Tracker> Tracking = Track(mTouched[Entry]))
                {
                    (Entry < Lost ? Tracking->Dropped : Tracking->Dirty).Set(Index);
                    Enqueue(mTouched[Entry], * Tracking);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ZyScene::Entity Publisher::Find(UInt64 Identifier) const
    {
        const ZyScene::Entity Actor = mReplicas.FindOrDefault(Identifier);
        return Actor.IsAlive() ? Actor : ZyScene::Entity();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Place(ZyScene::Entity Actor)
    {
        // What is kept to one side never travels.
        if (!Actor.IsAlive() || Actor.Has<Server>() || Actor.Has<Client>())
        {
            return;
        }

        Ref<Tracker> Tracking = Enlist(Actor);

        if (Tracking.Identifier == 0)
        {
            const ConstPtr<Replica> Record = Actor.Get<const Replica>();

            if (Record && Record->IsPersistent())
            {
                Tracking.Identifier = Record->GetIdentifier();
                mReplicas.Assign(Tracking.Identifier, Actor);
            }
            else
            {
                Tracking.Identifier = mNext++;
                mReplicas.Assign(Tracking.Identifier, Actor);

                // Given now, before anything is written, so a link to it written this tick names it.
                Actor.Set(Replica(Tracking.Identifier));
            }
        }
        Enqueue(Actor, Tracking);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Enqueue(ZyScene::Entity Actor, Ref<Tracker> Tracking)
    {
        if (!Tracking.Queued)
        {
            Tracking.Queued = true;
            mQueue.Append(Actor);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Retire(ZyScene::Entity Actor, Ref<Tracker> Tracking)
    {
        ZY_ASSERT(
            !Actor.IsAlive() || Tracking.Parent != 0 || Tracking.Key == 0,
            "A published root cannot lose its locus alive");

        // A child that died with its parent goes on the peer with it, so it needs no word of its own.
        const Bool Orphaned   = Tracking.Parent != 0 && !Find(Tracking.Parent).IsAlive();

        // Peers load a persistent replica from disk, so they keep their copy.
        const Bool Persistent = (Tracking.Identifier & kPersistent) != 0;

        if (Tracking.Key != 0)
        {
            Ref<Channel> Target = GetChannel(Tracking.Key);

            if (Tracking.Parent == 0)
            {
                Target.Members.Erase(Actor.GetID());
            }

            if (!Persistent && !Orphaned)
            {
                Ref<Writer> Output = Open(Target.Stream, Opcode::Forget);
                Output.WriteUInt(Tracking.Identifier);
                Output.WriteUInt(0);
            }
        }
        mReplicas.Erase(Tracking.Identifier);
        Tracking = Tracker();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Process(ZyScene::Entity Actor)
    {
        const Ptr<Tracker> Tracking = Track(Actor);

        if (Tracking == nullptr)
        {
            return;
        }
        Tracking->Queued = false;

        // A replica that died since it was put in line is retired by the next read of its removal.
        if (!Actor.IsAlive())
        {
            return;
        }

        // Without a key nobody sees it yet, and setting one puts it in line again.
        const UInt64 Key = GetKey(Actor);

        if (Key == 0)
        {
            ZY_ASSERT(Tracking->Key == 0, "A published replica lost its key");
            return;
        }

        const Bool   Persistent = (Tracking->Identifier & kPersistent) != 0;
        const UInt64 Parent     = GetParentOf(Actor);

        // A child waits until its parent stands under the same key, since the parent's arrival there carries it.
        if (Parent != 0)
        {
            const ZyScene::Entity Above   = Find(Parent);
            const Ptr<Tracker>    Holding = Above.IsAlive() ? Track(Above) : nullptr;

            if (Holding == nullptr || Holding->Key != Key)
            {
                return;
            }
        }

        if (Tracking->Key == 0)
        {
            Arrive(Actor, * Tracking, Key, Parent);

            // A runtime arrival carried everything, while a persistent one still has its changes to send.
            if (!Persistent)
            {
                return;
            }
        }
        else if (Tracking->Key != Key)
        {
            // Peers load a persistent replica from the key it was placed under, so it never moves.
            ZY_ASSERT(!Persistent, "A persistent replica changed key");

            Depart(Actor, * Tracking, Key);
            Arrive(Actor, * Tracking, Key, Parent);
            return;
        }
        else if (Tracking->Parent != Parent || Tracking->Reenter)
        {
            // Reattached under the same key or owned anew, arriving again is an upsert that sets it whole.
            Ref<Channel> Target = GetChannel(Key);

            if (Tracking->Parent != Parent)
            {
                if (Tracking->Parent == 0)
                {
                    Target.Members.Erase(Actor.GetID());
                }
                if (Parent == 0)
                {
                    Target.Members.Insert(Actor.GetID());
                }
                Tracking->Parent = Parent;
            }
            Reveal(Target.Stream, Actor, * Tracking);
            return;
        }
        Broadcast(Actor, * Tracking);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Arrive(ZyScene::Entity Actor, Ref<Tracker> Tracking, UInt64 Key, UInt64 Parent)
    {
        Ref<Channel> Target = GetChannel(Key);

        // A channel's members are its roots, while a child travels in its root's channel.
        if (Parent == 0)
        {
            Target.Members.Insert(Actor.GetID());
        }
        Tracking.Key    = Key;
        Tracking.Parent = Parent;

        // Peers load a persistent replica themselves, so only its divergence travels.
        if ((Tracking.Identifier & kPersistent) == 0)
        {
            Reveal(Target.Stream, Actor, Tracking);
        }

        Descend(Actor, [this, Key](ZyScene::Entity Child)
        {
            const Ptr<Tracker> Below = Track(Child);

            if (Below == nullptr)
            {
                return;
            }

            // A persistent child is loaded by the peer, so it only goes back in line for its divergence.
            if (Below->Identifier & kPersistent)
            {
                Below->Key    = Key;
                Below->Parent = GetParentOf(Child);
                Enqueue(Child, * Below);
                return;
            }

            // A child published under another key before this one arrived leaves it first, or its peers keep it.
            if (Below->Key != 0 && Below->Key != Key)
            {
                Depart(Child, * Below, Key);
            }
            Below->Key    = Key;
            Below->Parent = GetParentOf(Child);

            // Looked up again, since a departure may have made room in the channels for the key it left.
            Reveal(GetChannel(Key).Stream, Child, * Below);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Depart(ZyScene::Entity Actor, ConstRef<Tracker> Tracking, UInt64 Key)
    {
        Ref<Channel> Old    = GetChannel(Tracking.Key);
        Ref<Writer>  Output = Open(Old.Stream, Opcode::Forget);
        Output.WriteUInt(Tracking.Identifier);
        Output.WriteUInt(Key);

        if (Tracking.Parent == 0)
        {
            Old.Members.Erase(Actor.GetID());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Reveal(Ref<Block> Output, ZyScene::Entity Actor, Ref<Tracker> Tracking)
    {
        WriteEnter(Output, Actor, Tracking);

        const Mask Owned = mManifest.GetOwnerMask();

        if (Owned.Any())
        {
            if (const Ptr<Session> Owner = FindOwner(Actor, Tracking.Key))
            {
                WriteUpdate(Owner->Private, Tracking.Identifier, Actor, Owned, Mask(), Mask());
            }
        }

        // Everything went out whole, so nothing waits any longer.
        Tracking.Dirty.Reset();
        Tracking.Dropped.Reset();
        Tracking.Reenter = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Broadcast(ZyScene::Entity Actor, Ref<Tracker> Tracking)
    {
        const Mask Wanted  = Exchange(Tracking.Dirty, Mask());
        const Mask Dropped = Exchange(Tracking.Dropped, Mask());
        const Mask Shared  = mManifest.GetSharedMask();
        const Mask Owned   = mManifest.GetOwnerMask();

        // A persistent replica remembers how it left disk, which a peer arriving later is told.
        if (Tracking.Identifier & kPersistent)
        {
            Tracking.Diverged |= Wanted | Dropped;
        }

        // Peers hold what the replica arrived or loaded with, which is set at spawn, save what was lost and set again.
        const Mask Delta = Wanted & ~Dropped;

        // Nobody reads the stream of a key no peer sees, and a peer that comes to see it reads its catch-up instead.
        Ref<Channel> Target = GetChannel(Tracking.Key);

        if (Target.Subscribers > 0 && ((Wanted | Dropped) & Shared).Any())
        {
            WriteUpdate(Target.Stream, Tracking.Identifier, Actor, Wanted & Shared, Dropped & Shared, Delta);
        }

        if (((Wanted | Dropped) & Owned).Any())
        {
            if (const Ptr<Session> Owner = FindOwner(Actor, Tracking.Key))
            {
                WriteUpdate(Owner->Private, Tracking.Identifier, Actor, Wanted & Owned, Dropped & Owned, Delta);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Deliver(ConstRef<ZyScene::Clock> Time)
    {
        // Catch-ups are built first, so assembling frames only reads channels.
        mSessions.ForEach([this](UInt64, Ref<Unique<Session>> Peer)
        {
            if (!Peer->Greeted)
            {
                Catchup(kGlobal);
            }

            for (const UInt64 Key : Peer->Fresh)
            {
                if (Peer->Keys.Contains(Key))
                {
                    Catchup(Key);
                    WriteOwned(* Peer, Key);
                }
            }
        });

        // Channels no longer change, and each worker writes only its own sessions' frames.
        const Bool   Heartbeat = Time.IsEvery(kHeartbeat);
        const UInt32 Count     = static_cast<UInt32>(mSessions.GetSize());

        mJobs.Parallel(ZyJob::Lane::Compute, Count, [this, &Time, Heartbeat](UInt32 First, UInt32 Last)
        {
            for (UInt32 Index = First; Index < Last; ++Index)
            {
                Assemble(* mSessions.GetData()[Index].Second, Time, Heartbeat);
            }
        });

        mSessions.ForEach([this](UInt64, Ref<Unique<Session>> Peer)
        {
            UInt32 Start = 0;

            for (const UInt32 End : Peer->Ends)
            {
                const ConstSpan<Byte> Frame(Peer->Out.GetData() + Start, End - Start);
                mNetwork.Send(Peer->Link, ZyNetwork::Delivery::Reliable, Frame);
                Start = End;
            }
            Peer->Fresh.Clear();
            Peer->Departed.Clear();
            Peer->Private.Bytes.Clear();
            Peer->Private.Cuts.Clear();
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Assemble(Ref<Session> Peer, ConstRef<ZyScene::Clock> Time, Bool Heartbeat)
    {
        Peer.Out.Clear();
        Peer.Ends.Clear();

        Peer.Frame.Clear();
        Peer.Frame.Write<UInt8>(static_cast<UInt8>(Opcode::Frame));
        Peer.Frame.Write<Real64>(Time.GetAbsolute());
        Peer.Frame.Write<Real32>(Time.GetMultiplier());

        const Ptr<Channel> Global = FindChannel(kGlobal);

        if (!Exchange(Peer.Greeted, true))
        {
            Begin(Peer, Opcode::Hello, 2 * sizeof(UInt64));
            Peer.Frame.Write<UInt64>(mManifest.GetHash());
            Peer.Frame.Write<UInt64>(Peer.Link.GetValue());
            WriteSection(Peer, kGlobal, Global->Catchup);
        }
        else if (Global->Stream.Bytes.GetSize() > 0)
        {
            WriteSection(Peer, kGlobal, Global->Stream);
        }

        for (const UInt64 Key : Peer.Departed)
        {
            Begin(Peer, Opcode::Leave, 10);
            Peer.Frame.WriteUInt(Key);
        }

        // A new key gets everything under it in place of this tick's traffic.
        for (const UInt64 Key : Peer.Fresh)
        {
            if (Peer.Keys.Contains(Key))
            {
                WriteSection(Peer, Key, FindChannel(Key)->Catchup);
            }
        }

        for (const UInt64 Key : Peer.Keys)
        {
            const Ptr<Channel> Target = FindChannel(Key);

            if (Target && Target->Stream.Bytes.GetSize() > 0 && !Peer.Fresh.Contains(Key))
            {
                WriteSection(Peer, Key, Target->Stream);
            }
        }

        if (Peer.Private.Bytes.GetSize() > 0)
        {
            WriteSection(Peer, kPrivate, Peer.Private);
        }

        // The last frame goes when it carries anything, or when a heartbeat is due.
        if (Peer.Frame.GetSize() > kHeader || Heartbeat)
        {
            Seal(Peer);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstRef<Publisher::Block> Publisher::Catchup(UInt64 Key)
    {
        Ref<Channel> Target = GetChannel(Key);

        // Built once per tick, and a channel holding nothing costs nothing to build again.
        if (Target.Catchup.Bytes.GetSize() > 0)
        {
            return Target.Catchup;
        }

        if (Key == kGlobal)
        {
            WriteUpdate(Target.Catchup, 0, mScene.GetWorld(), mManifest.GetSharedMask(), Mask(), Mask());
        }

        const auto Arrival = [this, &Target](ZyScene::Entity Actor)
        {
            const Ptr<Tracker> Tracking = Track(Actor);

            if (Tracking == nullptr)
            {
                return;
            }

            // Both sides hold a persistent replica from disk, so only what departed from disk is said.
            if ((Tracking->Identifier & kPersistent) == 0)
            {
                WriteEnter(Target.Catchup, Actor, * Tracking);
            }
            else if (const Mask Wanted = Tracking->Diverged & mManifest.GetSharedMask(); Wanted.Any())
            {
                WriteUpdate(Target.Catchup, Tracking->Identifier, Actor, Wanted, Mask(), Mask());
            }
        };

        for (const UInt64 Member : Target.Members)
        {
            const ZyScene::Entity Actor = mScene.GetEntity(Member);

            Arrival(Actor);
            Descend(Actor, Arrival);
        }
        return Target.Catchup;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteOwned(Ref<Session> Peer, UInt64 Key)
    {
        const Mask Owned = mManifest.GetOwnerMask();

        if (Owned.None())
        {
            return;
        }

        const auto Endow = [this, &Peer, Owned](ZyScene::Entity Actor)
        {
            const Ptr<Tracker> Tracking = Track(Actor);

            if (Tracking && GetOwner(Actor) == Peer.Link.GetValue())
            {
                WriteUpdate(Peer.Private, Tracking->Identifier, Actor, Owned, Mask(), Mask());
            }
        };

        for (const UInt64 Member : GetChannel(Key).Members)
        {
            const ZyScene::Entity Actor = mScene.GetEntity(Member);

            Endow(Actor);
            Descend(Actor, Endow);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteEnter(Ref<Block> Output, ZyScene::Entity Actor, ConstRef<Tracker> Tracking)
    {
        Ref<Writer> Bytes = Open(Output, Opcode::Enter);
        Bytes.WriteUInt(Tracking.Identifier);
        Bytes.WriteUInt(Tracking.Parent);

        // The archetype by its slot, which saves keep, so the peer finds the one it loaded there.
        Bytes.WriteUInt(static_cast<UInt32>(Actor.GetArchetype().GetID()));
        Bytes.WriteUInt(GetOwner(Actor));

        mManifest.Write(Bytes, Actor, mManifest.GetSharedMask(), Mask(), Mask());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteUpdate(
        Ref<Block>      Output,
        UInt64          Identifier,
        ZyScene::Entity Actor,
        Mask            Wanted,
        Mask            Dropped,
        Mask            Delta)
    {
        Ref<Writer> Bytes = Open(Output, Opcode::Update);
        Bytes.WriteUInt(Identifier);

        mManifest.Write(Bytes, Actor, Wanted, Dropped, Delta);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Publisher::Channel> Publisher::GetChannel(UInt64 Key)
    {
        Ref<Unique<Channel>> Holder = mChannels.FindOrInsert(Key);

        if (!Holder)
        {
            Holder = Unique<Channel>::Create();
        }
        return * Holder;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Publisher::Channel> Publisher::FindChannel(UInt64 Key)
    {
        const Ptr<Unique<Channel>> Holder = mChannels.Find(Key);
        return Holder ? Holder->Grab() : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Publisher::Session> Publisher::FindSession(UInt64 Peer)
    {
        const Ptr<Unique<Session>> Holder = Peer ? mSessions.Find(Peer) : nullptr;
        return Holder ? Holder->Grab() : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Publisher::Session> Publisher::FindOwner(ZyScene::Entity Actor, UInt64 Key)
    {
        const Ptr<Session> Owner = FindSession(GetOwner(Actor));
        return Owner && Owner->Keys.Contains(Key) ? Owner : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Publisher::GetParentOf(ZyScene::Entity Actor)
    {
        if (IsRoot(Actor))
        {
            return 0;
        }

        for (ZyScene::Entity Above = Actor.GetParent(); Above.IsAlive(); Above = Above.GetParent())
        {
            if (const Ptr<Tracker> Tracking = Track(Above))
            {
                return Tracking->Identifier;
            }
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Writer> Publisher::Open(Ref<Block> Output, Opcode Code)
    {
        const UInt32 Last = Output.Cuts.IsEmpty() ? 0 : Output.Cuts.GetBack();

        if (Output.Bytes.GetSize() - Last >= kMaxChunk)
        {
            Output.Cuts.Append(Output.Bytes.GetSize());
        }
        Output.Bytes.Write<UInt8>(static_cast<UInt8>(Code));
        return Output.Bytes;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Begin(Ref<Session> Peer, Opcode Code, UInt32 Needed)
    {
        ZY_ASSERT(kHeader + 1 + Needed <= ZyNetwork::kMaxFrame, "A protocol message is larger than a network frame");

        // Every section is whole on its own, so a frame may end between any two of them.
        if (Peer.Frame.GetSize() > kHeader && Peer.Frame.GetSize() + 1 + Needed > kMaxChunk)
        {
            Seal(Peer);
        }
        Peer.Frame.Write<UInt8>(static_cast<UInt8>(Code));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Seal(Ref<Session> Peer)
    {
        const UInt32 Size  = Peer.Frame.GetSize();
        const UInt32 Bound = LZ4Bound(Size);

        // Encoded where it goes, behind the room its opening takes, so nothing is copied after.
        Peer.Out.Ensure(kPacked + Bound);

        const Ptr<Byte> Room   = Peer.Out.GetData() + Peer.Out.GetSize();
        const UInt32    Shrunk = LZ4Encode(ConstSpan(Peer.Frame.GetData(), Size), Room + kPacked, Bound);

        // The opening costs five bytes, so a frame that would not shrink by more goes as it was built.
        if (Shrunk + kPacked < Size)
        {
            Peer.Out.Write<UInt8>(static_cast<UInt8>(Opcode::Packed));
            Peer.Out.Write<UInt32>(Size);
            Peer.Out.Advance(Shrunk);
        }
        else
        {
            Peer.Out.Write<Byte>(Peer.Frame.GetData(), Size);
        }
        Peer.Ends.Append(Peer.Out.GetSize());

        // The clock stays in place, so the next frame opens with it.
        Peer.Frame.Clear();
        Peer.Frame.Advance(kHeader);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteSection(Ref<Session> Peer, UInt64 Key, ConstRef<Block> Messages)
    {
        const ConstPtr<Byte> Data  = Messages.Bytes.GetData();
        UInt32               Start = 0;

        // A section may repeat its key, so a busy channel goes out in pieces no frame outgrows.
        const auto Piece = [&Peer, Key, Data](UInt32 First, UInt32 End)
        {
            Begin(Peer, Opcode::Section, 10 + (End - First));
            Peer.Frame.WriteUInt(Key);

            if (End > First)
            {
                Peer.Frame.Write<Byte>(Data + First, End - First);
            }
        };

        for (const UInt32 Cut : Messages.Cuts)
        {
            Piece(Start, Cut);
            Start = Cut;
        }
        Piece(Start, Messages.Bytes.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteMessage(Ref<Block> Output, UInt64 Skipped, ConstSpan<Byte> Message)
    {
        if (Message.GetSize() > kMaxChunk)
        {
            LOG_E("Replication: A message of {0} bytes is larger than a frame takes", Message.GetSize());
            return;
        }

        Ref<Writer> Bytes = Open(Output, Opcode::Message);
        Bytes.WriteUInt(Skipped);
        Bytes.WriteUInt(static_cast<UInt32>(Message.GetSize()));
        Bytes.Write(Message.GetData(), static_cast<UInt32>(Message.GetSize()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Publisher::GetKey(ZyScene::Entity Actor)
    {
        // A child stands under its root's key, the nearest ancestor naming one.
        for (ZyScene::Entity Above = Actor; Above.IsAlive(); Above = Above.GetParent())
        {
            if (const ConstPtr<Locus> Place = Above.Get<const Locus>(); Place && Place->GetKey() != 0)
            {
                return Place->GetKey();
            }
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Publisher::IsRoot(ZyScene::Entity Actor)
    {
        const ConstPtr<Locus> Place = Actor.Get<const Locus>();
        return Place && Place->GetKey() != 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Publisher::GetOwner(ZyScene::Entity Actor)
    {
        const ConstPtr<Owner> Holder = Actor.Get<const Owner>();
        return Holder ? Holder->GetPeer() : 0;
    }
}