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

#include "Envelope.hpp"
#include "Zyphryon.Network/Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::UDP
{
    /// \brief Represents everything one peer has been handed to say and everything it has heard back.
    /// TODO: Current support: Unreliable (Only)
    class Session final
    {
    public:

        /// \brief Number of bytes the header takes at the front of every packet.
        static constexpr UInt kHeader  = 12;

        /// \brief Number of bytes every message takes beyond its payload, being its guarantee and its length.
        static constexpr UInt kMessage = 3;

        /// \brief Maximum size, in bytes, of a payload, which is a whole datagram less everything that names it.
        static constexpr UInt kPayload = kMaxDatagram - kEnvelope - kHeader - kMessage;

    public:

        /// \brief Constructs a session that has neither said nor heard anything.
        ZY_INLINE Session()
        {
            Reset();
        }

        /// \brief Forgets everything said and everything heard, as though nothing ever had been.
        void Reset();

        /// \brief Gets everything this peer has cost and how well it is holding up.
        ///
        /// \return What the peer has cost.
        ZY_INLINE ConstRef<Statistics> GetStats() const
        {
            return mStats;
        }

        /// \brief Checks whether the peer has framed something no peer that agreed with this build ever would.
        ///
        /// \return `true` once the peer has sent something unreadable, `false` while it has not.
        ZY_INLINE Bool IsBroken() const
        {
            return mBroken;
        }

        /// \brief Writes down a message for the peer.
        ///
        /// \param Mode    The guarantee the message needs, of which only \ref Delivery::Unreliable is carried yet.
        /// \param Message The payload, which is copied.
        /// \return `true` if the message was written down, `false` if it was refused.
        Bool Push(Delivery Mode, ConstSpan<Byte> Message);

        /// \brief Fills a packet with whatever is waiting to go out.
        ///
        /// \param Time   The current time, in seconds.
        /// \param Output Receives the packet, which must have room for a whole datagram.
        /// \return The number of bytes written, or zero where there was nothing worth sending.
        UInt32 Build(Real64 Time, Span<Byte> Output);

        /// \brief Reads a packet, handing over every message it carried.
        ///
        /// \param Time     The current time, in seconds.
        /// \param Packet   The bytes the packet arrived as.
        /// \param Consumer The callable told about each message, as `(Delivery, ConstSpan<Byte>)`.
        /// \return `true` when the packet was read, `false` when it was too short to hold a header at all.
        template<typename Callable>
        Bool Parse(Real64 Time, ConstSpan<Byte> Packet, AnyRef<Callable> Consumer)
        {
            Header Value;

            if (!Decode(Packet, Value))
            {
                return false;
            }

            UInt Cursor = kHeader;

            while (Cursor + kMessage <= Packet.GetSize())
            {
                const Delivery Mode = static_cast<Delivery>(Packet[Cursor++]);

                if (Mode != Delivery::Unreliable)
                {
                    LOG_E("Network: a peer sent a message under a guarantee this build cannot read");

                    mBroken = true;
                    break;
                }

                const UInt16 Size = Read<UInt16>(Packet.GetData() + Cursor);
                Cursor += sizeof(UInt16);

                // A message reaching past the datagram that carried it is one nothing framing this the same way
                // could have written, so the peer is taken as speaking something other than this.
                if (Cursor + Size > Packet.GetSize())
                {
                    LOG_E("Network: a peer sent a message longer than the datagram it arrived in");

                    mBroken = true;
                    break;
                }

                Consumer(Mode, ConstSpan(Packet.GetData() + Cursor, Size));

                Cursor += Size;
            }

            mStats.Received += Packet.GetSize();
            return true;
        }

    private:

        /// \brief Represents the numbers a packet opens with, which every packet carries.
        struct Header final
        {
            /// The number this packet goes out under, which counts up and wraps around.
            UInt16 Sequence    = 0;

            /// The newest packet the far end had seen, left at nothing until something counts them.
            UInt16 Acknowledge = 0;

            /// Which packets behind \ref Ack it had also seen, left at nothing until something answers for them.
            UInt64 Bits        = 0;
        };

        /// \brief Represents a message handed over and not yet sent.
        struct Parcel final
        {
            /// Where its payload sits among those waiting to go out.
            UInt32   Offset = 0;

            /// How long that payload is.
            UInt16   Size   = 0;

            /// The guarantee it was handed over under.
            Delivery Mode   = Delivery::Unreliable;
        };

        /// \brief Writes a header into the bytes a packet opens with.
        ///
        /// \param Output Receives the header, which must have room for \ref kHeader bytes.
        /// \param Value  The header to write.
        static void Encode(Span<Byte> Output, ConstRef<Header> Value);

        /// \brief Reads the header a packet opens with.
        ///
        /// \param Input  The bytes the packet arrived as.
        /// \param Output Receives the header, left untouched where the packet is too short to hold one.
        /// \return `true` if a header was read, `false` otherwise.
        static Bool Decode(ConstSpan<Byte> Input, Ref<Header> Output);

        /// \brief Reads a number written the way this build writes them.
        ///
        /// \param Input The bytes to read it from.
        /// \return The number read.
        template<typename Type>
        ZY_INLINE static Type Read(ConstPtr<Byte> Input)
        {
            Type Value = 0;

            for (UInt Step = 0; Step < sizeof(Type); ++Step)
            {
                Value |= static_cast<Type>(static_cast<Type>(Input[Step]) << (Step * 8));
            }
            return Value;
        }

        /// \brief Writes a number so a build on any machine reads back what this one wrote.
        ///
        /// \param Output Receives the number.
        /// \param Value  The number to write.
        template<typename Type>
        ZY_INLINE static void Write(Ptr<Byte> Output, Type Value)
        {
            for (UInt Step = 0; Step < sizeof(Type); ++Step)
            {
                Output[Step] = static_cast<Byte>(Value >> (Step * 8));
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Parcel> mPending;
        Sequence<Byte>   mArena;
        UInt16           mSequence;
        Bool             mBroken;
        Statistics       mStats;
    };
}