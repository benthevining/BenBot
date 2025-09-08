# ======================================================================================
#
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
#
# ======================================================================================

FetchContent_Declare (
    magic_enum
    SYSTEM
    GIT_REPOSITORY "https://github.com/Neargye/magic_enum.git"
    GIT_TAG v0.9.7
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 0.9.7
)

FetchContent_Declare (
    inplace_vector
    SYSTEM
    GIT_REPOSITORY "https://github.com/bemanproject/inplace_vector.git"
    GIT_TAG origin/main
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable (magic_enum inplace_vector)

#

set (generated_includes "${CMAKE_CURRENT_BINARY_DIR}/generated")
set (magic_moves_header "${generated_includes}/MagicMoves.hpp")

add_executable (libchess_magics_writer)

target_compile_features (libchess_magics_writer PRIVATE cxx_std_23)

target_link_libraries (libchess_magics_writer PRIVATE magic_enum::magic_enum beman::inplace_vector)

set_target_properties (
    libchess_magics_writer PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF DEBUG_POSTFIX -d
)

target_include_directories (
    libchess_magics_writer PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include"
                                   "${CMAKE_CURRENT_LIST_DIR}/src/moves/"
)

target_sources (libchess_magics_writer PRIVATE GenerateMagics.cpp)

add_custom_command (
    OUTPUT "${magic_moves_header}" COMMAND libchess_magics_writer "${magic_moves_header}"
    DEPENDS libchess_magics_writer COMMENT "Generating libchess magic moves header..." CODEGEN
)

#

add_library (libchess STATIC)

target_compile_features (libchess PUBLIC cxx_std_23)

target_link_libraries (libchess PUBLIC magic_enum::magic_enum beman::inplace_vector)

set_target_properties (
    libchess PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF DEBUG_POSTFIX -d
)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    # needed for declaring large arrays of Zobrist keys
    target_compile_options (libchess PRIVATE -fbracket-depth=767)
endif ()

file (GLOB_RECURSE libchess_headers LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/libchess/*.hpp"
)

file (GLOB_RECURSE libchess_sources LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp" # include private headers too
)

target_sources (
    libchess PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${libchess_headers}
    PRIVATE ${libchess_sources} "${magic_moves_header}"
)

target_include_directories (
    libchess
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/game/" "${CMAKE_CURRENT_LIST_DIR}/src/moves/"
            "${CMAKE_CURRENT_LIST_DIR}/src/notation/" "${CMAKE_CURRENT_LIST_DIR}/src/util/"
            "${generated_includes}"
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${libchess_headers} ${libchess_sources})

add_library (ben_bot::libchess ALIAS libchess)
