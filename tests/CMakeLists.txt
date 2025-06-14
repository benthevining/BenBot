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

find_package (Python COMPONENTS Interpreter)

configure_file (CTestCustom.cmake "${PROJECT_BINARY_DIR}/CTestCustom.cmake" @ONLY)

add_subdirectory (unit)
add_subdirectory (perft)
add_subdirectory (rampart)
add_subdirectory (position-solver)

include (fastchess.cmake)
