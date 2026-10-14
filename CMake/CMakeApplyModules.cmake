## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
## Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
##
## This work is licensed under the terms of the MIT license.
##
## For a copy, see <https://opensource.org/licenses/MIT>.
## -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

# Applies all registered engine modules to a target.
#
# Usage:
#   ZyApplyModules(<target>)
FUNCTION(ZyApplyModules TARGET)

    GET_PROPERTY(MODULES GLOBAL PROPERTY ZY_MODULES)

    SET(GENERATED_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/_Generated")
    SET(GENERATED_SOURCE "${GENERATED_FOLDER}/${TARGET}.Modules.cpp")
    SET(GENERATED_HEADER "${GENERATED_FOLDER}/${TARGET}.Modules.hpp")

    SET(HEADERS   "")
    SET(CLASSES   "")
    SET(LIBRARIES "")

    FOREACH(MODULE ${MODULES})

        STRING(REPLACE "|" ";" PARTS "${MODULE}")

        LIST(GET PARTS 0 LIBRARY)
        LIST(GET PARTS 1 HEADER)
        LIST(GET PARTS 2 CLASS)

        LIST(APPEND LIBRARIES ${LIBRARY})

        STRING(APPEND HEADERS "#include \"${HEADER}\"\n")

        STRING(APPEND CLASSES "    Modules.Append(Unique<${CLASS}>::Create());\n")

    ENDFOREACH()

    FILE(MAKE_DIRECTORY "${GENERATED_FOLDER}")

    SET(SOURCE "#include <Zyphryon.Base/Base.hpp>\n${HEADERS}")
    STRING(APPEND SOURCE "#include \"${TARGET}.Modules.hpp\"\n\n")
    STRING(APPEND SOURCE "ZyEngine::Modules ZyRuntime::ZyRegisterModules()\n{\n")
    STRING(APPEND SOURCE "    ZyEngine::Modules Modules;\n\n")
    STRING(APPEND SOURCE "${CLASSES}")
    STRING(APPEND SOURCE "\n    return Modules;\n}\n")

    FILE(WRITE "${GENERATED_SOURCE}" "${SOURCE}")
    FILE(WRITE "${GENERATED_HEADER}" [=[
#pragma once

#include "Zyphryon.Runtime/Entry.hpp"
]=])

    TARGET_LINK_LIBRARIES(${TARGET} PRIVATE ${LIBRARIES})

    TARGET_SOURCES(${TARGET} PRIVATE "${GENERATED_SOURCE}" "${GENERATED_HEADER}")

    TARGET_INCLUDE_DIRECTORIES(${TARGET} PRIVATE "${GENERATED_FOLDER}")

ENDFUNCTION()