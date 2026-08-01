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

PROJECT(Zyphryon VERSION 1.0.0)

IF    (PROJECT_IS_TOP_LEVEL)

    SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    ## Multi-config generators append the configuration unless each variant is pinned to the same folder.
    FOREACH(CONFIG IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)

        SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/bin)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/bin)
        SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/bin)

    ENDFOREACH()

ENDIF ()

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Plugins
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

INCLUDE(FetchContent)
INCLUDE(CMake/CMakeApplyModules.cmake)
INCLUDE(CMake/CMakeApplyOptions.cmake)
INCLUDE(CMake/CMakeApplyResources.cmake)
INCLUDE(CMake/CMakeRegisterModule.cmake)
INCLUDE(CMake/CMakeRegisterResources.cmake)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Engine
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

OPTION(ZY_WEB_THREAD           "Core: Uses thread on Emscripten"          OFF)

ADD_SUBDIRECTORY(Engine)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Modules
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

OPTION(ZY_GRAPHIC_DRIVER_D3D11 "Graphic: Enable D3D11 as graphic backend" ${WIN32})
OPTION(ZY_GRAPHIC_DRIVER_GLES3 "Graphic: Enable GLES3 as graphic backend" ${UNIX})
OPTION(ZY_GRAPHIC_LOADER_STB   "Graphic: Enable STB image loader"         OFF)

IF    (ZY_GRAPHIC_DRIVER_D3D11)

    ADD_SUBDIRECTORY(Module/Graphic/Driver/D3D11)

ENDIF ()

IF    (ZY_GRAPHIC_DRIVER_GLES3)

    ADD_SUBDIRECTORY(Module/Graphic/Driver/GLES3)

ENDIF ()

IF    (ZY_GRAPHIC_LOADER_STB)

    ADD_SUBDIRECTORY(Module/Graphic/Loader/STB)

ENDIF ()

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Pipeline
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ADD_SUBDIRECTORY(Pipeline)