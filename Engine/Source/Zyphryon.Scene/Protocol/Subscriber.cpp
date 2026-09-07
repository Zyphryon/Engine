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
#include "Replica.hpp"
#include "Wire.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::Protocol
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Subscriber::Subscriber(Ref<Engine::Subsystem::Host> Host)
        : Locator   { Host },
          mIdentity { 0 },
          mAccepted { false },
          mRejected { false }
    {
        Ref<Service> Scene = GetService<Service>();

        // Observes a replica landing on an entity, from a spawn or from disk, to apply whatever waited for it.
        mObservers[0] = Scene.CreateObserver<>(
            "Scene::Subscriber::ObsSetReplica",
            EcsOnSet,
            [this](Entity Actor, ConstRef<Replica> Record)
            {
                if (const UInt64 Identifier = Record.GetIdentifier(); Identifier != 0)
                {
					mReplicas.Assign(Identifier, Known(Actor));

					// Whatever is held from disk belongs to the publisher, since that is the side that moves it.
					if (Record.IsPersistent())
					{
						Actor.Add<Remote>();
					}

					Sequence<Blob> Waiting;

					if (mPendingUpdates.Extract(Identifier, Waiting))
					{
						for (ConstRef<Blob> Body : Waiting)
						{
							Reader Input(Body);
							Wire::Decode(Input, Actor);
						}
					}
                }
            });

        // Observes a replica leaving an entity, so its identifier stops resolving to it.
        mObservers[1] = Scene.CreateObserver<>(
            "Scene::Subscriber::ObsRemoveReplica",
            EcsOnRemove,
            [this](Entity Actor, ConstRef<Replica> Record)
            {
                mReplicas.EraseIf(Record.GetIdentifier(), [Actor](ConstRef<Known> Entry)
                {
                    return Entry.Actor == Actor;
                });
            });

        // Observes a scope landing on an entity, to spawn whatever waited for it.
        mObservers[2] = Scene.CreateObserver<>(
            "Scene::Subscriber::ObsSetScope",
            EcsOnSet,
            [this](Entity Actor, ConstRef<Scope> Component)
            {
                const UInt64 Key = Component.GetKey();

                mScopes.Assign(Key, Actor);

                Sequence<Blob> Waiting;

                if (mPendingSpawns.Extract(Key, Waiting))
                {
                    for (ConstRef<Blob> Body : Waiting)
                    {
                        Materialize(Actor, Body);
                    }
                }
            });

        // Observes a scope leaving an entity, so its key stops resolving to it.
        mObservers[3] = Scene.CreateObserver<>(
            "Scene::Subscriber::ObsRemoveScope",
            EcsOnRemove,
            [this](Entity Actor, ConstRef<Scope> Component)
            {
                mScopes.EraseIf(Component.GetKey(), [Actor](Entity Held)
                {
                    return Held == Actor;
                });
            });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Subscriber::~Subscriber()
    {
        for (UInt Index = 0; Index < mObservers.GetSize(); ++Index)
        {
            mObservers[Index].Destruct();
        }
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

        // A reliable frame carries as many messages as the tick gathered, each whole on its own.
        while (Input.GetAvailable() > 0)
        {
            switch (static_cast<Opcode>(Input.Read<UInt8>()))
            {
            case Opcode::Hello:
                OnHello(Input);
                break;
            case Opcode::World:
                if (const World World = GetService<Service>().GetWorld(); !Wire::Decode(Input, World))
                {
                    LOG_W("Scene: The world named a singleton this build does not know");
                    return true;
                }
                break;
            case Opcode::Spawn:
                OnSpawn(Input);
                break;
            case Opcode::Forget:
                OnForget(Input);
                break;
            case Opcode::Update:
                OnUpdate(Input);
                break;
            case Opcode::Stream:
                OnStream(Input);
                break;
            default:
                LOG_W("Scene: A message this build does not know was received and the rest of the frame dropped");
                return true;
            }
        }
        return true;
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
        }
        else
        {
            LOG_I("Scene: Following the publisher's world as peer {0}", Identity);

            mAccepted = true;
            mIdentity = Identity;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnSpawn(Ref<Reader> Input)
    {
        const UInt64          Key  = Input.Read<UInt64>();
        const ConstSpan<Byte> Body = Input.ReadBlock<UInt32, Byte>();

        if (const Entity Scope = mScopes.FindOrDefault(Key); Scope.IsValid())
        {
            Materialize(Scope, Body);
        }
        else
        {
            mPendingSpawns.FindOrInsert(Key).Append(Blob::Copy(Body));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnForget(Ref<Reader> Input)
    {
        const UInt64 Identifier = Input.Read<UInt64>();

        mPendingUpdates.Erase(Identifier);

        if (const Entity Actor = mReplicas.FindOrDefault(Identifier).Actor; Actor.IsValid())
        {
            Actor.Destruct();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnUpdate(Ref<Reader> Input)
    {
        const UInt64          Identifier = Input.Read<UInt64>();
        const ConstSpan<Byte> Body       = Input.ReadBlock<UInt16, Byte>();

        if (const Entity Actor = mReplicas.FindOrDefault(Identifier).Actor; Actor.IsValid())
        {
            Reader Reader(Body);
            Wire::Decode(Reader, Actor);
        }
        else
        {
            // A persistent replica in a scope still loading from disk, whose state has to land once it does.
            mPendingUpdates.FindOrInsert(Identifier).Append(Blob::Copy(Body));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::OnStream(Ref<Reader> Input)
    {
        const UInt32 Sequence = Input.Read<UInt32>();

        while (Input.GetAvailable() > 0)
        {
            const UInt64          Identifier = Input.Read<UInt64>();
            const ConstSpan<Byte> Body       = Input.ReadBlock<UInt16, Byte>();

            const Ptr<Known> Entry = mReplicas.Find(Identifier);

            if (Entry == nullptr)
            {
                continue;
            }

            // A datagram that arrives behind a newer one has nothing to say about this entity.
            if (static_cast<SInt32>(Sequence - Entry->Sequence) <= 0)
            {
                continue;
            }
            Entry->Sequence = Sequence;

            const Entity Actor = Entry->Actor;

            Reader Reader(Body);
            Wire::Decode(Reader, Actor);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Subscriber::Materialize(Entity Scope, ConstSpan<Byte> Body)
    {
        Reader Input(Body);

        const UInt64 Identifier = Input.Read<UInt64>();
        const UInt64 Archetype  = Input.Read<UInt64>();
        const UInt64 Owner      = Input.Read<UInt64>();

        if (mReplicas.Contains(Identifier))
        {
            LOG_W("Scene: Replica '{0}' was spawned twice, the second is ignored", Identifier);
            return;
        }

        Ref<Service> Scene = GetService<Service>();

        // Everything the spawn names is queued and lands in one table move, rather than one per component.
        Scene.Defer([&]
        {
            const Entity Actor = Scene.CreateEntity();

            if (Archetype)
            {
                if (const Entity Prefab = Scene.GetEntity(Archetype); Prefab.IsAlive() && Prefab.IsArchetype())
                {
                    Actor.SetArchetype(Prefab);
                }
                else
                {
                    LOG_W("Scene: Replica '{0}' names an archetype '{1}' this world does not hold", Identifier, Archetype);
                }
            }

            // What the publisher owns on this side's behalf is this side's to move, everything else is remote.
            if (Owner == 0 || Owner != mIdentity)
            {
                Actor.Add<Remote>();
            }
            else
            {
                Actor.Add<Possessed>();
            }

            Actor.Emplace<Replica>(Identifier);

            Wire::Decode(Input, Actor);

            Actor.Attach(Scope, Hierarchy::Open);
        });

        LOG_D("Scene: Replica {0} spawned from archetype {1}", Identifier, Archetype);
    }
}