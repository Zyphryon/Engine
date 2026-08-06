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

#include "Stream.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::TCP
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Stream::Stream(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener)
        : Channel { Link, Address, Listener },
          mExpiry  { 0.0 },
          mProbe   { 0.0 },
          mCompact { 0.0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Stream::Stream(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener, ConstRef<Socket> Socket)
        : Channel { Link, Address, Listener },
          mSocket { Socket },
          mExpiry  { 0.0 },
          mProbe   { 0.0 },
          mCompact { 0.0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Stream::~Stream()
    {
        ZY_ASSERT(Channel::IsQuiet(), "A stream was destroyed while the platform still held an operation of its own");

        mSocket.Close();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Stream::Verify(Connection Link) const
    {
        return mState == State::Live;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Stream::Open(Ref<Proactor> Watcher, ConstRef<Endpoint> Address)
    {
        if (!mSocket.Open(Transport::TCP, Address) || !Watcher.Attach(mSocket) || !Watcher.Connect(mSocket, mLink, Address))
        {
            mSocket.Close();
            return false;
        }

        mState = State::Pending;

        Await(Operation::Connect);
        return IsAwaiting(Operation::Connect);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Stream::Adopt(Ref<Proactor> Watcher)
    {
        if (!Watcher.Attach(mSocket))
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

    void Stream::Complete(Ref<Proactor> Watcher, Real64 Time, ConstRef<Proactor::Event> Event, Ref<Mailbox> Output)
    {
        Settle(Event.Kind);

        if (Event.Cause != Reason::None)
        {
            Retire(Watcher, mLink, Event.Cause, Output);
            return;
        }

        switch (Event.Kind)
        {
        case Operation::Connect:
            OnConnect(Watcher, Output);
            break;
        case Operation::Receive:
            if (Event.Size > 0)
            {
                OnReceive(Watcher, Time, Event.Size, Output);
            }
            else
            {
                Retire(Watcher, mLink, Reason::Closed, Output);
            }
            break;
        case Operation::Send:
            OnSend(Watcher, Event.Size);
            break;
        default:
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::Tick(Ref<Proactor> Watcher, Real64 Time, Ref<Mailbox> Output)
    {
        if (mCause == Reason::Congested)
        {
            Retire(Watcher, mLink, mCause, Output);
            return;
        }

        if (const Real64 Budget = (mState == State::Pending) ? kMaxHandshake : mTimeout; Budget > 0.0)
        {
            if (mExpiry == 0.0)
            {
                mExpiry = Time + Budget;
                mProbe  = Time + Budget / 3.0;
            }
            else if (Time >= mExpiry)
            {
                Retire(Watcher, mLink, (mState == State::Pending) ? Reason::Unreachable : Reason::Timeout, Output);
                return;
            }
            else if (mState == State::Live && Time >= mProbe)
            {
                mOutbound.Frame(1 + sizeof(Time));
                mOutbound.Append(Enum::Cast(Tag::Ping));
                mOutbound.Append(ConstSpan(reinterpret_cast<ConstPtr<Byte>>(AddressOf(Time)), sizeof(Time)));

                mProbe = Time + Budget / 3.0;
            }
        }

        // Given back only while nothing is out on it, since a write in flight is reading straight out of it.
        if (Time >= mCompact && !IsAwaiting(Operation::Send))
        {
            mTransmit.Compact();
            mOutbound.Compact();
        }

        Drain(Watcher);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::Send(Connection Link, Delivery Delivery, ConstSpan<Byte> Message)
    {
        if (mState == State::Pending || mState == State::Live)
        {
            if (mTransmit.GetSize() + mOutbound.GetSize() > kMaxBacklog)
            {
                mCause = Reason::Congested;
                return;
            }

            mOutbound.Frame(static_cast<UInt32>(Message.GetSize()) + 1);
            mOutbound.Append(Enum::Cast(Tag::Message));
            mOutbound.Append(Message);

            mStats.Pending = mTransmit.GetSize() + mOutbound.GetSize();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::Broadcast(Delivery Delivery, ConstSpan<Byte> Message)
    {
        Send(mLink, Delivery, Message);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::Retire(Ref<Proactor> Watcher, Connection Link, Reason Cause, Ref<Mailbox> Output)
    {
        if (mState != State::Retired)
        {
            mState = State::Retired;
            mCause = Cause;

            Watcher.Cancel(mSocket);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::Probe(Connection Link, Ref<Statistics> Metrics, Ref<Address> Origin) const
    {
        Metrics = mStats;
        Origin  = mAddress;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::Drain(Ref<Proactor> Watcher)
    {
        if (mState == State::Live && !IsAwaiting(Operation::Send))
        {
            if (mTransmit.IsEmpty() && !mOutbound.IsEmpty())
            {
                mTransmit.Swap(mOutbound);
            }

            if (!mTransmit.IsEmpty())
            {
                Await(Operation::Send, Watcher.Send(mSocket, mLink, mTransmit));
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::Fill(Ref<Proactor> Watcher)
    {
        if (mState == State::Live && !IsAwaiting(Operation::Receive))
        {
            Await(Operation::Receive, Watcher.Receive(mSocket, mLink, mInbound.Stage(kMaxRead)));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::OnConnect(Ref<Proactor> Watcher, Ref<Mailbox> Output)
    {
        mState  = State::Live;
        mExpiry = 0.0;

        Output.Connect(mLink);

        Fill(Watcher);
        Drain(Watcher);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::OnReceive(Ref<Proactor> Watcher, Real64 Time, UInt32 Size, Ref<Mailbox> Output)
    {
        mInbound.Commit(Size);

        mStats.Received += Size;

        mExpiry = 0.0;

        while (mInbound.IsWhole())
        {
            const ConstSpan<Byte> Frame = mInbound.Take();

            if (Frame.IsEmpty())
            {
                LOG_E("Network: a peer sent a frame carrying not even the byte that says what it is");

                Retire(Watcher, mLink, Reason::Protocol, Output);
                return;
            }

            switch (static_cast<Tag>(Frame[0]))
            {
            case Tag::Message:
                Output.Message(mLink, Delivery::Reliable, ConstSpan(Frame.GetData() + 1, Frame.GetSize() - 1));
                break;
            case Tag::Ping:
                if (Frame.GetSize() != 1 + sizeof(Real64))
                {
                    Retire(Watcher, mLink, Reason::Protocol, Output);
                    return;
                }

                mOutbound.Frame(static_cast<UInt32>(Frame.GetSize()));
                mOutbound.Append(Enum::Cast(Tag::Pong));
                mOutbound.Append(ConstSpan(Frame.GetData() + 1, Frame.GetSize() - 1));

                Drain(Watcher);
                break;
            case Tag::Pong:
                if (Frame.GetSize() == 1 + sizeof(Real64))
                {
                    Real64 Sent = 0.0;
                    Blit(reinterpret_cast<Ptr<Byte>>(AddressOf(Sent)), sizeof(Sent), Frame.GetData() + 1);

                    mStats.Latency = static_cast<Real32>(Max(0.0, Time - Sent) * 1000.0);
                }
                break;
            default:
                LOG_E("Network: a peer sent a frame of a kind this build cannot read");

                Retire(Watcher, mLink, Reason::Protocol, Output);
                return;
            }
        }

        if (mInbound.IsBroken())
        {
            LOG_E("Network: a peer announced a {0} byte message, beyond what is accepted", mInbound.Peek());

            Retire(Watcher, mLink, Reason::Protocol, Output);
            return;
        }

        // Given back here, in the gap between the read that finished and the one about to be posted, since
        // the platform holds a pointer into this queue for as long as one is out on it.
        if (Time >= mCompact)
        {
            mInbound.Compact();

            mCompact = Time + kMaxHoard;
        }

        Fill(Watcher);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Stream::OnSend(Ref<Proactor> Watcher, UInt32 Size)
    {
        mTransmit.Consume(Size);

        mStats.Sent   += Size;
        mStats.Pending = mTransmit.GetSize() + mOutbound.GetSize();

        Drain(Watcher);
    }
}