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

#include "Server.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::TCP
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Server::Server(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener)
        : Channel  { Link, Address, Listener },
          mInvited { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Server::~Server()
    {
        ZY_ASSERT(IsQuiet(), "An endpoint was destroyed while the platform still held an operation of its own");

        mSocket.Close();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Server::IsQuiet() const
    {
        Bool Quiet = Channel::IsQuiet() && mInvited == 0;

        mPeers.ForEach([&](ConstRetainer<Stream> Peer)
        {
            Quiet &= Peer->IsQuiet();
        });
        return Quiet;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Server::Verify(Connection Link) const
    {
        if (Link != mLink)
        {
            const Ptr<Stream> Peer = Locate(Link);
            return Peer != nullptr && Peer->Verify(Link);
        }
        return mState == State::Live;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Server::Open(Ref<Proactor> Watcher, ConstRef<Endpoint> Address)
    {
        if (!mSocket.Open(Transport::TCP, Address) || !mSocket.Bind(Address) || !mSocket.Listen() || !Watcher.Attach(mSocket))
        {
            mSocket.Close();
            return false;
        }

        mState = State::Live;
        mLocal = Address;

        Invite(Watcher);
        return mInvited > 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Complete(Ref<Proactor> Watcher, Real64 Time, ConstRef<Proactor::Event> Event, Ref<Mailbox> Output)
    {
        if (const Ptr<Stream> Peer = Locate(Event.Link))
        {
            Peer->Complete(Watcher, Time, Event, Output);
        }
        else
        {
            if (mInvited > 0)
            {
                --mInvited;
            }

            if (Event.Cause == Reason::None)
            {
                Admit(Watcher, Event.Peer, Event.Origin, Output);
            }

            Invite(Watcher);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Tick(Ref<Proactor> Watcher, Real64 Time, Ref<Mailbox> Output)
    {
        mPeers.ForEachOrRemove([&](ConstRetainer<Stream> Peer)
        {
            Peer->Tick(Watcher, Time, Output);

            if (const Reason Cause = Peer->GetCause(); Cause != Reason::None && Peer->IsQuiet())
            {
                Output.Disconnect(Peer->GetLink(), Cause);
                return true;
            }
            return false;
        });

        Invite(Watcher);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Send(Connection Link, Delivery Delivery, ConstSpan<Byte> Message)
    {
        if (const Ptr<Stream> Peer = Locate(Link))
        {
            Peer->Send(Link, Delivery, Message);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Broadcast(Delivery Delivery, ConstSpan<Byte> Message)
    {
        mPeers.ForEach([Delivery, Message](ConstRetainer<Stream> Peer)
        {
            if (Peer->Verify(Peer->GetLink()))
            {
                Peer->Send(Peer->GetLink(), Delivery, Message);
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Retire(Ref<Proactor> Watcher, Connection Link, Reason Cause, Ref<Mailbox> Output)
    {
        if (const Ptr<Stream> Peer = Locate(Link))
        {
            Peer->Retire(Watcher, Link, Cause, Output);
        }
        else
        {
            Fold(Watcher, Cause, Output);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::SetTimeout(Connection Link, Real64 Seconds)
    {
        if (const Ptr<Stream> Peer = Locate(Link))
        {
            Peer->SetTimeout(Link, Seconds);
        }
        else
        {
            Channel::SetTimeout(Link, Seconds);

            mPeers.ForEach([Seconds](ConstRetainer<Stream> Peer)
            {
                Peer->SetTimeout(Peer->GetLink(), Seconds);
            });
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Probe(Connection Link, Ref<Statistics> Metrics, Ref<Address> Origin) const
    {
        if (const Ptr<Stream> Peer = Locate(Link))
        {
            Peer->Probe(Link, Metrics, Origin);
        }
        else
        {
            Metrics = Statistics();
            Origin  = mAddress;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Stream> Server::Locate(Connection Link) const
    {
        if (const UInt Slot = Link.GetPeer(); Slot != 0 && Slot <= kMaxPeers)
        {
            const ConstPtr<Retainer<Stream>> Entry = mPeers.TryGet(Slot);
            return (Entry != nullptr) ? static_cast<Ptr<Stream>>(* Entry) : nullptr;
        }
        return nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Invite(Ref<Proactor> Watcher)
    {
        if (mState != State::Live)
        {
            return;
        }

        while (mInvited < kMaxInvites && !mPeers.IsFull())
        {
            if (!Watcher.Accept(mSocket, mLink, mLocal))
            {
                break;
            }

            ++mInvited;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Admit(Ref<Proactor> Watcher, ConstRef<Socket> Taken, ConstRef<Endpoint> Origin, Ref<Mailbox> Output)
    {
        Socket Peer = Taken;

        if (mPeers.IsFull())
        {
            Peer.Close();

            LOG_W("Network: a peer was turned away because no room was left for it");
            return;
        }

        Address Address;
        Origin.Describe(Address);

        const UInt       Slot = mPeers.Allocate();
        const Connection Link(mLink.GetChannel(), Slot);

        mPeers[Slot] = Retainer<Stream>::Create(Link, Address, mListener, Peer);
        mPeers[Slot]->SetTimeout(Link, mTimeout);

        if (mPeers[Slot]->Adopt(Watcher))
        {
            Output.Connect(Link);
        }
        else
        {
            mPeers.Free(Slot);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Fold(Ref<Proactor> Watcher, Reason Cause, Ref<Mailbox> Output)
    {
        if (mState != State::Retired)
        {
            mState = State::Retired;
            mCause = Cause;

            Watcher.Cancel(mSocket);

            mPeers.ForEach([&](ConstRetainer<Stream> Peer)
            {
                Peer->Retire(Watcher, Peer->GetLink(), Cause, Output);
            });
        }
    }
}