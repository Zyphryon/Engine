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

#include "Zyphryon.Base/Container/Array.hpp"
#include "Zyphryon.Base/Container/Span.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Represents a SHA-256 digest under way, and the keyed and stretched digests built on it.
    class ZY_API SHA256 final
    {
    public:

        /// \brief The number of bytes a finished digest takes.
        static constexpr UInt32 kSize  = 32;

        /// \brief The number of bytes the digest works through at a time.
        static constexpr UInt32 kBlock = 64;

        /// \brief The bytes of a finished digest.
        using Sum = Array<Byte, kSize>;

    public:

        /// \brief Constructs a digest of nothing yet.
        SHA256();

        /// \brief Adds bytes to what the digest speaks for.
        ///
        /// \param Data The bytes to add, which are read but never kept.
        void Update(ConstSpan<Byte> Data);

        /// \brief Finishes the digest, which must not be added to afterwards.
        ///
        /// \return The digest of every byte added.
        Sum Finish();

    private:

        /// \brief Works one whole block into the state.
        ///
        /// \param Block The 64 bytes to work in.
        void Compress(ConstPtr<Byte> Block);

        /// \brief Starts the two digests a key signs with, each having taken the key in already.
        ///
        /// \param Key   The key, of any length.
        /// \param Inner Receives the digest the signed bytes are added to.
        /// \param Outer Receives the digest the inner one is closed under.
        static void Prime(ConstSpan<Byte> Key, Ref<SHA256> Inner, Ref<SHA256> Outer);

        /// \brief Closes a keyed digest, finishing the inner digest and taking it in under the outer one.
        ///
        /// \param Inner The inner digest, with the signed bytes already added.
        /// \param Outer The outer digest, as \ref Prime left it.
        /// \return The keyed digest of the bytes added.
        static Sum Seal(Ref<SHA256> Inner, SHA256 Outer);

        /// \brief Rotates a word right by a count of bits.
        ///
        /// \param Value The word to rotate.
        /// \param Count The number of bits to rotate by, from 1 to 31.
        /// \return The rotated word.
        ZY_INLINE static constexpr UInt32 Revolve(UInt32 Value, UInt32 Count)
        {
            return (Value >> Count) | (Value << (32 - Count));
        }

    public:

        /// \brief Digests a run of bytes.
        ///
        /// \param Data The bytes to digest.
        /// \return The SHA-256 digest of the bytes.
        static Sum Hash(ConstSpan<Byte> Data);

        /// \brief Digests a run of bytes under a key, as HMAC-SHA-256.
        ///
        /// \param Key  The key, of any length.
        /// \param Data The bytes to digest.
        /// \return The keyed digest of the bytes.
        static Sum Sign(ConstSpan<Byte> Key, ConstSpan<Byte> Data);

        /// \brief Stretches a password into a key, as PBKDF2 with HMAC-SHA-256 and one block of output.
        ///
        /// \param Password The password, of any length.
        /// \param Salt     The salt the password is mixed with.
        /// \param Rounds   The number of times the digest is run, at least 1.
        /// \return The key the password stretches into.
        static Sum Derive(ConstSpan<Byte> Password, ConstSpan<Byte> Salt, UInt32 Rounds);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<UInt32, 8>    mState;
        Array<Byte, kBlock> mPending;
        UInt32              mHeld;
        UInt64              mLength;
    };
}