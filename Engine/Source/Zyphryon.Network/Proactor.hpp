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

#include "Connection.hpp"
#include "Socket.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Represents the platform's completion queue, which whole operations are handed to and collected back from.
    class Proactor final
    {
    public:

        /// \brief Represents one operation the platform finished, and what it finished with.
        struct Event final
        {
            /// The connection the operation was posted under.
            Connection Link;

            /// The operation that finished.
            Operation  Kind  = Operation::Rouse;

            /// Why the operation failed, or \ref Reason::None when it did not.
            Reason     Cause = Reason::None;

            /// The number of bytes the operation moved, which is zero for anything that moves none.
            UInt32     Size  = 0;

            /// The socket a peer arrived on, left closed for anything but an accept.
            Socket     Peer;

            /// Where the traffic came from, left invalid where the platform had nothing to say about it.
            Endpoint   Origin;
        };

    public:

        /// \brief Constructs a proactor that carries nothing yet.
        Proactor();

        /// \brief Destroys the proactor, releasing the platform's queue and every operation still recorded.
        ~Proactor();

        /// \brief Opens the platform's completion queue.
        ///
        /// \return `true` if operations can now be posted, `false` otherwise.
        Bool Start();

        /// \brief Closes the platform's completion queue.
        ///
        /// \note Every operation must have come back before this is called.
        void Stop();

        /// \brief Puts a socket under the completion queue, which every later operation on it answers to.
        ///
        /// \param Handle The socket to take on.
        /// \return `true` if the socket was taken on, `false` otherwise.
        Bool Attach(ConstRef<Socket> Handle);

        /// \brief Asks the platform to give up on whatever a socket still has outstanding.
        ///
        /// \param Handle The socket to give up on.
        void Cancel(ConstRef<Socket> Handle);

        /// \brief Posts an operation that takes on the next peer reaching for a socket that accepts.
        ///
        /// \param Handle The socket that accepts.
        /// \param Link   The connection the finished operation is reported under.
        /// \param Family The address whose family the socket taken on will match.
        /// \return `true` if the operation was posted, `false` otherwise.
        Bool Accept(ConstRef<Socket> Handle, Connection Link, ConstRef<Endpoint> Family);

        /// \brief Posts an operation that reaches for a peer.
        ///
        /// \param Handle  The socket to reach with.
        /// \param Link    The connection the finished operation is reported under.
        /// \param Address The address of the peer to reach for.
        /// \return `true` if the operation was posted, `false` otherwise.
        Bool Connect(ConstRef<Socket> Handle, Connection Link, ConstRef<Endpoint> Address);

        /// \brief Posts an operation that fills a buffer from a connected socket.
        ///
        /// \param Handle The socket to read from.
        /// \param Link   The connection the finished operation is reported under.
        /// \param Buffer The buffer to fill, which stays the platform's until the operation comes back.
        /// \return `true` if the operation was posted, `false` otherwise.
        Bool Receive(ConstRef<Socket> Handle, Connection Link, Span<Byte> Buffer);

        /// \brief Posts an operation that fills a buffer from whatever reaches a socket, and says where it came from.
        ///
        /// \param Handle The socket to read from.
        /// \param Link   The connection the finished operation is reported under.
        /// \param Buffer The buffer to fill, which stays the platform's until the operation comes back.
        /// \return `true` if the operation was posted, `false` otherwise.
        Bool ReceiveFrom(ConstRef<Socket> Handle, Connection Link, Span<Byte> Buffer);

        /// \brief Posts an operation that empties a buffer onto a connected socket.
        ///
        /// \param Handle The socket to write to.
        /// \param Link   The connection the finished operation is reported under.
        /// \param Buffer The buffer to write, which stays the platform's until the operation comes back.
        /// \return `true` if the operation was posted, `false` otherwise.
        Bool Send(ConstRef<Socket> Handle, Connection Link, ConstSpan<Byte> Buffer);

        /// \brief Posts an operation that writes a buffer to an address as one datagram.
        ///
        /// \param Handle  The socket to write to.
        /// \param Link    The connection the finished operation is reported under.
        /// \param Buffer  The buffer to write, which stays the platform's until the operation comes back.
        /// \param Address The address to write to.
        /// \return `true` if the operation was posted, `false` otherwise.
        Bool Send(ConstRef<Socket> Handle, Connection Link, ConstSpan<Byte> Buffer, ConstRef<Endpoint> Address);

        /// \brief Ends whatever wait is in progress, from any thread.
        void Rouse();

        /// \brief Waits until at least one operation finishes or the timeout elapses.
        ///
        /// \param Timeout The maximum time to wait, in milliseconds.
        /// \return One event per finished operation, empty when nothing finished.
        ConstSpan<Event> Wait(UInt32 Timeout);

    private:

        /// \brief Represents where the operations handed to the platform are written down until it gives them back.
        template<typename Type>
        class Registry final
        {
        public:

            /// \brief Number of records one block holds, which is how many more are made room for at a time.
            static constexpr UInt32 kBlock  = 256;

            /// \brief Value a record's link takes when nothing follows it on the free list.
            static constexpr UInt32 kVacant = ~static_cast<UInt32>(0);

        public:

            /// \brief Checks whether the platform has given back everything it was ever handed.
            ///
            /// \return `true` when nothing is outstanding, `false` otherwise.
            ZY_INLINE Bool IsIdle() const
            {
                return mBusy == 0;
            }

            /// \brief Gets a record by where it sits.
            ///
            /// \param Index The place the record sits.
            /// \return The record.
            ZY_INLINE Ref<Type> Get(UInt32 Index)
            {
                return (* mPool[Index / kBlock])[Index % kBlock];
            }

            /// \brief Takes a record off the free list, making room for another block when it holds none.
            ///
            /// \return The record, with only its place written down and everything else left to the caller.
            ZY_INLINE Ref<Type> Take()
            {
                if (mFree == kVacant)
                {
                    const UInt32 Base = static_cast<UInt32>(mPool.GetSize() * kBlock);

                    mPool.Append(Unique<Array<Type, kBlock>>::Create());

                    for (UInt32 Offset = kBlock; Offset > 0; --Offset)
                    {
                        Ref<Type> Entry = Get(Base + Offset - 1);
                        Entry.Index     = Base + Offset - 1;
                        Entry.Next      = mFree;
                        mFree           = Entry.Index;
                    }
                }

                Ref<Type> Entry = Get(mFree);
                mFree = Entry.Next;

                ++mBusy;
                return Entry;
            }

            /// \brief Puts a record back on the free list, for whatever is posted next to take.
            ///
            /// \param Index The place the record to give back sits.
            ZY_INLINE void Release(UInt32 Index)
            {
                Get(Index).Next = mFree;
                mFree           = Index;

                --mBusy;
            }

            /// \brief Drops every block, which only what the platform has finished with may be among.
            ZY_INLINE void Clear()
            {
                mPool.Clear();

                mFree = kVacant;
                mBusy = 0;
            }

        private:

            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

            Sequence<Unique<Array<Type, kBlock>>> mPool;
            UInt32                                mFree = kVacant;
            UInt32                                mBusy = 0;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        struct Backend;
        Unique<Backend> mBackend;
    };
}