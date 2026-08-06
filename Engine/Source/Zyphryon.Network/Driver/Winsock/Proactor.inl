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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Reason Translate(DWORD Error)
    {
        switch (Error)
        {
        case ERROR_SUCCESS:
            return Reason::None;
        case WSAECONNREFUSED:
            return Reason::Refused;
        case WSAETIMEDOUT:
            return Reason::Timeout;
        case WSAECONNRESET:
        case WSAECONNABORTED:
        case WSAENETRESET:
        case WSAESHUTDOWN:
        case ERROR_OPERATION_ABORTED:
        case ERROR_NETNAME_DELETED:
            return Reason::Closed;
        default:
            return Reason::Unreachable;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Ptr<void> Discover(SOCKET Handle, GUID Name)
    {
        Ptr<void> Address = nullptr;
        DWORD     Written = 0;

        const INT Result = WSAIoctl(
            Handle,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
            AddressOf(Name),
            sizeof(Name),
            AddressOf(Address),
            sizeof(Address),
            AddressOf(Written),
            nullptr,
            nullptr);

        if (Result != 0)
        {
            LOG_E("Network: an entry point the platform only names at runtime was not there ({0})", WSAGetLastError());
            return nullptr;
        }
        return Address;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Proactor::Backend final
    {
        /// \brief One operation as the platform holds it, which outlives the socket it was posted on.
        struct Record final
        {
            /// What the platform threads its completion through, which must lead the record for the cast back.
            OVERLAPPED       Header;

            /// The connection the operation was posted under.
            Connection       Link;

            /// What the operation was asked to do.
            Operation        Kind;

            /// The socket the operation was posted on, which is what the platform is asked for its result.
            SOCKET           Owner;

            /// The socket an accept was given to fill, left closed for anything else.
            SOCKET           Peer;

            /// The buffer the operation fills or empties, as the platform names one.
            WSABUF           Buffer;

            /// Where the traffic came from, two of them because an accept answers with both ends of what it took on.
            sockaddr_storage Address[2];

            /// How much of \ref Address the platform may write, which it answers a datagram read back in.
            INT              Length;

            /// Where this record sits in the arena, kept so a completion gives it back without being looked for.
            UInt32           Index;

            /// The next free record, meaningful only while this one sits on the free list.
            UInt32           Next;

            /// \brief Points the platform's buffer at the given bytes.
            ///
            /// \param Data The bytes the operation fills or empties.
            ZY_INLINE void Wrap(ConstSpan<Byte> Data)
            {
                Buffer.buf = const_cast<Ptr<Char>>(reinterpret_cast<ConstPtr<Char>>(Data.GetData()));
                Buffer.len = static_cast<ULONG>(Data.GetSize());
            }
        };

        /// The operations recorded, which outlive the sockets they were posted on.
        Registry<Record>          Arena;

        /// The platform's completion queue, which every attached socket answers to.
        HANDLE                    Port = nullptr;

        /// What the last wait found, kept so the span handed out survives until the next one.
        Sequence<Event>           Events;

        /// The entry point an accept is posted through, which the platform only names at runtime.
        LPFN_ACCEPTEX             OnAccept  = nullptr;

        /// The entry point an outbound attempt is posted through, likewise only named at runtime.
        LPFN_CONNECTEX            OnConnect = nullptr;

        /// The entry point that pulls the two addresses back out of what an accept wrote.
        LPFN_GETACCEPTEXSOCKADDRS OnUnpack  = nullptr;

        /// \brief Takes a record and writes down everything an operation is posted with.
        ///
        /// \param Link  The connection the operation is posted under.
        /// \param Kind  The operation being asked for.
        /// \param Owner The socket the operation is posted on.
        /// \return The record, ready for the platform to be handed.
        Ref<Record> Acquire(Connection Link, Operation Kind, SOCKET Owner)
        {
            Ref<Record> Entry = Arena.Take();
            Zero(AddressOf(Entry.Header), 1);

            Entry.Link   = Link;
            Entry.Kind   = Kind;
            Entry.Owner  = Owner;
            Entry.Peer   = INVALID_SOCKET;
            Entry.Length = 0;

            Entry.Address[0].ss_family = AF_UNSPEC;

            return Entry;
        }

        /// \brief Takes the answer a post gave back, giving the record up when it never reached the platform.
        ///
        /// \param Record  The record the operation was posted with.
        /// \param Success The answer the call itself gave.
        /// \param Action  The operation, as it is named if it has to be reported.
        /// \return `true` when the platform took the operation, `false` otherwise.
        Bool Settle(Ref<Record> Record, Bool Success, ConstPtr<Char> Action)
        {
            if (Success)
            {
                return true;
            }

            const DWORD Error = WSAGetLastError();

            if (Error == WSA_IO_PENDING)
            {
                return true;
            }

            LOG_E("Network: {0} could not be posted ({1})", Action, Error);

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
        mBackend->Port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);

        if (mBackend->Port == nullptr)
        {
            LOG_E("Network: the platform's completion queue could not be opened ({0})", GetLastError());
            return false;
        }

        // Opened only to be asked, since the platform names these entry points through a socket rather than through
        // a library, and any socket of the provider that serves them answers for all of them.
        const SOCKET Oracle = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

        if (Oracle == INVALID_SOCKET)
        {
            LOG_E("Network: a socket to ask the platform for its entry points could not be opened ({0})", WSAGetLastError());

            Stop();
            return false;
        }

        mBackend->OnAccept  = reinterpret_cast<LPFN_ACCEPTEX>(Discover(Oracle, WSAID_ACCEPTEX));
        mBackend->OnConnect = reinterpret_cast<LPFN_CONNECTEX>(Discover(Oracle, WSAID_CONNECTEX));
        mBackend->OnUnpack  = reinterpret_cast<LPFN_GETACCEPTEXSOCKADDRS>(Discover(Oracle, WSAID_GETACCEPTEXSOCKADDRS));

        closesocket(Oracle);

        if (mBackend->OnAccept == nullptr || mBackend->OnConnect == nullptr || mBackend->OnUnpack == nullptr)
        {
            Stop();
            return false;
        }

        mBackend->Events.Reserve(kMaxHarvest);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Proactor::Stop()
    {
        ZY_ASSERT(mBackend->Arena.IsIdle(), "The completion queue is being closed while operations are still out on it");

        if (mBackend->Port)
        {
            CloseHandle(mBackend->Port);

            mBackend->Port = nullptr;
        }

        mBackend->Arena.Clear();

        mBackend->OnAccept  = nullptr;
        mBackend->OnConnect = nullptr;
        mBackend->OnUnpack  = nullptr;
        mBackend->Events.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Attach(ConstRef<Socket> Handle)
    {
        const HANDLE File = reinterpret_cast<HANDLE>(Handle.GetHandle());

        if (CreateIoCompletionPort(File, mBackend->Port, Handle.GetHandle(), 0) == nullptr)
        {
            LOG_E("Network: a socket could not be put under the completion queue ({0})", GetLastError());
            return false;
        }

        SetFileCompletionNotificationModes(File, FILE_SKIP_SET_EVENT_ON_HANDLE);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Proactor::Cancel(ConstRef<Socket> Handle)
    {
        if (Handle.IsValid())
        {
            CancelIoEx(reinterpret_cast<HANDLE>(Handle.GetHandle()), nullptr);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Accept(ConstRef<Socket> Handle, Connection Link, ConstRef<Endpoint> Family)
    {
        const SOCKET Taken = WSASocketW(
            Family.GetData<sockaddr>()->sa_family, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

        if (Taken == INVALID_SOCKET)
        {
            LOG_E("Network: a socket for the next peer could not be opened ({0})", WSAGetLastError());
            return false;
        }

        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Accept, Handle.GetHandle());
        Entry.Peer = Taken;

        DWORD Written = 0;

        const Bool Result = mBackend->OnAccept(
            Handle.GetHandle(),
            Taken,
            Entry.Address,
            0,
            sizeof(Entry.Address[0]),
            sizeof(Entry.Address[0]),
            AddressOf(Written),
            AddressOf(Entry.Header));

        if (!mBackend->Settle(Entry, Result, "WSAAccept"))
        {
            closesocket(Taken);
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Connect(ConstRef<Socket> Handle, Connection Link, ConstRef<Endpoint> Address)
    {
        sockaddr_storage Local { };
        Local.ss_family = Address.GetData<sockaddr>()->sa_family;

        const UInt Size = (Local.ss_family == AF_INET6) ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);

        if (bind(Handle.GetHandle(), reinterpret_cast<ConstPtr<sockaddr>>(AddressOf(Local)), Size) != 0)
        {
            LOG_E("Network: a socket that reaches could not be given a local address ({0})", WSAGetLastError());
            return false;
        }

        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Connect, Handle.GetHandle());

        const Bool Result = mBackend->OnConnect(
            Handle.GetHandle(),
            Address.GetData<sockaddr>(),
            static_cast<SInt32>(Address.GetSize()),
            nullptr,
            0,
            nullptr,
            AddressOf(Entry.Header));
        return mBackend->Settle(Entry, Result, "WSAConnect");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Receive(ConstRef<Socket> Handle, Connection Link, Span<Byte> Buffer)
    {
        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Receive, Handle.GetHandle());
        Entry.Wrap(Buffer);

        DWORD Flags = 0;
        DWORD Read  = 0;

        const INT Result = WSARecv(
            Entry.Owner,
            AddressOf(Entry.Buffer),
            1,
            AddressOf(Read),
            AddressOf(Flags),
            AddressOf(Entry.Header),
            nullptr);
        return mBackend->Settle(Entry, Result == 0, "WSARecv");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::ReceiveFrom(ConstRef<Socket> Handle, Connection Link, Span<Byte> Buffer)
    {
        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Receive, Handle.GetHandle());
        Entry.Wrap(Buffer);

        Entry.Length = sizeof(Entry.Address[0]);

        DWORD Flags = 0;
        DWORD Read  = 0;

        const INT Result = WSARecvFrom(
            Entry.Owner,
            AddressOf(Entry.Buffer),
            1,
            AddressOf(Read),
            AddressOf(Flags),
            reinterpret_cast<Ptr<sockaddr>>(AddressOf(Entry.Address[0])),
            AddressOf(Entry.Length),
            AddressOf(Entry.Header),
            nullptr);
        return mBackend->Settle(Entry, Result == 0, "WSARecvFrom");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Send(ConstRef<Socket> Handle, Connection Link, ConstSpan<Byte> Buffer)
    {
        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Send, Handle.GetHandle());
        Entry.Wrap(Buffer);

        DWORD Written = 0;

        const INT Result = WSASend(
            Entry.Owner,
            AddressOf(Entry.Buffer),
            1,
            AddressOf(Written),
            0,
            AddressOf(Entry.Header),
            nullptr);
        return mBackend->Settle(Entry, Result == 0, "WSASend");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Send(ConstRef<Socket> Handle, Connection Link, ConstSpan<Byte> Buffer, ConstRef<Endpoint> Address)
    {
        Ref<Backend::Record> Entry = mBackend->Acquire(Link, Operation::Send, Handle.GetHandle());
        Entry.Wrap(Buffer);

        Blit(AddressOf(Entry.Address[0]), Address.GetSize(), Address.GetData());
        Entry.Length = static_cast<INT>(Address.GetSize());

        DWORD Written = 0;

        const INT Result = WSASendTo(
            Entry.Owner,
            AddressOf(Entry.Buffer),
            1,
            AddressOf(Written),
            0,
            reinterpret_cast<ConstPtr<sockaddr>>(AddressOf(Entry.Address[0])),
            Entry.Length,
            AddressOf(Entry.Header),
            nullptr);
        return mBackend->Settle(Entry, Result == 0, "WSASendTo");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Proactor::Rouse()
    {
        if (mBackend->Port)
        {
            PostQueuedCompletionStatus(mBackend->Port, 0, ~static_cast<ULONG_PTR>(0), nullptr);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstSpan<Proactor::Event> Proactor::Wait(UInt32 Timeout)
    {
        OVERLAPPED_ENTRY Harvest[kMaxHarvest];
        ULONG            Count = 0;

        mBackend->Events.Clear();

        if (!GetQueuedCompletionStatusEx(mBackend->Port, Harvest, kMaxHarvest, AddressOf(Count), Timeout, FALSE))
        {
            if (const DWORD Error = GetLastError(); Error != WAIT_TIMEOUT)
            {
                LOG_E("Network: waiting on the completion queue failed ({0})", Error);
            }
            return mBackend->Events;
        }

        for (ULONG Index = 0; Index < Count; ++Index)
        {
            ConstRef<OVERLAPPED_ENTRY> Finished = Harvest[Index];

            // A rouse carries no operation at all, so it is the one completion that names no record.
            if (Finished.lpOverlapped == nullptr)
            {
                mBackend->Events.Append().Kind = Operation::Rouse;
                continue;
            }

            const Ptr<Backend::Record> Entry = reinterpret_cast<Ptr<Backend::Record>>(Finished.lpOverlapped);

            // Read off the operation the platform has already written its answer into.
            const Bool Success = (Entry->Header.Internal == 0);

            Ref<Event> Report = mBackend->Events.Append();
            Report.Link  = Entry->Link;
            Report.Kind  = Entry->Kind;

            if (Success)
            {
                Report.Cause = Reason::None;
                Report.Size  = static_cast<UInt32>(Finished.dwNumberOfBytesTransferred);
            }
            else
            {
                DWORD Transfer = 0;
                DWORD Flags    = 0;

                const Bool Result = WSAGetOverlappedResult(
                    Entry->Owner,
                    AddressOf(Entry->Header),
                    AddressOf(Transfer),
                    FALSE,
                    AddressOf(Flags));

                Report.Cause = Result ? Reason::None : Translate(WSAGetLastError());
                Report.Size  = 0;
            }

            switch (Entry->Kind)
            {
            case Operation::Accept:
                if (Success)
                {
                    const ConstPtr<Char> Context = reinterpret_cast<ConstPtr<Char>>(AddressOf(Entry->Owner));
                    setsockopt(Entry->Peer, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, Context, sizeof(Entry->Owner));

                    Ptr<sockaddr> Local  = nullptr;
                    Ptr<sockaddr> Remote = nullptr;
                    INT           Width  = 0;
                    INT           Length = 0;

                    mBackend->OnUnpack(
                        Entry->Address,
                        0,
                        sizeof(Entry->Address[0]),
                        sizeof(Entry->Address[0]),
                        AddressOf(Local),
                        AddressOf(Width),
                        AddressOf(Remote),
                        AddressOf(Length));

                    Report.Origin = Endpoint(ConstSpan(reinterpret_cast<ConstPtr<Byte>>(Remote), Length));
                    Report.Peer   = Socket(Entry->Peer);
                }
                else
                {
                    closesocket(Entry->Peer);
                }
                break;
            case Operation::Connect:
                if (Success)
                {
                    setsockopt(Entry->Owner, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0);
                }
                break;
            case Operation::Receive:
                if (Success)
                {
                    if (const UInt16 Family = Entry->Address[0].ss_family)
                    {
                        const Ptr<sockaddr_storage> Remote = AddressOf(Entry->Address[0]);

                        const UInt Size = (Family == AF_INET6) ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);

                        Report.Origin = Endpoint(ConstSpan(reinterpret_cast<ConstPtr<Byte>>(Remote), Size));
                    }
                }
                break;
            default:
                break;
            }

            mBackend->Arena.Release(Entry->Index);
        }
        return mBackend->Events;
    }
}