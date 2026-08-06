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

#include "Handler.hpp"
#include "Mailbox.hpp"
#include "Proactor.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Represents one endpoint the service holds open, and everything that belongs to it alone.
    class Channel : public Retainable<Channel>
    {
    public:

        /// \brief Maximum time, in seconds, a peer may say nothing before it is taken as gone.
        static constexpr Real64 kMaxSilence = 10.0;

        /// \brief Specifies the lifecycle of an endpoint.
        enum class State : UInt8
        {
            Latent,  ///< Constructed, with no socket opened for it yet.
            Pending, ///< Reaching for a peer, with the attempt not yet settled.
            Live,    ///< Open and able to carry traffic.
            Retired, ///< Finished, and waiting for the platform to give back what it still holds.
        };

    public:

        /// \brief Constructs an endpoint bound to a slot and the handler that speaks for it.
        ///
        /// \param Link     The connection naming this endpoint.
        /// \param Address  The address it was opened against.
        /// \param Listener The handler told about it and its peers.
        ZY_INLINE Channel(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener)
            : mLink     { Link },
              mState    { State::Latent },
              mCause    { Reason::Closed },
              mAwaiting { 0u },
              mTimeout  { 10.0 },
              mAddress  { Address },
              mListener { Listener }
        {
        }

        /// \brief Destroys the endpoint, closing whatever sockets it still holds.
        virtual ~Channel() = default;

        /// \brief Gets the connection naming this endpoint.
        ///
        /// \return The endpoint's connection.
        ZY_INLINE Connection GetLink() const
        {
            return mLink;
        }

        /// \brief Gets why the endpoint ended, or that it has not.
        ///
        /// \return The reason it ended, or \ref Reason::None while it has not.
        ZY_INLINE Reason GetCause() const
        {
            return (mState == State::Retired) ? mCause : Reason::None;
        }

        /// \brief Checks whether the platform still holds an operation of this endpoint's.
        ///
        /// \return `true` when the platform holds nothing of this endpoint's, otherwise `false`.
        virtual Bool IsQuiet() const
        {
            return mAwaiting == 0;
        }

        /// \brief Opens the endpoint against an address and posts whatever it starts out waiting on.
        ///
        /// \param Watcher The proactor the first operations are posted to.
        /// \param Address The resolved address to open against.
        /// \return `true` when the endpoint is carrying traffic, otherwise `false`.
        virtual Bool Open(Ref<Proactor> Watcher, ConstRef<Endpoint> Address) = 0;

        /// \brief Checks whether a peer is open and able to carry traffic.
        ///
        /// \param Link The peer to check, or this endpoint.
        /// \return `true` while it is live, otherwise `false`.
        virtual Bool Verify(Connection Link) const = 0;

        /// \brief Acts on an operation the platform has finished, and posts whatever should follow it.
        ///
        /// \param Watcher The proactor the follow-up operations are posted to.
        /// \param Time    The current time, in seconds.
        /// \param Event   The operation that finished.
        /// \param Output  Receives anything the application should be told about.
        virtual void Complete(Ref<Proactor> Watcher, Real64 Time, ConstRef<Proactor::Event> Event, Ref<Mailbox> Output) = 0;

        /// \brief Gives the endpoint a moment to act on time rather than on traffic, and to post what is waiting.
        ///
        /// \param Watcher The proactor the operations are posted to.
        /// \param Time    The current time, in seconds.
        /// \param Output  Receives anything the application should be told about.
        virtual void Tick(Ref<Proactor> Watcher, Real64 Time, Ref<Mailbox> Output) = 0;

        /// \brief Puts a message on a peer's outbound queue.
        ///
        /// \param Link     The peer to send to, or this endpoint itself.
        /// \param Delivery The guarantee the message needs.
        /// \param Message  The payload, which is copied.
        virtual void Send(Connection Link, Delivery Delivery, ConstSpan<Byte> Message) = 0;

        /// \brief Puts a message on every peer's outbound queue.
        ///
        /// \param Delivery The guarantee the message needs.
        /// \param Message  The payload, which is copied once per peer.
        virtual void Broadcast(Delivery Delivery, ConstSpan<Byte> Message) = 0;

        /// \brief Ends a peer, or the whole endpoint when the connection names the endpoint itself.
        ///
        /// \param Watcher The proactor whatever is still outstanding is given up on through.
        /// \param Link    The peer to end, or this endpoint.
        /// \param Cause   The reason it is retired.
        /// \param Output  Receives the notice for a peer that owned nothing of the platform's.
        virtual void Retire(Ref<Proactor> Watcher, Connection Link, Reason Cause, Ref<Mailbox> Output) = 0;

        /// \brief Sets how long a peer may say nothing before it is taken as gone.
        ///
        /// \param Link    The peer to set it on, or this endpoint and every peer it holds.
        /// \param Seconds The time silence is put up with, where zero never gives up on it at all.
        virtual void SetTimeout(Connection Link, Real64 Seconds)
        {
            mTimeout = Seconds;
        }

        /// \brief Reads everything worth knowing about a peer at once.
        ///
        /// \param Link    The peer to ask about, or this endpoint.
        /// \param Metrics Receives what the peer has cost, zeroed when the peer is unknown.
        /// \param Origin  Receives where the peer is, left as this endpoint's own address when it is unknown.
        virtual void Probe(Connection Link, Ref<Statistics> Metrics, Ref<Address> Origin) const = 0;

    protected:

        /// \brief Records that an operation has been handed to the platform.
        ///
        /// \param Kind The operation now outstanding.
        ZY_INLINE constexpr void Await(Operation Kind)
        {
            mAwaiting = SetBit(mAwaiting, 1u << Enum::Cast(Kind));
        }

        /// \brief Records whether an operation reached the platform at all.
        ///
        /// \param Kind    The operation that was posted.
        /// \param Success The answer to whether the platform took it.
        ZY_INLINE constexpr void Await(Operation Kind, Bool Success)
        {
            mAwaiting = SetOrClearBit(mAwaiting, 1u << Enum::Cast(Kind), Success);
        }

        /// \brief Records that an operation has come back.
        ///
        /// \param Kind The operation the platform has finished.
        ZY_INLINE constexpr void Settle(Operation Kind)
        {
            mAwaiting = ClearBit(mAwaiting, 1u << Enum::Cast(Kind));
        }

        /// \brief Checks whether one kind of operation is already out with the platform.
        ///
        /// \param Kind The operation to ask about.
        /// \return `true` while the platform still holds one, otherwise `false`.
        ZY_INLINE constexpr Bool IsAwaiting(Operation Kind) const
        {
            return HasBit(mAwaiting, 1u << Enum::Cast(Kind));
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Connection        mLink;
        State             mState;
        Reason            mCause;
        UInt8             mAwaiting;
        Real64            mTimeout;
        Address           mAddress;
        Retainer<Handler> mListener;
    };
}