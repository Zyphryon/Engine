# CMakeEmbedResources.cmake
#
# Registers a module/plugin resource directory to be embedded into consumer applications.
#
# Usage:
#   ZyEmbedResources(TARGET <target> DIRECTORY <dir> [PREFIX <prefix>])
#
# Each registered directory is materialized by ZyApplyEmbedded(<app>)
FUNCTION(ZyEmbedResources)

    SET(OPTIONS)
    SET(ONE_VALUE_ARGS TARGET DIRECTORY PREFIX)
    SET(MULTI_VALUE_ARGS)

    CMAKE_PARSE_ARGUMENTS(ARG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    IF(NOT ARG_TARGET)

        MESSAGE(FATAL_ERROR "ZyEmbedResources: TARGET is required.")

    ENDIF()

    IF(NOT ARG_DIRECTORY)

        MESSAGE(FATAL_ERROR "ZyEmbedResources: DIRECTORY is required.")

    ENDIF()

    GET_FILENAME_COMPONENT(ABSOLUTE "${ARG_DIRECTORY}" ABSOLUTE)

    SET_PROPERTY(GLOBAL APPEND PROPERTY ZY_EMBEDDED "${ARG_PREFIX}|${ABSOLUTE}")

ENDFUNCTION()
