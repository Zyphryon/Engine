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

FILE(GLOB_RECURSE PROJECT_SOURCE  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp")
LIST(APPEND       PROJECT_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/Source")

IF    (WIN32)

    LIST(APPEND PROJECT_DEPENDENCIES "shcore" "avrt" "ole32")

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

    ZyApplyCompileOptions("TracyClient")

    LIST(APPEND PROJECT_DEFINES      "ZY_PROFILE_BACKEND_TRACY")
    LIST(APPEND PROJECT_DEPENDENCIES "TracyClient")

ENDIF ()

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Flecs) (MIT) -> TODO: Remove
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

OPTION(FLECS_REST_SERVICE "Enable Flecs rest service" OFF)

SET(FLECS_STATIC                    ON  CACHE BOOL "" FORCE)
SET(FLECS_SHARED                    OFF CACHE BOOL "" FORCE)
SET(FLECS_CUSTOM_BUILD              ON  CACHE BOOL "" FORCE)
SET(FLECS_CPP_NO_AUTO_REGISTRATION  ON  CACHE BOOL "" FORCE)
SET(FLECS_CPP_NO_ENUM_REFLECTION    ON  CACHE BOOL "" FORCE)
SET(FLECS_CPP                       ON  CACHE BOOL "" FORCE)
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
    SET(FLECS_METRICS                ON CACHE BOOL "" FORCE)
    SET(FLECS_HTTP                   ON CACHE BOOL "" FORCE)
    SET(FLECS_REST                   ON CACHE BOOL "" FORCE)
    SET(FLECS_STATS                  ON CACHE BOOL "" FORCE)

    LIST(APPEND PROJECT_DEFINES "FLECS_REST_SERVICE")
ENDIF ()

FetchContent_Declare(External_Flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs
        GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(External_Flecs)

ZyApplyCompileOptions(flecs_static)

LIST(APPEND PROJECT_DEPENDENCIES "flecs_static")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## DLL
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ADD_LIBRARY(${PROJECT_NAME} STATIC ${PROJECT_SOURCE})

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## PCH
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} PUBLIC "Source/Zyphryon.Base/Base.hpp")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Libraries
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES})

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Definitions
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC ${PROJECT_DEFINES})

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Includes
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Properties
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ZyApplyCompileOptions(${PROJECT_NAME})
ZyEmbedResources(TARGET ${PROJECT_NAME} DIRECTORY "Resources")