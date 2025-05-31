# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (rampart)

target_link_libraries (rampart PRIVATE libchess::libchess nlohmann_json::nlohmann_json)

target_sources (rampart PRIVATE main.cpp)
