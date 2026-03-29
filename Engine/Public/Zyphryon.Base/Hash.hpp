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

#include "Concept.hpp"
#include "Text.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Concept for types that implement a custom Hash method.
    template<typename Type>
    concept IsHashable = requires (ConstRef<Type> Value, UInt64 Seed)
    {
        { Value.Hash(Seed) } -> IsCastable<UInt64>;
    };

    /// \brief Computes a hash for the given data using the rapid-hash algorithm.
    ///
    /// \param Data A pointer to the data to be hashed.
    /// \param Size The size of the data in bytes.
    /// \param Seed An optional seed value to initialize the hash computation.
    /// \return The computed hash value as a 64-bit unsigned integer.
    UInt64 Hash(ConstPtr<Byte> Data, UInt32 Size, UInt64 Seed = 0);

    /// \brief Computes a hash for the specified type using the rapid-hash algorithm.
    ///
    /// \param Data A reference to the instance of the type to be hashed.
    /// \param Seed An optional seed value to initialize the hash computation.
    /// \return The computed hash value as a 64-bit unsigned integer.
    template<typename Type>
    static UInt64 Hash(ConstRef<Type> Data, UInt64 Seed = 0)
    {
        if constexpr (IsIntegral<Type> || IsEnum<Type>)
        {
            return static_cast<UInt64>(Data) + Seed;
        }
        else if constexpr (IsHashable<Type>)
        {
            return Data.Hash(Seed);
        }
        else
        {
            static_assert(IsTrivial<Type>, "Hash requires trivially copyable type");

            return Hash(reinterpret_cast<ConstPtr<Byte>>(& Data), sizeof(Type), Seed);
        }
    }

    /// \brief Computes a hash for the specified UTF-8 string using the rapid-hash algorithm.
    ///
    /// \param Data A constant string view containing the UTF-8 data to be hashed.
    /// \param Seed An optional seed value to initialize the hash computation.
    /// \return The computed hash value as a 64-bit unsigned integer.
    static UInt64 Hash(ConstRef<Str8> Data, UInt64 Seed = 0)
    {
        return Hash(reinterpret_cast<ConstPtr<Byte>>(Data.data()), Data.size(), Seed);
    }

    /// \brief Computes a hash for the specified UTF-8 string using the rapid-hash algorithm.
    ///
    /// \param Data A constant string view containing the UTF-8 data to be hashed.
    /// \param Seed An optional seed value to initialize the hash computation.
    /// \return The computed hash value as a 64-bit unsigned integer.
    static UInt64 Hash(ConstStr8 Data, UInt64 Seed = 0)
    {
        return Hash(reinterpret_cast<ConstPtr<Byte>>(Data.data()), Data.size(), Seed);
    }

    /// \brief Combines the hash values of multiple parameters into a single hash value.
    ///
    /// \param Parameters The parameters to be hashed.
    /// \return The combined hash value as a 64-bit unsigned integer.
    template<typename... Arguments>
    static UInt64 HashCombine(AnyRef<Arguments>... Parameters)
    {
        UInt64 Seed = 0;
        (..., (Seed = Hash(Parameters, Seed)));
        return Seed;
    }

    /// \brief Computes a compile-time hash for the specified type using the FNV-1a algorithm.
    ///
    /// \param Data A constant string view containing the UTF-8 data to be hashed.
    /// \return The computed hash value as a 64-bit unsigned integer.
    static consteval UInt64 HashFNV1(ConstStr8 Data)
    {
        UInt64 Hash = 14695981039346656037ull;

        for (const Char Character : Data)
        {
            Hash = (Hash ^ static_cast<UInt64>(Character)) * 1099511628211ull;
        }
        return Hash;
    }

    /// \brief Computes a compile-time hash for the specified type using the FNV-1a algorithm.
    ///
    /// \tparam Type The type for which to compute the hash.
    /// \return The computed hash value as a 64-bit unsigned integer.
    template<typename Type>
    static consteval UInt64 HashFNV1()
    {
#if defined(ZYPHRYON_COMPILER_MSVC)
        return HashFNV1(__FUNCSIG__);
#else
        return HashFNV1(__PRETTY_FUNCTION__);
#endif
    }
}