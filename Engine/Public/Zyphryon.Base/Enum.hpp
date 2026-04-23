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

#include "Text.hpp"
#include <enchantum/enchantum.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Enum
{

/// \def ZYPHRYON_DEFINE_BITWISE_ENUM_TYPE
/// \brief Defines bitwise operators for the specified enum type with the given access level.
#define ZYPHRYON_DEFINE_BITWISE_ENUM_TYPE(Enum, Access)                                       \
    Access constexpr Enum operator&(Enum A, Enum B) noexcept                                  \
    {                                                                                         \
      using T = std::underlying_type_t<Enum>;                                                 \
      return static_cast<Enum>(static_cast<T>(A) & static_cast<T>(B));                        \
    }                                                                                         \
    Access constexpr Enum operator|(Enum A, Enum B) noexcept                                  \
    {                                                                                         \
      using T = std::underlying_type_t<Enum>;                                                 \
      return static_cast<Enum>(static_cast<T>(A) | static_cast<T>(B));                        \
    }                                                                                         \
    Access constexpr Enum operator^(Enum A, Enum B) noexcept                                  \
    {                                                                                         \
      using T = std::underlying_type_t<Enum>;                                                 \
      return static_cast<Enum>(static_cast<T>(A) ^ static_cast<T>(B));                        \
    }                                                                                         \
    Access constexpr Ref<Enum> operator&=(Ref<Enum> A, Enum B) noexcept { return A = A & B; } \
    Access constexpr Ref<Enum> operator|=(Ref<Enum> A, Enum B) noexcept { return A = A | B; } \
    Access constexpr Ref<Enum> operator^=(Ref<Enum> A, Enum B) noexcept { return A = A ^ B; } \
    Access constexpr Enum      operator~(Enum A) noexcept                                     \
    {                                                                                         \
      return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(A));                \
    }

/// \def ZYPHRYON_DEFINE_BITWISE_ENUM
/// \brief Defines bitwise operators for the specified enum type with inline access.
#define ZYPHRYON_DEFINE_BITWISE_ENUM(Enum)        ZYPHRYON_DEFINE_BITWISE_ENUM_TYPE(Enum, inline)

/// \def ZYPHRYON_DEFINE_BITWISE_FRIEND_ENUM
/// \brief Defines bitwise operators for the specified enum type with friend access, allowing them to be used in class scopes.
#define ZYPHRYON_DEFINE_BITWISE_FRIEND_ENUM(Enum) ZYPHRYON_DEFINE_BITWISE_ENUM_TYPE(Enum, friend)

    /// \brief Retrieves the name of the specified enum value as a string.
    ///
    /// \param Value The enum value whose name is to be retrieved.
    /// \return The name of the enum value as a string.
    template<typename Type>
    static constexpr ConstStr8 Name(Type Value)
    {
        return enchantum::to_string(Value);
    }

    /// \brief Retrieves all enum values of the specified enum type.
    ///
    /// \return A span containing all enum values.
    template<typename Type>
    static constexpr auto Values()
    {
        return enchantum::values<Type>;
    }

    /// \brief Casts a string to the corresponding enum value, using a case-insensitive comparison.
    ///
    /// \param Name    The name of the enum value as a string.
    /// \param Default The default value to return if not found.
    /// \return The corresponding enum value, or the default if not found.
    template<typename Type>
    static constexpr Type Cast(ConstStr8 Name, Type Default)
    {
        return enchantum::cast<Type>(Name, [](unsigned char X, unsigned char Y)
        {
            return std::tolower(X) == std::tolower(Y);
        }).value_or(Default);
    }

    /// \brief Casts an enum value to its underlying integral type.
    ///
    /// \param Value The enum value to cast.
    /// \return The enum value cast to its underlying integral type.
    template<typename Type>
    static constexpr auto Cast(Type Value)
    {
        return static_cast<std::underlying_type_t<Type>>(Value);
    }

    /// \brief Retrieves the total number of enumerators in the specified enum type.
    ///
    /// \return The count of enumerators in the enum.
    template<typename Type>
    static constexpr auto Count()
    {
        return enchantum::count<Type>;
    }

    /// \brief Checks if the specified flags are included in the given enum value.
    ///
    /// \param Value The enum value to check.
    /// \param Flag  The flags to check for inclusion.
    /// \return `true` if all flags are included in the value, `false` otherwise.
    template<typename Type>
    static constexpr Bool Includes(Type Value, Type Flag)
    {
        return (Value & Flag) == Flag;
    }
}