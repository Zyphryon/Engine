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
#include <quill/Logger.h>
#include <quill/LogMacros.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   DATA   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Log::_
{
    /// \brief The global logger instance used for logging throughout the application.
    ///
    /// This logger is initialized elsewhere and should be used for all logging operations.
    extern Ptr<quill::Logger> sInstance;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Log
{
    /// \brief Initializes the logging system with the specified configuration file.
    ///
    /// \param Filename The path to the logging configuration file.
    void Initialize(ConstStr8 Filename);

    /// \brief Shuts down the logging system, releasing all resources.
    void Shutdown();

/// \def LOG_DEBUG
/// \brief Logs a debug-level message.
#define LOG_DEBUG(Format, ...)    QUILL_LOG_DEBUG(Log::_::sInstance, Format, ##__VA_ARGS__)

/// \def LOG_INFO
/// \brief Logs an informational message.
#define LOG_INFO(Format, ...)     QUILL_LOG_INFO(Log::_::sInstance, Format, ##__VA_ARGS__)

/// \def LOG_WARNING
/// \brief Logs a warning-level message.
#define LOG_WARNING(Format, ...)  QUILL_LOG_WARNING(Log::_::sInstance, Format, ##__VA_ARGS__)

/// \def LOG_ERROR
/// \brief Logs an error-level message.
#define LOG_ERROR(Format, ...)    QUILL_LOG_ERROR(Log::_::sInstance, Format, ##__VA_ARGS__)

/// \def LOG_CRITICAL
/// \brief Logs a critical-level message.
#define LOG_CRITICAL(Format, ...) QUILL_LOG_CRITICAL(Log::_::sInstance, Format, ##__VA_ARGS__)

/// \def LOG_ASSERT
/// \brief Logs an assertion failure as a critical message if the condition fails.
#if defined(_DEBUG)
    #define LOG_ASSERT(Condition, Format, ...)                             \
        do {                                                                 \
            if (!(Condition)) {                                             \
                LOG_CRITICAL("Assertion failed: " Format, ##__VA_ARGS__);  \
                ZYPHRYON_DEBUG_BREAK();                                      \
            }                                                                \
        } while (false)
#else
    #define LOG_ASSERT(Condition, Format, ...) ((void) 0)
#endif
}