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

#include "Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Represents a value one side keeps telling the other, of which only the newest ever matters.
    ///
    /// \tparam Type   The value, which is serializable or trivially copyable.
    /// \tparam Opcode The type the application names its opcodes with, an enumeration or a plain integer, whose
    ///                width is what the message opens with.
    template<typename Type, typename Opcode = UInt8>
    class Signal final
    {
    public:

        /// \brief How long a held value goes unsaid before it is said again, in seconds.
        static constexpr Real64 kRepeat = 0.25;

    public:

        /// \brief Constructs a signal under the opcode its messages open with.
        ///
        /// \param Code The opcode, which the application keeps apart from every other message it exchanges.
        ZY_INLINE explicit Signal(Opcode Code)
            : mElapsed  { kRepeat },
              mOpcode   { Code },
              mSequence { 0 },
              mFresh    { false }
        {
        }

        /// \brief Sets the value the sender says from here on.
        ///
        /// \param Value The value, which goes out on the next tick if it differs from what was last said.
        ZY_INLINE void Set(ConstRef<Type> Value)
        {
            mValue = Value;
        }

        /// \brief Gets the value, which is what was last set on the sender and last received on the receiver.
        ///
        /// \return The value.
        ZY_INLINE ConstRef<Type> Get() const
        {
            return mValue;
        }

        /// \brief Takes the value if a newer one was received since it was last taken.
        ///
        /// \param Value Receives the value.
        /// \return `true` if a newer value had arrived, `false` if nothing came since the last take.
        ZY_INLINE Bool Poll(Ref<Type> Value)
        {
            if (!mFresh)
            {
                return false;
            }
            mFresh = false;
            Value  = mValue;
            return true;
        }

        /// \brief Writes the message when the value changed since it was last said, or has held for \ref kRepeat.
        ///
        /// \param Delta  The time since the last tick, in seconds.
        /// \param Output The stream to write the message into, which the sender then sends unreliably.
        /// \return `true` if a message was written, `false` if there was nothing to say yet.
        Bool Tick(Real64 Delta, Ref<Writer> Output)
        {
            mElapsed += Delta;

            // Compared as bytes, so a value that cannot say whether it changed still only goes out when it did.
            mScratch.Clear();
            Archive<Writer>(mScratch).Serialize(mValue);

            if (Same(mScratch, mSaid) && mElapsed < kRepeat)
            {
                return false;
            }

            mElapsed = 0.0;
            mSaid.Clear();
            mSaid.Write<Byte>(mScratch.GetData(), mScratch.GetSize());

            Archive<Writer>(Output).Serialize(mOpcode);
            Output.Write<UInt16>(++mSequence);
            Output.Write<Byte>(mScratch.GetData(), mScratch.GetSize());
            return true;
        }

        /// \brief Applies a message if it is this signal's, keeping the value only when it is newer than the last.
        ///
        /// \param Message The payload, exactly as the sender wrote it.
        /// \return `true` if the message was this signal's, `false` if it belongs to something else.
        Bool Receive(ConstSpan<Byte> Message)
        {
            Reader Input(Message);

            if (Input.GetAvailable() < sizeof(Opcode) + sizeof(UInt16))
            {
                return false;
            }

            // Read rather than peeked, since the opcode is as wide as the application made it.
            Opcode Code;
            Archive<Reader>(Input).Serialize(Code);

            if (Code != mOpcode)
            {
                return false;
            }

            // A datagram that arrives behind a newer one has nothing to say.
            if (const UInt16 Sequence = Input.Read<UInt16>(); static_cast<SInt16>(Sequence - mSequence) > 0)
            {
                mSequence = Sequence;
                mFresh    = true;

                Archive<Reader>(Input).Serialize(mValue);
            }
            return true;
        }

    private:

        /// \brief Checks whether two streams hold the same bytes.
        ///
        /// \param First  The first stream.
        /// \param Second The second stream.
        /// \return `true` if they hold the same bytes, `false` otherwise.
        ZY_INLINE static Bool Same(ConstRef<Writer> First, ConstRef<Writer> Second)
        {
            if (First.GetSize() != Second.GetSize())
            {
                return false;
            }

            for (UInt32 Index = 0, Limit = First.GetSize(); Index < Limit; ++Index)
            {
                if (First.GetData()[Index] != Second.GetData()[Index])
                {
                    return false;
                }
            }
            return true;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Writer mScratch;
        Writer mSaid;
        Real64 mElapsed;
        Type   mValue;
        Opcode mOpcode;
        UInt16 mSequence;
        Bool   mFresh;
    };
}