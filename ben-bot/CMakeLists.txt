# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (ben_bot)

target_link_libraries (ben_bot PRIVATE libchess::libchess)

target_sources (ben_bot PRIVATE main.cpp)
