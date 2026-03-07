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
    nlohmann_json
    SYSTEM
    GIT_REPOSITORY "https://github.com/nlohmann/json.git"
    GIT_TAG v3.12.0
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 3.12.0
)

FetchContent_MakeAvailable (nlohmann_json)

add_subdirectory (resources)

add_library (libbenbot STATIC)

target_compile_features (libbenbot PUBLIC cxx_std_23)

target_link_libraries (
    libbenbot PUBLIC ben_bot::libchess PRIVATE ben_bot::resources nlohmann_json::nlohmann_json
)

FetchContent_Declare (
    termcolor
    SYSTEM
    GIT_REPOSITORY "https://github.com/ikalnytskyi/termcolor.git"
    GIT_TAG v2.1.0
    GIT_SHALLOW ON
    SOURCE_SUBDIR foo # we need to disable adding termcolor as a subdirectory, because they list a
                      # too-old cmake version
    FIND_PACKAGE_ARGS 2.1.0
)

FetchContent_MakeAvailable (termcolor)

if (termcolor_SOURCE_DIR)
    set (termcolor_includes "${termcolor_SOURCE_DIR}/include")
elseif (termcolor_DIR)
    set (termcolor_includes "${termcolor_DIR}/include")
endif ()

set_property (
    SOURCE src/search/Callbacks.cpp src/engine/ColorPrinting.cpp APPEND
    PROPERTY INCLUDE_DIRECTORIES "${termcolor_includes}"
)

set_target_properties (
    libbenbot PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
                         VERIFY_INTERFACE_HEADER_SETS ON
)

file (GLOB_RECURSE libbenbot_headers LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/libbenbot/*.hpp"
)

file (GLOB_RECURSE libbenbot_sources LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.[c,h]pp" # include private headers too
)

target_sources (
    libbenbot PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${libbenbot_headers}
    PRIVATE ${libbenbot_sources}
)

target_include_directories (libbenbot PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/")

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${libbenbot_headers} ${libbenbot_sources})

add_library (ben_bot::libbenbot ALIAS libbenbot)
