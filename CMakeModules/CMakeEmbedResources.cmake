# EmbedResources.cmake
#
# Usage:
#   INCLUDE(EmbedResources.cmake)
#   EMBED_RESOURCES(<target> <directory> <namespace> <filename>)

SET(CMAKE_EMBED_RESOURCES_SCRIPT "${CMAKE_CURRENT_LIST_FILE}" CACHE INTERNAL "")

FUNCTION(CMAKE_EMBED_RESOURCES_GENERATE ABS_DIR NAMESPACE FILENAME GENERATED_HPP GENERATED_CPP)

    # Get all files recursively
    FILE(GLOB_RECURSE RESOURCE_FILES "${ABS_DIR}/*")

    #
    # HEADER
    #
    FILE(WRITE "${GENERATED_HPP}" "
        #pragma once

        #include <Zyphryon.Base/Base.hpp>

        namespace ${NAMESPACE}
        {
            struct Resource
            {
                ConstPtr<Byte> Data;
                UInt           Size;
            };

            ConstPtr<Resource> Fetch(ConstStr8 Path);
        }")

    #
    # SOURCE
    #
    FILE(WRITE "${GENERATED_CPP}" "
        #include \"${FILENAME}.hpp\"

        namespace ${NAMESPACE}
        {
        ")

    SET(MAP_ENTRIES "")

    FOREACH(FILE_PATH ${RESOURCE_FILES})

        # Skip directories
        IF(IS_DIRECTORY "${FILE_PATH}")
            CONTINUE()
        ENDIF()

        # Read binary content
        FILE(READ "${FILE_PATH}" CONTENT HEX)
        STRING(REGEX REPLACE "(..)" "0x\\1," BYTE_ARRAY "${CONTENT}")

        # Get path relative to resource folder
        FILE(RELATIVE_PATH REL_PATH "${ABS_DIR}" "${FILE_PATH}")
        FILE(TO_CMAKE_PATH "${REL_PATH}" REL_PATH)  # normalize slashes

        # Generate safe C++ variable name
        STRING(REPLACE "/" "_" VAR_NAME "${REL_PATH}")
        STRING(REPLACE "." "_" VAR_NAME "${VAR_NAME}")
        STRING(REPLACE "-" "_" VAR_NAME "${VAR_NAME}")

        # Append static data array
        FILE(APPEND "${GENERATED_CPP}" "static const Byte Data_${VAR_NAME}[] = { ${BYTE_ARRAY} };")

        # Append table entry
        SET(MAP_ENTRIES "${MAP_ENTRIES} {\"${REL_PATH}\", { Data_${VAR_NAME}, sizeof(Data_${VAR_NAME}) }},")
    ENDFOREACH()

    # Append embedded filesystem and functions
    FILE(APPEND "${GENERATED_CPP}" "

        static const Table<Str8, Resource> sEmbeddedFilesystem =
        {
            ${MAP_ENTRIES}
        };

        ConstPtr<Resource> Fetch(ConstStr8 Path)
        {
            if (auto it = sEmbeddedFilesystem.find(Path); it != sEmbeddedFilesystem.end())
                return &it->second;
            return nullptr;
        }

        }")

ENDFUNCTION()

FUNCTION(CMAKE_EMBED_RESOURCES TARGET DIR NAMESPACE FILENAME)

    # Make DIR absolute
    GET_FILENAME_COMPONENT(ABS_DIR "${DIR}" ABSOLUTE)

    SET(GENERATED_HPP "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.hpp")
    SET(GENERATED_CPP "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.cpp")

    # Generate once on clean configure if files don't exist yet
    IF(NOT EXISTS "${GENERATED_CPP}")
        CMAKE_EMBED_RESOURCES_GENERATE("${ABS_DIR}" "${NAMESPACE}" "${FILENAME}" "${GENERATED_HPP}" "${GENERATED_CPP}")
    ENDIF()

    # Manual target — run with: cmake --build . --target EmbedResources_<TARGET>
    ADD_CUSTOM_TARGET(EmbedResources_${TARGET}
            COMMAND ${CMAKE_COMMAND}
            -D "ABS_DIR=${ABS_DIR}"
            -D "NAMESPACE=${NAMESPACE}"
            -D "FILENAME=${FILENAME}"
            -D "GENERATED_HPP=${GENERATED_HPP}"
            -D "GENERATED_CPP=${GENERATED_CPP}"
            -D RUN_GENERATE=1
            -P "${CMAKE_EMBED_RESOURCES_SCRIPT}"
            COMMENT "Embedding resources for ${TARGET}")

    TARGET_SOURCES(${TARGET} PRIVATE "${GENERATED_CPP}")
    TARGET_INCLUDE_DIRECTORIES(${TARGET} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

ENDFUNCTION()

IF(RUN_GENERATE)
    CMAKE_EMBED_RESOURCES_GENERATE("${ABS_DIR}" "${NAMESPACE}" "${FILENAME}" "${GENERATED_HPP}" "${GENERATED_CPP}")
ENDIF()