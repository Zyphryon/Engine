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

PROJECT(Zyphryon_Engine VERSION 1.0.0 LANGUAGES CXX)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Option
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

OPTION(ZYPHRYON_PROFILE_MODE "Enable Profile Mode" OFF)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Code
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

FILE(GLOB_RECURSE PROJECT_SOURCE "Public/*.cpp" "Private/*.cpp")

IF    (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    LIST(FILTER PROJECT_SOURCE EXCLUDE REGEX ".*D3D11.*")
ENDIF (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Includes
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

LIST(APPEND PROJECT_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Public ${CMAKE_CURRENT_SOURCE_DIR}/Private)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (SDL3) (PUBLIC DOMAIN)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

SET(SDL_SHARED             OFF CACHE BOOL "" FORCE)
SET(SDL_STATIC             ON  CACHE BOOL "" FORCE)
SET(SDL_TEST_LIBRARY       OFF CACHE BOOL "" FORCE)
SET(SDL_DISABLE_UNINSTALL  ON  CACHE BOOL "" FORCE)
SET(SDL_AUDIO_DEFAULT      ON  CACHE BOOL "" FORCE)
SET(SDL_VIDEO_DEFAULT      ON  CACHE BOOL "" FORCE)
SET(SDL_JOYSTICK_DEFAULT   OFF CACHE BOOL "" FORCE)
SET(SDL_HAPTIC_DEFAULT     OFF CACHE BOOL "" FORCE)
SET(SDL_HIDAPI_DEFAULT     ON  CACHE BOOL "" FORCE)
SET(SDL_POWER_DEFAULT      OFF CACHE BOOL "" FORCE)
SET(SDL_SENSOR_DEFAULT     OFF CACHE BOOL "" FORCE)
SET(SDL_DIALOG_DEFAULT     ON  CACHE BOOL "" FORCE)
SET(SDL_CAMERA_DEFAULT     OFF CACHE BOOL "" FORCE)

FetchContent_Declare(External_SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG main)
FetchContent_MakeAvailable(External_SDL)

LIST(APPEND PROJECT_DEPENDENCIES "SDL3-static")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Flecs) (MIT)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

OPTION(FLECS_REST_SERVICE "Enable Flecs rest service" OFF)

SET(FLECS_STATIC  ON  CACHE BOOL "" FORCE)
SET(FLECS_SHARED  OFF CACHE BOOL "" FORCE)

LIST(APPEND PROJECT_DEFINES
        "FLECS_CUSTOM_BUILD"
        "FLECS_CPP_NO_AUTO_REGISTRATION"
        "FLECS_CPP_NO_ENUM_REFLECTION"
        "FLECS_CPP"
        "FLECS_DOC"
        "FLECS_OS_API_IMPL"
        "FLECS_PIPELINE"
        "FLECS_SYSTEM"
        "FLECS_TIMER")

IF    (ZYPHRYON_PROFILE_MODE)
    LIST(APPEND PROJECT_DEFINES "FLECS_LOG")
ENDIF (ZYPHRYON_PROFILE_MODE)

IF    (FLECS_REST_SERVICE)
    LIST(APPEND PROJECT_DEFINES "FLECS_METRICS" "FLECS_HTTP" "FLECS_REST" "FLECS_STATS")
ENDIF (FLECS_REST_SERVICE)

FetchContent_Declare(External_Flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs)
FetchContent_MakeAvailable(External_Flecs)

LIST(APPEND PROJECT_DEPENDENCIES "flecs_static")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Tracy) (BSD-3-Clause)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

IF    (ZYPHRYON_PROFILE_MODE)

    SET(TRACY_ENABLE     ON  CACHE BOOL "" FORCE)
    SET(TRACY_ON_DEMAND  OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(External_Tracy
            GIT_REPOSITORY https://github.com/wolfpld/tracy
            GIT_TAG 05cceee0df3b8d7c6fa87e9638af311dbabc63cb)
    FetchContent_MakeAvailable(External_Tracy)

    LIST(APPEND PROJECT_DEFINES      "ZYPHRYON_PROFILE_MODE")
    LIST(APPEND PROJECT_DEPENDENCIES "TracyClient")

ENDIF ()

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Miniaudio) (PUBLIC DOMAIN)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

SET(MINIAUDIO_NO_DECODING         ON CACHE BOOL   "" FORCE)
SET(MINIAUDIO_NO_ENCODING         ON CACHE BOOL   "" FORCE)
SET(MINIAUDIO_NO_WAV              ON CACHE BOOL   "" FORCE)
SET(MINIAUDIO_NO_FLAC             ON CACHE BOOL   "" FORCE)
SET(MINIAUDIO_NO_MP3              ON CACHE BOOL   "" FORCE)
SET(MINIAUDIO_NO_RESOURCE_MANAGER ON CACHE BOOL   "" FORCE)

FetchContent_Declare(External_Miniaudio
        GIT_REPOSITORY https://github.com/mackron/miniaudio)
FetchContent_MakeAvailable(External_Miniaudio)

LIST(APPEND PROJECT_DEPENDENCIES "miniaudio")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Enchantum) (MIT) -> TODO: Remove
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

FetchContent_Declare(External_Enchantum
        GIT_REPOSITORY https://github.com/ZXShady/enchantum
        GIT_TAG main)
FetchContent_MakeAvailable(External_Enchantum)

LIST(APPEND PROJECT_DEPENDENCIES "enchantum")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (TOML) (MIT)    -> TODO: Remove and implement own JSON parser
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ADD_COMPILE_DEFINITIONS(TOML_EXCEPTIONS=0)

FetchContent_Declare(External_TOML
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus)
FetchContent_MakeAvailable(External_TOML)

ADD_LIBRARY(Toml INTERFACE)
TARGET_INCLUDE_DIRECTORIES(Toml INTERFACE ${external_toml_SOURCE_DIR})

LIST(APPEND PROJECT_DEPENDENCIES "Toml")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Quill) (MIT) -> TODO: Remove
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

SET(QUILL_NO_THREAD_NAME_SUPPORT      ON CACHE BOOL "" FORCE)
SET(QUILL_DISABLE_NON_PREFIXED_MACROS ON CACHE BOOL "" FORCE)

ADD_COMPILE_DEFINITIONS(
        $<$<CONFIG:Debug>:QUILL_COMPILE_ACTIVE_LOG_LEVEL=QUILL_COMPILE_ACTIVE_LOG_LEVEL_DEBUG>
        $<$<NOT:$<CONFIG:Debug>>:QUILL_COMPILE_ACTIVE_LOG_LEVEL=QUILL_COMPILE_ACTIVE_LOG_LEVEL_INFO>)

FetchContent_Declare(External_Quill
        GIT_REPOSITORY https://github.com/odygrd/quill
        GIT_TAG master)
FetchContent_MakeAvailable(External_Quill)

LIST(APPEND PROJECT_DEPENDENCIES "quill")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (inplace_vector) (MIT) -> TODO: Remove
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

FetchContent_Declare(External_CXX26InPlaceVector
        GIT_REPOSITORY https://github.com/bemanproject/inplace_vector
        GIT_TAG main)
FetchContent_MakeAvailable(External_CXX26InPlaceVector)

LIST(APPEND PROJECT_DEPENDENCIES "beman::inplace_vector")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (in-drop replacement for unordered_map and unordered_set) (MIT) -> TODO: Remove
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

FetchContent_Declare(External_Ankerl
        GIT_REPOSITORY https://github.com/martinus/unordered_dense
        GIT_TAG main)
FetchContent_MakeAvailable(External_Ankerl)

ADD_LIBRARY(Ankerl INTERFACE)
TARGET_INCLUDE_DIRECTORIES(Ankerl INTERFACE ${external_ankerl_SOURCE_DIR}/include/ankerl)

LIST(APPEND PROJECT_DEPENDENCIES "Ankerl")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (GLTF) (MIT) -> TODO: Remove
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ADD_COMPILE_DEFINITIONS(TINYGLTF_NO_STB_IMAGE_WRITE)

FetchContent_Declare(External_GLTF
        GIT_REPOSITORY https://github.com/syoyo/tinygltf
        GIT_TAG release)
FetchContent_MakeAvailable(External_GLTF)

ADD_LIBRARY(GLTF INTERFACE)
TARGET_INCLUDE_DIRECTORIES(GLTF INTERFACE ${external_gltf_SOURCE_DIR})

LIST(APPEND PROJECT_DEPENDENCIES "GLTF")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (BMP / PNG / JPG / TGA) (PUBLIC_DOMAIN)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

LIST(APPEND PROJECT_DEFINES "STBI_NO_PSD")
LIST(APPEND PROJECT_DEFINES "STBI_NO_GIF")
LIST(APPEND PROJECT_DEFINES "STBI_NO_HDR")
LIST(APPEND PROJECT_DEFINES "STBI_NO_PIC")
LIST(APPEND PROJECT_DEFINES "STBI_NO_PNM")
LIST(APPEND PROJECT_DEFINES "STBI_NO_STDIO")
LIST(APPEND PROJECT_DEFINES "STB_IMAGE_STATIC")

FetchContent_Declare(External_STB
        GIT_REPOSITORY https://github.com/nothings/stb)
FetchContent_MakeAvailable(External_STB)

ADD_LIBRARY(STB INTERFACE)
TARGET_INCLUDE_DIRECTORIES(STB INTERFACE ${external_stb_SOURCE_DIR})

LIST(APPEND PROJECT_DEPENDENCIES "STB")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (WAV / MP3) (PUBLIC_DOMAIN)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

LIST(APPEND PROJECT_DEFINES "DR_MP3_FLOAT_OUTPUT")
LIST(APPEND PROJECT_DEFINES "DR_MP3_NO_STDIO")
LIST(APPEND PROJECT_DEFINES "DR_WAV_NO_STDIO")

FetchContent_Declare(External_DRLIBS
        GIT_REPOSITORY https://github.com/mackron/dr_libs)
FetchContent_MakeAvailable(External_DRLIBS)

ADD_LIBRARY(DrLibs INTERFACE)
TARGET_INCLUDE_DIRECTORIES(DrLibs INTERFACE ${external_drlibs_SOURCE_DIR})

LIST(APPEND PROJECT_DEPENDENCIES "DrLibs")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Library
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ADD_LIBRARY(${PROJECT_NAME} STATIC ${PROJECT_SOURCE})

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Embed Resources
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

CMAKE_EMBED_RESOURCES(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/Resources" "Zyphryon::RC" "ZyphryonRC")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Definitions
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC ${PROJECT_DEFINES})

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Libraries
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES})

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Includes
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## PCH
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Public/Zyphryon.Base/Base.hpp)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Properties
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_COMPILE_FEATURES(${PROJECT_NAME} PUBLIC cxx_std_23)

IF    (WIN32)

    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601 _CRT_SECURE_NO_WARNINGS)

ENDIF (WIN32)

IF     (MSVC)

    TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>: /O2 /fp:fast /Oy /Ob2 /Gw /GL /arch:SSE4.2>
            $<$<NOT:$<CONFIG:Release>>: /Od /RTC1>
    )

    TARGET_LINK_OPTIONS(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>: /LTCG>
    )

ELSEIF (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

    TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>: -O3 -ffast-math -funroll-loops -fomit-frame-pointer -msse4.2>
            $<$<NOT:$<CONFIG:Release>>: -O0 -g3>
    )

    TARGET_LINK_OPTIONS(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>: -flto>
    )

ENDIF  ()