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

#[[
This module sets up an imgui::imgui static library target and exports the IMGUI_BACKENDS_DIR variable.
]]

set_package_properties (
    imgui PROPERTIES URL "https://github.com/ocornut/imgui" DESCRIPTION "Cross-platform UI library"
)

set (imgui_FOUND TRUE)

if (TARGET imgui::imgui)
    return ()
endif ()

FetchContent_Declare (
    imgui
    SYSTEM
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG "v${imgui_FIND_VERSION}-docking"
    GIT_SHALLOW ON
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable (imgui)

set (imgui_misc_cpp_dir "${imgui_SOURCE_DIR}/misc/cpp")
set (imgui_backends_dir "${imgui_SOURCE_DIR}/backends")

set (IMGUI_BACKENDS_DIR "${imgui_backends_dir}"
     CACHE INTERNAL "Directory containing the backend source files"
)

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
