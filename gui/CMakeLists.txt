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

if (APPLE)
    enable_language (OBJCXX OBJC)
endif ()

add_subdirectory (libgui)

add_executable (benbot_gui)

target_compile_features (benbot_gui PRIVATE cxx_std_23)

target_link_libraries (benbot_gui PRIVATE ben_bot::libgui imgui::imgui)

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

target_sources (benbot_gui PRIVATE "${imgui_backends_dir}/imgui_impl_glfw.cpp")

target_include_directories (benbot_gui PRIVATE "${CMAKE_CURRENT_LIST_DIR}/platform")

if (APPLE)
    message (VERBOSE "Using GLFW + Metal backend for ImGUI app")

    target_sources (
        benbot_gui PRIVATE main_glfw_metal.mm "${imgui_backends_dir}/imgui_impl_metal.mm"
    )

    target_link_libraries (
        benbot_gui PRIVATE "-framework Metal" "-framework MetalKit" "-framework Cocoa"
                           "-framework IOKit" "-framework CoreVideo" "-framework QuartzCore"
    )
else ()
    find_package (Vulkan OPTIONAL_COMPONENTS volk)

    if (Vulkan_FOUND)
        message (VERBOSE "Using GLFW + Vulkan backend for ImGUI app")

        target_sources (
            benbot_gui PRIVATE main_glfw_vulkan.cpp "${imgui_backends_dir}/imgui_impl_vulkan.cpp"
        )

        target_link_libraries (benbot_gui PRIVATE Vulkan::Vulkan)

        if (TARGET Vulkan::volk)
            target_link_libraries (benbot_gui PRIVATE Vulkan::volk)
            target_compile_definitions (benbot_gui PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK=1)
        endif ()
    else ()
        message (WARNING "imgui backend not yet configured for this platform!")
    endif ()
endif ()
