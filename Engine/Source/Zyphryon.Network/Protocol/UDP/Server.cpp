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

namespace Network::UDP
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Server::Server(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener)
        : Channel  { Link, Address, Listener },
          mClock   { 0.0 },
          mCursor  { 1 }
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
        return Channel::IsQuiet() && mFlights.IsEmpty();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Server::Verify(Connection Link) const
    {
        return mState == State::Live && (Link == mLink || mPeers.IsAllocated(Link.GetPeer()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Server::Open(Ref<Proactor> Watcher, ConstRef<Endpoint> Address)
    {
        if (!mSocket.Open(Transport::UDP, Address) || !mSocket.Bind(Address) || !Watcher.Attach(mSocket))
        {
            mSocket.Close();
            return false;
        }

        mState = State::Live;

        Fill(Watcher);
        return IsAwaiting(Operation::Receive);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Complete(Ref<Proactor> Watcher, Real64 Time, ConstRef<Proactor::Event> Event, Ref<Mailbox> Output)
    {
        Settle(Event.Kind);

        switch (Event.Kind)
        {
        case Operation::Receive:
            if (Event.Cause == Reason::None)
            {
                OnReceive(Watcher, Time, Event.Origin, Event.Size, Output);
            }
            else
            {
                Retire(Watcher, mLink, Event.Cause, Output);
            }
            break;
        case Operation::Send:
            OnSend(Watcher, Time, Event);
            break;
        default:
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Tick(Ref<Proactor> Watcher, Real64 Time, Ref<Mailbox> Output)
    {
        mClock = Time;

        mSentry.Rotate(Time);

        mPeers.ForEachOrRemove([&](ConstRef<Peer> Entry)
        {
            if (mTimeout > 0.0 && Time - Entry.Freshness > mTimeout)
            {
                Output.Disconnect(Entry.Link, Reason::Timeout);

                Forget(Entry);
                return true;
            }
            return false;
        });

        Drain(Watcher, Time);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Send(Connection Link, Delivery Delivery, ConstSpan<Byte> Message)
    {
        if (mState == State::Live)
        {
            if (const Ptr<Peer> Entry = mPeers.TryGet(Link.GetPeer()))
            {
                Entry->Session.Push(Delivery, Message);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Broadcast(Delivery Delivery, ConstSpan<Byte> Message)
    {
        if (mState == State::Live)
        {
            mPeers.ForEach([Delivery, Message](Ref<Peer> Entry)
            {
                Entry.Session.Push(Delivery, Message);
            });
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Retire(Ref<Proactor> Watcher, Connection Link, Reason Cause, Ref<Mailbox> Output)
    {
        if (const Ptr<Peer> Entry = mPeers.TryGet(Link.GetPeer()))
        {
            Output.Disconnect(Entry->Link, Cause);

            Forget(* Entry);

            mPeers.Free(Link.GetPeer());
        }
        else
        {
            Fold(Watcher, Cause, Output);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Probe(Connection Link, Ref<Statistics> Metrics, Ref<Address> Origin) const
    {
        if (const ConstPtr<Peer> Entry = mPeers.TryGet(Link.GetPeer()))
        {
            Metrics = Entry->Session.GetStats();
            Origin  = Entry->Origin;
        }
        else
        {
            Metrics = Statistics();
            Origin  = mAddress;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt Server::Locate(ConstRef<Endpoint> Origin) const
    {
        if (const ConstPtr<UInt> Found = mRegistry.Find(Digest(Origin.Hash())))
        {
            if (ConstRef<Peer> Entry = mPeers[* Found]; Entry.Remote == Origin)
            {
                return * Found;
            }

            LOG_W("Network: two peers hashed alike, so the later one was turned away");
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt Server::Admit(ConstRef<Endpoint> Origin, Ref<Mailbox> Output)
    {
        if (mPeers.IsFull())
        {
            return 0;
        }

        // The slot is the peer half of the connection, since the pool counts from one already, and it is
        // constructed fresh so nothing the peer before it left queued can survive into this one.
        const UInt Slot = mPeers.Allocate();
        mRegistry.Assign(Digest(Origin.Hash()), Slot);

        Ref<Peer> Entry = mPeers[Slot];
        Entry.Link      = Connection(mLink.GetChannel(), Slot);
        Entry.Remote    = Origin;
        Entry.Freshness = mClock;
        Origin.Describe(Entry.Origin);

        Output.Connect(Entry.Link);
        return Slot;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Question(Ref<Proactor> Watcher, ConstRef<Endpoint> Origin)
    {
        if (mState != State::Live || mFlights.IsFull())
        {
            return;
        }

        // Sent out of the endpoint's own room like any other datagram, so an address that never answers costs
        // nothing but the moment it takes to leave and is never written down anywhere.
        const UInt  Index = mFlights.Allocate();
        Ref<Flight> Carry = mFlights[Index];

        Carry.Peer = 0;
        Carry.Data.Resize(kChallenge);

        mSentry.Inscribe(Origin, Span(Carry.Data.GetData(), kChallenge));

        const Connection Link(mLink.GetChannel(), Index);

        if (!Watcher.Send(mSocket, Link, ConstSpan(Carry.Data.GetData(), kChallenge), Origin))
        {
            mFlights.Free(Index);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Forget(ConstRef<Peer> Entry)
    {
        mRegistry.Erase(Digest(Hash(Entry.Remote)));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Drain(Ref<Proactor> Watcher, Real64 Time)
    {
        const UInt Top = mPeers.GetTop();

        if (mState != State::Live || Top == 0)
        {
            return;
        }

        // Resumed from where the last drain left off so no peer is served first every time.
        while (!mFlights.IsFull())
        {
            UInt32 Sent = 0;

            for (UInt Step = 0; Step < Top && !mFlights.IsFull(); ++Step)
            {
                const UInt Slot = 1 + (mCursor + Step) % Top;

                if (const Ptr<Peer> Entry = mPeers.TryGet(Slot))
                {
                    Sent += Push(Watcher, Time, Slot, * Entry) ? 1 : 0;
                }
            }

            if (Sent == 0)
            {
                break;
            }
        }
        mCursor = (mCursor + 1) % Top;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Server::Push(Ref<Proactor> Watcher, Real64 Time, UInt Slot, Ref<Peer> Entry)
    {
        if (mState != State::Live || mFlights.IsFull())
        {
            return false;
        }

        // Built into room the endpoint owns rather than the peer's own, so the peer may be let go of while the
        // datagram it asked for is still on its way.
        const UInt   Index = mFlights.Allocate();
        Ref<Flight>  Carry = mFlights[Index];

        const Ptr<Byte> Data = Carry.Data.GetData();
        const UInt32    Size = Entry.Session.Build(Time, Span(Data + kEnvelope, Carry.Data.GetCapacity() - kEnvelope));

        if (Size == 0)
        {
            mFlights.Free(Index);
            return false;
        }

        Data[0]    = Enum::Cast(Envelope::Data);
        Carry.Peer = Slot;
        mCursor    = Slot;

        // Posted under the room it went out of rather than under the peer, since that is what the answer has to
        // name for the room to be taken back, and a write is reported to nobody else.
        const Connection Link(mLink.GetChannel(), Index);

        if (!Watcher.Send(mSocket, Link, ConstSpan(Data, kEnvelope + Size), Entry.Remote))
        {
            mFlights.Free(Index);
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Fill(Ref<Proactor> Watcher)
    {
        if (mState == State::Live && !IsAwaiting(Operation::Receive))
        {
            Await(Operation::Receive, Watcher.ReceiveFrom(mSocket, mLink, mInbound));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::Absorb(Ref<Proactor> Watcher, Real64 Time, UInt Slot, UInt32 Size, Ref<Mailbox> Output)
    {
        Ref<Peer>        Entry = mPeers[Slot];
        const Connection Link  = Entry.Link;

        Entry.Freshness = mClock;

        Entry.Session.Parse(Time, ConstSpan(mInbound.GetData() + kEnvelope, Size - kEnvelope),
            [&](Delivery Mode, ConstSpan<Byte> Message)
            {
                Output.Message(Link, Mode, Message);
            });

        if (Entry.Session.IsBroken())
        {
            LOG_W("Network: a peer sent further ahead than it may and was let go of");

            Retire(Watcher, Link, Reason::Protocol, Output);
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

            mPeers.ForEach([&](ConstRef<Peer> Entry)
            {
                Output.Disconnect(Entry.Link, Cause);
            });

            mPeers.Clear();
            mRegistry.Clear();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::OnReceive(Ref<Proactor> Watcher, Real64 Time, ConstRef<Endpoint> Origin, UInt32 Size, Ref<Mailbox> Output)
    {
        if (Size >= kEnvelope)
        {
            const Envelope Kind  = static_cast<Envelope>(mInbound[0]);
            const Bool     Whole = (Size >= kChallenge);

            if (const UInt Slot = Locate(Origin))
            {
                if (Kind == Envelope::Data && Whole)
                {
                    Absorb(Watcher, Time, Slot, Size, Output);
                }
            }
            else if (Kind == Envelope::Answer)
            {
                if (mSentry.Approve(Origin, ConstSpan(mInbound.GetData(), Size)))
                {
                    Admit(Origin, Output);
                }
            }
            else if (Kind == Envelope::Data && Whole)
            {
                Question(Watcher, Origin);
            }
        }

        Fill(Watcher);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Server::OnSend(Ref<Proactor> Watcher, Real64 Time, ConstRef<Proactor::Event> Event)
    {
        const Ptr<Flight> Carry = mFlights.TryGet(Event.Link.GetPeer());

        if (Carry == nullptr)
        {
            return;
        }

        const UInt Slot = Carry->Peer;

        mFlights.Free(Event.Link.GetPeer());

        if (const Ptr<Peer> Entry = mPeers.TryGet(Slot))
        {
            Push(Watcher, Time, Slot, * Entry);
        }
    }
}