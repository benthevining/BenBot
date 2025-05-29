# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (play_chess)

target_link_libraries (play_chess PRIVATE libchess::libchess)

target_sources (play_chess PRIVATE main.cpp)
