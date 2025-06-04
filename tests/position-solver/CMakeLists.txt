# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (position_solver)

target_link_libraries (position_solver PRIVATE libchess::libchess)

target_sources (position_solver PRIVATE main.cpp)
