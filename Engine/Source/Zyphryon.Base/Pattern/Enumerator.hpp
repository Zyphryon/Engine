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

#include "Zyphryon.Base/Container/Array.hpp"
#include "Zyphryon.Base/Lexical/Algorithm.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyEnum
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
/// \brief Defines bitwise operators for the specified enum type with friend access, for use inside a class.
#define ZY_DEFINE_BITWISE_FRIEND_ENUM(Enum) ZY_DEFINE_BITWISE_ENUM_TYPE(Enum, friend)

    /// \brief Provides the customization point for the range of underlying values probed when reflecting \p Type.
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

    namespace Detail
    {
        /// \brief Represents a single reflected enumerator.
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

        /// \brief Represents the result of reflecting an enum, its dense entries and the names they point into.
        template<IsEnum Type, UInt Capacity, UInt Chars>
        struct Reflection final
        {
            /// The reflected enumerators, in ascending value order.
            Array<Descriptor<Type>, Capacity> Entries;

            /// The concatenated names of every entry, each followed by a null terminator.
            Array<Char, Chars>                Names;
        };

        /// \brief Holds the two counts needed to size a \c Reflection.
        struct Measurement final
        {
            /// The number of valid enumerators found.
            UInt Entries;

            /// The combined length, in characters, of every valid enumerator's name, including its null terminator.
            UInt Chars;
        };

        /// \brief Records one discovered enumerator into \p Result.
        ///
        /// \param Result      The reflection the enumerator is written into.
        /// \param EntryCursor The index of the next entry, advanced past this one.
        /// \param CharCursor  The offset of the next name in the pool, advanced past this name and its terminator.
        /// \param Candidate   The value the enumerator holds.
        /// \param Name        The name the enumerator is declared with.
        template<IsEnum Type, UInt Capacity, UInt Chars>
        constexpr void AppendEntry(
            Ref<Reflection<Type, Capacity, Chars>> Result,
            Ref<UInt>                              EntryCursor,
            Ref<UInt>                              CharCursor,
            Type                                   Candidate,
            Text                                   Name)
        {
            Copy(Result.Names.GetData() + CharCursor, Name.GetSize(), Name.GetData());
            Result.Names[CharCursor + Name.GetSize()] = '\0';

            Result.Entries[EntryCursor] = Descriptor<Type>(
                Candidate,
                static_cast<UInt16>(CharCursor),
                static_cast<UInt16>(Name.GetSize()));

            CharCursor += Name.GetSize() + 1;
            ++EntryCursor;
        }

        /// \brief Converts a probe index into the enumerator value it stands for.
        ///
        /// \param Index The index counted from \ref Range's minimum.
        /// \return The value at \p Index.
        template<IsEnum Type>
        constexpr Type Probe(UInt Index)
        {
            return static_cast<Type>(Range<Type>::Min + static_cast<SInt64>(Index));
        }

#if defined(ZY_EXTENSION_RESHARPER)

        /// \brief Measures how many entries are valid, and their combined name length, in one pass.
        ///
        /// \return The number of declared enumerators and their combined name length.
        template<typename Type, UInt Span>
        constexpr Measurement Measure()
        {
            Measurement Result { };

            for (UInt Index = 0; Index < Span; ++Index)
            {
                const Type Value = Probe<Type>(Index);

                if (const ConstPtr<Char> Name = __rscpp_enumerator_name(Value))
                {
                    ++Result.Entries;
                    Result.Chars += StrLength(Name) + 1;
                }
            }
            return Result;
        }

        /// \brief Builds the reflection data for \p Type, in ascending value order.
        ///
        /// \return The reflection of every declared enumerator.
        template<typename Type, UInt Span>
        constexpr auto BuildReflection()
        {
            constexpr Measurement Size = Measure<Type, Span>();

            Reflection<Type, Size.Entries, Size.Chars> Result;

            UInt EntryCursor = 0;
            UInt CharCursor  = 0;

            for (UInt Index = 0; Index < Span && EntryCursor != Result.Entries.GetSize(); ++Index)
            {
                const Type Candidate = Probe<Type>(Index);

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
        ///
        /// \return The signature, whose template arguments name every probed value.
        template<auto... Values>
        consteval Text ExtractSignature()
        {
            return ZY_SIGNATURE;
        }

        /// \brief The delimiters that wrap the argument list within a captured signature.
#if defined(ZY_COMPILER_GCC)
        inline constexpr Char kSignatureOpen  = '{';
        inline constexpr Char kSignatureClose = '}';
#else
        inline constexpr Char kSignatureOpen  = '<';
        inline constexpr Char kSignatureClose = '>';
#endif

        /// \brief Extracts the comma-separated argument-list portion of a captured \ref ExtractSignature.
        ///
        /// \param Signature The signature ExtractSignature captured.
        /// \return The argument list between the outermost delimiters, or empty when there is none.
        constexpr Text ExtractArguments(Text Signature)
        {
            const ConstPtr<Char> Head = Signature.GetData();

            UInt End = Signature.GetSize();

            while (End > 0 && Head[End - 1] != kSignatureClose)
            {
                --End;
            }

            UInt Start = 0;

            while (Start < End && Head[Start] != kSignatureOpen)
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
        ///
        /// \param Arguments The argument list to scan.
        /// \param Emit      The callable given the position and name of each declared enumerator.
        /// \return The number of declared enumerators and their combined name length.
        template<typename Callable>
        constexpr Measurement ScanArguments(Text Arguments, AnyRef<Callable> Emit)
        {
            Measurement Result { };

            const ConstPtr<Char> Head = Arguments.GetData();
            const UInt           Size = Arguments.GetSize();

            UInt Cursor   = 0;
            UInt Position = 0;
            UInt Cast     = 0;

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
                    // Every undeclared value is written behind the same cast, which is stepped over at once.
                    if (Cast != 0 && Cursor + Cast <= Size && Head[Cursor + Cast - 1] == ')')
                    {
                        Cursor += Cast;
                    }

                    while (Cursor < Size && Head[Cursor] != ',')
                    {
                        if (Cast == 0 && Head[Cursor] == ')')
                        {
                            Cast = Cursor - Begin + 1;
                        }
                        ++Cursor;
                    }
                }

                ++Cursor;
                ++Position;
            }
            return Result;
        }

        /// \brief Captures the argument list for every probed value of \p Type.
        ///
        /// \return The argument list captured for every probed value.
        template<typename Type, SInt64... Offsets>
        consteval Text CaptureArguments(IntegerSequence<SInt64, Offsets...>)
        {
            return ExtractArguments(ExtractSignature<static_cast<Type>(Range<Type>::Min + Offsets)...>());
        }

        /// \brief Captures the argument list for \p Type.
        ///
        /// \return The argument list naming every probed value of \p Type.
        template<IsEnum Type>
        consteval Text GetArguments()
        {
            return CaptureArguments<Type>(MakeIntegerSequence<SInt64, Range<Type>::Max - Range<Type>::Min + 1>{ });
        }

        /// \brief Represents where one declared enumerator stands within a captured argument list.
        struct Mark final
        {
            /// The position of the enumerator among the probed values.
            UInt32 Position;

            /// The offset of the enumerator's name within the argument list.
            UInt32 Offset;

            /// The number of characters in the enumerator's name.
            UInt32 Length;
        };

        /// \brief Represents every declared enumerator one scan of a captured argument list found.
        template<UInt Capacity>
        struct Survey final
        {
            /// The number of declared enumerators and their combined name length.
            Measurement           Size { };

            /// The declared enumerators, in the order they were probed.
            Array<Mark, Capacity> Marks;
        };

        /// \brief Scans the argument list of \p Type once, keeping where each declared enumerator stands.
        ///
        /// \return The declared enumerators of \p Type.
        template<IsEnum Type>
        consteval auto Scan()
        {
            Survey<static_cast<UInt>(Range<Type>::Max - Range<Type>::Min + 1)> Result;

            const Text Arguments = GetArguments<Type>();
            UInt       Cursor    = 0;

            Result.Size = ScanArguments(Arguments, [&](UInt Position, Text Name)
            {
                Result.Marks[Cursor++] = Mark(
                    static_cast<UInt32>(Position),
                    static_cast<UInt32>(Name.GetData() - Arguments.GetData()),
                    static_cast<UInt32>(Name.GetSize()));
            });
            return Result;
        }

        /// \brief The declared enumerators of \p Type, scanned once per translation unit.
        template<IsEnum Type>
        inline constexpr auto kSurvey = Scan<Type>();

        /// \brief Builds the reflection data for \p Type, in ascending value order.
        ///
        /// \return The reflection of every declared enumerator.
        template<IsEnum Type>
        consteval auto BuildReflection()
        {
            Reflection<Type, kSurvey<Type>.Size.Entries, kSurvey<Type>.Size.Chars> Result;

            const Text Arguments   = GetArguments<Type>();
            UInt       EntryCursor = 0;
            UInt       CharCursor  = 0;

            for (UInt Index = 0; Index < Result.Entries.GetSize(); ++Index)
            {
                const Mark Found = kSurvey<Type>.Marks[Index];

                AppendEntry(
                    Result,
                    EntryCursor,
                    CharCursor,
                    Probe<Type>(Found.Position),
                    Text(Arguments.GetData() + Found.Offset, Found.Length));
            }
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

    /// \brief Finds the enum value a string names using case-insensitive comparison.
    ///
    /// \param Name   The string name of the enum value.
    /// \param Output Receives the enum value named, unchanged when the name is not found.
    /// \return `true` if \p Name names a value of \p Type, `false` otherwise.
    template<IsEnum Type>
    constexpr Bool Find(Text Name, Ref<Type> Output)
    {
        ConstRef<decltype(Detail::kReflection<Type>)> Reflection = Detail::kReflection<Type>;

        for (UInt Index = 0; Index < Reflection.Entries.GetSize(); ++Index)
        {
            ConstRef<Detail::Descriptor<Type>> Item = Reflection.Entries[Index];

            const Text Entry(Reflection.Names.GetData() + Item.Offset, Item.Length);

            if (StrEqualCaseInsensitive(Entry, Name))
            {
                Output = Item.Value;
                return true;
            }
        }
        return false;
    }

    /// \brief Converts a string to an enum value using case-insensitive comparison.
    ///
    /// \param Name    The string name of the enum value.
    /// \param Default The default value to return if the name is not found.
    /// \return An enum value corresponding to \p Name, or \p Default if not found.
    template<IsEnum Type>
    constexpr Type Cast(Text Name, Type Default)
    {
        Type Result = Default;
        Find(Name, Result);
        return Result;
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