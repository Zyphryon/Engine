# CMakeApplyCompileOptions.cmake
#
# Applies the engine's standard compiler and linker settings to a target.
#
# Configurations:
#   Debug          Unoptimized, instrumented, sanitized.
#   RelWithDebInfo Optimized as Release, minus whole-program optimization, plus symbols and frame pointers.
#   Release        Fully optimized and stripped.
#
# Usage:
#   ZyApplyOptions(<target>)
FUNCTION(ZyApplyOptions TARGET)

    TARGET_COMPILE_FEATURES(${TARGET} PUBLIC cxx_std_23)

    IF    (WIN32)

        TARGET_COMPILE_DEFINITIONS(${TARGET} PRIVATE _WIN32_WINNT=0x0A00 _CRT_SECURE_NO_WARNINGS NOMINMAX UNICODE)

    ENDIF (WIN32)

    IF     (EMSCRIPTEN)

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

        IF (ZY_WEB_THREAD)
            TARGET_COMPILE_DEFINITIONS(${TARGET} PRIVATE ZY_WEB_THREAD)
        ENDIF  ()

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
            $<$<CONFIG:Debug,RelWithDebInfo>:
                /Zi
            >
            /GR-
            /Zc:inline
            /arch:SSE4.2
        )

        TARGET_LINK_OPTIONS(${TARGET} PRIVATE
            $<$<CONFIG:Release>:
                /LTCG
            >
            $<$<CONFIG:Release,RelWithDebInfo>:
                /OPT:REF
                /OPT:ICF
            >
            $<$<CONFIG:Debug,RelWithDebInfo>: /DEBUG>
        )

    ELSEIF (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

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
                -flto=auto
                -Wl,--gc-sections
                -Wl,--strip-all
            >
            $<$<CONFIG:RelWithDebInfo>:
                -Wl,--gc-sections
            >
            $<$<CONFIG:Debug>: -fsanitize=address>
        )

        IF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

            TARGET_COMPILE_OPTIONS(${TARGET} PRIVATE -fno-fat-lto-objects -Wno-changes-meaning)

        ENDIF()

    ENDIF  ()

ENDFUNCTION()
