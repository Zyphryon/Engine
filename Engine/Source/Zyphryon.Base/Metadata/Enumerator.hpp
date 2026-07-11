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

#include "Zyphryon.Base/Lexical/Text.hpp"
#include <enchantum/enchantum.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Enum
{
/// \def ZY_DEFINE_BITWISE_ENUM_TYPE
/// \brief Defines bitwise operators for the specified enum type with the given access level.
#define ZY_DEFINE_BITWISE_ENUM_TYPE(Enum, Access)                                             \
    Access constexpr Enum operator&(Enum A, Enum B) noexcept                                  \
    {                                                                                         \
      using T = Underlying<Enum>;                                                             \
      return static_cast<Enum>(static_cast<T>(A) & static_cast<T>(B));                        \
    }                                                                                         \
    Access constexpr Enum operator|(Enum A, Enum B) noexcept                                  \
    {                                                                                         \
      using T = Underlying<Enum>;                                                             \
      return static_cast<Enum>(static_cast<T>(A) | static_cast<T>(B));                        \
    }                                                                                         \
    Access constexpr Enum operator^(Enum A, Enum B) noexcept                                  \
    {                                                                                         \
      using T = Underlying<Enum>;                                                             \
      return static_cast<Enum>(static_cast<T>(A) ^ static_cast<T>(B));                        \
    }                                                                                         \
    Access constexpr Ref<Enum> operator&=(Ref<Enum> A, Enum B) noexcept { return A = A & B; } \
    Access constexpr Ref<Enum> operator|=(Ref<Enum> A, Enum B) noexcept { return A = A | B; } \
    Access constexpr Ref<Enum> operator^=(Ref<Enum> A, Enum B) noexcept { return A = A ^ B; } \
    Access constexpr Enum      operator~(Enum A) noexcept                                     \
    {                                                                                         \
      return static_cast<Enum>(~static_cast<Underlying<Enum>>(A));                            \
    }

/// \def ZY_DEFINE_BITWISE_ENUM
/// \brief Defines bitwise operators for the specified enum type with inline access.
#define ZY_DEFINE_BITWISE_ENUM(Enum)        ZY_DEFINE_BITWISE_ENUM_TYPE(Enum, inline)

/// \def ZY_DEFINE_BITWISE_FRIEND_ENUM
/// \brief Defines bitwise operators for the specified enum type with friend access, allowing them to be used in class scopes.
#define ZY_DEFINE_BITWISE_FRIEND_ENUM(Enum) ZY_DEFINE_BITWISE_ENUM_TYPE(Enum, friend)

    /// \brief Converts an enum value to its string representation.
    ///
    /// \param Value The enum value to convert.
    /// \return A \c Text view of the string name corresponding to the enum value, or empty if not found.
    template<IsEnum Type>
    constexpr Text GetName(Type Value)
    {
        const std::string_view Result = enchantum::to_string(Value);
        return Text(Result.data(), Result.size());
    }

    /// \brief Gets all declared values of an enum type.
    ///
    /// \return A range of all declared values in \p Type.ets all defined values for an enum type.
    template<IsEnum Type>
    constexpr auto GetValues()
    {
        return enchantum::values<Type>;
    }

    /// \brief Converts a string to an enum value using case-insensitive comparison.
    ///
    /// \param Name    The string name of the enum value.
    /// \param Default The default value to return if the name is not found.
    /// \return An enum value corresponding to \p Name, or \p Default if not found.
    template<IsEnum Type>
    constexpr Type Cast(Text Name, Type Default)
    {
        const std::string_view Key(Name.GetData(), Name.GetSize());

        return enchantum::cast<Type>(Key, [](unsigned char X, unsigned char Y)
        {
            return StrLowercase(X) == StrLowercase(Y);
        }).value_or(Default);
    }

    /// \brief Converts an enum value to its underlying integer type.
    ///
    /// \param Value The enum value to convert.
    /// \return A value as the underlying integer type of \p Type.
    template<IsEnum Type>
    constexpr auto Cast(Type Value)
    {
        return static_cast<Underlying<Type>>(Value);
    }

    /// \brief Gets the number of declared values in an enum type.
    ///
    /// \return The count of all declared values in \p Type.
    template<IsEnum Type>
    constexpr UInt Count()
    {
        return enchantum::count<Type>;
    }
}
