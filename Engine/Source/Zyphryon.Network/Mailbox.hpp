// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Connection.hpp"
#include "Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Represents everything the endpoints have had to say, held until the application is ready to hear it.
    class Mailbox final
    {
    public:

        /// \brief Specifies what a notification is about.
        enum class Signal : UInt8
        {
            Connect,     ///< A peer became ready.
            Disconnect,  ///< A peer stopped.
            Message,     ///< A peer delivered something.
        };

        /// \brief Represents one notification and where in the arena its payload sits.
        struct Entry final
        {
            /// The connection the notification concerns.
            Connection Link;

            /// The kind of notification this is.
            Signal     Kind   = Signal::Connect;

            /// The guarantee a message was sent under, meaningless for anything else.
            Delivery   Mode   = Delivery::Unreliable;

            /// The reason a connection ended, meaningless for anything else.
            Reason     Cause  = Reason::None;

            /// The offset into the arena where the payload begins.
            UInt32     Offset = 0;

            /// The length of the payload, in bytes.
            UInt32     Size   = 0;
        };

    public:

        /// \brief Checks whether anything at all was recorded.
        ///
        /// \return `true` if nothing is waiting, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mEntries.IsEmpty();
        }

        /// \brief Gets the notifications recorded so far.
        ///
        /// \return The notifications, in the order they happened.
        ZY_INLINE ConstSpan<Entry> GetEntries() const
        {
            return mEntries;
        }

        /// \brief Gets the payload one notification carries.
        ///
        /// \param Entry The notification to read.
        /// \return The payload, which is empty for anything that carries none.
        ZY_INLINE ConstSpan<Byte> GetPayload(ConstRef<Entry> Entry) const
        {
            return ConstSpan(mArena.GetData() + Entry.Offset, Entry.Size);
        }

        /// \brief Records that a peer became ready.
        ///
        /// \param Link The peer that is now open.
        ZY_INLINE void Connect(Connection Link)
        {
            Push(Link, Signal::Connect, Delivery::Unreliable, Reason::None, ConstSpan<Byte>());
        }

        /// \brief Records that a peer stopped.
        ///
        /// \param Link  The peer that is now closed.
        /// \param Cause The reason it closed.
        ZY_INLINE void Disconnect(Connection Link, Reason Cause)
        {
            Push(Link, Signal::Disconnect, Delivery::Unreliable, Cause, ConstSpan<Byte>());
        }

        /// \brief Records that a peer delivered something.
        ///
        /// \param Link    The peer the message came from.
        /// \param Mode    The guarantee it was sent under.
        /// \param Payload The payload, which is copied into the arena.
        ZY_INLINE void Message(Connection Link, Delivery Mode, ConstSpan<Byte> Payload)
        {
            Push(Link, Signal::Message, Mode, Reason::None, Payload);
        }

        /// \brief Drops everything recorded, keeping the room it was recorded into.
        ZY_INLINE void Clear()
        {
            mEntries.Clear();
            mArena.Clear();
        }

        /// \brief Takes everything another mailbox holds, leaving it empty.
        ///
        /// \param Other The mailbox to take from.
        ZY_INLINE void Absorb(Ref<Mailbox> Other)
        {
            const UInt32 Base = static_cast<UInt32>(mArena.GetSize());

            for (ConstRef<Entry> Item : Other.mEntries)
            {
                mEntries.Append(Item).Offset += Base;
            }

            mArena.Append(Other.mArena);

            Other.Clear();
        }

        /// \brief Trades contents with another mailbox.
        ///
        /// \param Other The mailbox to trade with.
        ZY_INLINE void Swap(Ref<Mailbox> Other)
        {
            Base::Swap(mEntries, Other.mEntries);
            Base::Swap(mArena,   Other.mArena);
        }

    private:

        /// \brief Records one notification and copies its payload into the arena.
        ///
        /// \param Link    The connection the notification concerns.
        /// \param Kind    The kind of notification to record.
        /// \param Mode    The guarantee a message was sent under, ignored for anything else.
        /// \param Cause   The reason a connection ended, ignored for anything else.
        /// \param Payload The payload, which is empty for anything that carries none.
        ZY_INLINE void Push(Connection Link, Signal Kind, Delivery Mode, Reason Cause, ConstSpan<Byte> Payload)
        {
            Ref<Entry> Slot = mEntries.Append();
            Slot.Link   = Link;
            Slot.Kind   = Kind;
            Slot.Mode   = Mode;
            Slot.Cause  = Cause;
            Slot.Offset = static_cast<UInt32>(mArena.GetSize());
            Slot.Size   = static_cast<UInt32>(Payload.GetSize());

            mArena.Append(Payload);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Entry> mEntries;
        Sequence<Byte>  mArena;
    };
}