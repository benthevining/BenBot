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

#include "GLFW_Wrapper.hpp" // NOLINT(build/include_subdir)
#include <GLFW/glfw3.h>
#include <cstdio>
#include <imgui_impl_glfw.h>
#include <libgui/AppUI.hpp>
#include <print>
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

void initialize(AppState& state)
{
    gui::initialize(get_main_scale(), state);
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
