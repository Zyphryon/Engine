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

    static Bool Prepare(SOCKET Handle, Transport Transport)
    {
        ULONG Pending = 1;

        if (ioctlsocket(Handle, FIONBIO, AddressOf(Pending)) != 0)
        {
            LOG_E("Network: a socket could not be made non-blocking ({0})", WSAGetLastError());

            return false;
        }

        if (Transport == Transport::TCP)
        {
            BOOL Immediate = TRUE;

            setsockopt(Handle, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<ConstPtr<Char>>(AddressOf(Immediate)), sizeof(Immediate));
        }
        else
        {
            // A datagram socket that hears an ICMP refusal would otherwise fail every later read with
            // `WSAECONNRESET`, which on a server means one peer going quiet takes the whole endpoint down.
            BOOL  Reset   = FALSE;
            DWORD Ignored = 0;

            WSAIoctl(Handle, SIO_UDP_CONNRESET, AddressOf(Reset), sizeof(Reset), nullptr, 0, AddressOf(Ignored), nullptr, nullptr);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Socket::Open(Transport Transport, ConstRef<Endpoint> Family)
    {
        Close();

        if (!Family.IsValid())
        {
            LOG_E("Network: a socket cannot be opened against an address that did not resolve");
            return false;
        }

        const SOCKET Handle = WSASocketW(Family.GetData<sockaddr>()->sa_family,
            (Transport == Transport::TCP) ? SOCK_STREAM : SOCK_DGRAM,
            (Transport == Transport::TCP) ? IPPROTO_TCP : IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);

        if (Handle == INVALID_SOCKET)
        {
            LOG_E("Network: a socket could not be opened ({0})", WSAGetLastError());
            return false;
        }

        if (!Prepare(Handle, Transport))
        {
            closesocket(Handle);
            return false;
        }

        mHandle = Handle;
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Socket::Close()
    {
        if (IsValid())
        {
            closesocket(mHandle);

            mHandle = kInvalid;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Socket::Bind(ConstRef<Endpoint> Address)
    {
        if (bind(mHandle, Address.GetData<sockaddr>(), static_cast<SInt32>(Address.GetSize())) != 0)
        {
            LOG_E("Network: a socket could not be bound ({0})", WSAGetLastError());
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Socket::Listen()
    {
        if (listen(mHandle, SOMAXCONN) != 0)
        {
            LOG_E("Network: a socket could not begin accepting ({0})", WSAGetLastError());
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Socket::Associate(ConstRef<Endpoint> Address)
    {
        if (connect(mHandle, Address.GetData<sockaddr>(), static_cast<SInt32>(Address.GetSize())) != 0)
        {
            LOG_E("Network: a socket could not be pointed at its peer ({0})", WSAGetLastError());
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Socket::Describe(Ref<Endpoint> Output) const
    {
        Byte   Storage[Endpoint::kCapacity];
        SInt32 Length = sizeof(Storage);

        if (getsockname(mHandle, reinterpret_cast<Ptr<sockaddr>>(Storage), AddressOf(Length)) != 0)
        {
            return false;
        }

        Output = Endpoint(ConstSpan(Storage, static_cast<UInt>(Length)));
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Socket::Initialize()
    {
        WSADATA Data;

        if (const SInt32 Code = WSAStartup(MAKEWORD(2, 2), AddressOf(Data)))
        {
            LOG_E("Network: the platform's networking stack could not be started ({0})", Code);
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Socket::Teardown()
    {
        WSACleanup();
    }
}