// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include <errno.h>
#include <linux/io_uring.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Reason Translate(SInt32 Error)
    {
        switch (Error)
        {
        case 0:
            return Reason::None;
        case ECONNREFUSED:
            return Reason::Refused;
        case ETIMEDOUT:
            return Reason::Timeout;
        case ECONNRESET:
        case ECONNABORTED:
        case EPIPE:
        case ECANCELED:
        case EINTR:
            return Reason::Closed;
        default:
            return Reason::Unreachable;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Ptr<void> Discover(SInt32 Ring, UInt Span, UInt64 Offset)
    {
        const Ptr<void> Address = mmap(nullptr, Span, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, Ring, Offset);

        if (Address == MAP_FAILED)
        {
            LOG_E("Network: a part of the ring the kernel shares could not be mapped ({0})", errno);
            return nullptr;
        }
        return Address;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Proactor::Backend final
    {
        /// The number of operations the ring makes room for, which caps how many may be out at once.
        static constexpr UInt32 kDepth  = 4096;

        /// The tag the read on the beacon is queued under, which names no record.
        static constexpr UInt64 kBeacon = ~static_cast<UInt64>(0);

        /// The tag an operation whose own completion says nothing is queued under, which names no record either.
        static constexpr UInt64 kIgnore = ~static_cast<UInt64>(1);

        /// \brief One operation as the kernel holds it, which outlives the socket it was posted on.
        struct Record final
        {
            /// The connection the operation was posted under.
            Connection       Link;

            /// What the operation was asked to do.
            Operation        Kind;

            /// The socket the operation was posted on.
            SInt32           Owner;

            /// Where the traffic came from, which an accept and a datagram read both write into.
            sockaddr_storage Address;

            /// How much of \ref Address the kernel may write, which it answers an accept back in.
            socklen_t        Length;

            /// What an operation carrying an address names its buffer through.
            msghdr           Message;

            /// The one buffer that message names.
            iovec            Buffer;

            /// Where this record sits in the arena, kept so a completion gives it back without being looked for.
            UInt32           Index;

            /// The next free record, meaningful only while this one sits on the free list.
            UInt32           Next;

            /// \brief Points the kernel's message at the given bytes and at the record's own address.
            ///
            /// \param Data The bytes the operation fills or empties.
            ZY_INLINE void Wrap(ConstSpan<Byte> Data)
            {
                Buffer.iov_base     = const_cast<Ptr<Byte>>(Data.GetData());
                Buffer.iov_len      = Data.GetSize();
                Message.msg_name    = AddressOf(Address);
                Message.msg_namelen = Length;
                Message.msg_iov     = AddressOf(Buffer);
                Message.msg_iovlen  = 1;
            }
        };

        /// \brief One of the two rings the kernel and this thread hand work across.
        struct Ring final
        {
            /// Where the mapping begins, kept so it can be given back.
            Ptr<void>   Base = nullptr;

            /// How much of it was mapped.
            UInt        Span = 0;

            /// Where whoever takes the work has reached.
            Ptr<UInt32> Head = nullptr;

            /// Where whoever hands it over has reached.
            Ptr<UInt32> Tail = nullptr;

            /// The mask every subscript is taken modulo.
            UInt32      Mask = 0;
        };

        /// The operations recorded, which outlive the sockets they were posted on.
        Registry<Record>  Arena;

        /// The ring's descriptor, which every operation is queued through.
        SInt32            Port       = -1;

        /// The descriptor a rouse is written to, which the ring carries a read on so a wait answers to the application.
        SInt32            Beacon     = -1;

        /// What a rouse is read back into, which the kernel needs somewhere lasting for.
        UInt64            Token      = 0;

        /// Where operations are handed to the kernel.
        Ring              Submission;

        /// Where the kernel hands back the ones it has finished.
        Ring              Completion;

        /// The submissions themselves, which the ring only holds subscripts into.
        Ptr<io_uring_sqe> Entries    = nullptr;

        /// How much of those submissions was mapped.
        UInt              Stride     = 0;

        /// The subscripts the kernel reads the submissions in.
        Ptr<UInt32>       Index      = nullptr;

        /// The completions themselves.
        Ptr<io_uring_cqe> Results    = nullptr;

        /// Whether the read on the beacon is still out, which is what keeps a second from being posted behind it.
        Bool              Watching   = false;

        /// How many submissions have been queued and not yet handed over.
        UInt32            Queued     = 0;

        /// What the last wait found, kept so the span handed out survives until the next one.
        Sequence<Event>   Events;

        /// \brief Takes a record and writes down everything an operation is posted with.
        ///
        /// \param Link  The connection the operation is posted under.
        /// \param Kind  The operation being asked for.
        /// \param Owner The socket the operation is posted on.
        /// \return The record, ready for the kernel to be handed.
        Ref<Record> Acquire(Connection Link, Operation Kind, SInt32 Owner)
        {
            Ref<Record> Entry = Arena.Take();
            Zero(AddressOf(Entry.Message), 1);

            Entry.Link   = Link;
            Entry.Kind   = Kind;
            Entry.Owner  = Owner;
            Entry.Length = 0;

            return Entry;
        }

        /// \brief Takes the next submission the ring has room for.
        ///
        /// \return The submission to fill in, or `nullptr` when the ring is full.
        Ptr<io_uring_sqe> Reserve()
        {
            const UInt32 Tail = __atomic_load_n(Submission.Tail, __ATOMIC_RELAXED);
            const UInt32 Head = __atomic_load_n(Submission.Head, __ATOMIC_ACQUIRE);

            if (Tail - Head > Submission.Mask)
            {
                LOG_E("Network: the ring had no room left for another operation");
                return nullptr;
            }

            const UInt32 Slot = Tail & Submission.Mask;

            Zero(AddressOf(Entries[Slot]), 1);

            Index[Slot] = Slot;

            __atomic_store_n(Submission.Tail, Tail + 1, __ATOMIC_RELEASE);

            ++Queued;
            return AddressOf(Entries[Slot]);
        }

        /// \brief Puts a read on the beacon, so that a rouse written to it brings a wait back early.
        void Watch()
        {
            if (Watching)
            {
                return;
            }

            if (const Ptr<io_uring_sqe> Slot = Reserve())
            {
                Slot->opcode    = IORING_OP_READ;
                Slot->fd        = Beacon;
                Slot->addr      = reinterpret_cast<UInt64>(AddressOf(Token));
                Slot->len       = sizeof(Token);
                Slot->user_data = kBeacon;

                Watching = true;
            }
        }

        /// \brief Hands the kernel everything queued so far.
        ///
        /// \return `true` if the kernel took them, `false` otherwise.
        Bool Submit()
        {
            while (Queued > 0)
            {
                const SInt32 Taken = syscall(__NR_io_uring_enter, Port, Queued, 0, 0, nullptr, 0);

                if (Taken <= 0)
                {
                    const UInt32 Tail = __atomic_load_n(Submission.Tail, __ATOMIC_RELAXED);

                    __atomic_store_n(Submission.Tail, Tail - Queued, __ATOMIC_RELEASE);

                    Queued = 0;
                    return false;
                }

                Queued -= static_cast<UInt32>(Taken);
            }
            return true;
        }

        /// \brief Takes the answer a post gave back, giving the record up when it never reached the kernel.
        ///
        /// \param Record  The record the operation was posted with.
        /// \param Success The answer the call itself gave.
        /// \param Action  The operation, as it is named if it has to be reported.
        /// \return `true` when the kernel took the operation, `false` otherwise.
        Bool Settle(Ref<Record> Record, Bool Success, ConstPtr<Char> Action)
        {
            if (Success)
            {
                return true;
            }

            LOG_E("Network: {0} could not be posted ({1})", Action, errno);

            Arena.Release(Record.Index);
            return false;
        }
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Proactor::Proactor()
        : mBackend { Unique<Backend>::Create() }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Proactor::~Proactor()
    {
        Stop();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Start()
    {
        io_uring_params Params;
        Zero(AddressOf(Params), 1);

        mBackend->Port = syscall(__NR_io_uring_setup, Backend::kDepth, AddressOf(Params));

        if (mBackend->Port < 0)
        {
            LOG_E("Network: the kernel would not open a ring ({0})", errno);
            return false;
        }

        // The two rings share one mapping wherever the kernel supports it, which is the usual case and
        // saves a mapping and a page of its own.
        const Bool Shared = (Params.features & IORING_FEAT_SINGLE_MMAP) != 0;

        mBackend->Submission.Span = Params.sq_off.array + Params.sq_entries * sizeof(UInt32);
        mBackend->Completion.Span = Params.cq_off.cqes  + Params.cq_entries * sizeof(io_uring_cqe);

        if (Shared)
        {
            mBackend->Submission.Span = Max(mBackend->Submission.Span, mBackend->Completion.Span);
            mBackend->Completion.Span = mBackend->Submission.Span;
        }

        mBackend->Submission.Base = Discover(mBackend->Port, mBackend->Submission.Span, IORING_OFF_SQ_RING);

        if (Shared)
        {
            mBackend->Completion.Base = mBackend->Submission.Base;
        }
        else
        {
            mBackend->Completion.Base = Discover(mBackend->Port, mBackend->Completion.Span, IORING_OFF_CQ_RING);
        }

        mBackend->Stride  = Params.sq_entries * sizeof(io_uring_sqe);
        mBackend->Entries = static_cast<Ptr<io_uring_sqe>>(Discover(mBackend->Port, mBackend->Stride, IORING_OFF_SQES));

        if (mBackend->Submission.Base == nullptr || mBackend->Completion.Base == nullptr || mBackend->Entries == nullptr)
        {
            Stop();
            return false;
        }

        const Ptr<Byte> Asked    = static_cast<Ptr<Byte>>(mBackend->Submission.Base);
        const Ptr<Byte> Answered = static_cast<Ptr<Byte>>(mBackend->Completion.Base);

        mBackend->Submission.Head = reinterpret_cast<Ptr<UInt32>>(Asked + Params.sq_off.head);
        mBackend->Submission.Tail = reinterpret_cast<Ptr<UInt32>>(Asked + Params.sq_off.tail);
        mBackend->Submission.Mask = * reinterpret_cast<Ptr<UInt32>>(Asked + Params.sq_off.ring_mask);
        mBackend->Index           = reinterpret_cast<Ptr<UInt32>>(Asked + Params.sq_off.array);

        mBackend->Completion.Head = reinterpret_cast<Ptr<UInt32>>(Answered + Params.cq_off.head);
        mBackend->Completion.Tail = reinterpret_cast<Ptr<UInt32>>(Answered + Params.cq_off.tail);
        mBackend->Completion.Mask = * reinterpret_cast<Ptr<UInt32>>(Answered + Params.cq_off.ring_mask);
        mBackend->Results         = reinterpret_cast<Ptr<io_uring_cqe>>(Answered + Params.cq_off.cqes);

        // Required rather than worked around, since a kernel that will not take the deadline on the wait itself
        // needs a timeout operation behind every pass, and none old enough to want that carries the rest of this.
        if ((Params.features & IORING_FEAT_EXT_ARG) == 0)
        {
            LOG_E("Network: this kernel will not time a wait of its own, which asks for a newer one than 5.11");

            Stop();
            return false;
        }

        mBackend->Beacon = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);

        if (mBackend->Beacon < 0)
        {
            LOG_E("Network: the descriptor a rouse is written to could not be opened ({0})", errno);

            Stop();
            return false;
        }

        mBackend->Watch();
        mBackend->Events.Reserve(kMaxHarvest);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Proactor::Stop()
    {
        ZY_ASSERT(mBackend->Arena.IsIdle(), "The ring is being closed while operations are still out on it");

        if (mBackend->Entries)
        {
            munmap(mBackend->Entries, mBackend->Stride);

            mBackend->Entries = nullptr;
        }

        if (mBackend->Completion.Base && mBackend->Completion.Base != mBackend->Submission.Base)
        {
            munmap(mBackend->Completion.Base, mBackend->Completion.Span);
        }

        if (mBackend->Submission.Base)
        {
            munmap(mBackend->Submission.Base, mBackend->Submission.Span);
        }

        mBackend->Submission = Backend::Ring();
        mBackend->Completion = Backend::Ring();

        if (mBackend->Beacon >= 0)
        {
            close(mBackend->Beacon);

            mBackend->Beacon = -1;
        }

        if (mBackend->Port >= 0)
        {
            close(mBackend->Port);

            mBackend->Port = -1;
        }

        mBackend->Arena.Clear();

        mBackend->Queued   = 0;
        mBackend->Watching = false;
        mBackend->Events.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Attach(ConstRef<Socket> Handle)
    {
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Proactor::Cancel(ConstRef<Socket> Handle)
    {
        if (!Handle.IsValid())
        {
            return;
        }

        if (const Ptr<io_uring_sqe> Slot = mBackend->Reserve())
        {
            Slot->opcode       = IORING_OP_ASYNC_CANCEL;
            Slot->fd           = static_cast<SInt32>(Handle.GetHandle());
            Slot->cancel_flags = IORING_ASYNC_CANCEL_ALL | IORING_ASYNC_CANCEL_FD;
            Slot->user_data    = Backend::kIgnore;

            mBackend->Submit();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Accept(ConstRef<Socket> Handle, Connection Link, ConstRef<Endpoint> Family)
    {
        const Ptr<io_uring_sqe> Slot = mBackend->Reserve();

        if (Slot != nullptr)
        {
            return false;
        }

        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Accept, static_cast<SInt32>(Handle.GetHandle()));
        Entry.Length = sizeof(Entry.Address);

        Slot->opcode       = IORING_OP_ACCEPT;
        Slot->fd           = Entry.Owner;
        Slot->addr         = reinterpret_cast<UInt64>(AddressOf(Entry.Address));
        Slot->off          = reinterpret_cast<UInt64>(AddressOf(Entry.Length));
        Slot->accept_flags = SOCK_NONBLOCK | SOCK_CLOEXEC;
        Slot->user_data    = Entry.Index;

        return mBackend->Settle(Entry, mBackend->Submit(), "IORING_OP_ACCEPT");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Connect(ConstRef<Socket> Handle, Connection Link, ConstRef<Endpoint> Address)
    {
        const Ptr<io_uring_sqe> Slot = mBackend->Reserve();

        if (Slot == nullptr)
        {
            return false;
        }

        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Connect, static_cast<SInt32>(Handle.GetHandle()));
        Entry.Length = static_cast<socklen_t>(Address.GetSize());

        Blit(reinterpret_cast<Ptr<Byte>>(AddressOf(Entry.Address)), Address.GetSize(), Address.GetData());

        Slot->opcode    = IORING_OP_CONNECT;
        Slot->fd        = Entry.Owner;
        Slot->addr      = reinterpret_cast<UInt64>(AddressOf(Entry.Address));
        Slot->off       = Entry.Length;
        Slot->user_data = Entry.Index;

        return mBackend->Settle(Entry, mBackend->Submit(), "IORING_OP_CONNECT");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Receive(ConstRef<Socket> Handle, Connection Link, Span<Byte> Buffer)
    {
        const Ptr<io_uring_sqe> Slot = mBackend->Reserve();

        if (Slot == nullptr)
        {
            return false;
        }

        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Receive, static_cast<SInt32>(Handle.GetHandle()));

        Slot->opcode    = IORING_OP_RECV;
        Slot->fd        = Entry.Owner;
        Slot->addr      = reinterpret_cast<UInt64>(Buffer.GetData());
        Slot->len       = static_cast<UInt32>(Buffer.GetSize());
        Slot->user_data = Entry.Index;

        return mBackend->Settle(Entry, mBackend->Submit(), "IORING_OP_RECV");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::ReceiveFrom(ConstRef<Socket> Handle, Connection Link, Span<Byte> Buffer)
    {
        const Ptr<io_uring_sqe> Slot = mBackend->Reserve();

        if (Slot == nullptr)
        {
            return false;
        }

        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Receive, static_cast<SInt32>(Handle.GetHandle()));
        Entry.Length = sizeof(Entry.Address);
        Entry.Wrap(Buffer);

        Slot->opcode    = IORING_OP_RECVMSG;
        Slot->fd        = Entry.Owner;
        Slot->addr      = reinterpret_cast<UInt64>(AddressOf(Entry.Message));
        Slot->len       = 1;
        Slot->user_data = Entry.Index;

        return mBackend->Settle(Entry, mBackend->Submit(), "IORING_OP_RECVMSG");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Send(ConstRef<Socket> Handle, Connection Link, ConstSpan<Byte> Buffer)
    {
        const Ptr<io_uring_sqe> Slot = mBackend->Reserve();

        if (Slot == nullptr)
        {
            return false;
        }

        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Send, static_cast<SInt32>(Handle.GetHandle()));

        Slot->opcode    = IORING_OP_SEND;
        Slot->fd        = Entry.Owner;
        Slot->addr      = reinterpret_cast<UInt64>(Buffer.GetData());
        Slot->len       = static_cast<UInt32>(Buffer.GetSize());
        Slot->msg_flags = MSG_NOSIGNAL;
        Slot->user_data = Entry.Index;

        return mBackend->Settle(Entry, mBackend->Submit(), "IORING_OP_SEND");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Send(ConstRef<Socket> Handle, Connection Link, ConstSpan<Byte> Buffer, ConstRef<Endpoint> Address)
    {
        const Ptr<io_uring_sqe> Slot = mBackend->Reserve();

        if (Slot == nullptr)
        {
            return false;
        }

        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Send, static_cast<SInt32>(Handle.GetHandle()));
        Entry.Length = static_cast<socklen_t>(Address.GetSize());

        Blit(reinterpret_cast<Ptr<Byte>>(AddressOf(Entry.Address)), Address.GetSize(), Address.GetData());
        Entry.Wrap(Buffer);

        Slot->opcode    = IORING_OP_SENDMSG;
        Slot->fd        = Entry.Owner;
        Slot->addr      = reinterpret_cast<UInt64>(AddressOf(Entry.Message));
        Slot->len       = 1;
        Slot->msg_flags = MSG_NOSIGNAL;
        Slot->user_data = Entry.Index;

        return mBackend->Settle(Entry, mBackend->Submit(), "IORING_OP_SENDMSG");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Proactor::Rouse()
    {
        if (mBackend->Beacon >= 0)
        {
            constexpr UInt64 One = 1;
            write(mBackend->Beacon, AddressOf(One), sizeof(One));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstSpan<Proactor::Event> Proactor::Wait(UInt32 Timeout)
    {
        mBackend->Events.Clear();

        // Reached for here only when the harvest below could find no room to replace the read it took, since every
        // other pass comes in with one already out and this asks nothing of the ring at all.
        mBackend->Watch();

        __kernel_timespec Deadline;
        Deadline.tv_sec  = Timeout / 1000;
        Deadline.tv_nsec = static_cast<long long>(Timeout % 1000) * 1000000;

        io_uring_getevents_arg Argument;
        Zero(AddressOf(Argument), 1);
        Argument.ts = reinterpret_cast<UInt64>(AddressOf(Deadline));

        syscall(__NR_io_uring_enter, mBackend->Port, mBackend->Queued, 1,
            IORING_ENTER_GETEVENTS | IORING_ENTER_EXT_ARG, AddressOf(Argument), sizeof(Argument));

        mBackend->Queued = 0;

        UInt32       Head = __atomic_load_n(mBackend->Completion.Head, __ATOMIC_RELAXED);
        const UInt32 Tail = __atomic_load_n(mBackend->Completion.Tail, __ATOMIC_ACQUIRE);

        for (UInt32 Taken = 0; Head != Tail && Taken < kMaxHarvest; ++Head, ++Taken)
        {
            ConstRef<io_uring_cqe> Finished = mBackend->Results[Head & mBackend->Completion.Mask];

            if (Finished.user_data == Backend::kBeacon)
            {
                // Replaced where it is taken, since the ring has just been drained and is at its emptiest here.
                mBackend->Watching = false;
                mBackend->Watch();

                mBackend->Events.Append().Kind = Operation::Rouse;
                continue;
            }

            // A cancel answers for nothing of its own, since whatever it gave up on comes back on its own.
            if (Finished.user_data == Backend::kIgnore)
            {
                continue;
            }

            Ref<Backend::Record> Entry   = mBackend->Arena.Get(static_cast<UInt32>(Finished.user_data));
            const Bool           Success = Finished.res >= 0;

            Ref<Event> Report = mBackend->Events.Append();
            Report.Link  = Entry.Link;
            Report.Kind  = Entry.Kind;
            Report.Cause = Translate(Success ? 0 : -Finished.res);
            Report.Size  = Success ? static_cast<UInt32>(Finished.res) : 0;

            const ConstPtr<Byte> Remote = reinterpret_cast<ConstPtr<Byte>>(AddressOf(Entry.Address));

            switch (Entry.Kind)
            {
            case Operation::Accept:
                if (Success)
                {
                    Report.Peer   = Socket(static_cast<UInt64>(Finished.res));
                    Report.Origin = Endpoint(ConstSpan(Remote, Entry.Length));
                    Report.Size   = 0;
                }
                break;
            case Operation::Receive:
                if (Success && Entry.Message.msg_namelen > 0)
                {
                    Report.Origin = Endpoint(ConstSpan(Remote, Entry.Message.msg_namelen));
                }
                break;
            default:
                break;
            }

            mBackend->Arena.Release(Entry.Index);
        }

        __atomic_store_n(mBackend->Completion.Head, Head, __ATOMIC_RELEASE);

        return mBackend->Events;
    }
}