# CMakeApplyEmbedded.cmake
#
# Materializes every resource directory registered with ZyEmbedResources() into a consumer application,
# generating <target>.Embedded.{hpp,cpp} that exposes:
#
#   void ZyRegisterEmbedded(Ref<Content::Service> Service);
#
# which registers the "Embedded" mount. The application calls it once (e.g. from OnInitialize).
#
#   - Native : bytes are compiled in and served by a Content::Memory mount (zero-copy borrow).
#   - Web    : each directory is preloaded into MEMFS under /Embedded/<prefix> and served by a Content::Disk mount.
#
# Usage:
#   ZyApplyEmbedded(<target>)
FUNCTION(ZyApplyEmbedded TARGET)

    GET_PROPERTY(EMBEDDED GLOBAL PROPERTY ZY_EMBEDDED)

    SET(GENERATED_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/_Generated")
    SET(GENERATED_SOURCE "${GENERATED_FOLDER}/${TARGET}.Embedded.cpp")
    SET(GENERATED_HEADER "${GENERATED_FOLDER}/${TARGET}.Embedded.hpp")

    FILE(MAKE_DIRECTORY "${GENERATED_FOLDER}")

    # The public declaration is identical on every platform.
    FILE(WRITE "${GENERATED_HEADER}" [=[
#pragma once

#include <Zyphryon.Content/Service.hpp>

void ZyRegisterEmbedded(Ref<Content::Service> Service);
]=])

    IF(EMSCRIPTEN)

        # Web: preload each directory into MEMFS and serve the whole tree through one Disk mount.
        FOREACH(ENTRY ${EMBEDDED})

            STRING(REPLACE "|" ";" PARTS "${ENTRY}")
            LIST(GET PARTS 0 PREFIX)
            LIST(GET PARTS 1 DIRECTORY)

            TARGET_LINK_LIBRARIES(${TARGET} PRIVATE "--preload-file=${DIRECTORY}@/Embedded/${PREFIX}")

        ENDFOREACH()

        SET(SOURCE "#include \"Zyphryon.Content/Mount/Disk.hpp\"\n")
        STRING(APPEND SOURCE "#include \"${TARGET}.Embedded.hpp\"\n\n")
        STRING(APPEND SOURCE "void ZyRegisterEmbedded(Ref<Content::Service> Service)\n{\n")
        STRING(APPEND SOURCE "    Service.AddMount(\"Embedded\", Retainer<Content::Disk>::Create(\"/Embedded\"));\n")
        STRING(APPEND SOURCE "}\n")

    ELSE()

        # Native: compile every file's bytes into the binary and serve them from a Memory mount.
        SET(BLOBS   "")
        SET(ENTRIES "")
        SET(INDEX   0)

        FOREACH(ENTRY ${EMBEDDED})

            STRING(REPLACE "|" ";" PARTS "${ENTRY}")
            LIST(GET PARTS 0 PREFIX)
            LIST(GET PARTS 1 DIRECTORY)

            FILE(GLOB_RECURSE FILES CONFIGURE_DEPENDS "${DIRECTORY}/*")

            FOREACH(FILE ${FILES})

                FILE(RELATIVE_PATH RELATIVE "${DIRECTORY}" "${FILE}")
                FILE(READ "${FILE}" CONTENT HEX)
                STRING(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," CONTENT "${CONTENT}")

                # Join prefix and relative path, avoiding a leading slash when no prefix is set.
                IF(PREFIX)
                    SET(PATH "${PREFIX}/${RELATIVE}")
                ELSE()
                    SET(PATH "${RELATIVE}")
                ENDIF()

                STRING(APPEND BLOBS   "    static const Byte kBlob${INDEX}[] = { ${CONTENT} };\n")
                STRING(APPEND ENTRIES "        { Text(\"${PATH}\"), ConstSpan<Byte>(kBlob${INDEX}, sizeof(kBlob${INDEX})) },\n")

                MATH(EXPR INDEX "${INDEX} + 1")

            ENDFOREACH()

        ENDFOREACH()

        IF(INDEX EQUAL 0)

            # Nothing registered: emit a no-op so the application still links.
            SET(SOURCE "#include \"${TARGET}.Embedded.hpp\"\n\n")
            STRING(APPEND SOURCE "void ZyRegisterEmbedded(Ref<Content::Service> Service)\n{\n}\n")

        ELSE()

            SET(SOURCE "#include \"Zyphryon.Content/Mount/Memory.hpp\"\n")
            STRING(APPEND SOURCE "#include \"${TARGET}.Embedded.hpp\"\n\n")
            STRING(APPEND SOURCE "namespace\n{\n")
            STRING(APPEND SOURCE "${BLOBS}\n")
            STRING(APPEND SOURCE "    static const Content::Memory::Entry kEmbedded[] =\n    {\n")
            STRING(APPEND SOURCE "${ENTRIES}")
            STRING(APPEND SOURCE "    };\n}\n\n")
            STRING(APPEND SOURCE "void ZyRegisterEmbedded(Ref<Content::Service> Service)\n{\n")
            STRING(APPEND SOURCE "    Service.AddMount(\"Embedded\", Retainer<Content::Memory>::Create(ConstSpan<Content::Memory::Entry>(kEmbedded)));\n")
            STRING(APPEND SOURCE "}\n")

        ENDIF()

    ENDIF()

    FILE(WRITE "${GENERATED_SOURCE}" "${SOURCE}")

    TARGET_SOURCES(${TARGET} PRIVATE "${GENERATED_SOURCE}" "${GENERATED_HEADER}")
    TARGET_INCLUDE_DIRECTORIES(${TARGET} PRIVATE "${GENERATED_FOLDER}")

ENDFUNCTION()
