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

add_subdirectory (resources)

find_package (imgui 1.92.6 REQUIRED)

FetchContent_Declare (
    nativefiledialog-extended
    SYSTEM
    GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git"
    GIT_TAG v1.3.0
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 1.3.0
)

FetchContent_Declare (
    platform-folders
    SYSTEM
    GIT_REPOSITORY "https://github.com/sago007/PlatformFolders.git"
    GIT_TAG 4.3.0
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 4.3.0
)

FetchContent_Declare (
    lunasvg
    SYSTEM
    GIT_REPOSITORY "https://github.com/sammycage/lunasvg.git"
    GIT_TAG v3.5.0
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 3.5.0
)

set (LUNASVG_BUILD_EXAMPLES OFF)
set (LUNASVG_DISABLE_LOAD_SYSTEM_FONTS ON)

FetchContent_MakeAvailable (nativefiledialog-extended platform-folders lunasvg)

add_library (libgui STATIC)

target_compile_features (libgui PUBLIC cxx_std_23)

target_link_libraries (
    libgui PUBLIC ben_bot::libbenbot nfd::nfd
    PRIVATE imgui::imgui nlohmann_json::nlohmann_json ben_bot::resources ben_bot::gui_resources
            sago::platform_folders lunasvg::lunasvg
)

set_target_properties (
    libgui PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
                      VERIFY_INTERFACE_HEADER_SETS ON
)

file (GLOB_RECURSE libgui_headers LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/libgui/*.hpp"
)

file (GLOB_RECURSE libgui_sources LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
      "${CMAKE_CURRENT_LIST_DIR}/src/ImUtil.hpp"
)

target_sources (
    libgui PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${libgui_headers}
    PRIVATE ${libgui_sources}
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${libgui_headers} ${libgui_sources})

target_include_directories (libgui PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")

get_cmake_property (debug_configs DEBUG_CONFIGURATIONS)

if (NOT debug_configs)
    set (debug_configs Debug)
endif ()

list (JOIN debug_configs "," debug_configs)

set (config_debug "$<CONFIG:${debug_configs}>")

target_compile_definitions (
    libgui
    PRIVATE
        "$<${config_debug}:BENBOT_RES_SRC_TREE_PATH=\"${CMAKE_CURRENT_LIST_DIR}/resources/res\">"
)

add_library (ben_bot::libgui ALIAS libgui)
