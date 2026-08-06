// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

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

        Blit(AddressOf(Output.Port), sizeof(Output.Port), GetData());

        Output.Host = Text(GetData<Char>() + sizeof(UInt16), GetSize() - sizeof(UInt16));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt Endpoint::Canonicalize(Span<Byte> Output) const
    {
        ZY_ASSERT(Output.GetSize() >= GetSize(), "There is no room to write the address out in full");

        Blit(Output.GetData(), GetSize(), GetData());

        return GetSize();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Endpoint::operator==(ConstRef<Endpoint> Other) const
    {
        return IsValid() && Other.IsValid()
            && GetSize() == Other.GetSize() && Compare(GetData(), Other.GetData(), GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Endpoint::Resolve(ConstRef<Address> Address, Transport Transport, Ref<Endpoint> Output)
    {
        if (Transport != Transport::TCP)
        {
            LOG_E("Network: the browser carries no datagrams, so '{0}' cannot be reached over UDP", Address.Host);
            return false;
        }

        if (Address.IsAny())
        {
            LOG_E("Network: the browser accepts no peers, so an address that names no host reaches nothing");
            return false;
        }

        if (Address.Host.GetSize() + sizeof(UInt16) > kCapacity)
        {
            LOG_E("Network: '{0}' is longer than an endpoint has room for", Address.Host);
            return false;
        }

        Byte Storage[kCapacity];

        Blit(Storage, sizeof(Address.Port), AddressOf(Address.Port));
        Blit(Storage + sizeof(UInt16), Address.Host.GetSize(), Address.Host.GetData());

        Output = Endpoint(ConstSpan(Storage, sizeof(UInt16) + Address.Host.GetSize()));
        return true;
    }
}