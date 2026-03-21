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

#pragma once

#ifndef GLFW_INCLUDE_NONE
#    define GLFW_INCLUDE_NONE
#endif

#if defined(__APPLE__) and not defined(GLFW_EXPOSE_NATIVE_COCOA)
#    define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3.h>
#include <cstdio>
#include <imgui_impl_glfw.h>
#include <libgui/AppUI.hpp>
#include <print>
#include <utility>

namespace ben_bot::gui::glfw {

using Window = GLFWwindow;

[[nodiscard]] inline auto create_window() -> Window*
{
    glfwSetErrorCallback([](const int error, const char* description) {
        std::println(stderr, "GLFW error code {}: {}", error, description);
    });

    if (not glfwInit())
        return nullptr;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    return glfwCreateWindow(
        static_cast<int>(1280 * main_scale),
        static_cast<int>(800 * main_scale),
        "BenBot GUI", nullptr, nullptr);
}

inline void initialize(AppState& state)
{
    gui::initialize(
        ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()),
        state);
}

[[nodiscard]] inline auto get_framebuffer_size(Window* window)
    -> std::pair<int, int>
{
    int width { 0 };
    int height { 0 };

    glfwGetFramebufferSize(window, &width, &height);

    return std::make_pair(width, height);
}

inline void shutdown(Window* window, const AppState& state)
{
    ImGui_ImplGlfw_Shutdown();

    gui::shutdown(state);

    glfwDestroyWindow(window);
    glfwTerminate();
}

} // namespace ben_bot::gui::glfw
