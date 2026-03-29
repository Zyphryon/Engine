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

#include "Hash.hpp"
#include <array>
#include <beman/inplace_vector/inplace_vector.hpp>
#include <bitset>
#include <unordered_dense.h>
#include <vector>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A static array with a fixed capacity defined at compile-time.
    ///
    /// \tparam Value    The type of elements stored in the array.
    /// \tparam Capacity The maximum number of elements the array can hold.
    template<typename Value, UInt32 Capacity>
    using Array  = std::array<Value, Capacity>;

    /// \brief A static array of bits with a fixed capacity defined at compile-time.
    ///
    /// \tparam Capacity The number of bits in the bitset.
    template<UInt32 Capacity>
    using Bitset = std::bitset<Capacity>;

    /// \brief Default hasher for string types.
    struct DefaultStringHasher
    {
        using is_transparent = void; ///< Indicates that the hash function supports heterogeneous lookup.
        using is_avalanching = void; ///< Indicates that the hash function provides good avalanche properties.

        ZYPHRYON_INLINE UInt64 operator()(ConstPtr<Char> Value) const
        {
            return Hash(Value);
        }

        ZYPHRYON_INLINE UInt64 operator()(ConstStr8 Value) const
        {
            return Hash(Value);
        }

        ZYPHRYON_INLINE UInt64 operator()(ConstRef<Str8> Value) const
        {
            return Hash(Value);
        }
    };

    /// \brief Default hasher for generic object types.
    struct DefaultObjectHasher
    {
        using is_transparent = void; ///< Indicates that the hash function supports heterogeneous lookup.
        using is_avalanching = void; ///< Indicates that the hash function provides good avalanche properties.

        template<typename Type>
        ZYPHRYON_INLINE UInt64 operator()(ConstRef<Type> Value) const noexcept
        {
            return Hash(Value);
        }

        template<typename Type>
        ZYPHRYON_INLINE UInt64 operator()(ConstPtr<Type> Value) const noexcept
        {
            return Hash(* Value);
        }
    };

    /// \brief A high-performance hash map using ankerl::unordered_dense.
    template<typename Key, typename Hash = DefaultObjectHasher, typename Predicate = std::equal_to<>>
    using Set    = ankerl::unordered_dense::set<Key, Hash, Predicate>;

    /// \brief A high-performance hash table using ankerl::unordered_dense.
    template<typename Key, typename Value, typename Hash = DefaultObjectHasher, typename Predicate = std::equal_to<>>
    using Table  = Switch<IsEqual<Key, Str8>,
        ankerl::unordered_dense::map<Key, Value, DefaultStringHasher, Predicate>,
        ankerl::unordered_dense::map<Key, Value, Hash, Predicate>>;

    /// \brief A dynamic array with optional inline storage based on capacity.
    ///
    /// `Vector` acts as an alias for either `std::vector` or a small-vector-style container depending
    /// on the compile-time `Capacity` parameter. If `Capacity > 0`, it uses `beman::inplace_vector`
    /// to avoid heap allocation for small sizes; otherwise, it defaults to `std::vector`.
    ///
    /// \tparam Value The type of elements stored in the vector.
    /// \tparam Capacity The number of inline elements to store before falling back to heap allocation.
    template<typename Value, UInt32 Capacity = 0>
    using Vector = Switch<(Capacity > 0), beman::inplace_vector::inplace_vector<Value, Capacity>, std::vector<Value>>;
}