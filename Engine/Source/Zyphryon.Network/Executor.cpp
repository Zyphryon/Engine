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

#include "Executor.hpp"
#include "Protocol/TCP/Server.hpp"
#include "Protocol/TCP/Stream.hpp"
#include "Protocol/UDP/Client.hpp"
#include "Protocol/UDP/Server.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Executor::~Executor()
    {
        Stop();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Executor::Start()
    {
        if (mWatcher.Start())
        {
#if !defined(ZY_PLATFORM_WEB)
            mWorker = Thread([this](std::stop_token Token)
            {
                ZY_PROFILE_THREAD("Network Thread");

                while (!Token.stop_requested())
                {
                    Run();
                }
            });
#endif
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Stop()
    {
        if (mWorker.joinable())
        {
            mWorker.request_stop();
            mWatcher.Rouse();
            mWorker.join();
        }

        mChannels.ForEach([this](ConstRetainer<Channel> Entry)
        {
            Entry->Retire(mWatcher, Entry->GetLink(), Reason::Closed, mReport);
        });

        Settle();

        mChannels.Clear();
        mWatcher.Stop();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Open(AnyRef<Request> Entry)
    {
        {
            const Guard Lock(mMutex);

            mRequests.Append(Move(Entry));
        }
        mWatcher.Rouse();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Send(Connection Link, Delivery Delivery, ConstSpan<Byte> Message)
    {
        Post(Link, Order::Send, Delivery, Message);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Broadcast(Connection Link, Delivery Delivery, ConstSpan<Byte> Message)
    {
        Post(Link, Order::Broadcast, Delivery, Message);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Close(Connection Link)
    {
        Post(Link, Order::Close, Delivery::Unreliable, ConstSpan<Byte>());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::SetTimeout(Connection Link, Real64 Seconds)
    {
        const ConstSpan Message(reinterpret_cast<ConstPtr<Byte>>(AddressOf(Seconds)), sizeof(Seconds));
        Post(Link, Order::Timeout, Delivery::Unreliable, Message);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Drain(Ref<Mailbox> Output)
    {
#if defined(ZY_PLATFORM_WEB)
        Run();
#endif

        Bool Waiting;

        {
            const Guard Lock(mMutex);

            Waiting = !mPending.Entries.IsEmpty();

            Output.Swap(mIncoming);
        }

        if (Waiting)
        {
            mWatcher.Rouse();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Run()
    {
        // Taken before the pass is timed, since a wait that found nothing is time the thread slept for.
        const ConstSpan<Proactor::Event> Ready = mWatcher.Wait(kMaxIdle);

        ZY_PROFILE_SCOPE("Network::Pass");

        const Real64 Time = mClock.GetSeconds();

        // Acted on where it is taken, since what the proactor hands back stands only until the next wait.
        Dispatch(Time, Ready);

        {
            const Guard Lock(mMutex);

            mScratch.Clear();
            Base::Swap(mScratch, mPending);

            mAdmit.Clear();
            Base::Swap(mAdmit, mRequests);
        }

        Admit(mAdmit);
        Apply(mScratch);
        Sweep(Time);

        {
            const Guard Lock(mMutex);

            // Traded for when the application has taken everything, since that hands the report back the drained
            // buffers and the room they already hold, and copies nothing while the main thread waits on this lock.
            if (mIncoming.IsEmpty())
            {
                mIncoming.Swap(mReport);
            }
            else
            {
                mIncoming.Absorb(mReport);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Post(Connection Link, Order Kind, Delivery Mode, ConstSpan<Byte> Message)
    {
        ZY_PROFILE_SCOPE("Network::Post");

        if (Message.GetSize() > kMaxFrame)
        {
            LOG_W("Network: a message of {0} byte(s) was dropped for being longer than a peer may announce", Message.GetSize());
            return;
        }

        if (Link.IsValid())
        {
            const Guard Lock(mMutex);

            Ref<Command> Slot = mPending.Entries.Append();
            Slot.Link   = Link;
            Slot.Kind   = Kind;
            Slot.Mode   = Mode;
            Slot.Offset = static_cast<UInt32>(mPending.Arena.GetSize());
            Slot.Size   = static_cast<UInt32>(Message.GetSize());

            mPending.Arena.Append(Message);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Dispatch(Real64 Time, ConstSpan<Proactor::Event> Ready)
    {
        for (ConstRef<Proactor::Event> Event : Ready)
        {
            if (Event.Kind != Operation::Rouse)
            {
                if (const Ptr<Retainer<Channel>> Entry = mChannels.TryGet(Event.Link.GetChannel()))
                {
                    (* Entry)->Complete(mWatcher, Time, Event, mReport);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Admit(Ref<Sequence<Request>> Batch)
    {
        for (Ref<Request> Entry : Batch)
        {
            const UInt Slot = Entry.Link.GetChannel().GetSlot();

            ZY_ASSERT(Slot > 0 && Slot <= kMaxEndpoints, "An endpoint was admitted under a slot that names none");

            if (!Entry.Remote.IsValid())
            {
                mReport.Disconnect(Entry.Link, Reason::Unreachable);
                continue;
            }

            Retainer<Channel> Endpoint = nullptr;

            if (Entry.Transport == Transport::UDP)
            {
                Endpoint = Entry.Server
                    ? Retainer<Channel>(Retainer<UDP::Server>::Create(Entry.Link, Entry.Origin, Entry.Listener))
                    : Retainer<Channel>(Retainer<UDP::Client>::Create(Entry.Link, Entry.Origin, Entry.Listener));
            }
            else
            {
                Endpoint = Entry.Server
                    ? Retainer<Channel>(Retainer<TCP::Server>::Create(Entry.Link, Entry.Origin, Entry.Listener))
                    : Retainer<Channel>(Retainer<TCP::Stream>::Create(Entry.Link, Entry.Origin, Entry.Listener));
            }

            if (Endpoint->Open(mWatcher, Entry.Remote))
            {
                // Filed at the slot the service already named, rather than at one chosen here, since the
                // application was handed that slot before this thread ever saw the request.
                mChannels.Acquire(Entry.Link.GetChannel(), Endpoint);

                // An endpoint that accepts is open the moment it is bound, and so is one that carries datagrams, since
                // neither has an attempt to settle. A stream that reaches announces itself once its attempt has.
                if (Entry.Server || Entry.Transport == Transport::UDP)
                {
                    mReport.Connect(Entry.Link);
                }
            }
            else
            {
                mReport.Disconnect(Entry.Link, Reason::Unreachable);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Apply(Ref<Outbox> Batch)
    {
        for (ConstRef<Command> Entry : Batch.Entries)
        {
            if (const Ptr<Retainer<Channel>> Endpoint = mChannels.TryGet(Entry.Link.GetChannel()); Endpoint)
            {
                switch (Entry.Kind)
                {
                case Order::Send:
                    (* Endpoint)->Send(Entry.Link, Entry.Mode, Batch.GetPayload(Entry));
                    break;
                case Order::Broadcast:
                    (* Endpoint)->Broadcast(Entry.Mode, Batch.GetPayload(Entry));
                    break;
                case Order::Close:
                    (* Endpoint)->Retire(mWatcher, Entry.Link, Reason::Closed, mReport);
                    break;
                case Order::Timeout:
                {
                    Real64 Seconds = 0.0;
                    Blit(reinterpret_cast<Ptr<Byte>>(AddressOf(Seconds)), sizeof(Seconds), Batch.GetPayload(Entry).GetData());

                    (* Endpoint)->SetTimeout(Entry.Link, Seconds);
                    break;
                }
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Settle()
    {
        for (UInt32 Pass = 0; ; ++Pass)
        {
            Bool Quiet = true;

            mChannels.ForEach([&](ConstRetainer<Channel> Entry)
            {
                Quiet &= Entry->IsQuiet();
            });

            if (Quiet)
            {
                break;
            }

            if (Pass > 0 && (Pass % kMaxPatience) == 0)
            {
                LOG_W("Network: still waiting on the platform after {0} second(s)", (Pass * kMaxIdle) / 1000);
            }

            Dispatch(mClock.GetSeconds(), mWatcher.Wait(kMaxIdle));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Sweep(Real64 Time)
    {
        mChannels.ForEachOrRemove([this, Time](ConstRetainer<Channel> Entry)
        {
            Entry->Tick(mWatcher, Time, mReport);

            if (const Reason Cause = Entry->GetCause(); Cause != Reason::None && Entry->IsQuiet())
            {
                mReport.Disconnect(Entry->GetLink(), Cause);
                return true;
            }
            return false;
        });
    }
}