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

#include "Session.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::UDP
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Session::Reset()
    {
        mPending.Clear();
        mArena.Clear();

        mStats    = Statistics();
        mSequence = 0;
        mBroken   = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Session::Push(Delivery Mode, ConstSpan<Byte> Message)
    {
        if (Mode != Delivery::Unreliable)
        {
            LOG_E("Network: a message that must arrive was refused, which this build cannot promise yet");
            return false;
        }

        if (Message.GetSize() > kPayload)
        {
            LOG_E("Network: a message of {0} byte(s) is more than one datagram carries", Message.GetSize());
            return false;
        }

        if (mArena.GetSize() >= kMaxBacklog)
        {
            return false;
        }

        Ref<Parcel> Entry = mPending.Append();
        Entry.Offset = static_cast<UInt32>(mArena.GetSize());
        Entry.Size   = static_cast<UInt16>(Message.GetSize());
        Entry.Mode   = Mode;

        mArena.Append(Message);

        mStats.Pending = mArena.GetSize();
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Session::Build(Real64 Time, Span<Byte> Output)
    {
        ZY_ASSERT(Output.GetSize() > kHeader, "The packet has no room for anything beyond its header");

        UInt32 Cursor = kHeader;

        for (const Parcel Entry : mPending)
        {
            if (Cursor + kMessage + Entry.Size > Output.GetSize())
            {
                break;
            }

            Output[Cursor++] = Enum::Cast(Entry.Mode);

            Write(Output.GetData() + Cursor, Entry.Size);
            Cursor += sizeof(UInt16);

            Blit(Output.GetData() + Cursor, Entry.Size, mArena.GetData() + Entry.Offset);
            Cursor += Entry.Size;
        }

        if (Cursor != kHeader)
        {
            Header Value;
            Value.Sequence    = mSequence++;
            Value.Acknowledge = 0;
            Value.Bits        = 0;
            Encode(Output, Value);

            mPending.Clear();
            mArena.Clear();

            mStats.Sent   += Cursor;
            mStats.Pending = 0;
            return Cursor;
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Session::Encode(Span<Byte> Output, ConstRef<Header> Value)
    {
        ZY_ASSERT(Output.GetSize() >= kHeader, "The packet has no room for the header it opens with");

        Write(Output.GetData() + 0, Value.Sequence);
        Write(Output.GetData() + 2, Value.Acknowledge);
        Write(Output.GetData() + 4, Value.Bits);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Session::Decode(ConstSpan<Byte> Input, Ref<Header> Output)
    {
        if (Input.GetSize() < kHeader)
        {
            return false;
        }

        Output.Sequence    = Read<UInt16>(Input.GetData() + 0);
        Output.Acknowledge = Read<UInt16>(Input.GetData() + 2);
        Output.Bits        = Read<UInt64>(Input.GetData() + 4);
        return true;
    }
}