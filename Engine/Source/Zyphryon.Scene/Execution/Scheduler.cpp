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

#include "Scheduler.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Scheduler::Scheduler()
        : mRunning { false }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Scheduler::CreatePhase(Text Name, UInt32 After, UInt32 Package)
    {
        mPhases.Append(Unique<Stage>::Create(Name, Package));

        const UInt32 Handle   = static_cast<UInt32>(mPhases.GetSize());
        UInt         Position = mOrder.GetSize();

        if (After)
        {
            const SInt Found = mOrder.Find(After);

            ZY_ASSERT(Found >= 0, "The phase to run after does not exist");
            Position = static_cast<UInt>(Found) + 1;
        }
        mOrder.Insert(Position, Handle);
        return Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Scheduler::CreateSystem(Text Name, UInt32 Phase, UInt16 Rate, AnyRef<Runner> Runner, UInt32 Package)
    {
        ZY_ASSERT(Phase > 0 && Phase <= mPhases.GetSize(), "No phase answers to that handle");

        mSystems.Append(Unique<Routine>::Create(Name, Move(Runner), Rate, Phase, Package));

        const UInt32 Handle = static_cast<UInt32>(mSystems.GetSize());
        mPhases[Phase - 1]->Systems.Append(Handle);
        return Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Scheduler::DestroySystem(UInt32 Handle)
    {
        if (Handle == 0 || Handle > mSystems.GetSize())
        {
            return;
        }

        // A system already taken out leaves its slot empty.
        if (Ref<Unique<Routine>> Slot = mSystems[Handle - 1]; Slot && Slot->Alive)
        {
            // It may be the one running, so it only stops being run until the phases end.
            Slot->Alive = false;
            mDismissed.Append(Handle);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Scheduler::Discard(UInt32 Package)
    {
        for (UInt32 Index = 0; Index < mSystems.GetSize(); ++Index)
        {
            if (mSystems[Index] && mSystems[Index]->Package == Package)
            {
                DestroySystem(Index + 1);
            }
        }

        // Phases keep their handles, so they only leave the order.
        for (UInt Index = mOrder.GetSize(); Index > 0; --Index)
        {
            if (mPhases[mOrder[Index - 1] - 1]->Package == Package)
            {
                mOrder.Remove(Index - 1);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Scheduler::Progress()
    {
        mRunning = true;

        for (UInt Order = 0; Order < mOrder.GetSize(); ++Order)
        {
            ConstRef<Stage> Phase = * mPhases[mOrder[Order] - 1];

            // Indexed, and each system held by its own address, since one may add another and move the list.
            for (UInt Index = 0; Index < Phase.Systems.GetSize(); ++Index)
            {
                Ref<Routine> Target = * mSystems[Phase.Systems[Index] - 1];

                if (Target.Alive && (Target.Rate <= 1 || ++Target.Count >= Target.Rate))
                {
                    ZY_PROFILE_NAMED(Target.Name.GetData(), Target.Name.GetSize());

                    Target.Count = 0;
                    Target.Run();
                }
            }
        }

        mRunning = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Scheduler::Purge()
    {
        if (mRunning)
        {
            return;
        }

        for (const UInt32 Handle : mDismissed)
        {
            Ref<Sequence<UInt32>> Systems = mPhases[mSystems[Handle - 1]->Phase - 1]->Systems;

            Systems.Remove(static_cast<UInt>(Systems.Find(Handle)));
            mSystems[Handle - 1] = nullptr;
        }
        mDismissed.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Scheduler::Clear()
    {
        mSystems.Clear();
        mDismissed.Clear();
    }
}