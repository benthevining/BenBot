# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_library (libchess)

target_compile_features (libchess PUBLIC cxx_std_23)

set_target_properties (
    libchess PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF DEBUG_POSTFIX -d
)

set (libchess_headers # cmake-format: sortable
                      geometry/Bitboard.hpp geometry/BitboardSet.hpp geometry/Square.hpp
)

set (libchess_sources # cmake-format: sortable
)

list (TRANSFORM libchess_headers PREPEND "${CMAKE_CURRENT_LIST_DIR}/include/libchess/")

list (TRANSFORM libchess_sources PREPEND "${CMAKE_CURRENT_LIST_DIR}/src/")

target_sources (
    libchess PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${libchess_headers}
    PRIVATE ${libchess_sources}
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${libchess_headers} ${libchess_sources})

add_library (libchess::libchess ALIAS libchess)
