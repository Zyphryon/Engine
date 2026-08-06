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
    /// \brief Represents an address resolved into the form the platform's sockets accept.
    class ZY_ALIGN(8) Endpoint final
    {
    public:

#if   defined(ZY_PLATFORM_WEB)
        /// \brief Maximum size, in bytes, of the URL address the platform accepts.
        static constexpr UInt32 kCapacity = 256;
#else
        /// \brief Maximum size, in bytes, of the widest address the platform accepts.
        static constexpr UInt32 kCapacity = 32;
#endif

    public:

        /// \brief Constructs an endpoint that holds no address yet.
        ZY_INLINE Endpoint() = default;

        /// \brief Constructs an endpoint around an address the platform has already written.
        ///
        /// \param Data The address, as the bytes a socket handed back, which may be empty.
        ZY_INLINE Endpoint(ConstSpan<Byte> Data)
        {
            ZY_ASSERT(Data.GetSize() <= kCapacity, "The address is wider than any supported family occupies");

            mStorage.Append(Data);
        }

        /// \brief Checks whether the endpoint holds a resolved address.
        ///
        /// \return `true` if the endpoint was resolved, `false` otherwise.
        ZY_INLINE Bool IsValid() const
        {
            return !mStorage.IsEmpty();
        }

        /// \brief Gets a pointer to the underlying address storage.
        ///
        /// \return A pointer to the first byte of the address.
        template<typename Type = Byte>
        ZY_INLINE ConstPtr<Type> GetData() const
        {
            return reinterpret_cast<ConstPtr<Type>>(mStorage.GetData());
        }

        /// \brief Gets the portion of the storage the address occupies.
        ///
        /// \return The size of the address, in bytes.
        ZY_INLINE UInt GetSize() const
        {
            return mStorage.GetSize();
        }

        /// \brief Converts the endpoint back into a host and port.
        ///
        /// \param Output Receives the host and port, left unchanged if the endpoint cannot be converted.
        void Describe(Ref<Address> Output) const;

        /// \brief Writes the address out in the one form this build weighs it by.
        ///
        /// \param Output Receives the address, which must have room for \ref kCapacity bytes.
        /// \return The number of bytes written, or zero where the address is of no family this build knows.
        UInt Canonicalize(Span<Byte> Output) const;

        /// \brief Computes a 64-bit hash of the address the endpoint refers to.
        ///
        /// \return A 64-bit hash of the endpoint.
        ZY_INLINE UInt64 Hash() const
        {
            Byte       Digest[kCapacity];
            const UInt Length = Canonicalize(Span(Digest, sizeof(Digest)));

            return Length ? ::Hash(reinterpret_cast<ConstPtr<Char>>(Digest), Length) : 0;
        }

        /// \brief Compares two endpoints for equality.
        ///
        /// \param Other The endpoint to compare against.
        /// \return `true` if both refer to the same address and port, `false` otherwise.
        Bool operator==(ConstRef<Endpoint> Other) const;

    public:

        /// \brief Resolves a host and port into the form the platform's sockets accept.
        ///
        /// \param Address   The address to resolve, where naming no host resolves to every local interface.
        /// \param Transport The transport the address will be used with.
        /// \param Output    Receives the resolved address, left invalid if the host could not be resolved.
        /// \return `true` if the address was resolved, `false` otherwise.
        static Bool Resolve(ConstRef<Address> Address, Transport Transport, Ref<Endpoint> Output);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Byte, kCapacity> mStorage;
    };
}