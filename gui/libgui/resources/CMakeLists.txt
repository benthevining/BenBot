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

set (BENBOT_APP_ICON_SVG "${CMAKE_CURRENT_LIST_DIR}/res/pieces/Queen-White.svg"
     CACHE INTERNAL "Path to SVG file used for app icon"
)

file (GLOB_RECURSE resources LIST_DIRECTORIES false CONFIGURE_DEPENDS
                                                    "${CMAKE_CURRENT_LIST_DIR}/res/*.svg"
)

cmrc_add_resource_library (
    libgui_resources ${resources} res/default_layout.ini res/default_state.json res/icon.png
)

target_compile_features (libgui_resources PRIVATE cxx_std_23 PUBLIC cxx_std_17)

set_target_properties (
    libgui_resources PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
                                POSITION_INDEPENDENT_CODE ON VERIFY_INTERFACE_HEADER_SETS ON
)

target_link_libraries (libgui_resources PUBLIC ben_bot::libchess)

set (header_file "include/libgui/Resources.hpp")
set (source_file "src/Resources.cpp")

target_sources (
    libgui_resources PUBLIC FILE_SET HEADERS BASE_DIRS include FILES "${header_file}"
    PRIVATE "${source_file}"
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES "${header_file}" "${source_file}")

add_library (ben_bot::gui_resources ALIAS libgui_resources)
