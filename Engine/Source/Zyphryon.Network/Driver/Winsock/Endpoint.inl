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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Endpoint::Describe(Ref<Address> Output) const
    {
        if (!IsValid())
        {
            return;
        }

        Char Host[NI_MAXHOST] { };

        const ConstPtr<sockaddr> Source = GetData<sockaddr>();

        if (getnameinfo(Source, static_cast<socklen_t>(mStorage.GetSize()), Host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST) != 0)
        {
            return;
        }

        Output.Host = StrConvert(Host);

        switch (Source->sa_family)
        {
        case AF_INET:
            Output.Port = ntohs(reinterpret_cast<ConstPtr<sockaddr_in>>(Source)->sin_port);
            break;
        case AF_INET6:
            Output.Port = ntohs(reinterpret_cast<ConstPtr<sockaddr_in6>>(Source)->sin6_port);
            break;
        default:
            Output.Port = 0;
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt Endpoint::Canonicalize(Span<Byte> Output) const
    {
        ZY_ASSERT(Output.GetSize() >= kCapacity, "There is no room to write the address out in full");

        Zero(Output.GetData(), Output.GetSize());

        if (!IsValid())
        {
            return 0;
        }

        const ConstPtr<sockaddr> Source = GetData<sockaddr>();

        switch (Source->sa_family)
        {
        case AF_INET:
        {
            const ConstPtr<sockaddr_in> Value = reinterpret_cast<ConstPtr<sockaddr_in>>(Source);

            Blit(Output.GetData() + 0, sizeof(Value->sin_family), AddressOf(Value->sin_family));
            Blit(Output.GetData() + 2, sizeof(Value->sin_port),   AddressOf(Value->sin_port));
            Blit(Output.GetData() + 4, sizeof(Value->sin_addr),   AddressOf(Value->sin_addr));

            return 8;
        }
        case AF_INET6:
        {
            const ConstPtr<sockaddr_in6> Value = reinterpret_cast<ConstPtr<sockaddr_in6>>(Source);

            // The scope tells two peers apart when the address alone cannot, and the flow label is left out of
            // this and of the comparison alike, since nothing promises a peer keeps it the same between datagrams.
            Blit(Output.GetData() + 0, sizeof(Value->sin6_family),   AddressOf(Value->sin6_family));
            Blit(Output.GetData() + 2, sizeof(Value->sin6_port),     AddressOf(Value->sin6_port));
            Blit(Output.GetData() + 4, sizeof(Value->sin6_scope_id), AddressOf(Value->sin6_scope_id));
            Blit(Output.GetData() + 8, sizeof(Value->sin6_addr),     AddressOf(Value->sin6_addr));

            return 24;
        }
        default:
            return 0;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Endpoint::operator==(ConstRef<Endpoint> Other) const
    {
        if (!IsValid() || !Other.IsValid())
        {
            return false;
        }

        const ConstPtr<sockaddr> Left  = GetData<sockaddr>();
        const ConstPtr<sockaddr> Right = Other.GetData<sockaddr>();

        if (Left->sa_family != Right->sa_family)
        {
            return false;
        }

        // Compared field by field rather than as raw bytes, because the platform's own structures carry padding
        // it never promises to zero, and two views of one peer would then read as different peers.
        if (Left->sa_family == AF_INET)
        {
            const ConstPtr<sockaddr_in> First  = reinterpret_cast<ConstPtr<sockaddr_in>>(Left);
            const ConstPtr<sockaddr_in> Second = reinterpret_cast<ConstPtr<sockaddr_in>>(Right);

            return First->sin_port == Second->sin_port
                && First->sin_addr.S_un.S_addr == Second->sin_addr.S_un.S_addr;
        }

        if (Left->sa_family == AF_INET6)
        {
            const ConstPtr<sockaddr_in6> First  = reinterpret_cast<ConstPtr<sockaddr_in6>>(Left);
            const ConstPtr<sockaddr_in6> Second = reinterpret_cast<ConstPtr<sockaddr_in6>>(Right);

            // The scope tells two peers apart when the address alone cannot, which link-local ones cannot.
            return First->sin6_port == Second->sin6_port
                && First->sin6_scope_id == Second->sin6_scope_id
                && Compare(reinterpret_cast<ConstPtr<Byte>>(AddressOf(First->sin6_addr)),
                           reinterpret_cast<ConstPtr<Byte>>(AddressOf(Second->sin6_addr)), sizeof(in6_addr));
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Endpoint::Resolve(ConstRef<Address> Address, Transport Transport, Ref<Endpoint> Output)
    {
        addrinfo Hint { };
        Hint.ai_family   = AF_UNSPEC;
        Hint.ai_socktype = (Transport == Transport::TCP) ? SOCK_STREAM : SOCK_DGRAM;
        Hint.ai_protocol = (Transport == Transport::TCP) ? IPPROTO_TCP : IPPROTO_UDP;
        Hint.ai_flags    = Address.IsAny() ? AI_PASSIVE : 0;

        Str16 Service;
        Service.Format<"{0}">(Address.Port);

        Ptr<addrinfo> List = nullptr;

        const PCSTR Name = Address.IsAny() ? nullptr : Address.Host.GetData();

        if (const INT Code = getaddrinfo(Name, Service.GetData(), AddressOf(Hint), AddressOf(List)))
        {
            LOG_E("Network: '{0}' could not be resolved ({1})", Address.Host, Code);
            return false;
        }

        // The first answer is taken, which is the order the resolver itself prefers them in, so a host that
        // answers on both families is reached over whichever one the platform believes in more.
        Output = Endpoint(ConstSpan(reinterpret_cast<ConstPtr<Byte>>(List->ai_addr), List->ai_addrlen));

        freeaddrinfo(List);
        return true;
    }
}