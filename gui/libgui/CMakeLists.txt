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
    imgui
    SYSTEM
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG v1.92.6-docking
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 1.92.6
)

FetchContent_Declare (
    nativefiledialog-extended
    SYSTEM
    GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git"
    GIT_TAG v1.3.0
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 1.3.0
)

FetchContent_MakeAvailable (imgui nativefiledialog-extended)

set (imgui_misc_cpp_dir "${imgui_SOURCE_DIR}/misc/cpp")
set (imgui_backends_dir "${imgui_SOURCE_DIR}/backends")
set (imgui_backends_dir "${imgui_backends_dir}" PARENT_SCOPE)

# create our own target for imgui
add_library (imgui STATIC)

target_sources (
    imgui
    PRIVATE "${imgui_SOURCE_DIR}/imgui.cpp" "${imgui_SOURCE_DIR}/imgui_draw.cpp"
            "${imgui_SOURCE_DIR}/imgui_tables.cpp" "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
            "${imgui_misc_cpp_dir}/imgui_stdlib.cpp"
)

target_include_directories (
    imgui PUBLIC "${imgui_SOURCE_DIR}" "${imgui_misc_cpp_dir}" "${imgui_backends_dir}"
)

add_library (imgui::imgui ALIAS imgui)

#

add_subdirectory (resources)

add_library (libgui STATIC)

target_compile_features (libgui PUBLIC cxx_std_23)

target_link_libraries (
    libgui PUBLIC ben_bot::libbenbot PRIVATE imgui::imgui nfd::nfd nlohmann_json::nlohmann_json
                                             ben_bot::resources ben_bot::gui_resources
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

add_library (ben_bot::libgui ALIAS libgui)
