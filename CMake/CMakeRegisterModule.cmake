# CMakeRegisterModule.cmake
#
# Registers an engine module so applications can automatically discover,
# link, and instantiate it.
#
# Usage:
#   ZyRegisterModule(TARGET <target> HEADER <header> CLASS  <class>)
FUNCTION(ZyRegisterModule)

    SET(OPTIONS)
    SET(ONE_VALUE_ARGS TARGET HEADER CLASS)
    SET(MULTI_VALUE_ARGS)

    CMAKE_PARSE_ARGUMENTS(ARG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    IF(NOT ARG_TARGET)

        MESSAGE(FATAL_ERROR "ZyRegisterModule: TARGET is required.")

    ENDIF()

    IF(NOT ARG_HEADER)

        MESSAGE(FATAL_ERROR "ZyRegisterModule: HEADER is required.")

    ENDIF()

    IF(NOT ARG_CLASS)

        MESSAGE(FATAL_ERROR "ZyRegisterModule: CLASS is required.")

    ENDIF()

    SET_PROPERTY(GLOBAL APPEND PROPERTY ZY_MODULES "${ARG_TARGET}|${ARG_HEADER}|${ARG_CLASS}")

ENDFUNCTION()