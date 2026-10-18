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

namespace ZyScene::Protocol
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Publisher::Publisher(Ref<ZyEngine::Subsystem::Host> Host)
        : mScene      { * Host.GetService<Service>() },
          mJobs       { * Host.GetService<ZyJob::Service>() },
          mNetwork    { Host.GetService<ZyNetwork::Service>() },
          mPlaced     { mScene },
          mUprooted   { mScene },
          mClaimed    { mScene },
          mDisowned   { mScene },
          mWorld      { mScene.GetWorld().GetID() },
          mNext       { 1 },
          mTick       { 0 },
          mHeartbeat  { false },
          mPublishing { false }
    {
        mScene.Register<Replica>();

        // The global channel always stands, since every greeting reads its catch-up.
        Acquire(kGlobal);

        Adopt();
        Watch();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Publisher::Admit(ZyNetwork::Connection Link)
    {
        const UInt64 Peer = Link.GetValue();
        Expel(Peer);

        mSessions.Assign(Peer, Unique<Session>::Create(Peer, Link));
        return Peer;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Expel(UInt64 Peer)
    {
        ZY_ASSERT(!mPublishing, "A peer cannot be expelled while frames are handed out");

        // Seeing nothing lets go of every channel the peer saw.
        Subscribe(Peer, ConstSpan<UInt64>());
        mSessions.Erase(Peer);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Subscribe(UInt64 Peer, ConstSpan<UInt64> Keys)
    {
        const Ptr<Session> Target = Reach(Peer);

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

            ZY_ASSERT(Acquire(Gone).Subscribers > 0, "A channel lost a subscriber it never had");
            --Acquire(Gone).Subscribers;
        }

        // Every peer reads the global and private keys anyway, so neither is subscribed to.
        for (const UInt64 Asked : Keys)
        {
            if (Asked != kGlobal && Asked != kPrivate && Target->Keys.Insert(Asked))
            {
                Target->Fresh.Append(Asked);
                ++Acquire(Asked).Subscribers;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Publish()
    {
        ZY_PROFILE_SCOPE("Scene::Publish");

        Read(false, false);
        Drain();
        Proclaim();
        Deliver(* mScene.GetWorld().Get<const Clock>());
        Reset();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Publisher::Find(UInt64 Identifier) const
    {
        const Entity Actor = mReplicas.FindOrDefault(Identifier);
        return Actor.IsAlive() ? Actor : Entity();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Entity Publisher::Resolve(Ref<Link> Value) const
    {
        return Value.Resolve([this](UInt64 Identifier)
        {
            return Find(Identifier);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Adopt()
    {
        // A world loaded before the publisher already holds replicas its cursors never saw land.
        const ZyScene::Query Placed = mScene.Query<const Locus>().Also<Asleep>();

        Placed.Run([this](Entity Actor)
        {
            Place(Actor);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Watch()
    {
        ConstRef<Manifest> Table = Manifest::Get();

        // A component made replicated after the publisher is read from then on.
        for (UInt Index = mFeeds.GetSize(); Index < Table.GetCount(); ++Index)
        {
            mFeeds.Append(Table.GetEntry(static_cast<UInt8>(Index)).Watch(mScene));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Read(Bool Restoring, Bool Whole)
    {
        Watch();

        // Departures first, so a replica given its locus again within the tick keeps its identity.
        mUprooted.Each([this](Entity Actor)
        {
            Depart(Actor);
        });
        mPlaced.Each([this](Entity Actor)
        {
            Place(Actor);
        });
        mClaimed.Each([this](Entity Actor)
        {
            Claim(Actor);
        });
        mDisowned.Each([this](Entity Actor)
        {
            Claim(Actor);
        });

        for (UInt8 Index = 0; Index < mFeeds.GetSize(); ++Index)
        {
            Sift(Index, Restoring, Whole);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Sift(UInt8 Index, Bool Restoring, Bool Whole)
    {
        ConstRef<Manifest>  Table = Manifest::Get();
        Ref<Manifest::Feed> Lists = mFeeds[Index];

        mTouched.Clear();
        Lists.Removals(mTouched);

        if (!Restoring)
        {
            for (const Entity Actor : mTouched)
            {
                Mark(Actor, Index, true);
            }
        }

        // A rated component's changes wait in its list until its turn, merging there, unless every list is read.
        if (!Whole && !Table.IsDue(Index, mTick))
        {
            return;
        }

        mTouched.Clear();
        Lists.Changes(mTouched);

        // What a load wrote is the baseline.
        if (Restoring)
        {
            return;
        }

        for (const Entity Actor : mTouched)
        {
            Mark(Actor, Index, false);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Depart(Entity Actor)
    {
        const Ptr<Tracker> Tracking = Track(Actor);

        // Given its locus again within the tick, it is still the same replica.
        if (Tracking && !(Actor.IsAlive() && Actor.Owns<Locus>()))
        {
            Retire(Actor, * Tracking);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Place(Entity Actor)
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

    void Publisher::Claim(Entity Actor)
    {
        const Ptr<Tracker> Tracking = Track(Actor);

        // Not published yet, its arrival carries the owner anyway.
        if (Tracking && Tracking->Key != 0)
        {
            ZY_ASSERT((Tracking->Identifier & kPersistent) == 0, "A persistent replica cannot have an owner");

            Tracking->Reenter = true;
            Enqueue(Actor, * Tracking);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Mark(Entity Actor, UInt8 Index, Bool Gone)
    {
        // The world entity holds the singletons, which travel in the global channel.
        if (Actor.GetID() == mWorld)
        {
            if (Gone)
            {
                mDropped.Set(Index);
            }
            else
            {
                mDirty.Set(Index);
            }
            return;
        }

        if (const Ptr<Tracker> Tracking = Track(Actor))
        {
            if (Gone)
            {
                Tracking->Dropped.Set(Index);
            }
            else
            {
                Tracking->Dirty.Set(Index);
            }
            Enqueue(Actor, * Tracking);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Enqueue(Entity Actor, Ref<Tracker> Tracking)
    {
        if (!Tracking.Queued)
        {
            Tracking.Queued = true;
            mQueue.Append(Actor);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Retire(Entity Actor, Ref<Tracker> Tracking)
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
            Ref<Channel> Target = Acquire(Tracking.Key);

            if (Tracking.Parent == 0)
            {
                Target.Members.Erase(Actor.GetID());
            }

            if (!Persistent && !Orphaned)
            {
                Target.Stream.Open(Opcode::Forget).WriteUInt(Tracking.Identifier);
            }
        }
        mReplicas.Erase(Tracking.Identifier);
        Untrack(Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Drain()
    {
        // Walked by index, since publishing one replica may put another in line.
        for (UInt Index = 0; Index < mQueue.GetSize(); ++Index)
        {
            Process(mQueue[Index]);
        }
        mQueue.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Process(Entity Actor)
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
        if (Parent != 0 && !IsPublished(Parent, Key))
        {
            return;
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
            Relocate(Actor, * Tracking, Key);
            return;
        }
        else if (Tracking->Parent != Parent || Tracking->Reenter)
        {
            // Reattached under the same key or owned anew: arriving again is an upsert that sets it whole.
            Ref<Channel> Target = Acquire(Key);

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

    void Publisher::Arrive(Entity Actor, Ref<Tracker> Tracking, UInt64 Key, UInt64 Parent)
    {
        Ref<Channel> Target = Acquire(Key);

        // A channel's members are its roots; a child travels in its root's channel.
        if (Parent == 0)
        {
            Target.Members.Insert(Actor.GetID());
        }
        Tracking.Key    = Key;
        Tracking.Parent = Parent;

        // Peers load a persistent replica themselves; only its divergence travels.
        if ((Tracking.Identifier & kPersistent) == 0)
        {
            Reveal(Target.Stream, Actor, Tracking);
        }
        Descend(Actor, [this, &Target, Key](Entity Child)
        {
            Embrace(Target.Stream, Child, Key);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Relocate(Entity Actor, Ref<Tracker> Tracking, UInt64 Key)
    {
        Ref<Channel> Old    = Acquire(Tracking.Key);
        Ref<Writer>  Output = Old.Stream.Open(Opcode::Move);
        Output.WriteUInt(Tracking.Identifier);
        Output.WriteUInt(Key);

        if (Tracking.Parent == 0)
        {
            Old.Members.Erase(Actor.GetID());
        }
        Arrive(Actor, Tracking, Key, GetParentOf(Actor));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Reveal(Ref<Block> Output, Entity Actor, Ref<Tracker> Tracking)
    {
        WriteEnter(Output, Actor, Tracking);

        const Mask Owned = Manifest::Get().GetOwnerMask();

        if (Owned.Any())
        {
            if (const Ptr<Session> Owner = Witness(Actor, Tracking.Key))
            {
                WriteUpdate(Owner->Private, Tracking.Identifier, Actor, Owned, Mask());
            }
        }

        // Everything went out whole, so nothing waits any longer.
        Tracking.Dirty.Reset();
        Tracking.Dropped.Reset();
        Tracking.Reenter = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Embrace(Ref<Block> Output, Entity Child, UInt64 Key)
    {
        if (const Ptr<Tracker> Tracking = Track(Child))
        {
            Tracking->Key    = Key;
            Tracking->Parent = GetParentOf(Child);

            // A persistent child is loaded by the peer, so it only goes back in line for its divergence.
            if (Tracking->Identifier & kPersistent)
            {
                Enqueue(Child, * Tracking);
            }
            else
            {
                Reveal(Output, Child, * Tracking);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Broadcast(Entity Actor, Ref<Tracker> Tracking)
    {
        ConstRef<Manifest> Table = Manifest::Get();

        const Mask Wanted  = Exchange(Tracking.Dirty, Mask());
        const Mask Dropped = Exchange(Tracking.Dropped, Mask());
        const Mask Shared  = Table.GetSharedMask();
        const Mask Owned   = Table.GetOwnerMask();

        // A persistent replica remembers how it left disk, which a peer arriving later is told.
        if (Tracking.Identifier & kPersistent)
        {
            Tracking.Diverged |= Wanted | Dropped;
        }

        // Nobody reads the stream of a key no peer sees, and a peer that comes to see it reads its catch-up instead.
        Ref<Channel> Target = Acquire(Tracking.Key);

        if (Target.Subscribers > 0 && ((Wanted | Dropped) & Shared).Any())
        {
            WriteUpdate(Target.Stream, Tracking.Identifier, Actor, Wanted & Shared, Dropped & Shared);
        }

        if (((Wanted | Dropped) & Owned).Any())
        {
            if (const Ptr<Session> Owner = Witness(Actor, Tracking.Key))
            {
                WriteUpdate(Owner->Private, Tracking.Identifier, Actor, Wanted & Owned, Dropped & Owned);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Proclaim()
    {
        const Mask Shared  = Manifest::Get().GetSharedMask();
        const Mask Wanted  = Exchange(mDirty, Mask()) & Shared;
        const Mask Dropped = Exchange(mDropped, Mask()) & Shared;

        if ((Wanted | Dropped).Any())
        {
            WriteUpdate(Acquire(kGlobal).Stream, 0, mScene.GetWorld(), Wanted, Dropped);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Deliver(ConstRef<Clock> Time)
    {
        mHeartbeat = Time.IsEvery(kHeartbeat);

        mOrder.Clear();
        mSessions.ForEach([this](UInt64, Ref<Unique<Session>> Holder)
        {
            mOrder.Append(Holder.Grab());
        });

        // Catch-ups are built first, so assembling frames only reads channels.
        for (const Ptr<Session> Peer : mOrder)
        {
            Prepare(* Peer);
        }

        // Channels no longer change, and each worker writes only its own sessions' frames.
        const UInt32 Count = static_cast<UInt32>(mOrder.GetSize());

        mJobs.Parallel(ZyJob::Lane::Compute, Count, [this, &Time](UInt32 Begin, UInt32 End)
        {
            AssembleRange(Begin, End, Time);
        });

        mPublishing = true;

        for (const Ptr<Session> Peer : mOrder)
        {
            Hand(* Peer);
        }
        mPublishing = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Prepare(Ref<Session> Peer)
    {
        if (!Peer.Greeted)
        {
            Catchup(kGlobal);
        }

        for (const UInt64 Key : Peer.Fresh)
        {
            if (Peer.Keys.Contains(Key))
            {
                Catchup(Key);
                Bestow(Peer, Key);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Assemble(Ref<Session> Peer, ConstRef<Clock> Time)
    {
        Peer.Open(Time);

        const Ptr<Channel> Global = Seek(kGlobal);

        if (!Exchange(Peer.Greeted, true))
        {
            Peer.WriteHello(Manifest::Get().GetHash());
            Peer.WriteSection(kGlobal, Global->Catchup);
        }
        else if (Global->Stream.GetSize() > 0)
        {
            Peer.WriteSection(kGlobal, Global->Stream);
        }

        for (const UInt64 Key : Peer.Departed)
        {
            Peer.WriteLeave(Key);
        }

        // A new key gets everything under it in place of this tick's traffic.
        for (const UInt64 Key : Peer.Fresh)
        {
            if (Peer.Keys.Contains(Key))
            {
                Peer.WriteSection(Key, Seek(Key)->Catchup);
            }
        }

        for (const UInt64 Key : Peer.Keys)
        {
            const Ptr<Channel> Target = Seek(Key);

            if (Target && Target->Stream.GetSize() > 0 && !Peer.Fresh.Contains(Key))
            {
                Peer.WriteSection(Key, Target->Stream);
            }
        }

        if (Peer.Private.GetSize() > 0)
        {
            Peer.WriteSection(kPrivate, Peer.Private);
        }

        // The last frame goes when it carries anything, or when a heartbeat is due.
        if (Peer.Carries || mHeartbeat)
        {
            Peer.Ends.Append(Peer.Frame.GetSize());
        }

        // Packed here, so the cost is spread over the workers with the rest of the peer's frames.
        Peer.Pack();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::AssembleRange(UInt32 Begin, UInt32 End, ConstRef<Clock> Time)
    {
        for (UInt32 Index = Begin; Index < End; ++Index)
        {
            Assemble(* mOrder[Index], Time);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Hand(Ref<Session> Peer)
    {
        // Without a network, frames have nowhere to go.
        if (mNetwork)
        {
            for (ConstRef<Session::Parcel> Item : Peer.Parcels)
            {
                ConstRef<Writer> Bytes = Item.Shrunk ? Peer.Packed : Peer.Frame;
                mNetwork->Send(
                    Peer.Link,
                    ZyNetwork::Delivery::Reliable,
                    ConstSpan(Bytes.GetData() + Item.Start, Item.End - Item.Start));
            }
        }
        Peer.Reset();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstRef<Block> Publisher::Catchup(UInt64 Key)
    {
        Ref<Channel> Target = Acquire(Key);

        if (!Target.Caught)
        {
            Target.Caught = true;

            if (Key == kGlobal)
            {
                WriteUpdate(Target.Catchup, 0, mScene.GetWorld(), Manifest::Get().GetSharedMask(), Mask());
            }

            for (const UInt64 Member : Target.Members)
            {
                const Entity Actor = mScene.GetEntity(Member);

                WriteArrival(Target.Catchup, Actor);

                Descend(Actor, [this, &Target](Entity Child)
                {
                    if (Track(Child))
                    {
                        WriteArrival(Target.Catchup, Child);
                    }
                });
            }
        }
        return Target.Catchup;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Bestow(Ref<Session> Peer, UInt64 Key)
    {
        if (Manifest::Get().GetOwnerMask().None())
        {
            return;
        }

        for (const UInt64 Member : Acquire(Key).Members)
        {
            const Entity Actor = mScene.GetEntity(Member);

            Endow(Peer, Actor);

            Descend(Actor, [this, &Peer](Entity Child)
            {
                Endow(Peer, Child);
            });
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Endow(Ref<Session> Peer, Entity Actor)
    {
        const Ptr<Tracker> Tracking = Track(Actor);

        if (Tracking && GetOwner(Actor) == Peer.Key)
        {
            WriteUpdate(Peer.Private, Tracking->Identifier, Actor, Manifest::Get().GetOwnerMask(), Mask());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Reset()
    {
        // A channel nobody sees and nothing stands under goes, save the global one every greeting reads.
        mChannels.ForEach([](UInt64, Ref<Unique<Channel>> Holder)
        {
            Holder->Reset();
        });
        mChannels.EraseIf([](UInt64 Key, Ref<Unique<Channel>> Holder)
        {
            return Key != kGlobal && Holder->IsIdle();
        });

        ++mTick;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteArrival(Ref<Block> Output, Entity Actor)
    {
        ConstRef<Tracker> Tracking = * Track(Actor);

        if ((Tracking.Identifier & kPersistent) == 0)
        {
            WriteEnter(Output, Actor, Tracking);
            return;
        }

        // Both sides hold it from disk, so only what departed from disk is said.
        const Mask Wanted = Tracking.Diverged & Manifest::Get().GetSharedMask();

        if (Wanted.Any())
        {
            WriteUpdate(Output, Tracking.Identifier, Actor, Wanted, Mask());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteEnter(Ref<Block> Output, Entity Actor, ConstRef<Tracker> Tracking)
    {
        Ref<Writer> Bytes = Output.Open(Opcode::Enter);
        Bytes.WriteUInt(Tracking.Identifier);
        Bytes.WriteUInt(Tracking.Parent);

        // The archetype by its slot, which saves keep, so the peer finds the one it loaded there.
        Bytes.WriteUInt(Actor.GetArchetype().GetHandle().GetIndex());
        Bytes.WriteUInt(GetOwner(Actor));

        Manifest::Get().Write(Bytes, Actor, Manifest::Get().GetSharedMask(), Mask());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteUpdate(Ref<Block> Output, UInt64 Identifier, Entity Actor, Mask Wanted, Mask Dropped)
    {
        Ref<Writer> Bytes = Output.Open(Opcode::Update);
        Bytes.WriteUInt(Identifier);

        Manifest::Get().Write(Bytes, Actor, Wanted, Dropped);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Publisher::Channel> Publisher::Acquire(UInt64 Key)
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

    Ptr<Publisher::Channel> Publisher::Seek(UInt64 Key)
    {
        const Ptr<Unique<Channel>> Holder = mChannels.Find(Key);
        return Holder ? Holder->Grab() : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Publisher::Session> Publisher::Reach(UInt64 Peer)
    {
        const Ptr<Unique<Session>> Holder = Peer ? mSessions.Find(Peer) : nullptr;
        return Holder ? Holder->Grab() : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Publisher::Session> Publisher::Witness(Entity Actor, UInt64 Key)
    {
        const Ptr<Session> Owner = Reach(GetOwner(Actor));
        return Owner && Owner->Keys.Contains(Key) ? Owner : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Publisher::GetParentOf(Entity Actor)
    {
        if (IsRoot(Actor))
        {
            return 0;
        }

        for (Entity Above = Actor.GetParent(); Above.IsAlive(); Above = Above.GetParent())
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

    Bool Publisher::IsPublished(UInt64 Identifier, UInt64 Key)
    {
        const Entity       Actor    = Find(Identifier);
        const Ptr<Tracker> Tracking = Actor.IsAlive() ? Track(Actor) : nullptr;
        return Tracking && Tracking->Key == Key;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Publisher::GetKey(Entity Actor)
    {
        // A child stands under its root's key, the nearest ancestor naming one.
        for (Entity Above = Actor; Above.IsAlive(); Above = Above.GetParent())
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

    Bool Publisher::IsRoot(Entity Actor)
    {
        const ConstPtr<Locus> Place = Actor.Get<const Locus>();
        return Place && Place->GetKey() != 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Publisher::GetOwner(Entity Actor)
    {
        const ConstPtr<Owner> Holder = Actor.Get<const Owner>();
        return Holder ? Holder->GetPeer() : 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Publisher::Session::Session(UInt64 Peer, ZyNetwork::Connection Connection)
        : Key  { Peer },
          Link { Connection }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::Open(ConstRef<ZyScene::Clock> Clock)
    {
        Time       = Clock.GetAbsolute();
        Multiplier = Clock.GetMultiplier();
        Carries    = false;

        Frame.Clear();
        Ends.Clear();
        Header();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::WriteHello(UInt64 Hash)
    {
        Begin(Opcode::Hello, 2 * sizeof(UInt64));
        Frame.Write<UInt64>(Hash);
        Frame.Write<UInt64>(Key);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::WriteLeave(UInt64 Gone)
    {
        Begin(Opcode::Leave, 10);
        Frame.WriteUInt(Gone);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::WriteSection(UInt64 Section, ConstRef<Block> Messages)
    {
        const ConstPtr<Byte> Data  = Messages.Bytes.GetData();
        UInt32               Start = 0;

        // A section may repeat its key, so a busy channel goes out in pieces no frame outgrows.
        for (const UInt32 Cut : Messages.Cuts)
        {
            WritePiece(Section, ConstSpan(Data + Start, Cut - Start));
            Start = Cut;
        }
        WritePiece(Section, ConstSpan(Data + Start, Messages.GetSize() - Start));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::Pack()
    {
        Packed.Clear();
        Parcels.Clear();

        UInt32 Start = 0;

        for (const UInt32 End : Ends)
        {
            Seal(Start, End);
            Start = End;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::Seal(UInt32 Start, UInt32 End)
    {
        const UInt32 Size  = End - Start;
        const UInt32 Bound = LZ4Bound(Size);

        // Encoded where it would go, behind the room its opening takes, so nothing is copied after.
        Packed.Ensure(kPacked + Bound);

        const UInt32 First  = Packed.GetSize();
        const UInt32 Shrunk = LZ4Encode(
            ConstSpan(Frame.GetData() + Start, Size), Packed.GetData() + First + kPacked, Bound);

        // The opening costs five bytes, so a frame that would not shrink by more goes as it was built.
        if (Shrunk + kPacked < Size)
        {
            Packed.Write<UInt8>(static_cast<UInt8>(Opcode::Packed));
            Packed.Write<UInt32>(Size);
            Packed.Advance(Shrunk);
            Parcels.Append(true, First, Packed.GetSize());
        }
        else
        {
            Parcels.Append(false, Start, End);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::Reset()
    {
        Fresh.Clear();
        Departed.Clear();
        Private.Clear();
        Frame.Clear();
        Ends.Clear();
        Packed.Clear();
        Parcels.Clear();
        Carries = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::WritePiece(UInt64 Section, ConstSpan<Byte> Piece)
    {
        Begin(Opcode::Section, 10 + static_cast<UInt32>(Piece.GetSize()));
        Frame.WriteUInt(Section);

        if (!Piece.IsEmpty())
        {
            Frame.Write<Byte>(Piece.GetData(), static_cast<UInt32>(Piece.GetSize()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::Header()
    {
        Frame.Write<UInt8>(static_cast<UInt8>(Opcode::Frame));
        Frame.Write<Real64>(Time);
        Frame.Write<Real32>(Multiplier);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Session::Begin(Opcode Code, UInt32 Needed)
    {
        ZY_ASSERT(kHeader + 1 + Needed <= ZyNetwork::kMaxFrame, "A protocol message is larger than a network frame");

        const UInt32 Start = Ends.IsEmpty() ? 0 : Ends.GetBack();

        // Every section is whole on its own, so a frame may end between any two of them.
        if (Carries && Frame.GetSize() - Start + 1 + Needed > kMaxChunk)
        {
            Ends.Append(Frame.GetSize());
            Header();
        }

        Frame.Write<UInt8>(static_cast<UInt8>(Code));
        Carries = true;
    }
}