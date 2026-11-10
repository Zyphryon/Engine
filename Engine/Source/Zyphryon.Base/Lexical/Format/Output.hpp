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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyFormat
{
    /// \brief Represents the string a format writes into, held without its type so the format compiles once.
    class Output final
    {
    public:

        /// \brief Constructs an output over a string.
        ///
        /// \param Buffer The string to append to, which must outlive the output.
        template<typename Type>
        ZY_INLINE Output(Ref<Type> Buffer)
            requires (!IsAnyOf<Type, Output>)
            : mBuffer { AddressOf(Buffer) },
              mAppend { AddressOf(Output::Relay<Type>) }
        {
        }

        /// \brief Appends text to the end of the string.
        ///
        /// \param Content The text to append.
        ZY_INLINE void Append(Text Content) const
        {
            mAppend(mBuffer, Content);
        }

        /// \brief Appends a single character to the end of the string.
        ///
        /// \param Character The character to append.
        ZY_INLINE void Append(Char Character) const
        {
            mAppend(mBuffer, Text(AddressOf(Character), 1));
        }

    private:

        /// \brief Signature of the handler that appends text to the string an output was made over.
        using Appender = void (*)(Ptr<void> String, Text Content);

        /// \brief Appends text to the string an output was made over, as the handler the output keeps.
        ///
        /// \param String  The address of the string, which is of \p Type.
        /// \param Content The text to append.
        template<typename Type>
        static void Relay(Ptr<void> String, Text Content)
        {
            static_cast<Ptr<Type>>(String)->Append(Content);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<void> mBuffer;
        Appender  mAppend;
    };
}