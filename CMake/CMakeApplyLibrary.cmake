## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
##
## This work is licensed under the terms of the MIT license.
##
## For a copy, see <https://opensource.org/licenses/MIT>.
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

# Makes a target the one image that carries the engine's marked symbols out to the plugins loaded beside it.
#
# ALL carries every symbol of the engine and the modules out, for programs that use more than their marked parts.
#
# Usage:
#   ZyApplyExports(<target> [ALL] [MODULES <targets>...])
FUNCTION(ZyApplyExports TARGET)

    SET(OPTIONS ALL)
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

    # A plugin is given the engine and whatever the host wrote its own kernel in.
    SET(OFFERED "ZyEngine" ${ARG_MODULES})
    LIST(REMOVE_DUPLICATES OFFERED)

    # A module that only gathers others, as an INTERFACE library does, is taken as the libraries it gathers.
    SET(GATHERED)
    SET(PENDING ${OFFERED})

    WHILE(PENDING)

        LIST(POP_FRONT PENDING MEMBER)
        GET_TARGET_PROPERTY(MEMBER_KIND ${MEMBER} TYPE)

        IF(MEMBER_KIND STREQUAL "INTERFACE_LIBRARY")

            GET_TARGET_PROPERTY(INNER ${MEMBER} INTERFACE_LINK_LIBRARIES)

            FOREACH(LIBRARY ${INNER})

                IF(TARGET ${LIBRARY})

                    LIST(APPEND PENDING ${LIBRARY})

                ENDIF()

            ENDFOREACH()

        ELSE()

            LIST(APPEND GATHERED ${MEMBER})

        ENDIF()

    ENDWHILE()

    LIST(REMOVE_DUPLICATES GATHERED)
    SET(OFFERED ${GATHERED})

    # Everything inside the image compiles as the exporter, the graphic drivers included.
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

    IF(ARG_ALL)

        # Every object of the engine and the modules is built into the image and every symbol carried out, so a program
        # written against the whole engine links against the image, not only one written against its marked parts.
        FOREACH(MEMBER ${OFFERED})

            # The object that creates a precompiled header defines a symbol the exports cannot carry, so it is left to
            # the library, which still hands it over should anything need it.
            TARGET_SOURCES(${TARGET} PRIVATE "$<FILTER:$<TARGET_OBJECTS:${MEMBER}>,EXCLUDE,cmake_pch>")

            # Linked as well for what it depends on, which never pulls one of its own objects in a second time.
            TARGET_LINK_LIBRARIES(${TARGET} PRIVATE ${MEMBER})

            # A precompiled header leaves a nameless symbol in each object, which the generated exports cannot carry.
            IF(MSVC)

                TARGET_COMPILE_OPTIONS(${MEMBER} PRIVATE /Yl-)

            ENDIF()

        ENDFOREACH()

        FOREACH(MEMBER ${WHOLE})

            TARGET_LINK_LIBRARIES(${TARGET} PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,${MEMBER}>")

        ENDFOREACH()


        # The image's own objects are carried out as well, so it creates no precompiled header that would leak its symbol.
        SET_TARGET_PROPERTIES(${TARGET} PROPERTIES DISABLE_PRECOMPILE_HEADERS ON WINDOWS_EXPORT_ALL_SYMBOLS ON)

    ELSE()

        FOREACH(MEMBER ${OFFERED} ${WHOLE})

            TARGET_LINK_LIBRARIES(${TARGET} PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,${MEMBER}>")

        ENDFOREACH()

        SET_TARGET_PROPERTIES(${TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS OFF)

    ENDIF()

    FOREACH(MEMBER ${OFFERED})

        TARGET_LINK_LIBRARIES(${TARGET} INTERFACE "$<COMPILE_ONLY:${MEMBER}>")

    ENDFOREACH()

    TARGET_COMPILE_DEFINITIONS(${TARGET} INTERFACE ZY_USE_SHARED)

ENDFUNCTION()

# Turns a target into a plugin that reaches the engine through the host that loads it.
#
# A plugin marks each entry point with ZY_EXPORT_C, and the host resolves it by name through ZyBase::Library.
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

    # Linking the host gives the plugin its headers and the import side of ZY_API.
    # On Windows that links the host's import library; elsewhere the loader resolves it and nothing is linked.
    TARGET_LINK_LIBRARIES(${TARGET} PRIVATE ${ARG_HOST})

    # A plugin carries out only what it was written to be found by, which is what it marked itself.
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS OFF)

ENDFUNCTION()