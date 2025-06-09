# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (play_cli)

target_link_libraries (play_cli PRIVATE libchess::libchess)

target_sources (play_cli PRIVATE main.cpp)
