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

#include "Touch.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Input
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Touch::Begin()
    {
        mPoints.RemoveFastSomeIf([](ConstRef<Point> Entry)
        {
            return Entry.Phase == Phase::Ended || Entry.Phase == Phase::Cancelled;
        });

        for (Ref<Point> Entry : mPoints)
        {
            Entry.Phase  = Phase::Moved;
            Entry.DeltaX = 0.0f;
            Entry.DeltaY = 0.0f;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Touch::Process(ConstRef<Event> Event)
    {
        switch (Event.Kind)
        {
        case Event::Type::TouchDown:
        {
            if (Locate(Event.TouchAction.ID))
            {
                return;
            }

            if (mPoints.IsFull())
            {
                LOG_W("Input: a touch was turned away because the surface carries all it may at once");
                return;
            }

            Ref<Point> Entry = mPoints.Append();
            Entry.ID     = Event.TouchAction.ID;
            Entry.Phase  = Phase::Began;
            Entry.X      = Event.TouchAction.X;
            Entry.Y      = Event.TouchAction.Y;
            Entry.DeltaX = 0.0f;
            Entry.DeltaY = 0.0f;
            break;
        }
        case Event::Type::TouchMove:
            if (const Ptr<Point> Entry = Locate(Event.TouchAction.ID))
            {
                Entry->DeltaX += Event.TouchAction.X - Entry->X;
                Entry->DeltaY += Event.TouchAction.Y - Entry->Y;
                Entry->X       = Event.TouchAction.X;
                Entry->Y       = Event.TouchAction.Y;
            }
            break;
        case Event::Type::TouchUp:
            if (const Ptr<Point> Entry = Locate(Event.TouchAction.ID))
            {
                Entry->DeltaX += Event.TouchAction.X - Entry->X;
                Entry->DeltaY += Event.TouchAction.Y - Entry->Y;
                Entry->X       = Event.TouchAction.X;
                Entry->Y       = Event.TouchAction.Y;
                Entry->Phase   = Phase::Ended;
            }
            break;
        case Event::Type::TouchCancel:
            if (const Ptr<Point> Entry = Locate(Event.TouchAction.ID))
            {
                Entry->Phase = Phase::Cancelled;
            }
            break;
        default:
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Touch::Reset()
    {
        mPoints.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Touch::CancelAllPoints(Ref<Sequence<Event>> Output)
    {
        for (Ref<Point> Entry : mPoints)
        {
            if (Entry.Phase != Phase::Ended && Entry.Phase != Phase::Cancelled)
            {
                Entry.Phase = Phase::Cancelled;

                Ref<Event> Release         = Output.Append(Event::Type::TouchCancel);
                Release.TouchAction.ID     = Entry.ID;
                Release.TouchAction.X      = Entry.X;
                Release.TouchAction.Y      = Entry.Y;
                Release.TouchAction.DeltaX = 0.0f;
                Release.TouchAction.DeltaY = 0.0f;
            }
        }
    }
}