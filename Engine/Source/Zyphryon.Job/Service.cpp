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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Job
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host }
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

    void Service::OnTick(Real64 Time)
    {
        ZY_PROFILE_SCOPE("Job::Tick");

        Slot Pending;

        {
            Guard Guard(mMutex);

            Pending = GetExecutor(Lane::Main).Drain();
        }

        while (Pending != 0)
        {
            const Slot Following = mRegistry[Pending].Next;

            Execute(Pending);

            Pending = Following;
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

    Handle Service::Submit(Lane Target, AnyRef<Task> Work, Handle Dependency)
    {
        const Lane Actual = Reconcile(Target);

        Handle Result;
        Bool   Queued  = false;
        Bool   Starved = false;

        {
            Guard Guard(mMutex);

            if (const Slot Value = mRegistry.Acquire(Forward<Task>(Work), Actual); Value)
            {
                Result = mRegistry.Mint(Value);

                // Park the job on its dependency rather than queueing it, and let that job's completion release it.
                Queued = !mRegistry.Park(Value, Dependency);

                if (Queued)
                {
                    GetExecutor(Actual).Push(mRegistry, Value);
                }
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

            // Rather than idle, drain the lane this thread is allowed to run; this is what keeps a waiting
            // thread from wasting a core, and on a platform without workers it is the only thing that makes
            // progress at all.
            if (const Slot Stolen = GetExecutor(Reconcile(Lane::Compute)).Pop(mRegistry); Stolen != 0)
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

    void Service::Block(Handle Job)
    {
        ZY_PROFILE_SCOPE("Job::Block");

        Lock Lock(mMutex);

        while (true)
        {
            const Slot Value = mRegistry.Resolve(Job);

            if (Value == 0 || mRegistry[Value].Complete)
            {
                mRegistry.Retire(Value);
                return;
            }

            mGate.wait(Lock);
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

    Bool Service::IsComplete(Handle Job) const
    {
        Guard Guard(mMutex);

        // A stale handle means the slot was already recycled, which only happens once the job has finished.
        const Slot Value = mRegistry.Resolve(Job);
        return (Value == 0 || mRegistry[Value].Complete);
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

        Array<Bool, kMaxLanes> Woken { };

        {
            Guard Guard(mMutex);

            Job.Complete = true;

            for (Slot Next = mRegistry.Detach(Value); Next != 0; )
            {
                const Slot Following = mRegistry[Next].Next;
                const Lane Target    = mRegistry[Next].Target;

                GetExecutor(Target).Push(mRegistry, Next);

                Woken[static_cast<UInt32>(Target)] = true;

                Next = Following;
            }

            mRegistry.Retire(Value);
        }

        // Waiters key off any completion, since a helping wait has to re-check its own job after each one.
        mGate.notify_all();

        for (UInt32 Index = 0; Index < kMaxLanes; ++Index)
        {
            if (Woken[Index])
            {
                mExecutors[Index].Signal();
            }
        }
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

                Value = Source.Pop(mRegistry);
            }

            if (Value != 0)
            {
                Execute(Value);
            }
        }
    }
}