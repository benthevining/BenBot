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

add_library (libbenbot STATIC)

target_compile_features (libbenbot PUBLIC cxx_std_23)

target_link_libraries (libbenbot PUBLIC ben_bot::libchess PRIVATE nlohmann_json::nlohmann_json)

set_target_properties (
    libbenbot PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF DEBUG_POSTFIX -d
)

file (GLOB_RECURSE libbenbot_headers LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/libbenbot/*.hpp"
)

file (GLOB_RECURSE libbenbot_sources LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp" # include private headers too
)

target_sources (
    libbenbot PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${libbenbot_headers}
    PRIVATE ${libbenbot_sources}
)

target_include_directories (
    libbenbot PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/search/" "${CMAKE_CURRENT_LIST_DIR}/src/eval/"
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${libbenbot_headers} ${libbenbot_sources})

add_library (ben_bot::libbenbot ALIAS libbenbot)
