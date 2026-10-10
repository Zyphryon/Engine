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

#if !defined(ZY_HAS_THREADS)
#include "Zyphryon.Platform/Timer.hpp"
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyJob
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host },
          mMain     { GetThreadID() }
    {
#if defined(ZY_HAS_THREADS)
        const UInt Cores = Max(1u, std::thread::hardware_concurrency());

        // Leave one core for the main thread, which joins the compute lane through helping waits.
        GetExecutor(Lane::Compute).Start(Max(1u, Cores - 1), [this](std::stop_token Token)
        {
            ZY_PROFILE_THREAD("Job::Compute");

            OnWorkerThread(Lane::Compute, Token);
        });

        // Sized for latency rather than throughput, since these threads spend their time parked in the kernel.
        GetExecutor(Lane::IO).Start(Clamp(Cores / 4, 2u, 4u), [this](std::stop_token Token)
        {
            ZY_PROFILE_THREAD("Job::IO");

            OnWorkerThread(Lane::IO, Token);
        });
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Real64 Delta)
    {
        ZY_PROFILE_SCOPE("Job::Tick");

        // Only what was queued before the tick began runs now, so work a job queues waits for the next tick.
        UInt Count;

        {
            Guard Guard(mMutex);
            Count = GetExecutor(Lane::Main).GetSize();
        }

#if !defined(ZY_HAS_THREADS)
        // Every lane lands here, so a load burst is spread over frames rather than run to the end in one callback.
        const ZyPlatform::Timer Clock    = ZyPlatform::Timer();
        const Real64            Deadline = Clock.GetSeconds() + kMaxTickBudget;
#endif

        for (; Count > 0; --Count)
        {
            Slot Value;

            {
                Guard Guard(mMutex);
                Value = GetExecutor(Lane::Main).Pop();
            }

            // A helping wait inside a job may already have taken the rest.
            if (Value == 0)
            {
                break;
            }

            Execute(Value);

#if !defined(ZY_HAS_THREADS)
            // What is left stays queued, and runs ahead of anything queued after it next tick.
            if (Clock.GetSeconds() >= Deadline)
            {
                break;
            }
#endif
        }

        // Work a worker handed over while the table was full runs after what the table had queued.
        Sequence<Task> Overflow;

        {
            Guard Guard(mMutex);
            Overflow = Move(mOverflow);
        }

        for (Ref<Task> Work : Overflow)
        {
            Work();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTeardown()
    {
#if defined(ZY_HAS_THREADS)
        for (Ref<Executor> Each : mExecutors)
        {
            Each.Stop();
        }
#endif

        // Anything still queued will never run, so drop the captures it holds rather than leaking them.
        Guard Guard(mMutex);

        mRegistry.Clear();
        mOverflow.Clear();

        for (Ref<Executor> Each : mExecutors)
        {
            Each.Clear();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Dispatch(Lane Target, AnyRef<Task> Work)
    {
        Discard(Submit(Target, Forward<Task>(Work)));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Handle Service::Submit(Lane Target, AnyRef<Task> Work)
    {
        const Lane Actual = Reconcile(Target);

        Handle Result;
        Bool   Queued  = false;
        Bool   Starved = false;

        {
            Guard Guard(mMutex);

            if (const Slot Value = mRegistry.Acquire(Forward<Task>(Work)); Value)
            {
                Result = mRegistry.Mint(Value);
                Queued = true;

                GetExecutor(Actual).Push(Value);
            }
            else if (Actual == Lane::Main && GetThreadID() != mMain)
            {
                mOverflow.Append(Forward<Task>(Work));
            }
            else
            {
                Starved = true;
            }
        }

        if (Queued)
        {
            GetExecutor(Actual).Signal();
        }
        else
        {
            if (Starved)
            {
                Work();
            }
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Wait(Handle Job)
    {
        ZY_PROFILE_SCOPE("Job::Wait");

        Lock Lock(mMutex);

        while (true)
        {
            const Slot Value = mRegistry.Resolve(Job);

            if (Value == 0 || mRegistry[Value].Complete)
            {
                mRegistry.Retire(Value);
                return;
            }

            // Rather than idle, run queued compute work, since without workers nothing else runs it.
            if (const Slot Stolen = GetExecutor(Reconcile(Lane::Compute)).Pop(); Stolen != 0)
            {
                Lock.unlock();

                Execute(Stolen);

                Lock.lock();
            }
            else
            {
                mGate.wait(Lock);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Discard(Handle Job)
    {
        Guard Guard(mMutex);

        mRegistry.Retire(mRegistry.Resolve(Job));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Execute(Slot Value)
    {
        ZY_PROFILE_SCOPE("Job::Execute");

        // The slot cannot be recycled while its execution claim is held, so this reference survives the unlocked
        // call below even though the registry is only serialized inside the guard.
        Ref<Registry::Entry> Job = mRegistry[Value];

        // Consume it, since a waiter may recycle the slot the instant it observes it.
        Job.Work.Consume();

        {
            Guard Guard(mMutex);

            Job.Complete = true;

            mRegistry.Retire(Value);
        }

        // Waiters key off any completion, since a helping wait has to re-check its own job after each one.
        mGate.notify_all();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnWorkerThread(Lane Target, ConstRef<std::stop_token> Token)
    {
        Ref<Executor> Source = GetExecutor(Target);

        while (!Token.stop_requested())
        {
            Slot Value;

            {
                Lock Lock(mMutex);

                Source.Await(Lock, [&]
                {
                    return !Source.IsEmpty() || Token.stop_requested();
                });

                if (Token.stop_requested())
                {
                    return;
                }

                Value = Source.Pop();
            }

            if (Value != 0)
            {
                Execute(Value);
            }
        }
    }
}