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

#include "Zyphryon.Base/Concept.hpp"
#include "Zyphryon.Base/Container/Array.hpp"
#include "Zyphryon.Base/Lexical/Algorithm.hpp"
#include "Zyphryon.Base/Lexical/Text.hpp"

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

    /// \brief Customization point that defines the range of underlying values probed when reflecting \p Type.
    ///
    /// \note Specialize this trait for a specific enum if it ever needs a wider or negative range.
    template<IsEnum Type>
    struct Range
    {
        /// The smallest underlying value probed while discovering declared enumerators.
        static constexpr SInt64 Min = 0;

        /// The largest underlying value probed while discovering declared enumerators.
        static constexpr SInt64 Max = 128;
    };

    namespace Detail    // TODO: Untested in GCC/CLang
    {
        /// \brief A single reflected enumerator.
        template<IsEnum Type>
        struct Descriptor final
        {
            /// The enumerator's value.
            Type   Value;

            /// The offset of the enumerator's name within the owning \c Reflection's name pool.
            UInt16 Offset;

            /// The number of characters in the enumerator's name.
            UInt16 Length;
        };

        /// \brief The result of reflecting an enum: a dense array of entries.
        template<IsEnum Type, UInt Capacity, UInt Chars>
        struct Reflection final
        {
            /// The reflected enumerators, in ascending value order.
            Array<Descriptor<Type>, Capacity> Entries;

            /// The concatenated names of every entry, each followed by a null terminator.
            Array<Char, Chars>                Names;
        };

        /// \brief The two counts needed to size a \c Reflection.
        struct Measurement final
        {
            /// The number of valid enumerators found.
            UInt Entries;

            /// The combined length, in characters, of every valid enumerator's name, including its null terminator.
            UInt Chars;
        };

        /// \brief Records one discovered enumerator into \p Result.
        template<IsEnum Type, UInt Capacity, UInt Chars>
        constexpr void AppendEntry(Ref<Reflection<Type, Capacity, Chars>> Result, Ref<UInt> EntryCursor,
            Ref<UInt> CharCursor, Type Candidate, Text Name)
        {
            for (UInt Offset = 0; Offset < Name.GetSize(); ++Offset)
            {
                Result.Names[CharCursor + Offset] = Name[Offset];
            }
            Result.Names[CharCursor + Name.GetSize()] = '\0';

            Result.Entries[EntryCursor] = Descriptor<Type>(
                Candidate,
                static_cast<UInt16>(CharCursor),
                static_cast<UInt16>(Name.GetSize()));

            CharCursor += Name.GetSize() + 1;
            ++EntryCursor;
        }

#if defined(ZY_EXTENSION_RESHARPER)

        /// \brief Measures how many entries are valid, and their combined name length, in one pass.
        template<typename Type, UInt Span>
        constexpr Measurement Measure()
        {
            Measurement Result { };

            for (UInt Index = 0; Index < Span; ++Index)
            {
                const Type Value = static_cast<Type>(Range<Type>::Min + static_cast<SInt64>(Index));

                if (const ConstPtr<Char> Name = __rscpp_enumerator_name(Value))
                {
                    ++Result.Entries;
                    Result.Chars += StrLength(Name) + 1;
                }
            }
            return Result;
        }

        /// \brief Builds the reflection data for \p Type, in ascending value order.
        template<typename Type, UInt Span>
        constexpr auto BuildReflection()
        {
            constexpr Measurement Size = Measure<Type, Span>();

            Reflection<Type, Size.Entries, Size.Chars> Result;

            UInt EntryCursor = 0;
            UInt CharCursor  = 0;

            for (UInt Index = 0; Index < Span && EntryCursor != Result.Entries.GetSize(); ++Index)
            {
                const Type Candidate = static_cast<Type>(Range<Type>::Min + static_cast<SInt64>(Index));

                if (const ConstPtr<Char> Name = __rscpp_enumerator_name(Candidate))
                {
                    AppendEntry(Result, EntryCursor, CharCursor, Candidate, StrConvert(Name));
                }
            }
            return Result;
        }

        /// \brief The dense, compile-time reflection data for \p Type.
        template<IsEnum Type>
        inline constexpr auto kReflection =
            BuildReflection<Type, static_cast<UInt>(Range<Type>::Max - Range<Type>::Min + 1)>();

#else

        /// \brief Captures the compiler-generated signature of this function.
        template<auto... Values>
        constexpr Text ExtractSignature()
        {
#if     defined(ZY_COMPILER_MSVC)
            return __FUNCSIG__;
#else
            return __PRETTY_FUNCTION__;
#endif
        }

        /// \brief Extracts the comma-separated argument-list portion of a captured \ref ExtractSignature.
        constexpr Text ExtractArguments(Text Signature)
        {
            const ConstPtr<Char> Head = Signature.GetData();

            UInt End = Signature.GetSize();

            while (End > 0 && Head[End - 1] != '>')
            {
                --End;
            }

            UInt Start = 0;

            while (Start < End && Head[Start] != '<')
            {
                ++Start;
            }

            if (Start + 2 > End)
            {
                return Text::Empty();
            }
            return Text(Head + Start + 1, End - Start - 2);
        }

        /// \brief Scans \p Arguments once and invokes \p Emit for every declared enumerator found.
        template<typename Callable>
        constexpr Measurement ScanArguments(Text Arguments, AnyRef<Callable> Emit)
        {
            Measurement Result { };

            const ConstPtr<Char> Head = Arguments.GetData();
            const UInt           Size = Arguments.GetSize();

            UInt Cursor   = 0;
            UInt Position = 0;

            while (Cursor < Size)
            {
                while (Cursor < Size && (Head[Cursor] == ' ' || Head[Cursor] == '\t'))
                {
                    ++Cursor;
                }

                const UInt Begin = Cursor;

                if (Cursor < Size && Head[Cursor] != '(')
                {
                    UInt Separator = Begin;

                    while (Cursor < Size && Head[Cursor] != ',')
                    {
                        if (Head[Cursor] == ':' && Cursor + 1 < Size && Head[Cursor + 1] == ':')
                        {
                            Cursor   += 2;
                            Separator = Cursor;
                        }
                        else
                        {
                            ++Cursor;
                        }
                    }

                    UInt Last = Cursor;

                    while (Last > Begin && Head[Last - 1] == ' ')
                    {
                        --Last;
                    }

                    if (Separator > Begin && Separator < Last)
                    {
                        Bool Identifier = true;

                        for (UInt Index = Separator; Identifier && Index < Last; ++Index)
                        {
                            Identifier = StrIsIdentifier(Head[Index]);
                        }

                        if (Identifier)
                        {
                            ++Result.Entries;
                            Result.Chars += Last - Separator + 1;

                            Emit(Position, Text(Head + Separator, Last - Separator));
                        }
                    }
                }
                else
                {
                    while (Cursor < Size && Head[Cursor] != ',')
                    {
                        ++Cursor;
                    }
                }

                ++Cursor;
                ++Position;
            }
            return Result;
        }

        /// \brief Captures the argument list for every probed value of \p Type.
        template<typename Type, SInt64... Offsets>
        constexpr Text CaptureArguments(IntegerSequence<SInt64, Offsets...>)
        {
            return ExtractArguments(ExtractSignature<static_cast<Type>(Range<Type>::Min + Offsets)...>());
        }

        /// \brief The captured argument list for \p Type, evaluated once per translation unit.
        template<IsEnum Type>
        inline constexpr Text kArguments =
            CaptureArguments<Type>(MakeIntegerSequence<SInt64, Range<Type>::Max - Range<Type>::Min + 1>{ });

        /// \brief The measured counts for \p Type, evaluated once per translation unit.
        template<IsEnum Type>
        inline constexpr Measurement kMeasurement = ScanArguments(kArguments<Type>, [](UInt, Text) { });

        /// \brief Builds the reflection data for \p Type, in ascending value order.
        template<IsEnum Type>
        constexpr auto BuildReflection()
        {
            Reflection<Type, kMeasurement<Type>.Entries, kMeasurement<Type>.Chars> Result;

            UInt EntryCursor = 0;
            UInt CharCursor  = 0;

            ScanArguments(kArguments<Type>, [&](UInt Position, Text Name)
            {
                AppendEntry(Result, EntryCursor, CharCursor,
                    static_cast<Type>(Range<Type>::Min + static_cast<SInt64>(Position)), Name);
            });

            return Result;
        }

        /// \brief The dense, compile-time reflection data for \p Type.
        template<IsEnum Type>
        inline constexpr auto kReflection = BuildReflection<Type>();

#endif
    }

    /// \brief Converts an enum value to its string representation.
    ///
    /// \param Value The enum value to convert.
    /// \return A \c Text view of the string name corresponding to the enum value, or empty if not found.
    template<IsEnum Type>
    constexpr Text GetName(Type Value)
    {
        ConstRef<decltype(Detail::kReflection<Type>)> Reflection = Detail::kReflection<Type>;

        for (UInt Index = 0; Index < Reflection.Entries.GetSize(); ++Index)
        {
            ConstRef<Detail::Descriptor<Type>> Item = Reflection.Entries[Index];

            if (Item.Value == Value)
            {
                return Text(Reflection.Names.GetData() + Item.Offset, Item.Length);
            }
        }
        return Text::Empty();
    }

    /// \brief Gets all declared values of an enum type.
    ///
    /// \return A range of all declared values in \p Type.
    template<IsEnum Type>
    constexpr auto GetValues()
    {
        constexpr UInt Size = Detail::kReflection<Type>.Entries.GetSize();

        Array<Type, Size> Result;

        for (UInt Index = 0; Index < Size; ++Index)
        {
            Result[Index] = Detail::kReflection<Type>.Entries[Index].Value;
        }
        return Result;
    }

    /// \brief Converts a string to an enum value using case-insensitive comparison.
    ///
    /// \param Name    The string name of the enum value.
    /// \param Default The default value to return if the name is not found.
    /// \return An enum value corresponding to \p Name, or \p Default if not found.
    template<IsEnum Type>
    constexpr Type Cast(Text Name, Type Default)
    {
        ConstRef<decltype(Detail::kReflection<Type>)> Reflection = Detail::kReflection<Type>;

        for (UInt Index = 0; Index < Reflection.Entries.GetSize(); ++Index)
        {
            ConstRef<Detail::Descriptor<Type>> Item = Reflection.Entries[Index];

            const Text Entry(Reflection.Names.GetData() + Item.Offset, Item.Length);

            if (StrEqualCaseInsensitive(Entry, Name))
            {
                return Item.Value;
            }
        }
        return Default;
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
        return Detail::kReflection<Type>.Entries.GetSize();
    }
}
