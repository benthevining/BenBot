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

set (GLFW_BUILD_DOCS OFF)
set (GLFW_INSTALL OFF)

FetchContent_Declare (
    glfw
    SYSTEM
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG 3.4
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 3.4
)

FetchContent_Declare (
    stb
    SYSTEM
    GIT_REPOSITORY "https://github.com/nothings/stb.git"
    GIT_TAG origin/master
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable (glfw stb)

add_library (
    benbotgui_glfw_wrapper STATIC "${imgui_backends_dir}/imgui_impl_glfw.cpp" GLFW_Wrapper.cpp
)

target_link_libraries (
    benbotgui_glfw_wrapper PUBLIC glfw ben_bot::libgui PRIVATE imgui::imgui ben_bot::gui_resources
)

target_include_directories (
    benbotgui_glfw_wrapper PUBLIC "${CMAKE_CURRENT_LIST_DIR}" PRIVATE "${stb_SOURCE_DIR}"
)

add_library (ben_bot::glfw_wrapper ALIAS benbotgui_glfw_wrapper)
