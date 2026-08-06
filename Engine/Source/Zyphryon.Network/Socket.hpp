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

#include "Endpoint.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Represents a non-blocking socket provided by the platform's networking stack.
    class Socket final
    {
    public:

        /// \brief The value a descriptor takes when it refers to no socket.
        static constexpr UInt64 kInvalid = ~static_cast<UInt64>(0);

    public:

        /// \brief Constructs a closed socket.
        ZY_INLINE Socket()
            : mHandle { kInvalid }
        {
        }

        /// \brief Constructs a socket around a descriptor the platform has already opened.
        ///
        /// \param Handle The descriptor to wrap, which the socket refers to but does not close on its own.
        ZY_INLINE explicit Socket(UInt64 Handle)
            : mHandle { Handle }
        {
        }

        /// \brief Checks whether the socket refers to anything.
        ///
        /// \return `true` if the socket is open, `false` otherwise.
        ZY_INLINE Bool IsValid() const
        {
            return mHandle != kInvalid;
        }

        /// \brief Opens a socket of the given transport, ready to be bound or associated.
        ///
        /// \param Transport The transport the socket will carry.
        /// \param Family    The address whose family the socket will match.
        /// \return `true` if the socket was opened, `false` otherwise.
        Bool Open(Transport Transport, ConstRef<Endpoint> Family);

        /// \brief Closes the socket, releasing it back to the platform.
        void Close();

        /// \brief Gets the platform's descriptor for the socket.
        ///
        /// \return The descriptor, or \ref kInvalid if the socket is closed.
        ZY_INLINE UInt64 GetHandle() const
        {
            return mHandle;
        }

        /// \brief Binds the socket to a local address.
        ///
        /// \param Address The address to bind to.
        /// \return `true` if the socket was bound, `false` otherwise.
        Bool Bind(ConstRef<Endpoint> Address);

        /// \brief Places a stream socket into the state where it can accept peers.
        ///
        /// \return `true` if the socket is accepting, `false` otherwise.
        Bool Listen();

        /// \brief Associates a datagram socket with one peer, so that everything else it hears is turned away.
        ///
        /// \param Address The address of the peer to associate with.
        /// \return `true` if the socket was associated, `false` otherwise.
        Bool Associate(ConstRef<Endpoint> Address);

        /// \brief Gets the local address the socket is bound to.
        ///
        /// \param Output Receives the local address, left unchanged if the socket is closed or unbound.
        /// \return `true` if the address was read, `false` otherwise.
        Bool Describe(Ref<Endpoint> Output) const;

    public:

        /// \brief Initializes the platform's networking stack, which some platforms require before any socket.
        ///
        /// \return `true` if sockets can now be opened, `false` otherwise.
        static Bool Initialize();

        /// \brief Releases the platform's networking stack.
        static void Teardown();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64 mHandle;
    };
}