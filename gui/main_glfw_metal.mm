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

#ifndef GLFW_INCLUDE_NONE
#    define GLFW_INCLUDE_NONE
#endif

#ifndef GLFW_EXPOSE_NATIVE_COCOA
#    define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <cstdio>
#include <cstdlib>
#include <libgui/AppUI.hpp>
#include <print>

int main(
    [[maybe_unused]] const int    argc,
    [[maybe_unused]] const char** argv)
{
    glfwSetErrorCallback([](const int error, const char* description) {
        std::println(stderr, "GLFW error code {}: {}", error, description);
    });

    if (not glfwInit())
        return EXIT_FAILURE;

    const auto main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto* window = glfwCreateWindow(
        static_cast<int>(1280 * main_scale),
        static_cast<int>(800 * main_scale),
        "BenBot GUI", nullptr, nullptr);

    if (window == nullptr)
        return EXIT_FAILURE;

    ben_bot::gui::initialize(main_scale);

    const id<MTLDevice>       device       = MTLCreateSystemDefaultDevice();
    const id<MTLCommandQueue> commandQueue = [device newCommandQueue];

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplMetal_Init(device);

    NSWindow*     nswin          = glfwGetCocoaWindow(window);
    CAMetalLayer* layer          = [CAMetalLayer layer];
    layer.device                 = device;
    layer.pixelFormat            = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer      = layer;
    nswin.contentView.wantsLayer = YES;

    MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor new];

    ben_bot::gui::AppState state;

    while (not glfwWindowShouldClose(window)) {
        @autoreleasepool {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            glfwPollEvents();

            int width { 0 };
            int height { 0 };
            glfwGetFramebufferSize(window, &width, &height);
            layer.drawableSize                 = CGSizeMake(width, height);
            const id<CAMetalDrawable> drawable = [layer nextDrawable];

            const id<MTLCommandBuffer> commandBuffer             = [commandQueue commandBuffer];
            renderPassDescriptor.colorAttachments[0].clearColor  = MTLClearColorMake(0.45, 0.55, 0.6, 1.);
            renderPassDescriptor.colorAttachments[0].texture     = drawable.texture;
            renderPassDescriptor.colorAttachments[0].loadAction  = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            const id<MTLRenderCommandEncoder> renderEncoder      = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
            [renderEncoder pushDebugGroup:@"BenBot GUI"];

            ImGui_ImplMetal_NewFrame(renderPassDescriptor);
            ImGui_ImplGlfw_NewFrame();

            ben_bot::gui::render(state);

            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

            [renderEncoder popDebugGroup];
            [renderEncoder endEncoding];

            [commandBuffer presentDrawable:drawable];
            [commandBuffer commit];
        }
    }

    ImGui_ImplMetal_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ben_bot::gui::shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
