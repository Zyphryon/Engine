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

PROJECT(ZyBakerFont VERSION 1.0.0 LANGUAGES CXX)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Target
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

IF    (ZY_BUILD_PIPELINE_BAKER_LIBRARY)

    ZyAddTarget(${PROJECT_NAME} STATIC EXCLUDE "/Main\\.cpp$")

ELSE  ()

    ZyAddTarget(${PROJECT_NAME} EXECUTABLE)

ENDIF ()

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (Zyphryon Engine)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC "ZyEngine")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (STB) (PUBLIC DOMAIN)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC
        "STB_IMAGE_STATIC"
        "STBI_NO_STDIO"
        "STBI_ONLY_PNG"
        "STBI_ONLY_JPEG"
        "STBI_ONLY_TGA"
        "STBI_ONLY_BMP"
        "STBI_ONLY_HDR"
        "STBI_MAX_DIMENSIONS=65535")

FetchContent_Declare(External_STB
        GIT_REPOSITORY https://github.com/nothings/stb
        GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(External_STB)

IF (NOT TARGET STB)

    ADD_LIBRARY(STB INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(STB INTERFACE ${external_stb_SOURCE_DIR})

ENDIF ()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE "STB")

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Dependency (MSDFGEN) (MIT)
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

SET(MSDFGEN_CORE_ONLY        ON  CACHE BOOL "" FORCE)
SET(MSDFGEN_DYNAMIC_RUNTIME  ON  CACHE BOOL "" FORCE)
SET(MSDFGEN_BUILD_STANDALONE OFF CACHE BOOL "" FORCE)
SET(MSDFGEN_USE_VCPKG        OFF CACHE BOOL "" FORCE)
SET(MSDFGEN_USE_SKIA         OFF CACHE BOOL "" FORCE)
SET(MSDFGEN_INSTALL          OFF CACHE BOOL "" FORCE)
SET(MSDFGEN_DISABLE_SVG      ON  CACHE BOOL "" FORCE)
SET(MSDFGEN_DISABLE_PNG      ON  CACHE BOOL "" FORCE)

FetchContent_Declare(External_MSDFGEN
        GIT_REPOSITORY https://github.com/Chlumsky/msdfgen
        GIT_SHALLOW    TRUE
        GIT_TAG        v1.12)
FetchContent_MakeAvailable(External_MSDFGEN)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE "msdfgen::msdfgen")