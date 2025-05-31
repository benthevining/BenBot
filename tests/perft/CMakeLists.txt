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
