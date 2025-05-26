# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (printer printer.cpp)

target_link_libraries (printer PRIVATE libchess::libchess)
