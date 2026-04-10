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

include ("${CMAKE_CURRENT_LIST_DIR}/icons/SVGtoPNG.cmake")

add_subdirectory (libgui)

add_executable (benbot_gui MACOSX_BUNDLE WIN32)

target_compile_features (benbot_gui PRIVATE cxx_std_23)

target_link_libraries (benbot_gui PRIVATE ben_bot::libgui imgui::imgui)

target_sources (benbot_gui PRIVATE icons/icon.ico icons/icon.rc)

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

function (benbot_use_glfw)
    FetchContent_MakeAvailable (glfw stb)

    add_library (
        benbotgui_glfw_wrapper STATIC "${imgui_backends_dir}/imgui_impl_glfw.cpp"
                                      platform/GLFW_Wrapper.cpp
    )

    target_link_libraries (
        benbotgui_glfw_wrapper PUBLIC glfw ben_bot::libgui PRIVATE imgui::imgui
                                                                   ben_bot::gui_resources
    )

    target_include_directories (
        benbotgui_glfw_wrapper PUBLIC "${CMAKE_CURRENT_LIST_DIR}/platform"
        PRIVATE "${stb_SOURCE_DIR}"
    )

    target_link_libraries (benbot_gui PRIVATE benbotgui_glfw_wrapper)
endfunction ()

if (APPLE)
    message (VERBOSE "Using GLFW + Metal backend for ImGUI app")

    benbot_use_glfw ()

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

        benbot_use_glfw ()

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
