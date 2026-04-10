/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

#define STB_IMAGE_IMPLEMENTATION

#include "GLFW_Wrapper.hpp" // NOLINT(build/include_subdir)
#include <GLFW/glfw3.h>
#include <array>
#include <cstdio>
#include <imgui_impl_glfw.h>
#include <libgui/AppUI.hpp>
#include <libgui/Resources.hpp>
#include <print>
#include <stb_image.h>
#include <string>
#include <utility>

namespace ben_bot::gui::glfw {

namespace {
    [[nodiscard]] auto get_main_scale() -> float
    {
        return ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    }
} // namespace

auto create_window() -> Window*
{
    glfwSetErrorCallback([](const int error, const char* description) {
        std::println(stderr, "GLFW error code {}: {}", error, description);
    });

    if (glfwInit() != GLFW_TRUE)
        return nullptr;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto [width, height] = get_scaled_default_dimensions(get_main_scale());

    return glfwCreateWindow(
        width, height, AppName, nullptr, nullptr);
}

namespace {
    [[maybe_unused]] void set_window_app_icon(Window* window)
    {
        const auto resourceData = resources::get_app_icon();

        if (resourceData.empty())
            return;

        // this is needed because stbi_load_from_memory() takes a mutable pointer to the data!
        std::string iconData { resourceData };

        std::array<GLFWimage, 1uz> images { };

        images.front().pixels = stbi_load_from_memory(
            reinterpret_cast<stbi_uc*>(iconData.data()),
            static_cast<int>(iconData.length()),
            &images.front().width,
            &images.front().height,
            nullptr, 4);

        glfwSetWindowIcon(window, 1, images.data());

        stbi_image_free(images.front().pixels);
    }
} // namespace

void initialize([[maybe_unused]] Window* window, AppState& state)
{
    gui::initialize(get_main_scale(), state);

    // without this guard, GLFW complains that regular windows don't have icons on MacOS
#ifndef __APPLE__
    set_window_app_icon(window);
#endif
}

void shutdown(Window* window, const AppState& state)
{
    ImGui_ImplGlfw_Shutdown();

    gui::shutdown(state);

    glfwDestroyWindow(window);
    glfwTerminate();
}

auto get_framebuffer_size(Window* window)
    -> std::pair<int, int>
{
    int width { 0 };
    int height { 0 };

    glfwGetFramebufferSize(window, &width, &height);

    return std::make_pair(width, height);
}

} // namespace ben_bot::gui::glfw
