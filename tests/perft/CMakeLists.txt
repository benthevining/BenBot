# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (perft)

target_link_libraries (perft PRIVATE libchess::libchess)

target_sources (perft PRIVATE main.cpp)
