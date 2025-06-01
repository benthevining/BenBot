# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (chess_uci)

target_link_libraries (chess_uci PRIVATE libchess::libchess)

target_sources (chess_uci PRIVATE main.cpp)
