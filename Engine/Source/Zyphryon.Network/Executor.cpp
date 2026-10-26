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

namespace ZyNetwork
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
            mWorker = Thread([this](ConstRef<Thread::Token> Token)
            {
                OnWorkerThread(Token);
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
        if (mWorker.Joinable())
        {
            mWorker.Stop();
            mWatcher.Rouse();
            mWorker.Join();
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
            ZyBase::Swap(mScratch, mPending);

            mAdmit.Clear();
            ZyBase::Swap(mAdmit, mRequests);
        }

        Admit(mAdmit);
        Apply(mScratch);
        Sweep(Time);

        {
            const Guard Lock(mMutex);

            // Swapped when the application took everything, so nothing is copied while it waits on this lock.
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

    void Executor::OnWorkerThread(ConstRef<Thread::Token> Token)
    {
        ZY_PROFILE_THREAD("Network Thread");

        while (!Token.Stopped())
        {
            Run();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Executor::Post(Connection Link, Order Kind, Delivery Mode, ConstSpan<Byte> Message)
    {
        ZY_PROFILE_SCOPE("Network::Post");

        if (Message.GetSize() > kMaxFrame)
        {
            LOG_W("Network: a message of {0} byte(s) was dropped for being too large", Message.GetSize());
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

            if (Revoke(Entry.Link))
            {
                mReport.Disconnect(Entry.Link, Reason::Closed);
                continue;
            }

            if (!Entry.Remote.IsValid())
            {
                mReport.Disconnect(Entry.Link, Reason::Unreachable);
                continue;
            }

            Retainer<Channel> Target;

            if (Entry.Transport == Transport::UDP)
            {
                Target = Entry.Server
                    ? Retainer<Channel>(Retainer<UDP::Server>::Create(Entry.Link, Entry.Origin, Entry.Listener))
                    : Retainer<Channel>(Retainer<UDP::Client>::Create(Entry.Link, Entry.Origin, Entry.Listener));
            }
            else
            {
                Target = Entry.Server
                    ? Retainer<Channel>(Retainer<TCP::Server>::Create(Entry.Link, Entry.Origin, Entry.Listener))
                    : Retainer<Channel>(Retainer<TCP::Stream>::Create(Entry.Link, Entry.Origin, Entry.Listener));
            }

            if (Target->Open(mWatcher, Entry.Remote))
            {
                // Filed at the slot the service already handed the application.
                mChannels.Acquire(Entry.Link.GetChannel(), Target);

                // A listener or a datagram endpoint has no attempt to settle, so it is open once bound.
                if (Entry.Server || Entry.Transport == Transport::UDP)
                {
                    mReport.Connect(Entry.Link, Entry.Origin);
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
            const Ptr<Retainer<Channel>> Target = mChannels.TryGet(Entry.Link.GetChannel());

            if (Target == nullptr)
            {
                // An endpoint still waiting on its address has no channel yet, so a close for it waits for it to arrive.
                if (Entry.Kind == Order::Close && !Entry.Link.IsPeer() && !IsRevoked(Entry.Link))
                {
                    mRevoked.Append(Entry.Link);
                }
                continue;
            }

            switch (Entry.Kind)
            {
            case Order::Send:
                (* Target)->Send(Entry.Link, Entry.Mode, Batch.GetPayload(Entry));
                break;
            case Order::Broadcast:
                (* Target)->Broadcast(Entry.Mode, Batch.GetPayload(Entry));
                break;
            case Order::Close:
                (* Target)->Retire(mWatcher, Entry.Link, Reason::Closed, mReport);
                break;
            case Order::Timeout:
            {
                Real64 Seconds = 0.0;
                Blit(reinterpret_cast<Ptr<Byte>>(AddressOf(Seconds)), sizeof(Seconds), Batch.GetPayload(Entry).GetData());

                (* Target)->SetTimeout(Entry.Link, Seconds);
                break;
            }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Executor::Revoke(Connection Link)
    {
        const Bool Revoked = IsRevoked(Link);

        mRevoked.RemoveFastSomeIf([Slot = Link.GetChannel().GetSlot()](ConstRef<Connection> Entry)
        {
            return Entry.GetChannel().GetSlot() == Slot;
        });
        return Revoked;
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