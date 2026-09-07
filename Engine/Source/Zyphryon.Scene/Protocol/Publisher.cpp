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

namespace Scene::Protocol
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Publisher::Publisher(Ref<Engine::Subsystem::Host> Host)
        : Locator { Host },
          mSweep  { 0 }
    {
        Ref<Service> Scene = GetService<Service>();

        // From here on a touch has somebody to reach, so the ledger starts keeping what was touched.
        Scene.GetWorld().Get<Ledger>().Publishing = true;

        // Observes a replica landing on an entity, which the next publish then announces to its scope.
        mObservers[0] = Scene.CreateObserver<>(
            "Scene::Publisher::ObsSetReplica",
            EcsOnSet,
            [this](Entity Actor, ConstRef<Replica> Record)
            {
                // A file that held a runtime replica reads one back that names nothing, and it has nothing to say.
                if (!Record.IsValid())
                {
                    return;
                }

                Ref<Ledger>  Book     = GetService<Service>().GetWorld().Get<Ledger>();
                Ref<Tracker> Tracking = Actor.Get<Tracker>();

                if (!Tracking.Queued)
                {
                    Tracking.Queued = true;
                    Book.Pending.Append(Actor);
                }

                // A replica held from disk only ever lands from disk, so outside a restore a wrap was forgotten.
                if (Record.IsPersistent() && !Book.Restoring)
                {
                    LOG_W("Scene: Persistent replica {0} landed outside a restore", Record.GetIdentifier());
                }
            });

        // Observes a replica leaving an entity, so whoever was told about it is told to forget it.
        mObservers[1] = Scene.CreateObserver<>(
            "Scene::Publisher::ObsRemoveReplica",
            EcsOnRemove,
            [this](Entity Actor, ConstRef<Replica> Record)
            {
                const ConstPtr<Tracker> Tracking = Actor.TryGet<const Tracker>();

                if (Tracking && Tracking->Announced && !Record.IsPersistent())
                {
                    mDepartures.Append(Departure(Record.GetIdentifier(), Tracking->Scope));
                }
            });

        // Observes a scope landing on an entity to tell whoever already subscribed to its key what stands beneath it.
        mObservers[2] = Scene.CreateObserver<>(
            "Scene::Publisher::ObsSetScope",
            EcsOnSet,
            [this](Entity Actor, ConstRef<Scope> Component)
            {
                Ref<Group> Group = mGroups.FindOrInsert(Component.GetKey());
                Group.Actor = Actor;

                for (const Ptr<Member> Peer : Group.Subscribers)
                {
                    Announce(Actor, * Peer, true);
                }
            });

        // Observes a scope leaving an entity, which keeps its subscribers since the key may land again.
        mObservers[3] = Scene.CreateObserver<>(
            "Scene::Publisher::ObsRemoveScope",
            EcsOnRemove,
            [this](Entity Actor, ConstRef<Scope> Component)
            {
                if (const Ptr<Group> Group = mGroups.Find(Component.GetKey()); Group && Group->Actor == Actor)
                {
                    Group->Actor = Entity();
                }
            });

        // Observes every replicated component landing on, being set on or leaving a replica, so a game that adds,
        // sets or removes a component never has to touch it by hand; a system writing through a query still does.
        ConstRef<Manifest> Table = Manifest::Get();

        for (UInt8 Index = 0; Index < Table.GetCount(); ++Index)
        {
            const Entity Component(Scene.GetWorld().GetHandle(), Table.GetEntry(Index).Component);

            mWatchers.Append(Scene.CreateObserver(Text::Empty(), EcsOnAdd, [Index](Entity Actor, Ref<Tracker> Tracking)
            {
                if (Enlist(Actor, Tracking))
                {
                    Tracking.Touched.Set(Index);
                    Tracking.Removed.Reset(Index);
                }
            }, DSL::In(Component)));

            mWatchers.Append(Scene.CreateObserver(Text::Empty(), EcsOnSet, [Index](Entity Actor, Ref<Tracker> Tracking)
            {
                if (Enlist(Actor, Tracking))
                {
                    Tracking.Touched.Set(Index);
                    Tracking.Removed.Reset(Index);
                }
            }, DSL::In(Component)));

            mWatchers.Append(Scene.CreateObserver(Text::Empty(), EcsOnRemove, [Index](Entity Actor, Ref<Tracker> Tracking)
            {
                if (Enlist(Actor, Tracking))
                {
                    Tracking.Removed.Set(Index);
                    Tracking.Touched.Reset(Index);
                }
            }, DSL::In(Component)));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Publisher::~Publisher()
    {
        for (UInt Index = 0; Index < mObservers.GetSize(); ++Index)
        {
            mObservers[Index].Destruct();
        }
        for (const Entity Watcher : mWatchers)
        {
            Watcher.Destruct();
        }

        GetService<Service>().GetWorld().Get<Ledger>().Publishing = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Admit(Network::Connection Link)
    {
        const UInt64 Key = KeyOf(Link);

        Ref<Unique<Member>> Holder = mMembers.FindOrInsert(Key);

        if (!Holder)
        {
            Holder = Unique<Member>::Create();
        }

        Ref<Member> Peer = (* Holder);
        Peer.Link = Link;
        Peer.Scopes.Clear();
        Peer.Known.Clear();

        ConstRef<Manifest> Table = Manifest::Get();

        Peer.Reliable.Write<UInt8>(static_cast<UInt8>(Opcode::Hello));
        Peer.Reliable.Write<UInt64>(Table.GetHash());
        Peer.Reliable.Write<UInt64>(Key);

        // Every replicated singleton the world holds, so the peer starts from the same world as everyone else.
        const World World = GetService<Service>().GetWorld();

        mScratch.Clear();

        if (const Mask Held = Wire::Encode(mScratch, World, Table.GetMask(), mSlices); Held.Any())
        {
            Peer.Reliable.Write<UInt8>(static_cast<UInt8>(Opcode::World));
            Wire::Compose(Peer.Reliable, mScratch, mSlices, Held, Mask());
        }

        Flush(Peer);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Expel(Network::Connection Link)
    {
        const UInt64 Key = KeyOf(Link);

        if (const Ptr<Unique<Member>> Holder = mMembers.Find(Key))
        {
            const Ptr<Member> Peer = Holder->Grab();

            for (const UInt64 Scope : Peer->Scopes)
            {
                if (const Ptr<Group> Group = mGroups.Find(Scope))
                {
                    Leave(* Group, Peer);
                }
            }
            mMembers.Erase(Key);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Subscribe(UInt64 Key, ConstSpan<UInt64> Scopes)
    {
        const Ptr<Member> Peer = Find(Key);

        if (Peer == nullptr)
        {
            return;
        }

        LOG_D("Scene: Peer {0} now sees {1} scope(s)", Key, Scopes.GetSize());

        // What the peer stops seeing is gathered first, since the bag cannot be walked while it is emptied.
        Sequence<UInt64> Departed;

        for (const UInt64 Scope : Peer->Scopes)
        {
            if (!Scopes.Contains(Scope))
            {
                Departed.Append(Scope);
            }
        }

        for (const UInt64 Scope : Departed)
        {
            Peer->Scopes.Erase(Scope);

            if (const Ptr<Group> Group = mGroups.Find(Scope))
            {
                Leave(* Group, Peer);

                if (Group->Actor.IsValid())
                {
                    Retract(Group->Actor, * Peer);
                }
            }
        }

        for (const UInt64 Scope : Scopes)
        {
            if (Peer->Scopes.Insert(Scope))
            {
                Ref<Group> Group = mGroups.FindOrInsert(Scope);
                Group.Subscribers.Append(Peer);

                if (Group.Actor.IsValid())
                {
                    Announce(Group.Actor, * Peer, true);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Publish()
    {
        ZY_PROFILE_SCOPE("Scene::Publish");

        // Whatever was destroyed since the last publish is forgotten by whoever saw it, before anything else.
        for (ConstRef<Departure> Gone : mDepartures)
        {
            if (const Ptr<Group> Group = mGroups.Find(Gone.Scope))
            {
                for (const Ptr<Member> Peer : Group->Subscribers)
                {
                    Conceal(* Peer, Gone.Identifier);
                }
            }
        }
        mDepartures.Clear();

        // Only what was touched is looked at: the ones that moved change hands, the ones with changes go out.
        const World World = GetService<Service>().GetWorld();
        Ref<Ledger> Book  = World.Get<Ledger>();

        for (const Entity Actor : Book.Pending)
        {
            if (!Actor.IsAlive())
            {
                continue;
            }

            const ConstPtr<Replica> Record   = Actor.TryGet<const Replica>();
            const Ptr<Tracker>      Tracking = Actor.TryGet<Tracker>();

            if (Record == nullptr || Tracking == nullptr || !Record->IsValid())
            {
                continue;
            }
            Tracking->Queued = false;

            const UInt64 Key = Locate(Actor);

            if (Key != Tracking->Scope || !Tracking->Announced)
            {
                const Bool Fresh = !Tracking->Announced;

                Move(Actor, * Record, * Tracking, Key);

                // A spawn carried everything the entity has, so whatever gathered before it has nothing left to say.
                if (Fresh && !Record->IsPersistent())
                {
                    Tracking->Touched.Reset();
                    Tracking->Removed.Reset();
                }
            }

            if (Tracking->Touched.Any() || Tracking->Removed.Any())
            {
                Broadcast(Actor, * Record, * Tracking);
            }
        }
        Book.Pending.Clear();

        // Sight is asked again now and then over everything, since a peer moves out of it without touching a thing.
        if (!mVisibility.IsEmpty() && ++mSweep >= kSweep)
        {
            mSweep = 0;
            Sweep();
        }

        // The singletons are seen by everyone, so they go out to everyone.
        if (Book.Touched.Any())
        {
            const Mask Changed = Exchange(Book.Touched, Mask()) & ~Manifest::Get().GetOnceMask();

            mScratch.Clear();

            if (const Mask Held = Wire::Encode(mScratch, World, Changed, mSlices); Held.Any())
            {
                for (Ref<Table<UInt64, Unique<Member>>::Pair> Entry : mMembers)
                {
                    Ref<Member> Peer = (* Entry.Second);

                    Peer.Reliable.Write<UInt8>(static_cast<UInt8>(Opcode::World));
                    Wire::Compose(Peer.Reliable, mScratch, mSlices, Held, Mask());
                }
            }
        }

        for (Ref<Table<UInt64, Unique<Member>>::Pair> Entry : mMembers)
        {
            Flush(* Entry.Second);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Publisher::Locate(Entity Actor)
    {
        for (Entity Parent = Actor.GetParent(); Parent.IsValid(); Parent = Parent.GetParent())
        {
            if (const ConstPtr<Scope> Component = Parent.TryGet<const Scope>())
            {
                return Component->GetKey();
            }
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Mask Publisher::Audience(ConstRef<Member> Peer, ConstRef<Replica> Record)
    {
        ConstRef<Manifest> Table = Manifest::Get();
        return Peer.Link == Record.GetOwner() ? Table.GetSelfMask() : Table.GetOthersMask();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Publisher::Member> Publisher::Find(UInt64 Key)
    {
        const Ptr<Unique<Member>> Holder = mMembers.Find(Key);
        return Holder ? Holder->Grab() : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Leave(Ref<Group> Group, Ptr<Member> Peer)
    {
        Group.Subscribers.RemoveFastIf([Peer](Ptr<Member> Entry)
        {
            return Entry == Peer;
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Reveal(Ref<Member> Peer, Entity Actor, ConstRef<Replica> Record, ConstRef<Tracker> Tracking, UInt64 Key, Bool Fresh)
    {
        const Bool Seen = Visible(Peer, Actor);

        if (Record.IsPersistent())
        {
            // Both sides hold it from disk, so only what departed from disk has to be said, and only on first sight.
            if (!Fresh || !Seen)
            {
                return;
            }

            if (const Mask Diverged = Tracking.Diverged & Audience(Peer, Record); Diverged.Any())
            {
                WriteUpdate(Peer, Actor, Record, Diverged);
            }
        }
        else if (!Seen)
        {
            // Out of the peer's sight, and forgotten there if it was ever in it.
            Conceal(Peer, Record.GetIdentifier());
        }
        else if (Peer.Known.Insert(Record.GetIdentifier()))
        {
            WriteSpawn(Peer, Actor, Record, Key);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Publisher::Visible(ConstRef<Member> Peer, Entity Actor)
    {
        return mVisibility.IsEmpty() || mVisibility(Peer.Link, Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Sweep()
    {
        for (Ref<Table<UInt64, Group>::Pair> Entry : mGroups)
        {
            Ref<Group> Group = Entry.Second;

            if (!Group.Actor.IsAlive())
            {
                continue;
            }

            for (const Ptr<Member> Peer : Group.Subscribers)
            {
                Announce(Group.Actor, * Peer, false);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Conceal(Ref<Member> Peer, UInt64 Identifier)
    {
        if (Peer.Known.Erase(Identifier))
        {
            WriteForget(Peer, Identifier);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Announce(Entity Actor, Ref<Member> Peer, Bool Fresh)
    {
        Actor.Children([this, & Peer, Fresh](Entity Child)
        {
            if (const ConstPtr<Replica> Record = Child.TryGet<const Replica>())
            {
                ConstRef<Tracker> Tracking = Child.Get<const Tracker>();

                // A replica that was never published has no scope yet, and the next publish announces it.
                if (!Tracking.Announced)
                {
                    return;
                }

                Reveal(Peer, Child, * Record, Tracking, Tracking.Scope, Fresh);
            }
            else if (!Child.Has<Scope>())
            {
                // A nested scope is a group of its own, anything else is looked through.
                Announce(Child, Peer, Fresh);
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Retract(Entity Actor, Ref<Member> Peer)
    {
        Actor.Children([this, & Peer](Entity Child)
        {
            if (const ConstPtr<Replica> Record = Child.TryGet<const Replica>())
            {
                Conceal(Peer, Record->GetIdentifier());
            }
            else if (!Child.Has<Scope>())
            {
                Retract(Child, Peer);
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Move(Entity Actor, ConstRef<Replica> Record, Ref<Tracker> Tracking, UInt64 Key)
    {
        const Ptr<Group> Old = Tracking.Scope ? mGroups.Find(Tracking.Scope) : nullptr;
        const Ptr<Group> New = Key ? AddressOf(mGroups.FindOrInsert(Key)) : nullptr;

        const UInt64 Identifier = Record.GetIdentifier();

        // Whoever saw the old scope and not the new one loses the replica.
        if (Old)
        {
            for (const Ptr<Member> Peer : Old->Subscribers)
            {
                if (New && New->Subscribers.Contains(Peer))
                {
                    continue;
                }
                Conceal(* Peer, Identifier);
            }
        }

        // Whoever sees the new scope and did not see the old one gains it.
        if (New)
        {
            for (const Ptr<Member> Peer : New->Subscribers)
            {
                if (Old && Old->Subscribers.Contains(Peer))
                {
                    continue;
                }
                Reveal(* Peer, Actor, Record, Tracking, Key, true);
            }
        }

        LOG_D("Scene: Replica {0} now stands in scope {1}", Identifier, Key);

        Tracking.Scope     = Key;
        Tracking.Announced = true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Broadcast(Entity Actor, ConstRef<Replica> Record, Ref<Tracker> Tracking)
    {
        ConstRef<Manifest> Table = Manifest::Get();

        // A component that only travels with a spawn is dropped here, however often it is touched or even removed.
        const Mask Changed = Exchange(Tracking.Touched, Mask()) & ~Table.GetOnceMask();
        const Mask Dropped = Exchange(Tracking.Removed, Mask()) & ~Table.GetOnceMask();

        if (Record.IsPersistent())
        {
            Tracking.Diverged |= Changed | Dropped;
        }

        const Ptr<Group> Group = Tracking.Scope ? mGroups.Find(Tracking.Scope) : nullptr;

        if (Group == nullptr || Group->Subscribers.IsEmpty() || (Changed.None() && Dropped.None()))
        {
            return;
        }

        // Serialized once, then composed per peer from the slices, since the audience differs by ownership.
        mScratch.Clear();

        const Mask Held = Changed.Any() ? Wire::Encode(mScratch, Actor, Changed, mSlices) : Mask();

        if (Held.None() && Dropped.None())
        {
            return;
        }

        const UInt64 Identifier = Record.GetIdentifier();

        for (const Ptr<Member> Peer : Group->Subscribers)
        {
            if (!Visible(* Peer, Actor))
            {
                if (!Record.IsPersistent())
                {
                    Conceal(* Peer, Identifier);
                }
                continue;
            }

            if (!Record.IsPersistent() && Peer->Known.Insert(Identifier))
            {
                WriteSpawn(* Peer, Actor, Record, Tracking.Scope);
                continue;
            }

            const Mask Allowed  = Audience(* Peer, Record);
            const Mask Present  = Held & Allowed;
            const Mask Reliable = Present & ~Table.GetStreamMask();
            const Mask Streamed = Present &  Table.GetStreamMask();
            const Mask Absent   = Dropped & Allowed;

            // A removal goes reliably whatever the component's policy, since it has to arrive exactly once.
            if (Reliable.Any() || Absent.Any())
            {
                Peer->Reliable.Write<UInt8>(static_cast<UInt8>(Opcode::Update));
                Peer->Reliable.Write<UInt64>(Identifier);
                Peer->Reliable.WriteBlock<UInt16>([&](Ref<Writer> Output)
                {
                    Wire::Compose(Output, mScratch, mSlices, Reliable, Absent);
                });
                Drain(* Peer);
            }

            if (Streamed.Any())
            {
                mEntry.Clear();
                mEntry.Write<UInt64>(Identifier);
                mEntry.WriteBlock<UInt16>([&](Ref<Writer> Output)
                {
                    Wire::Compose(Output, mScratch, mSlices, Streamed, Mask());
                });
                WriteStream(* Peer, mEntry);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteSpawn(Ref<Member> Peer, Entity Actor, ConstRef<Replica> Record, UInt64 Key)
    {
        const Entity Archetype = Actor.GetArchetype();

        Peer.Reliable.Write<UInt8>(static_cast<UInt8>(Opcode::Spawn));
        Peer.Reliable.Write<UInt64>(Key);
        Peer.Reliable.WriteBlock<UInt32>([&](Ref<Writer> Output)
        {
            Output.Write<UInt64>(Record.GetIdentifier());
            Output.Write<UInt64>(Archetype.IsValid() ? Archetype.GetID() : 0);
            Output.Write<UInt64>(Record.GetOwner().IsValid() ? KeyOf(Record.GetOwner()) : 0);

            mScratch.Clear();

            const Mask Wanted = Manifest::Get().GetMask() & Audience(Peer, Record);
            const Mask Held   = Wire::Encode(mScratch, Actor, Wanted, mSlices);

            Wire::Compose(Output, mScratch, mSlices, Held, Mask());
        });

        Drain(Peer);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteUpdate(Ref<Member> Peer, Entity Actor, ConstRef<Replica> Record, Mask Wanted)
    {
        mScratch.Clear();

        // What departed from disk and is no longer held was removed, so the peer is told to drop it.
        const Mask Held   = Wire::Encode(mScratch, Actor, Wanted, mSlices);
        const Mask Absent = Wanted & ~Held;

        if (Held.None() && Absent.None())
        {
            return;
        }

        Peer.Reliable.Write<UInt8>(static_cast<UInt8>(Opcode::Update));
        Peer.Reliable.Write<UInt64>(Record.GetIdentifier());
        Peer.Reliable.WriteBlock<UInt16>([&](Ref<Writer> Output)
        {
            Wire::Compose(Output, mScratch, mSlices, Held, Absent);
        });

        Drain(Peer);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteForget(Ref<Member> Peer, UInt64 Identifier)
    {
        Peer.Reliable.Write<UInt8>(static_cast<UInt8>(Opcode::Forget));
        Peer.Reliable.Write<UInt64>(Identifier);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::WriteStream(Ref<Member> Peer, ConstSpan<Byte> Entry)
    {
        constexpr UInt32 kHeader = sizeof(UInt8) + sizeof(UInt32);

        if (Entry.GetSize() + kHeader > Network::kMaxDatagram)
        {
            LOG_W("Scene: An entry of {0} bytes cannot be streamed, it is bigger than a datagram", Entry.GetSize());
            return;
        }

        if (Peer.Datagram.GetSize() + Entry.GetSize() > Network::kMaxDatagram)
        {
            GetService<Network::Service>().Send(Peer.Link, Network::Delivery::Unreliable, Peer.Datagram);
            Peer.Datagram.Clear();
        }

        if (Peer.Datagram.GetSize() == 0)
        {
            Peer.Datagram.Write<UInt8>(static_cast<UInt8>(Opcode::Stream));
            Peer.Datagram.Write<UInt32>(++Peer.Sequence);
        }

        Peer.Datagram.Write<Byte>(Entry.GetData(), Entry.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Drain(Ref<Member> Peer)
    {
        // Every message is whole on its own, so a stream may be cut between any two of them.
        if (Peer.Reliable.GetSize() >= kMaxChunk)
        {
            GetService<Network::Service>().Send(Peer.Link, Network::Delivery::Reliable, Peer.Reliable);

            Peer.Reliable.Clear();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Publisher::Flush(Ref<Member> Peer)
    {
        Ref<Network::Service> Network = GetService<Network::Service>();

        if (Peer.Reliable.GetSize() > 0)
        {
            Network.Send(Peer.Link, Network::Delivery::Reliable, Peer.Reliable);

            Peer.Reliable.Clear();
        }

        if (Peer.Datagram.GetSize() > 0)
        {
            Network.Send(Peer.Link, Network::Delivery::Unreliable, Peer.Datagram);

            Peer.Datagram.Clear();
        }
    }
}