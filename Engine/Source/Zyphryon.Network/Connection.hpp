// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Represents a handle naming one endpoint, or one peer of one endpoint.
    class Connection final
    {
    public:

        /// \brief The number of bits the handle gives to the peer half of its value.
        static constexpr UInt kPeerBits = 16;

    public:

        /// \brief Constructs a handle that names nothing.
        ZY_INLINE constexpr Connection()
            : mValue { 0 }
        {
        }

        /// \brief Constructs a handle naming one endpoint, or one peer of it.
        ///
        /// \param Channel The endpoint the handle belongs to.
        /// \param Peer    The peer's slot within it, counted from one, or zero to name the endpoint itself.
        ZY_INLINE constexpr Connection(UInt16 Channel, UInt16 Peer)
            : mValue { (static_cast<UInt32>(Channel) << kPeerBits) | Peer }
        {
        }

        /// \brief Checks whether the handle names anything.
        ///
        /// \return `true` if the handle names an endpoint or a peer, otherwise `false`.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mValue != 0;
        }

        /// \brief Gets the endpoint half of the handle.
        ///
        /// \return The endpoint the handle belongs to.
        ZY_INLINE constexpr UInt16 GetChannel() const
        {
            return mValue >> kPeerBits;
        }

        /// \brief Gets the peer half of the handle.
        ///
        /// \return The peer's slot within the endpoint, or zero when the handle names the endpoint itself.
        ZY_INLINE constexpr UInt16 GetPeer() const
        {
            return mValue & ((1u << kPeerBits) - 1);
        }

        /// \brief Compares this handle against another for equality.
        ///
        /// \param Other The handle to compare against.
        /// \return `true` if both name the same peer of the same endpoint, otherwise `false`.
        ZY_INLINE constexpr Bool operator==(ConstRef<Connection> Other) const = default;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32 mValue;
    };
}