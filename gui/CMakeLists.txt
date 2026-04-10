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

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_subdirectory (libgui)

add_executable (benbot_gui MACOSX_BUNDLE WIN32)

target_compile_features (benbot_gui PRIVATE cxx_std_23)

target_link_libraries (benbot_gui PRIVATE ben_bot::libgui imgui::imgui)

add_subdirectory (glfw_wrapper)

target_link_libraries (benbot_gui PRIVATE ben_bot::glfw_wrapper)

if (APPLE)
    message (VERBOSE "Using GLFW + Metal backend for ImGUI app")

    set_target_properties (
        benbot_gui
        PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "BenBot GUI"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.benbot.gui"
            MACOSX_BUNDLE_INFO_STRING "Chess GUI"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}"
            MACOSX_BUNDLE_COPYRIGHT
            "Written in 2026 by Ben Vining and released under the terms of the GNU Public License"
    )

    include ("${CMAKE_CURRENT_LIST_DIR}/icons/MacOSIcon.cmake")

    target_sources (benbot_gui PRIVATE metal_main.mm "${IMGUI_BACKENDS_DIR}/imgui_impl_metal.mm")

    target_link_libraries (
        benbot_gui PRIVATE "-framework Metal" "-framework MetalKit" "-framework Cocoa"
                           "-framework IOKit" "-framework CoreVideo" "-framework QuartzCore"
    )

    return ()
endif ()

if (WIN32)
    include ("${CMAKE_CURRENT_LIST_DIR}/icons/WindowsIcon.cmake")
endif ()

find_package (Vulkan OPTIONAL_COMPONENTS volk)

if (Vulkan_FOUND)
    message (VERBOSE "Using GLFW + Vulkan backend for ImGUI app")

    target_sources (
        benbot_gui PRIVATE vulkan_main.cpp "${IMGUI_BACKENDS_DIR}/imgui_impl_vulkan.cpp"
    )

    target_link_libraries (benbot_gui PRIVATE Vulkan::Vulkan)

    if (TARGET Vulkan::volk)
        target_link_libraries (benbot_gui PRIVATE Vulkan::volk)
        target_compile_definitions (benbot_gui PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK=1)
    endif ()

    return ()
endif ()

message (FATAL_ERROR "imgui backend not yet configured for this platform!")
