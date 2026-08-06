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
#include "Zyphryon.Network/Endpoint.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::UDP
{
    /// \brief Represents the proof a stranger has to send back before an endpoint writes anything down about it.
    class Sentry final
    {
    public:

        /// \brief Time, in seconds, one key stands for before the next takes over from it.
        static constexpr Real64 kRotation = 15.0;

    public:

        /// \brief Constructs a sentry under two keys nothing has been proven against yet.
        ZY_INLINE Sentry()
            : mRotation { 0.0 }
        {
            mSecret[0] = Conjure();
            mSecret[1] = Conjure();
        }

        /// \brief Stands the key in use down and takes up a fresh one, once it has stood long enough.
        ///
        /// \param Time The current time, in seconds.
        ZY_INLINE void Rotate(Real64 Time)
        {
            if (mRotation == 0.0)
            {
                mRotation = Time;
                return;
            }

            if (Time - mRotation > kRotation)
            {
                mSecret[1] = mSecret[0];
                mSecret[0] = Conjure();

                mRotation  = Time;
            }
        }

        /// \brief Writes the challenge an address is owed.
        ///
        /// \param Origin The address the challenge is bound to.
        /// \param Output Receives the challenge, which must have room for \ref kProof bytes.
        ZY_INLINE void Inscribe(ConstRef<Endpoint> Origin, Span<Byte> Output) const
        {
            ZY_ASSERT(Output.GetSize() >= kChallenge, "The datagram has no room for the challenge it is to carry");

            const UInt64 Cookie = Reckon(Origin, 0);

            Output[0] = Enum::Cast(Envelope::Challenge);

            Blit(Output.GetData() + kEnvelope, sizeof(Cookie), AddressOf(Cookie));
        }

        /// \brief Checks whether a datagram carries the proof the address it came from was owed.
        ///
        /// \param Origin The address the datagram came from.
        /// \param Packet The datagram, envelope and all.
        /// \return `true` when the proof is the one that address was owed, `false` otherwise.
        ZY_INLINE Bool Approve(ConstRef<Endpoint> Origin, ConstSpan<Byte> Packet) const
        {
            if (Packet.GetSize() < kChallenge)
            {
                return false;
            }

            UInt64 Given = 0;
            Blit(AddressOf(Given), sizeof(Given), Packet.GetData() + kEnvelope);

            const UInt64 Now = Reckon(Origin, 0);
            const UInt64 Was = Reckon(Origin, 1);
            return ((Given ^ Now) == 0) | ((Given ^ Was) == 0);
        }

    private:

        /// \brief Makes a key nothing can guess, or a key of nothing where the system had none to give.
        ///
        /// \return The key.
        ZY_INLINE static Signer Conjure()
        {
            UInt64 Key[2] = { };

            if (!Entropy::Gather(Span(reinterpret_cast<Ptr<Byte>>(Key), sizeof(Key))))
            {
                LOG_E("Network: the system gave nothing to key a proof with, so no address can be held to one");
            }
            return Signer(Key[0], Key[1]);
        }

        /// \brief Works out the proof an address is owed under one of the keys standing.
        ///
        /// \param Origin The address the proof is bound to.
        /// \param Epoch  The key to work it out under, being the one standing now or the one before it.
        /// \return The proof that address is owed.
        ZY_INLINE UInt64 Reckon(ConstRef<Endpoint> Origin, UInt Epoch) const
        {
            Byte       Digest[Endpoint::kCapacity];
            const UInt Length = Origin.Canonicalize(Span(Digest, sizeof(Digest)));

            return mSecret[Epoch].Sign(ConstSpan(Digest, Length));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Signer, 2> mSecret;
        Real64           mRotation;
    };
}