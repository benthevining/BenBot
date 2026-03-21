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

#include "GLFW_Wrapper.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <cstdlib>
#include <libgui/AppUI.hpp>

namespace glfw_wrapper = ben_bot::gui::glfw;

int main(
    [[maybe_unused]] const int    argc,
    [[maybe_unused]] const char** argv)
{
    auto* window = glfw_wrapper::create_window();

    if (window == nullptr)
        return EXIT_FAILURE;

    ben_bot::gui::AppState state;

    glfw_wrapper::initialize(window, state);

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

    while (not glfwWindowShouldClose(window)) {
        @autoreleasepool {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            glfwPollEvents();

            const auto [width, height] = glfw_wrapper::get_framebuffer_size(window);

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

            render(state);

            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

            [renderEncoder popDebugGroup];
            [renderEncoder endEncoding];

            [commandBuffer presentDrawable:drawable];
            [commandBuffer commit];
        }
    }

    ImGui_ImplMetal_Shutdown();

    glfw_wrapper::shutdown(window, state);

    return EXIT_SUCCESS;
}
