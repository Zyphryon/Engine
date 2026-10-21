## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
##
## This work is licensed under the terms of the MIT license.
##
## For a copy, see <https://opensource.org/licenses/MIT>.
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

# Applies the engine's standard compiler and linker settings to a target.
#
# Configurations:
#   Debug          Unoptimized and instrumented, with AddressSanitizer under GCC and Clang.
#   RelWithDebInfo Optimized as Release, minus whole-program optimization, plus symbols and frame pointers.
#   Release        Fully optimized and stripped.
#
# Usage:
#   ZyApplyOptions(<target>)
FUNCTION(ZyApplyOptions TARGET)

    TARGET_COMPILE_FEATURES(${TARGET} PUBLIC cxx_std_23)

    IF    (WIN32)

        TARGET_COMPILE_DEFINITIONS(${TARGET} PRIVATE _WIN32_WINNT=0x0A00 _CRT_SECURE_NO_WARNINGS NOMINMAX UNICODE)

    ENDIF ()

    IF    (EMSCRIPTEN)

        TARGET_COMPILE_OPTIONS(${TARGET} PRIVATE
            $<$<CONFIG:Release>:
                -O3
            >
            $<$<CONFIG:RelWithDebInfo>:
                -O2
                -g
            >
            $<$<CONFIG:Debug>:
                -O0
                -g
            >
            $<$<BOOL:${ZY_WEB_THREAD}>:
                -pthread
            >
            -fno-exceptions
            -fno-rtti
            -msimd128
        )

        TARGET_LINK_OPTIONS(${TARGET} PRIVATE
            -sALLOW_MEMORY_GROWTH=1
            -sSTACK_SIZE=2MB
            -sINITIAL_MEMORY=256MB
            -sMAXIMUM_MEMORY=1GB
            -sMIN_WEBGL_VERSION=2
            -sMAX_WEBGL_VERSION=2
            -sUSE_WEBGL2=1
            -sFETCH=1
            $<$<CONFIG:Debug>:
                -sASSERTIONS=2
                -sSAFE_HEAP=1
                -sSTACK_OVERFLOW_CHECK=2
                -g
                -gsource-map
                -sGL_DEBUG=1
            >
            $<$<CONFIG:RelWithDebInfo>:
                -g
                -gsource-map
                -sASSERTIONS=0
                -sSTACK_OVERFLOW_CHECK=0
            >
            $<$<CONFIG:Release>:
                --closure 1
                -sASSERTIONS=0
                -sSTACK_OVERFLOW_CHECK=0
            >
            $<$<BOOL:${ZY_WEB_THREAD}>:
                -pthread
                -sWASM_WORKERS=1
                -sAUDIO_WORKLET=1
                -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency
            >
        )

        IF    (ZY_WEB_THREAD)

            TARGET_COMPILE_DEFINITIONS(${TARGET} PRIVATE ZY_WEB_THREAD)

        ENDIF ()

    ELSEIF (MSVC)

        TARGET_COMPILE_OPTIONS(${TARGET} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:
                /O2
                /Gw
                /GS-
                /Zc:preprocessor
            >
            $<$<CONFIG:Release>:
                /GL
            >
            $<$<CONFIG:Debug>:
                /Od
                /RTC1
                /sdl
            >
            /GR-
            /Zc:inline
            /arch:SSE4.2
        )

        TARGET_LINK_OPTIONS(${TARGET} PRIVATE
            $<$<CONFIG:Release>:
                /LTCG
                /OPT:REF
                /OPT:ICF
            >
            $<$<CONFIG:Debug,RelWithDebInfo>: /DEBUG>
        )

        # Symbols go into each object rather than one shared database, so parallel compiles never queue on its writer.
        SET_TARGET_PROPERTIES(${TARGET} PROPERTIES
            MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")

    ELSEIF (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

        # CMake adds -flto to compile and link, and archives with gcc-ar or llvm-ar so the index covers the IR.
        SET_TARGET_PROPERTIES(${TARGET} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

        TARGET_COMPILE_OPTIONS(${TARGET} PRIVATE
            $<$<CONFIG:Release,RelWithDebInfo>:
                -O3
                -funroll-loops
            >
            $<$<CONFIG:Release>:
                -fomit-frame-pointer
            >
            $<$<CONFIG:RelWithDebInfo>:
                -g
                -fno-omit-frame-pointer
            >
            $<$<CONFIG:Debug>:
                -O0
                -g3
                -fsanitize=address
                -fno-omit-frame-pointer
            >
            -fno-exceptions
            -fno-rtti
            -msse4.2
            -fvisibility=hidden
        )

        TARGET_LINK_OPTIONS(${TARGET} PRIVATE
            $<$<CONFIG:Release>:
                -Wl,--gc-sections
                -Wl,--strip-all
            >
            $<$<CONFIG:RelWithDebInfo>:
                -Wl,--gc-sections
            >
            $<$<CONFIG:Debug>: -fsanitize=address>
        )

        IF    (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

            TARGET_COMPILE_OPTIONS(${TARGET} PRIVATE -Wno-changes-meaning)

        ENDIF ()

    ENDIF ()

ENDFUNCTION()