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

file (GLOB_RECURSE libchess_headers LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/libchess/*.hpp"
)

file (GLOB_RECURSE libchess_sources LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp" # include private headers too
)

target_sources (
    libchess PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${libchess_headers}
    PRIVATE ${libchess_sources}
)

target_include_directories (
    libchess PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/notation/"
                     "${CMAKE_CURRENT_LIST_DIR}/src/search/"
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${libchess_headers} ${libchess_sources})

add_library (libchess::libchess ALIAS libchess)
