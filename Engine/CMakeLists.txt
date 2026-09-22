## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
##
## This work is licensed under the terms of the MIT license.
##
## For a copy, see <https://opensource.org/licenses/MIT>.
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

CMAKE_MINIMUM_REQUIRED(VERSION 3.26)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Project
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

PROJECT(ZyEngine VERSION 1.0.0 LANGUAGES CXX)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Code
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

IF    (ZY_MODE_HEADLESS)

    ZyAddTarget(${PROJECT_NAME} STATIC PCH "Source/Zyphryon.Base/Base.hpp"
            EXCLUDE "/Zyphryon\\.(Audio|Graphic|Input|Render)/"
                    "/Zyphryon\\.Platform/(Dispatcher|Display|Monitor|Service|Window)\\.cpp$")

    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC "ZY_MODE_HEADLESS")

ELSE  ()

    ZyAddTarget(${PROJECT_NAME} STATIC PCH "Source/Zyphryon.Base/Base.hpp")

ENDIF ()

IF    (WIN32)

    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC "shcore" "avrt" "ole32" "ws2_32" "bcrypt")

ELSEIF (EMSCRIPTEN)

    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC "websocket.js")

ELSEIF (UNIX AND NOT APPLE AND NOT EMSCRIPTEN)

    IF    (NOT ZY_MODE_HEADLESS)

        FIND_PACKAGE(X11 REQUIRED)

        IF    (NOT X11_Xrandr_FOUND)

            MESSAGE(FATAL_ERROR "Zyphryon needs the RandR extension for monitor enumeration (install libxrandr-dev)")

        ENDIF ()

        FIND_PACKAGE(ALSA REQUIRED)

        TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ALSA::ALSA X11::X11 X11::Xrandr)

    ENDIF ()

ENDIF ()

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Tracy) (BSD-3-Clause)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

OPTION(ZY_PROFILE_BACKEND_TRACY "Use Tracy profiler" OFF)

IF    (ZY_PROFILE_BACKEND_TRACY)

    SET(TRACY_ENABLE     ON  CACHE BOOL "" FORCE)
    SET(TRACY_ON_DEMAND  OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(External_Tracy
            GIT_REPOSITORY https://github.com/wolfpld/tracy
            GIT_SHALLOW    TRUE
            GIT_TAG        v0.13.1)
    FetchContent_MakeAvailable(External_Tracy)

    ZyApplyOptions("TracyClient")

    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC "ZY_PROFILE_BACKEND_TRACY")
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}      PUBLIC "TracyClient")

ENDIF ()

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Opus) (BSD-3-Clause) TODO: Remove
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

IF    (NOT ZY_MODE_HEADLESS)

    SET(OPUS_BUILD_SHARED_LIBRARY        OFF CACHE BOOL "" FORCE)
    SET(OPUS_BUILD_PROGRAMS              OFF CACHE BOOL "" FORCE)
    SET(OPUS_BUILD_TESTING               OFF CACHE BOOL "" FORCE)
    SET(OPUS_INSTALL_PKG_CONFIG_MODULE   OFF CACHE BOOL "" FORCE)
    SET(OPUS_INSTALL_CMAKE_CONFIG_MODULE OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(External_Opus
            GIT_REPOSITORY https://github.com/xiph/opus
            GIT_SHALLOW    TRUE
            GIT_TAG        v1.5.2)
    FetchContent_MakeAvailable(External_Opus)

    ZyApplyOptions("opus")

    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE "opus")

ENDIF ()

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Flecs) (MIT) TODO: Remove
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY ZY_SHARED_WHOLE   "flecs_static")
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY ZY_SHARED_EXPORTS "flecs_EXPORTS")

OPTION(FLECS_REST_SERVICE "Enable Flecs rest service" OFF)

SET(FLECS_STATIC                    ON  CACHE BOOL "" FORCE)
SET(FLECS_SHARED                    OFF CACHE BOOL "" FORCE)
SET(FLECS_CUSTOM_BUILD              ON  CACHE BOOL "" FORCE)
SET(FLECS_CPP                       OFF CACHE BOOL "" FORCE)
SET(FLECS_DOC                       ON  CACHE BOOL "" FORCE)
SET(FLECS_OS_API_IMPL               ON  CACHE BOOL "" FORCE)
SET(FLECS_PIPELINE                  ON  CACHE BOOL "" FORCE)
SET(FLECS_ENTITY_RANGES             ON  CACHE BOOL "" FORCE)
SET(FLECS_CACHED_QUERIES            ON  CACHE BOOL "" FORCE)
SET(FLECS_CONSTRAINT_TRAITS         ON  CACHE BOOL "" FORCE)
SET(FLECS_SYSTEM                    ON  CACHE BOOL "" FORCE)
SET(FLECS_PREFAB                    ON  CACHE BOOL "" FORCE)
SET(FLECS_TIMER                     ON  CACHE BOOL "" FORCE)

IF    (ZY_PROFILE_BACKEND_TRACY)
    SET(FLECS_LOG                   ON CACHE BOOL "" FORCE)
ENDIF ()

IF    (FLECS_REST_SERVICE)
    SET(FLECS_METRICS               ON CACHE BOOL "" FORCE)
    SET(FLECS_HTTP                  ON CACHE BOOL "" FORCE)
    SET(FLECS_REST                  ON CACHE BOOL "" FORCE)
    SET(FLECS_STATS                 ON CACHE BOOL "" FORCE)

    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC "FLECS_REST_SERVICE")
ENDIF ()

FetchContent_Declare(External_Flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs
        GIT_TAG        1fca290cf0b155c93f85e5857ff2749e292b2988)
FetchContent_MakeAvailable(External_Flecs)

ZyApplyOptions(flecs_static)

TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC "FLECS_NO_CPP")
TARGET_LINK_LIBRARIES(${PROJECT_NAME}      PUBLIC "flecs_static")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Resources
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ZyRegisterResources(TARGET ${PROJECT_NAME} DIRECTORY "Resources")