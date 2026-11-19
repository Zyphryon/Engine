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

#include "Service.hpp"
#include "Publisher.hpp"
#include "Subscriber.hpp"
#include "Wire.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host }
    {
        Ref<ZyScene::Service> Scene = * Host.GetService<ZyScene::Service>();

        // An identity and a key belong to the entity itself, so an instance never takes them from its archetype.
        Scene.Declare<Replica>().Serializable().Local();
        Scene.Declare<Locus>().Serializable().Local();

        // An owner is a live peer, which no save outlives.
        Scene.Declare<Owner>().Local();

        // Authority belongs to the entity itself too.
        Scene.Declare<Remote>().Local();
        Scene.Declare<Possessed>().Local();
        Scene.Declare<Server>().Local();
        Scene.Declare<Client>().Local();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::~Service()
    {
        Close();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Real64 Delta)
    {
        if (mPublisher)
        {
            mPublisher->Publish();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTeardown()
    {
        Close();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Serve(
        ConstRef<ZyNetwork::Address> Address,
        ZyNetwork::Transport         Transport,
        ConstRetainer<Listener>      Listener)
    {
        return Open(Address, Transport, Listener, true, ConstSpan<Byte>());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Join(
        ConstRef<ZyNetwork::Address> Address,
        ZyNetwork::Transport         Transport,
        ConstRetainer<Listener>      Listener,
        ConstSpan<Byte>              Ticket)
    {
        ZY_ASSERT(Ticket.GetSize() <= kTicket, "A ticket must fit in kTicket bytes");

        return Open(Address, Transport, Listener, false, Ticket);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Close()
    {
        if (mLink.IsValid())
        {
            if (ConstRetainer<ZyNetwork::Service> Network = GetHost().GetService<ZyNetwork::Service>())
            {
                Network->Close(mLink);
            }
            mLink = ZyNetwork::Connection();
        }

        mPublisher  = Unique<Publisher>();
        mSubscriber = Unique<Subscriber>();
        mGate       = Retainer<Gate>();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Watch(UInt64 Peer, ConstSpan<UInt64> Keys)
    {
        if (mPublisher)
        {
            mPublisher->Subscribe(Peer, Keys);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Possess(UInt64 Peer, ZyScene::Entity Actor)
    {
        // The publisher reads the change of owner, which makes the replica arrive again with its private part.
        if (Peer != 0)
        {
            Actor.Set(Owner(Peer));
        }
        else
        {
            Actor.Remove<Owner>();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Expel(UInt64 Peer, Text Reason)
    {
        if (!mPublisher)
        {
            return;
        }

        // The listener hears the peer leave once the network says the connection closed.
        if (const ZyNetwork::Connection Link = mPublisher->Expel(Peer); Link.IsValid())
        {
            Part(Link, Reason);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<ZyNetwork::Address> Service::GetAddress(UInt64 Peer) const
    {
        return mPublisher ? mPublisher->GetAddress(Peer) : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Send(ConstSpan<Byte> Message)
    {
        if (mSubscriber && mLink.IsValid())
        {
            GetHost().GetService<ZyNetwork::Service>()->Send(mLink, ZyNetwork::Delivery::Reliable, Message);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Send(UInt64 Peer, ConstSpan<Byte> Message)
    {
        if (mPublisher)
        {
            mPublisher->Post(Peer, Message);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Send(ZyScene::Entity Actor, Audience Audience, ConstSpan<Byte> Message)
    {
        if (mPublisher)
        {
            mPublisher->Post(Actor, Audience, Message);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::IsWatched(ZyScene::Entity Actor)
    {
        return mPublisher && mPublisher->IsWatched(Actor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ZyScene::Entity Service::Find(UInt64 Identifier)
    {
        if (mPublisher)
        {
            return mPublisher->Find(Identifier);
        }
        return mSubscriber ? mSubscriber->Find(Identifier) : ZyScene::Entity();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ZyScene::Entity Service::Resolve(Ref<Link> Value)
    {
        return Value.Resolve([this](UInt64 Identifier)
        {
            return Find(Identifier);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Part(ZyNetwork::Connection Link, Text Reason)
    {
        Writer Output(kMaximum<UInt8> + 8);
        Output.Write<UInt8>(static_cast<UInt8>(Opcode::Expel));
        Output.WriteText(Reason.Slice(0, Min<UInt>(Reason.GetSize(), kMaximum<UInt8>)));

        // The network closes the connection once the reason went out ahead of it.
        Ref<ZyNetwork::Service> Network = * GetHost().GetService<ZyNetwork::Service>();
        Network.Send(Link, ZyNetwork::Delivery::Reliable, ConstSpan<Byte>(Output.GetData(), Output.GetSize()));
        Network.Close(Link);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Open(
        ConstRef<ZyNetwork::Address> Address,
        ZyNetwork::Transport         Transport,
        ConstRetainer<Listener>      Listener,
        Bool                         Serving,
        ConstSpan<Byte>              Ticket)
    {
        ConstRetainer<ZyNetwork::Service> Network = GetHost().GetService<ZyNetwork::Service>();

        if (!Network || mPublisher || mSubscriber)
        {
            return false;
        }

        // Both sides order the table alike before anything is written with it.
        mManifest.Seal();

        Ref<ZyScene::Service> Scene = * GetHost().GetService<ZyScene::Service>();

        // A game that hears nothing still gets a listener, so nothing asks whether it has one.
        const Retainer<ZyReplication::Listener> Hearing
            = Listener ? Listener : Retainer<ZyReplication::Listener>::Create();

        if (Serving)
        {
            Ref<ZyJob::Service> Jobs = * GetHost().GetService<ZyJob::Service>();
            mPublisher = Unique<Publisher>::Create(Scene, Jobs, * Network, mManifest);
        }
        else
        {
            mSubscriber = Unique<Subscriber>::Create(Scene, mManifest, Hearing);
        }

        mGate = Retainer<Gate>::Create(* this, Hearing, Ticket);
        mLink = Serving ? Network->Listen(Address, Transport, mGate) : Network->Connect(Address, Transport, mGate);

        if (!mLink.IsValid())
        {
            Close();
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Settle(Bool Loaded)
    {
        // Changes waiting their turn are taken before the load, so its reads cannot swallow them.
        if (mPublisher)
        {
            mPublisher->Read(Loaded, true);
        }
        else if (mSubscriber && Loaded)
        {
            mSubscriber->Refresh();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Gate::Gate(Ref<Service> Owner, ConstRetainer<Listener> Listener, ConstSpan<Byte> Ticket)
        : mOwner    { Owner },
          mListener { Listener }
    {
        mTicket.Append(Ticket);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Gate::OnConnect(ZyNetwork::Connection Link, ConstRef<ZyNetwork::Address> Origin)
    {
        // The serving side's own endpoint opening is no peer, and a peer is sent nothing before its ticket holds.
        if (mOwner.mPublisher && Link.IsPeer())
        {
            mKnocks.Assign(Link.GetValue(), Knock { Link, Origin });
        }
        else if (mOwner.mSubscriber)
        {
            Writer Output(kTicket + 1);
            Output.Write<UInt8>(static_cast<UInt8>(Opcode::Knock));
            Output.Write(mTicket.GetData(), static_cast<UInt32>(mTicket.GetSize()));

            Ref<ZyNetwork::Service> Network = * mOwner.GetHost().GetService<ZyNetwork::Service>();
            Network.Send(Link, ZyNetwork::Delivery::Reliable, ConstSpan<Byte>(Output.GetData(), Output.GetSize()));

            mListener->OnJoin(0);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Gate::OnDisconnect(ZyNetwork::Connection Link, ZyNetwork::Reason Reason)
    {
        // A peer that never got past its ticket never joined, so it is not heard leaving.
        if (mOwner.mPublisher && Link.IsPeer() && !mKnocks.Erase(Link.GetValue()))
        {
            mOwner.mPublisher->Expel(Link.GetValue());
            mListener->OnLeave(Link.GetValue(), Text::Empty());
        }
        else if (mOwner.mSubscriber)
        {
            LOG_W("Replication: The serving side went away ({0})", static_cast<UInt32>(Reason));

            mOwner.mLink = ZyNetwork::Connection();
            mListener->OnLeave(0, mFarewell);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Gate::OnMessage(ZyNetwork::Connection Link, ZyNetwork::Delivery Delivery, ConstSpan<Byte> Message)
    {
        Reader Input(Message);

        // The serving side hears a waiting peer's ticket and then the game's messages, the following side only frames.
        if (mOwner.mPublisher)
        {
            const UInt64 Peer = Link.GetValue();

            if (const Ptr<Knock> Waiting = mKnocks.Find(Peer))
            {
                // Only the first message is a ticket, and one turned away is heard no more while it goes.
                if (Exchange(Waiting->Shown, true))
                {
                    return;
                }

                const Bool Knocked = Input.Read<UInt8>() == static_cast<UInt8>(Opcode::Knock);
                const Bool Fits    = Input.GetAvailable() <= kTicket;
                const ConstSpan<Byte> Ticket(Message.GetData() + Input.GetOffset(), Input.GetAvailable());

                if (Knocked && Fits && mListener->OnKnock(Peer, Ticket))
                {
                    Knock Admitted;
                    mKnocks.Extract(Peer, Admitted);
                    mListener->OnJoin(mOwner.mPublisher->Admit(Admitted.Link, Admitted.Origin));
                }
                else
                {
                    mOwner.Part(Link, "Ticket");
                }
            }
            else if (mOwner.mPublisher->GetAddress(Peer))
            {
                mListener->OnMessage(Peer, Message);
            }
        }
        else if (mOwner.mSubscriber)
        {
            // The serving side's last word says why this side is expelled, and the connection closes behind it.
            if (Input.Read<UInt8>() == static_cast<UInt8>(Opcode::Expel))
            {
                mFarewell = Input.ReadText();
                return;
            }
            mOwner.mSubscriber->Receive(Message);
        }
    }
}