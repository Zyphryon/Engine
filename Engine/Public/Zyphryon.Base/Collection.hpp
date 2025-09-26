// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Utility.hpp"
#include <beman/inplace_vector/inplace_vector.hpp>
#include <bitset>
#include <unordered_dense.h>
#include <queue>
#include <vector>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A fixed-size array of elements with static capacity.
    /// 
    /// \tparam Value The type of each element.
    /// \tparam Capacity The number of elements in the array.
    template<typename Value, UInt32 Capacity>
    using Array  = std::array<Value, Capacity>;

    /// \brief A fixed-size sequence of bits.
    /// 
    /// \tparam Capacity The number of bits in the set.
    template<UInt32 Capacity>
    using Bitset = std::bitset<Capacity>;

    /// \brief Alias for a fixed list of values.
    /// 
    /// Provides a shorthand for \c std::initializer_list, typically used
    /// to supply a compile-time list of values for initialization.
    /// 
    /// \tparam Value The type of the elements in the list.
    template<typename Value>
    using List   = std::initializer_list<Value>;

    /// \brief Generic hash functor for arbitrary types.
    ///
    /// Prefers using a type’s custom `Hash()` member if available, otherwise falls back to `wyhash`
    /// for compatibility with `ankerl::unordered_dense`.
    template<typename T>
    struct DefaultTableHash
    {
        /// \brief Marks this hash as avalanching for compatibility with `ankerl::unordered_dense`.
        using is_avalanching = void;

        /// \brief Computes a hash using `wyhash` when the type does not provide a `Hash()` member.
        ///
        /// \param Value The object to hash.
        /// \return Hash value of the object.
        [[nodiscard]] size_t operator()(ConstRef<T>  Value) const
            requires (!requires { Value.Hash(); })
        {
            return HashCombine(Value);
        }

        /// \brief Computes a hash using the object’s custom `Hash()` member.
        ///
        /// \param Value The object to hash.
        /// \return Hash value of the object as returned by `Hash()`.
        [[nodiscard]] size_t operator()(ConstRef<T> Value) const
            requires requires { { Value.Hash() } -> std::convertible_to<size_t>; }
        {
            return static_cast<size_t>(Value.Hash());
        }
    };

    /// \brief A high-performance unordered set container.
    ///
    /// \tparam Key       The type of elements to store in the set.
    /// \tparam Hash      The hash function used for keys. Defaults to `ankerl::unordered_dense::hash<Key>`.
    /// \tparam Predicate The equality comparison function for keys. Defaults to `std::equal_to<>`.
    template<typename Key,
             typename Hash = DefaultTableHash<Key>,
             typename Predicate = std::equal_to<>>
    using Set    = ankerl::unordered_dense::set<Key, Hash, Predicate>;

    /// \brief A high-performance associative container.
    /// 
    /// \tparam Key       The key type.
    /// \tparam Value     The mapped value type.
    /// \tparam Hash      The hash function used for keys. Defaults to `ankerl::unordered_dense::hash<Key>`.
    /// \tparam Predicate The equality comparison function for keys. Defaults to `std::equal_to<>`.
    template<typename Key,
             typename Value,
             typename Hash = DefaultTableHash<Key>,
             typename Predicate = std::equal_to<>>
    using Table  = ankerl::unordered_dense::map<Key, Value, Hash, Predicate>;

    /// \brief FIFO (first-in, first-out) queue container.
    /// 
    /// This type alias wraps `std::queue` to represent a sequential container
    /// that allows insertion at the back and removal from the front.
    /// It is primarily used for task scheduling, event queues, and any
    /// structure requiring ordered processing.
    /// 
    /// \tparam Value The type of elements stored in the queue.
    template<typename Value>
    using Queue  = std::queue<Value>;

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

    /// \brief Hash functor for UTF-8 text types with heterogeneous lookup support.
    struct TextTableHash
    {
        /// \brief Enables heterogeneous lookup with transparent hashing.
        using is_transparent = void;

        /// \brief Marks this hash as avalanching for compatibility with `ankerl::unordered_dense`.
        using is_avalanching = void;

        /// \brief Computes the hash of a null-terminated UTF-8 string.
        ///
        /// \param Value Pointer to a UTF-8 character array.
        /// \return Hash value of the string contents.
        [[nodiscard]] size_t operator()(ConstPtr<Char> Value) const
        {
            return HashText(Value);
        }

        /// \brief Computes the hash of a UTF-8 string view.
        ///
        /// \param Value View into the UTF-8 text.
        /// \return Hash value of the string contents.
        [[nodiscard]] size_t operator()(ConstStr8 Value) const
        {
            return HashText(Value);
        }

        /// \brief Computes the hash of a UTF-8 string reference.
        ///
        /// \param Value Reference to a string object.
        /// \return Hash value of the string contents.
        [[nodiscard]] size_t operator()(ConstRef<Str8> Value) const
        {
            return HashText(Value);
        }
    };

    /// \brief Hash map with `Text` keys supporting fast UTF-8 string lookups.
    /// 
    /// \tparam Value The value type to associate with each key.
    template<typename Value>
    using TextTable = Table<Str8, Value, TextTableHash>;
}