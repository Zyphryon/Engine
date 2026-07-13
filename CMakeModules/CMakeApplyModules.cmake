# CMakeApplyModules.cmake
#
# Applies all registered engine modules to a target.
#
# Usage:
#   ZyApplyModules(<target>)
FUNCTION(ZyApplyModules TARGET)

    GET_PROPERTY(MODULES GLOBAL PROPERTY ZY_MODULES)

    SET(GENERATED_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/_Generated")
    SET(GENERATED_SOURCE "${GENERATED_FOLDER}/${TARGET}.Modules.cpp")
    SET(GENERATED_HEADER "${GENERATED_FOLDER}/${TARGET}.Modules.hpp")

    SET(INCLUDES  "")
    SET(CREATES   "")
    SET(LIBRARIES "")

    FOREACH(MODULE ${MODULES})

        STRING(REPLACE "|" ";" PARTS "${MODULE}")

        LIST(GET PARTS 0 LIBRARY)
        LIST(GET PARTS 1 HEADER)
        LIST(GET PARTS 2 CLASS)

        LIST(APPEND LIBRARIES ${LIBRARY})

        STRING(APPEND HEADERS "#include \"${HEADER}\"\n")

        STRING(APPEND CLASSES "Modules.Append(Unique<${CLASS}>::Create());\n")

    ENDFOREACH()

    FILE(MAKE_DIRECTORY "${GENERATED_FOLDER}")
    FILE(WRITE "${GENERATED_SOURCE}"
        "
        ${HEADERS}
        #include \"${TARGET}.Modules.hpp\"

        Engine::Modules ZyRegisterModules()
        {
            Engine::Modules Modules;

            ${CLASSES}

            return Modules;
        }")
    FILE(WRITE "${GENERATED_HEADER}"
        "
        #pragma once

        #include <Zyphryon.Engine/Kernel.hpp>

        Engine::Modules ZyRegisterModules();
        ")

    TARGET_LINK_LIBRARIES(${TARGET} PRIVATE ${LIBRARIES})

    TARGET_SOURCES(${TARGET} PRIVATE "${GENERATED_SOURCE}" "${GENERATED_HEADER}")

    TARGET_INCLUDE_DIRECTORIES(${TARGET} PRIVATE "${GENERATED_FOLDER}")

ENDFUNCTION()