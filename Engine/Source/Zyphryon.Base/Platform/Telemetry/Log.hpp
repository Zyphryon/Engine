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

#include "Zyphryon.Base/Lexical/Format/Processor.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Log   // TODO: Proper implementation with sink/timing
{
    namespace Detail
    {
        static constexpr UInt kPrintBufferCapacity = 4096;

        ZY_INLINE Ref<String<kPrintBufferCapacity>> GetPrintBuffer()
        {
            thread_local String<kPrintBufferCapacity> Buffer;
            return Buffer;
        }

        template<typename ...Arguments>
        ZY_INLINE Text Print(AnyRef<Format::Pattern<>> Format, AnyRef<Arguments>... Parameters)
        {
            Ref<String<kPrintBufferCapacity>> Buffer = GetPrintBuffer();
            Buffer.Clear();

            Format::Print(Buffer, Forward<Format::Pattern<>>(Format), Parameters ...);

#ifdef    ZY_PLATFORM_ANDROID

            Buffer.Append('\0');

#endif // ZY_PLATFORM_ANDROID

            return Buffer;
        }
    }

    /// \brief Severity level assigned to a log message, controlling its visibility and routing.
    enum class Priority : UInt8
    {
        Debug,      ///< Detailed diagnostic information.
        Info,       ///< General operational information.
        Warning,    ///< Unexpected events that may indicate problems.
        Error       ///< Serious failures preventing normal operation.
    };

    /// \brief Writes a formatted message to all active log outputs.
    ///
    /// \param Priority The severity level of the message. Controls prefix, color, and routing.
    /// \param Message  The fully formatted message text to emit.
    void Write(Priority Priority, Text Message);

    /// \brief Flushes the logging console.
    void Flush();

/// \def LOG_DEBUG
/// \brief Logs a debug-level message.
#define LOG_DEBUG(Message, ...)   Log::Write(Log::Priority::Debug,   Log::Detail::Print(Message ## _Text, ##__VA_ARGS__))

/// \def LOG_INFO
/// \brief Logs an informational message.
#define LOG_INFO(Message, ...)    Log::Write(Log::Priority::Info,    Log::Detail::Print(Message ## _Text, ##__VA_ARGS__))

/// \def LOG_WARNING
/// \brief Logs a warning-level message.
#define LOG_WARNING(Message, ...) Log::Write(Log::Priority::Warning, Log::Detail::Print(Message ## _Text, ##__VA_ARGS__))

/// \def LOG_ERROR
/// \brief Logs an error-level message.
#define LOG_ERROR(Message, ...)   Log::Write(Log::Priority::Error,   Log::Detail::Print(Message ## _Text, ##__VA_ARGS__))
}