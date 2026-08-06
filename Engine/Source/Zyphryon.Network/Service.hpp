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

#include "Executor.hpp"
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Names every endpoint the application opens, and speaks for them on the application's own thread.
    class Service final : public Engine::Subsystem
    {
    public:

        /// \brief Constructs the network service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Tells the application everything the endpoints have had to say since the last frame.
        ///
        /// \param Time The current time in seconds.
        void OnTick(Real64 Time) override;

        /// \brief Ends every endpoint and stops the thread that runs them.
        void OnTeardown() override;

        /// \brief Initializes the platform's networking stack and starts the thread the endpoints run on.
        ///
        /// \return `true` if endpoints can now be opened, `false` otherwise.
        Bool Initialize();

        /// \brief Opens an endpoint that accepts peers at an address.
        ///
        /// \param Address   The address to accept on, where naming no host accepts on every local interface.
        /// \param Transport The transport the endpoint carries.
        /// \param Listener  The handler told about the endpoint and its peers.
        /// \return The connection naming the endpoint, or an invalid connection if no room was left for it.
        Connection Listen(ConstRef<Address> Address, Transport Transport, ConstRetainer<Handler> Listener);

        /// \brief Opens an endpoint that reaches for a peer at an address.
        ///
        /// \param Address   The address to reach for.
        /// \param Transport The transport the endpoint carries.
        /// \param Listener  The handler told about the endpoint.
        /// \return The connection naming the endpoint, or an invalid connection if no room was left for it.
        Connection Connect(ConstRef<Address> Address, Transport Transport, ConstRetainer<Handler> Listener);

        /// \brief Ends a peer, or the whole endpoint when the connection names the endpoint itself.
        ///
        /// \param Link The peer to end, or the endpoint.
        void Close(Connection Link);

        /// \brief Sets how long a peer, or every peer of an endpoint, may say nothing before it is taken as gone.
        ///
        /// \note An endpoint whose peers are quiet by design is given zero, which never gives up on silence at all.
        ///
        /// \param Link    The peer to set it on, or the endpoint and every peer it holds.
        /// \param Seconds The time silence is put up with, where zero never gives up on it at all.
        void SetTimeout(Connection Link, Real64 Seconds);

        /// \brief Sends a message to a peer.
        ///
        /// \param Link     The peer to send to, or the endpoint itself.
        /// \param Delivery The guarantee the message needs.
        /// \param Message  The payload, which is copied and may be released as soon as this returns.
        void Send(Connection Link, Delivery Delivery, ConstSpan<Byte> Message);

        /// \brief Sends a message to every peer of an endpoint.
        ///
        /// \param Link     The endpoint whose peers receive the message.
        /// \param Delivery The guarantee the message needs.
        /// \param Message  The payload, which is copied once per peer.
        void Broadcast(Connection Link, Delivery Delivery, ConstSpan<Byte> Message);

    private:

        /// \brief Names an endpoint and resolves its address away from both the application's thread and the reactor's.
        ///
        /// \param Origin    The address the endpoint is opened against.
        /// \param Transport The transport the endpoint carries.
        /// \param Listener  The handler told about the endpoint and its peers.
        /// \param Server    The answer to whether the endpoint accepts peers rather than reaching for one.
        /// \return The connection naming the endpoint, or an invalid connection if no room was left for it.
        Connection Open(ConstRef<Address> Origin, Transport Transport, ConstRetainer<Handler> Listener, Bool Server);

        /// \brief Tells the application everything the reactor handed over, then drops it.
        void Dispatch();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Executor                                    mExecutor;
        Freelist<kMaxEndpoints>                     mSlots;
        Array<Retainer<Handler>, kMaxEndpoints + 1> mHandlers;
        Mailbox                                     mDrain;
    };
}