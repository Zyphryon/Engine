# Creates a target out of the sources under its own folder, and gives it the engine's standard settings.
#
# Usage:
#   ZyAddTarget(<name> [STATIC|SHARED|MODULE|EXECUTABLE] [SOURCE <dir>...] [EXCLUDE <regex>...] [PCH <header>])
FUNCTION(ZyAddTarget NAME)

    SET(OPTIONS          STATIC SHARED MODULE EXECUTABLE)
    SET(ONE_VALUE_ARGS   PCH)
    SET(MULTI_VALUE_ARGS SOURCE EXCLUDE)

    CMAKE_PARSE_ARGUMENTS(ARG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    IF(NOT ARG_SOURCE)

        SET(ARG_SOURCE "Source")

    ENDIF()

    # Globbed with CONFIGURE_DEPENDS, so adding or deleting a file is noticed rather than waited on.
    SET(SOURCES)
    SET(FOLDERS)

    FOREACH(FOLDER ${ARG_SOURCE})

        GET_FILENAME_COMPONENT(WHERE "${FOLDER}" ABSOLUTE)

        FILE(GLOB_RECURSE FOUND CONFIGURE_DEPENDS "${WHERE}/*.cpp")

        LIST(APPEND SOURCES ${FOUND})
        LIST(APPEND FOLDERS "${WHERE}")

    ENDFOREACH()

    FOREACH(PATTERN ${ARG_EXCLUDE})

        LIST(FILTER SOURCES EXCLUDE REGEX "${PATTERN}")

    ENDFOREACH()

    IF(NOT SOURCES)

        MESSAGE(FATAL_ERROR "ZyAddTarget: ${NAME} was left with no sources to build.")

    ENDIF()

    IF(ARG_EXECUTABLE)

        ADD_EXECUTABLE(${NAME} ${SOURCES})

    ELSEIF(ARG_SHARED)

        ADD_LIBRARY(${NAME} SHARED ${SOURCES})

    ELSEIF(ARG_MODULE)

        ADD_LIBRARY(${NAME} MODULE ${SOURCES})

    ELSE()

        ADD_LIBRARY(${NAME} STATIC ${SOURCES})

    ENDIF()

    # The folder the sources were found under is the one they include each other by.
    TARGET_INCLUDE_DIRECTORIES(${NAME} PUBLIC ${FOLDERS})

    IF(ARG_PCH)

        TARGET_PRECOMPILE_HEADERS(${NAME} PUBLIC "${ARG_PCH}")

    ENDIF()

    ZyApplyOptions(${NAME})

ENDFUNCTION()