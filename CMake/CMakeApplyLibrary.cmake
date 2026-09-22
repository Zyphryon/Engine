# Makes a target the one image the engine's marked symbols are carried out of, so a plugin loaded beside it calls
# into this engine rather than holding one of its own.
#
# An executable is as good a host as a library, and the cheaper one: nothing is linked twice and the whole program
# is optimized as one. A build with no plugins to load needs none of this and should not ask.
#
# Usage:
#   ZyApplyExports(<target> [MODULES <targets>...])
FUNCTION(ZyApplyExports TARGET)

    SET(OPTIONS)
    SET(ONE_VALUE_ARGS)
    SET(MULTI_VALUE_ARGS MODULES)

    CMAKE_PARSE_ARGUMENTS(ARG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    IF(NOT TARGET ${TARGET})

        MESSAGE(FATAL_ERROR "ZyApplyExports: ${TARGET} is not a target.")

    ENDIF()

    GET_TARGET_PROPERTY(KIND ${TARGET} TYPE)

    IF(KIND STREQUAL "EXECUTABLE")

        SET_TARGET_PROPERTIES(${TARGET} PROPERTIES ENABLE_EXPORTS TRUE)

    ELSEIF(NOT KIND STREQUAL "SHARED_LIBRARY")

        MESSAGE(FATAL_ERROR "ZyApplyExports: ${TARGET} is a ${KIND}, which carries no symbols out.")

    ENDIF()

    # What a plugin is given: the engine, and whatever the host wrote its own kernel in.
    SET(OFFERED "ZyEngine" ${ARG_MODULES})
    LIST(REMOVE_DUPLICATES OFFERED)

    # Everything standing inside the image, the graphic drivers included, since each holds marked symbols and
    # would otherwise compile as though reading them in from somewhere else.
    SET(MARKED ${OFFERED} ${TARGET})

    GET_PROPERTY(REGISTERED GLOBAL PROPERTY ZY_MODULES)

    FOREACH(ENTRY ${REGISTERED})

        STRING(REPLACE "|" ";" PARTS "${ENTRY}")
        LIST(GET PARTS 0 LIBRARY)

        LIST(APPEND MARKED ${LIBRARY})

    ENDFOREACH()

    LIST(REMOVE_DUPLICATES MARKED)

    GET_TARGET_PROPERTY(WHOLE   ZyEngine ZY_SHARED_WHOLE)
    GET_TARGET_PROPERTY(EXPORTS ZyEngine ZY_SHARED_EXPORTS)

    IF(NOT WHOLE)

        SET(WHOLE "")

    ENDIF()

    IF(NOT EXPORTS)

        SET(EXPORTS "")

    ENDIF()

    FOREACH(MEMBER ${MARKED})

        TARGET_COMPILE_DEFINITIONS(${MEMBER} PRIVATE ZY_BUILD_SHARED ${EXPORTS})

    ENDFOREACH()

    FOREACH(LIBRARY ${WHOLE})

        GET_TARGET_PROPERTY(OWN      ${LIBRARY} COMPILE_DEFINITIONS)
        GET_TARGET_PROPERTY(PUBLICLY ${LIBRARY} INTERFACE_COMPILE_DEFINITIONS)

        IF(NOT OWN)

            SET(OWN "")

        ENDIF()

        IF(PUBLICLY)

            LIST(REMOVE_ITEM OWN ${PUBLICLY})

        ENDIF()

        LIST(APPEND OWN ${EXPORTS})

        SET_PROPERTY(TARGET ${LIBRARY} PROPERTY COMPILE_DEFINITIONS           "${OWN}")
        SET_PROPERTY(TARGET ${LIBRARY} PROPERTY INTERFACE_COMPILE_DEFINITIONS "")

    ENDFOREACH()

    FOREACH(MEMBER ${OFFERED} ${WHOLE})

        TARGET_LINK_LIBRARIES(${TARGET} PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,${MEMBER}>")

    ENDFOREACH()

    FOREACH(MEMBER ${OFFERED})

        TARGET_LINK_LIBRARIES(${TARGET} INTERFACE "$<COMPILE_ONLY:${MEMBER}>")

    ENDFOREACH()

    TARGET_COMPILE_DEFINITIONS(${TARGET} INTERFACE ZY_USE_SHARED)

    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS OFF)

ENDFUNCTION()

# Turns a target into a plugin a host loads once it is already running, reaching the engine by calling into the
# host that loaded it.
#
# Which entry points a host looks for is the host's own business, so nothing here names them. A plugin marks each
# one with ZY_EXPORT_C and the host resolves it by name through ZyBase::Library. Where the file has to land is the
# host's business too, since the host is what goes looking for it.
#
# Usage:
#   ZyApplyPlugin(<target> HOST <target>)
FUNCTION(ZyApplyPlugin TARGET)

    SET(OPTIONS)
    SET(ONE_VALUE_ARGS HOST)
    SET(MULTI_VALUE_ARGS)

    CMAKE_PARSE_ARGUMENTS(ARG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    IF(NOT TARGET ${TARGET})

        MESSAGE(FATAL_ERROR "ZyApplyPlugin: ${TARGET} is not a target.")

    ENDIF()

    GET_TARGET_PROPERTY(KIND ${TARGET} TYPE)

    IF(NOT KIND STREQUAL "SHARED_LIBRARY" AND NOT KIND STREQUAL "MODULE_LIBRARY")

        MESSAGE(FATAL_ERROR "ZyApplyPlugin: ${TARGET} is a ${KIND}, and only a library of its own is loaded beside a program.")

    ENDIF()

    # Named rather than looked up, since a plugin is written for one host and a build may hold several.
    IF(NOT ARG_HOST)

        MESSAGE(FATAL_ERROR "ZyApplyPlugin: HOST is required, naming the host this plugin is loaded by.")

    ENDIF()

    IF(NOT TARGET ${ARG_HOST})

        MESSAGE(FATAL_ERROR "ZyApplyPlugin: ${ARG_HOST} is not a target.")

    ENDIF()

    # Linking the host hands over its headers and the mark that makes every ZY_API symbol one to call in rather
    # than one to carry out. On Windows that is the host's import library; elsewhere the loader answers for it and
    # nothing is linked at all.
    TARGET_LINK_LIBRARIES(${TARGET} PRIVATE ${ARG_HOST})

    # A plugin carries out only what it was written to be found by, which is what it marked itself.
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS OFF)

ENDFUNCTION()