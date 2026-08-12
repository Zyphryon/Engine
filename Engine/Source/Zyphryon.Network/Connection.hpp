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
    /// \brief Represents a handle naming one endpoint, or one peer of one endpoint.
    class Connection final
    {
    public:

        /// \brief The key naming an endpoint's slot in the service.
        using Endpoint = Freelist<kMaxEndpoints>::Key;

        /// \brief The key naming a peer's slot within an endpoint.
        using Peer     = Freelist<kMaxPeers>::Key;

    public:

        /// \brief Constructs a handle that names nothing.
        ZY_INLINE constexpr Connection() = default;

        /// \brief Constructs a handle naming one endpoint.
        ///
        /// \param Endpoint The key naming the endpoint's slot.
        ZY_INLINE constexpr explicit Connection(Endpoint Endpoint)
            : mEndpoint { Endpoint }
        {
        }

        /// \brief Derives the handle naming one peer of the endpoint this handle names.
        ///
        /// \param Peer The key naming the peer's slot within this endpoint.
        /// \return The handle naming that peer of this endpoint.
        ZY_INLINE constexpr Connection Derive(Peer Peer) const
        {
            Connection Result(mEndpoint);
            Result.mPeer = Peer;
            return Result;
        }

        /// \brief Gets the handle naming the endpoint this one belongs to.
        ///
        /// \return This handle with its peer half cleared, which is the endpoint's own handle.
        ZY_INLINE constexpr Connection GetEndpoint() const
        {
            return Connection(mEndpoint);
        }

        /// \brief Gets the endpoint half of the handle.
        ///
        /// \return The key naming the endpoint's slot.
        ZY_INLINE constexpr Endpoint GetChannel() const
        {
            return mEndpoint;
        }

        /// \brief Gets the peer half of the handle.
        ///
        /// \return The key naming the peer's slot, which names nothing when the handle is the endpoint's own.
        ZY_INLINE constexpr Peer GetPeer() const
        {
            return mPeer;
        }

        /// \brief Checks whether the handle names anything.
        ///
        /// \return `true` if the handle names an endpoint or a peer, otherwise `false`.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mEndpoint.IsValid();
        }

        /// \brief Checks whether the handle names a peer rather than the endpoint itself.
        ///
        /// \return `true` if the handle names a peer, otherwise `false`.
        ZY_INLINE constexpr Bool IsPeer() const
        {
            return mPeer.IsValid();
        }

        /// \brief Compares this handle against another for equality.
        ///
        /// \param Other The handle to compare against.
        /// \return `true` if both name the same incarnation of the same peer of the same endpoint, otherwise `false`.
        ZY_INLINE constexpr Bool operator==(ConstRef<Connection> Other) const = default;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Endpoint mEndpoint;
        Peer     mPeer;
    };
}
