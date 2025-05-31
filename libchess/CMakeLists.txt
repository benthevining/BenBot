# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

include (FetchContent)

FetchContent_Declare (
    magic_enum
    SYSTEM
    GIT_REPOSITORY "https://github.com/Neargye/magic_enum.git"
    GIT_TAG v0.9.7
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 0.9.7
)

FetchContent_MakeAvailable (magic_enum)

add_library (libchess STATIC)

target_compile_features (libchess PUBLIC cxx_std_23)

target_link_libraries (libchess PUBLIC magic_enum::magic_enum)

set_target_properties (
    libchess PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF DEBUG_POSTFIX -d
)

set (
    libchess_headers
    # cmake-format: sortable
    board/Bitboard.hpp
    board/BitboardIndex.hpp
    board/BitboardMasks.hpp
    board/Distances.hpp
    board/File.hpp
    board/Fills.hpp
    board/Pieces.hpp
    board/Rank.hpp
    board/Shifts.hpp
    board/Square.hpp
    game/CastlingRights.hpp
    game/Position.hpp
    moves/Move.hpp
    moves/MoveGen.hpp
    moves/Patterns.hpp
    moves/Perft.hpp
    moves/PseudoLegal.hpp
    notation/Algebraic.hpp
    notation/FEN.hpp
    notation/UCI.hpp
    pieces/Colors.hpp
    pieces/PieceTypes.hpp
    pieces/UTF8.hpp
    util/Math.hpp
    util/Strings.hpp
)

set (
    libchess_sources
    # cmake-format: sortable
    board/Bitboard.cpp
    board/Distances.cpp
    game/Position.cpp
    notation/Algebraic.cpp
    notation/FEN.cpp
    notation/UCI.cpp
    util/Strings.cpp
)

list (TRANSFORM libchess_headers PREPEND "${CMAKE_CURRENT_LIST_DIR}/include/libchess/")

list (TRANSFORM libchess_sources PREPEND "${CMAKE_CURRENT_LIST_DIR}/src/")

target_sources (
    libchess PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${libchess_headers}
    PRIVATE ${libchess_sources}
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${libchess_headers} ${libchess_sources})

add_library (libchess::libchess ALIAS libchess)
