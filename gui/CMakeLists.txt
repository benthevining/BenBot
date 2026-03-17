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

add_subdirectory (libgui)

add_executable (benbot_gui)

target_compile_features (benbot_gui PRIVATE cxx_std_23)

target_link_libraries (benbot_gui PRIVATE ben_bot::libgui imgui::imgui)

if (APPLE)
    message (VERBOSE "Using Metal + GLFW backend for ImGUI app")

    enable_language (OBJCXX OBJC)

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

    FetchContent_MakeAvailable (glfw)

    target_link_libraries (imgui PUBLIC glfw)

    target_sources (
        benbot_gui PRIVATE main_glfw_metal.mm "${imgui_backends_dir}/imgui_impl_glfw.cpp"
                           "${imgui_backends_dir}/imgui_impl_metal.mm"
    )

    target_link_libraries (
        benbot_gui PRIVATE "-framework Metal" "-framework MetalKit" "-framework Cocoa"
                           "-framework IOKit" "-framework CoreVideo" "-framework QuartzCore"
    )
else ()
    message (WARNING "imgui backend not yet configured for this platform!")
endif ()
