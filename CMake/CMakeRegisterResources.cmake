## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
##
## This work is licensed under the terms of the MIT license.
##
## For a copy, see <https://opensource.org/licenses/MIT>.
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

# Registers a resource directory that ZyApplyResources(<app>) embeds into each consumer application.
#
# Usage:
#   ZyRegisterResources(TARGET <target> DIRECTORY <dir> [PREFIX <prefix>])
FUNCTION(ZyRegisterResources)

    SET(OPTIONS)
    SET(ONE_VALUE_ARGS TARGET DIRECTORY PREFIX)
    SET(MULTI_VALUE_ARGS)

    CMAKE_PARSE_ARGUMENTS(ARG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    IF(NOT ARG_TARGET)

        MESSAGE(FATAL_ERROR "ZyRegisterResources: TARGET is required.")

    ENDIF()

    IF(NOT ARG_DIRECTORY)

        MESSAGE(FATAL_ERROR "ZyRegisterResources: DIRECTORY is required.")

    ENDIF()

    GET_FILENAME_COMPONENT(ABSOLUTE "${ARG_DIRECTORY}" ABSOLUTE)

    SET_PROPERTY(GLOBAL APPEND PROPERTY ZY_EMBEDDED "${ARG_PREFIX}|${ABSOLUTE}")

ENDFUNCTION()