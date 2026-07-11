// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Log.hpp"

#if   defined(ZY_PLATFORM_WINDOWS)
#   include <windows.h>
#elif defined(ZY_PLATFORM_ANDROID)
#   include <android/log.h>
#elif defined(ZY_PLATFORM_WEB)
#   include <emscripten/emscripten.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Log
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool Initialize()
    {
#if   defined(ZY_PLATFORM_WINDOWS)

        const HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD        Mode    = 0;
        GetConsoleMode(Console, AddressOf(Mode));
        SetConsoleMode(Console, Mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

#endif
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Write(Priority Priority, Text Message)
    {
        static Bool _Initialized = Initialize();

#if   defined(ZY_PLATFORM_WINDOWS) || defined(ZY_PLATFORM_LINUX) || defined(ZY_PLATFORM_MACOS)

        static constexpr Text kPrefixes[] =
        {
            "\033[36m[DEBUG] ",
            "\033[37m[INFO] ",
            "\033[33m[WARNING] ",
            "\033[31m[ERROR] "
        };

        const UInt8 Level = static_cast<UInt8>(Priority);

        String<Detail::kPrintBufferCapacity> Output;
        Output.Append(kPrefixes[Level]);
        Output.Append(Message);
        Output.Append("\033[0m\n"_Text);

        fwrite(Output.GetData(), 1, Output.GetSize(), stdout);

        if (Level >= 2)
        {
            fflush(stdout);
        }

#elif defined(ZY_PLATFORM_ANDROID)

        static constexpr Array kPriorities(ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN, ANDROID_LOG_ERROR);
        __android_log_write(kPriorities[static_cast<UInt8>(Priority)], "Zyphryon", Message.GetData());

#elif defined(ZY_PLATFORM_WEB)

        EM_ASM(
        {
            const jsMessage = UTF8ToString($0, $1);
            const jsMethod  = (["debug", "info", "warn", "error"])[$2] || "log";
            console[jsMethod](jsMessage);
        }, Message.GetData(), Message.GetSize(), static_cast<UInt32>(Priority));

#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Flush()
    {
#if   defined(ZY_PLATFORM_WINDOWS) || defined(ZY_PLATFORM_LINUX) || defined(ZY_PLATFORM_MACOS)
        fflush(stdout);
#endif
    }
}