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

#include "Client.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::UDP
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Client::Client(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener)
        : Channel    { Link, Address, Listener },
          mCookie    { 0 },
          mAnswering { false }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Client::~Client()
    {
        ZY_ASSERT(Channel::IsQuiet(), "A datagram was destroyed while the platform still held an operation of its own");

        mSocket.Close();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Client::Verify(Connection Link) const
    {
        return mState == State::Live;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Client::Open(Ref<Proactor> Watcher, ConstRef<Endpoint> Address)
    {
        if (!mSocket.Open(Transport::UDP, Address) || !mSocket.Associate(Address) || !Watcher.Attach(mSocket))
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

    void Client::Complete(Ref<Proactor> Watcher, Real64 Time, ConstRef<Proactor::Event> Event, Ref<Mailbox> Output)
    {
        Settle(Event.Kind);

        if (Event.Cause != Reason::None)
        {
            Retire(Watcher, mLink, Event.Cause, Output);
            return;
        }

        switch (Event.Kind)
        {
        case Operation::Receive:
            OnReceive(Watcher, Time, Event.Size, Output);
            break;
        case Operation::Send:
            OnSend(Watcher, Time, Event.Size);
            break;
        default:
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Client::Tick(Ref<Proactor> Watcher, Real64 Time, Ref<Mailbox> Output)
    {
        Drain(Watcher, Time);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Client::Send(Connection Link, Delivery Delivery, ConstSpan<Byte> Message)
    {
        if (mState == State::Live)
        {
            mSession.Push(Delivery, Message);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Client::Broadcast(Delivery Delivery, ConstSpan<Byte> Message)
    {
        Send(mLink, Delivery, Message);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Client::Retire(Ref<Proactor> Watcher, Connection Link, Reason Cause, Ref<Mailbox> Output)
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

    void Client::Probe(Connection Link, Ref<Statistics> Metrics, Ref<Address> Origin) const
    {
        Metrics = mSession.GetStats();
        Origin  = mAddress;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Client::Drain(Ref<Proactor> Watcher, Real64 Time)
    {
        if (mState != State::Live || IsAwaiting(Operation::Send))
        {
            return;
        }

        const Ptr<Byte> Data = mTransmit.GetData();

        // Answered ahead of anything else and on its own, since nothing this end says is read at all until the
        // endpoint has taken the address on, and the answer is the only thing that makes it do so.
        if (mAnswering)
        {
            mAnswering = false;

            Data[0] = Enum::Cast(Envelope::Answer);
            Blit(Data + kEnvelope, sizeof(mCookie), AddressOf(mCookie));

            Await(Operation::Send, Watcher.Send(mSocket, mLink, ConstSpan(Data, kChallenge)));
            return;
        }

        if (const UInt32 Size = mSession.Build(Time, Span(Data + kEnvelope, mTransmit.GetCapacity() - kEnvelope)))
        {
            Data[0] = Enum::Cast(Envelope::Data);

            Await(Operation::Send, Watcher.Send(mSocket, mLink, ConstSpan(Data, kEnvelope + Size)));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Client::Fill(Ref<Proactor> Watcher)
    {
        if (mState == State::Live && !IsAwaiting(Operation::Receive))
        {
            Await(Operation::Receive, Watcher.ReceiveFrom(mSocket, mLink, mInbound));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Client::OnReceive(Ref<Proactor> Watcher, Real64 Time, UInt32 Size, Ref<Mailbox> Output)
    {
        if (Size >= kEnvelope)
        {
            switch (static_cast<Envelope>(mInbound[0]))
            {
            case Envelope::Data:
                mSession.Parse(Time, ConstSpan(mInbound.GetData() + kEnvelope, Size - kEnvelope),
                    [&](Delivery Mode, ConstSpan<Byte> Message)
                    {
                        Output.Message(mLink, Mode, Message);
                    });

                if (mSession.IsBroken())
                {
                    LOG_W("Network: the peer sent further ahead than it may and was let go of");

                    Retire(Watcher, mLink, Reason::Protocol, Output);
                    return;
                }
                break;
            case Envelope::Challenge:
                if (Size >= kChallenge)
                {
                    Blit(AddressOf(mCookie), sizeof(mCookie), mInbound.GetData() + kEnvelope);

                    mAnswering = true;

                    Drain(Watcher, Time);
                }
                break;
            default:
                break;
            }
        }

        Fill(Watcher);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Client::OnSend(Ref<Proactor> Watcher, Real64 Time, UInt32 Size)
    {
        Drain(Watcher, Time);
    }
}