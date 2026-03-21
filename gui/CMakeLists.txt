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

add_executable (benbot_gui MACOSX_BUNDLE WIN32)

target_compile_features (benbot_gui PRIVATE cxx_std_23)

target_link_libraries (benbot_gui PRIVATE ben_bot::libgui imgui::imgui)

target_include_directories (benbot_gui PRIVATE "${CMAKE_CURRENT_LIST_DIR}/platform")

target_sources (benbot_gui PRIVATE icons/icon.icns icons/icon.ico icons/icon.rc)

set_target_properties (
    benbot_gui
    PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "BenBot GUI"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.benbot.gui"
        MACOSX_BUNDLE_INFO_STRING "Chess GUI"
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}"
        MACOSX_BUNDLE_COPYRIGHT
        "Written in 2026 by Ben Vining and released under the terms of the GNU Public License"
)

set_source_files_properties (icons/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

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

# all currently supported backends use GLFW

FetchContent_MakeAvailable (glfw)

target_link_libraries (imgui PUBLIC glfw)

target_sources (benbot_gui PRIVATE "${imgui_backends_dir}/imgui_impl_glfw.cpp")

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
