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

#include "Zyphryon.Base/Container/Sequence.hpp"
#include "Zyphryon.Base/Lexical/Algorithm.hpp"
#include "Zyphryon.Base/Pattern/Enumerator.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A parsed command line, exposed as named switches plus ordered operands.
    ///
    /// A switch is written as `--name`, `--name=value`, `--name value`, or `--no-name`.
    class Environment final
    {
    public:

        /// \brief Constructs an empty option set.
        ZY_INLINE Environment() = default;

        /// \brief Parses a raw argument vector, skipping the program name.
        ///
        /// \param Count     The number of entries in \p Environment.
        /// \param Arguments The argument vector, as handed to `main`.
        void Parse(UInt Count, ConstPtr<ConstPtr<Char>> Arguments);

        /// \brief Checks whether a switch was supplied.
        ///
        /// \param Name The switch name, without its leading dashes.
        /// \return `true` if the switch appeared on the command line, otherwise `false`.
        ZY_INLINE Bool Contains(Text Name) const
        {
            return Find(Name) != nullptr;
        }

        /// \brief Gets the raw value of a switch.
        ///
        /// \param Name    The switch name, without its leading dashes.
        /// \param Default The value returned when the switch is absent.
        /// \return The switch's value, or \p Default when the switch is absent.
        ZY_INLINE Text GetText(Text Name, Text Default) const
        {
            const ConstPtr<Switch> Entry = Find(Name);

            return Entry ? Entry->Value : Default;
        }

        /// \brief Gets a switch as a boolean.
        ///
        /// \note A valueless switch reads as `true`, and its negated spelling reads as `false`.
        ///
        /// \param Name    The switch name, without its leading dashes.
        /// \param Default The value returned when the switch is absent.
        /// \return The switch's value, or \p Default when the switch is absent.
        ZY_INLINE Bool GetBool(Text Name, Bool Default) const
        {
            const ConstPtr<Switch> Entry = Find(Name);

            if (Entry == nullptr)
            {
                return Default;
            }

            // A switch supplied without a value asserts the flag; `--no-name` is rewritten to "false" while parsing.
            if (Entry->Value.IsEmpty())
            {
                return true;
            }

            UInt Cursor = 0;
            return StrExtractBool(Entry->Value, Cursor);
        }

        /// \brief Gets a switch as an integral or real number.
        ///
        /// \param Name    The switch name, without its leading dashes.
        /// \param Default The value returned when the switch is absent or carries no value.
        /// \return The switch's value, or \p Default when the switch is absent or carries no value.
        template<typename Type>
        ZY_INLINE Type GetNumber(Text Name, Type Default) const
            requires (IsIntegral<Type> || IsReal<Type>)
        {
            const ConstPtr<Switch> Entry = Find(Name);

            if (Entry == nullptr || Entry->Value.IsEmpty())
            {
                return Default;
            }

            UInt Cursor = 0;

            if constexpr (IsReal<Type>)
            {
                return StrExtractNumber<Type>(Entry->Value, Cursor);
            }
            else
            {
                return StrExtractNumber<10, Type>(Entry->Value, Cursor);
            }
        }

        /// \brief Gets a switch as an enumerator, matched case-insensitively against its declared names.
        ///
        /// \param Name    The switch name, without its leading dashes.
        /// \param Default The value returned when the switch is absent or unrecognized.
        /// \return The switch's value, or \p Default when the switch is absent or unrecognized.
        template<IsEnum Type>
        ZY_INLINE Type GetEnum(Text Name, Type Default) const
        {
            const Text Value = GetText(Name, Text::Empty());

            return Value.IsEmpty() ? Default : Enum::Cast<Type>(Value, Default);
        }

        /// \brief Gets the operands, in the order they appeared.
        ///
        /// \return A view over every argument that was not a switch.
        ZY_INLINE ConstSpan<Text> GetOperands() const
        {
            return mOperands;
        }

    private:

        /// \brief A single name/value pair produced by \ref Parse.
        struct Switch final
        {
            /// The switch name, with its leading dashes and any negation prefix stripped.
            Text Name;

            /// The switch value, empty when the switch was supplied without one.
            Text Value;
        };

        /// \brief Finds the last switch declared under the given name.
        ///
        /// \note Later occurrences win, so a wrapper script can append overrides to a base command line.
        ///
        /// \param Name The switch name, without its leading dashes.
        /// \return A pointer to the matching switch, or `nullptr` when the switch is absent.
        ConstPtr<Switch> Find(Text Name) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Switch> mSwitches;
        Sequence<Text>   mOperands;
    };
}
