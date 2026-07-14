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

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Plugins
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

INCLUDE(FetchContent)
INCLUDE(CMakeModules/CMakeApplyCompileOptions.cmake)
INCLUDE(CMakeModules/CMakeApplyModules.cmake)
INCLUDE(CMakeModules/CMakeRegisterModule.cmake)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Options
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

OPTION(ZY_AUDIO_DRIVER_MINIAUDIO    "Audio: Enable Miniaudio as audio backend" OFF)
OPTION(ZY_AUDIO_LOADER_WAV          "Audio: Enable WAV audio loader"           OFF)
OPTION(ZY_AUDIO_LOADER_MP3          "Audio: Enable MP3 audio loader"           OFF)

OPTION(ZY_GRAPHIC_DRIVER_D3D11      "Graphic: Enable D3D11 as graphic backend" ${WIN32})
OPTION(ZY_GRAPHIC_DRIVER_GLES3      "Graphic: Enable GLES3 as graphic backend" $<NOT:$<BOOL:${WIN32}>>)
OPTION(ZY_GRAPHIC_LOADER_STB        "Graphic: Enable STB image loader"         OFF)

OPTION(ZY_RENDER_LOADER_ARTERY      "Render: Enable Artery font loader"        OFF)

## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Module
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ADD_SUBDIRECTORY(Engine)

IF    (ZY_AUDIO_DRIVER_MINIAUDIO)

    ADD_SUBDIRECTORY(Module/Audio/Driver/Miniaudio)   ## TODO: Eventually implement my own audio mixin in the core.

ENDIF ()

IF    (ZY_AUDIO_LOADER_WAV)

    ADD_SUBDIRECTORY(Module/Audio/Loader/WAV)

ENDIF ()

IF    (ZY_AUDIO_LOADER_MP3)

    ADD_SUBDIRECTORY(Module/Audio/Loader/MP3)

ENDIF ()

IF    (ZY_GRAPHIC_DRIVER_D3D11)

    ADD_SUBDIRECTORY(Module/Graphic/Driver/D3D11)

ENDIF ()

IF    (ZY_GRAPHIC_DRIVER_GLES3)

    ADD_SUBDIRECTORY(Module/Graphic/Driver/GLES3)

ENDIF ()

IF    (ZY_GRAPHIC_LOADER_STB)

    ADD_SUBDIRECTORY(Module/Graphic/Loader/STB)

ENDIF ()

IF    (ZY_RENDER_LOADER_ARTERY)

    ADD_SUBDIRECTORY(Module/Render/Loader/Artery)

ENDIF ()

ADD_SUBDIRECTORY(Tutorial)