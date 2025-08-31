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

#include "Primitive.hpp"
#include <quill/Logger.h>
#include <quill/LogMacros.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   DATA   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Log::_
{
    /// \brief The default global logger instance used by logging macros.
    /// 
    /// This pointer is initialized by `Log::Initialize()` and released by `Log::Shutdown()`.
    /// All log macros route their output through this logger.
    extern Ptr<quill::Logger> sLogger;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Log
{
    /// \brief Initializes the logging system with the specified output file.
    /// 
    /// \param Filename The output file path for log messages.
    void Initialize(ConstText Filename);

    /// \brief Shuts down the logging system.
    void Shutdown();

/// \def LOG_DEBUG
/// \brief Logs a debug-level message.
#define LOG_DEBUG(Format, ...)    QUILL_LOG_DEBUG(Log::_::sLogger, Format, ##__VA_ARGS__)

/// \def LOG_INFO
/// \brief Logs an informational message.
#define LOG_INFO(Format, ...)     QUILL_LOG_INFO(Log::_::sLogger, Format, ##__VA_ARGS__)

/// \def LOG_WARNING
/// \brief Logs a warning-level message.
#define LOG_WARNING(Format, ...)  QUILL_LOG_WARNING(Log::_::sLogger, Format, ##__VA_ARGS__)

/// \def LOG_ERROR
/// \brief Logs an error-level message.
#define LOG_ERROR(Format, ...)    QUILL_LOG_ERROR(Log::_::sLogger, Format, ##__VA_ARGS__)

/// \def LOG_CRITICAL
/// \brief Logs a critical-level message.
#define LOG_CRITICAL(Format, ...) QUILL_LOG_CRITICAL(Log::_::sLogger, Format, ##__VA_ARGS__)

/// \def LOG_ASSERT
/// \brief Logs an assertion failure as a critical message if the condition fails.
#if defined(_DEBUG)
    #define LOG_ASSERT(Condition, Format, ...) \
        if (!(Condition)) { LOG_CRITICAL("Assertion failed: " Format, ##__VA_ARGS__); std::abort(); }
#else
    #define LOG_ASSERT(Condition, Format, ...)
#endif
}