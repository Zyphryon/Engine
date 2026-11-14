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

#include "Zyphryon.Base/Container/Sequence.hpp"
#include "Zyphryon.Base/Lexical/Text.hpp"
#include "Zyphryon.Base/Memory/Unique.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Represents the client side of a TLS connection, which encrypts and decrypts for a socket it does not own.
    class ZY_API TLS final
    {
    public:

        /// \brief Specifies the stage of the connection.
        enum class State : UInt8
        {
            Idle,        ///< No connection was begun.
            Handshaking, ///< The handshake is under way.
            Open,        ///< The handshake is done, and bytes are encrypted and decrypted.
            Closed,      ///< The server closed the connection.
            Failed,      ///< The handshake or a record failed, and nothing more passes.
        };

    public:

        /// \brief Constructs a connection that has not begun.
        TLS();

        /// \brief Releases the platform's handles of the connection.
        ~TLS();

        /// \brief Begins the handshake with a server, releasing any earlier connection.
        ///
        /// \param Host      The server's host name, sent to it and matched against its certificate when verified.
        /// \param Verified  `true` to refuse an untrusted certificate or one for another name, `false` to accept any.
        /// \param Authority The PEM certificates of the only authorities trusted, or empty to trust the system's.
        /// \param Reply     Receives the handshake bytes to send to the server, appended.
        /// \return `true` when the handshake began, `false` when the platform refused it.
        Bool Begin(Text Host, Bool Verified, ConstSpan<Byte> Authority, Ref<Sequence<Byte>> Reply);

        /// \brief Takes bytes received from the server, continuing the handshake or decrypting them.
        ///
        /// \param Wire  The bytes as the socket received them, in order and split anywhere.
        /// \param Plain Receives the decrypted bytes, appended.
        /// \param Reply Receives any bytes to send to the server, appended.
        /// \return `true` while the connection stands, `false` once it closed or failed.
        Bool Feed(ConstSpan<Byte> Wire, Ref<Sequence<Byte>> Plain, Ref<Sequence<Byte>> Reply);

        /// \brief Encrypts bytes to send, which only an open connection does.
        ///
        /// \param Plain The bytes to encrypt.
        /// \param Wire  Receives the encrypted bytes to send to the server, appended.
        /// \return `true` when every byte was encrypted, `false` when the connection is not open or failed.
        Bool Seal(ConstSpan<Byte> Plain, Ref<Sequence<Byte>> Wire);

        /// \brief Gets the stage of the connection.
        ///
        /// \return The stage of the connection.
        ZY_INLINE State GetState() const
        {
            return mState;
        }

    private:

        /// \brief Represents the platform's state of the connection.
        struct Backend;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Unique<Backend> mBackend;
        State           mState;
    };
}