# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

include (FetchContent)

FetchContent_Declare (
    nlohmann_json
    SYSTEM
    GIT_REPOSITORY "https://github.com/nlohmann/json.git"
    GIT_TAG v3.11.3
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 3.11.3
)

FetchContent_MakeAvailable (nlohmann_json)

add_executable (perft)

target_link_libraries (perft PRIVATE libchess::libchess nlohmann_json::nlohmann_json)

target_sources (perft PRIVATE main.cpp)

#

# in the template script, we want to expand both variables and generator expressions

file (READ RunPerft.py python_script_content)

set (CORRECT_RESULTS_FILE "${CMAKE_CURRENT_LIST_DIR}/PerftResults.json")
set (TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/results")

string (CONFIGURE "${python_script_content}" python_script_content @ONLY)

set (generated_script "${CMAKE_CURRENT_BINARY_DIR}/RunPerft$<CONFIG>.py")

# cmake-format: off
file (
    GENERATE
    OUTPUT "${generated_script}"
    CONTENT "${python_script_content}"
    TARGET perft
    FILE_PERMISSIONS
        OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)
# cmake-format: on

set_property (DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS RunPerft.py)

find_package (Python COMPONENTS Interpreter)

if (TARGET Python::Interpreter)
    add_custom_target (
        run_perft
        COMMAND Python::Interpreter "${generated_script}"
        BYPRODUCTS "${TMP_DIR}"
        COMMENT "Running perft tests..."
        USES_TERMINAL COMMAND_EXPAND_LISTS
        SOURCES RunPerft.py PerftResults.json
    )

    add_dependencies (run_perft perft)
endif ()
