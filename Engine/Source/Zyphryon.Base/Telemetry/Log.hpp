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

#include "Zyphryon.Base/Lexical/String.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyLog   // TODO: Proper implementation
{
    namespace Detail
    {
        template<typename ...Arguments>
        ZY_INLINE Text Print(AnyRef<ZyFormat::Pattern<>> Format, AnyRef<Arguments>... Parameters)
        {
            thread_local String<4096> Buffer;

            Buffer.Format(Format, Parameters...);
            return Buffer;
        }
    }

    /// \brief Writes a formatted message to all active log outputs.
    ///
    /// \param Priority The severity level of the message. Controls prefix, color, and routing.
    /// \param Message  The fully formatted message text to emit.
    void Write(UInt8 Priority, Text Message);

    /// \brief Flushes the logging console.
    void Flush();

/// \def LOG_D
/// \brief Logs a debug-level message.
#define LOG_D(Message, ...) ::ZyLog::Write(0, ::ZyLog::Detail::Print(Message ## _Text, ##__VA_ARGS__))

/// \def LOG_I
/// \brief Logs an info-level message.
#define LOG_I(Message, ...) ::ZyLog::Write(1, ::ZyLog::Detail::Print(Message ## _Text, ##__VA_ARGS__))

/// \def LOG_W
/// \brief Logs a warning-level message.
#define LOG_W(Message, ...) ::ZyLog::Write(2, ::ZyLog::Detail::Print(Message ## _Text, ##__VA_ARGS__))

/// \def LOG_E
/// \brief Logs an error-level message.
#define LOG_E(Message, ...) ::ZyLog::Write(3, ::ZyLog::Detail::Print(Message ## _Text, ##__VA_ARGS__))
}