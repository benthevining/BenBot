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
#include <cstdint>
#include <cstdlib>
#include <libgui/AppUI.hpp>
#include <libgui/PieceTextures.hpp>

namespace glfw_wrapper = ben_bot::gui::glfw;

namespace {
// Uploads the piece sprite sheet to a new mipmapped texture.
// The caller owns the returned texture & must release it.
[[nodiscard]] auto create_piece_texture(
    const id<MTLDevice>       device,
    const id<MTLCommandQueue> commandQueue) -> id<MTLTexture>
{
    const auto sheet = ben_bot::gui::rasterize_piece_sprites(
        ben_bot::gui::DefaultPieceSpriteSize);

    const auto width  = static_cast<NSUInteger>(sheet.width());
    const auto height = static_cast<NSUInteger>(sheet.height());

    MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                          width:width
                                                                                         height:height
                                                                                      mipmapped:YES];
    descriptor.usage                 = MTLTextureUsageShaderRead;
    descriptor.storageMode           = MTLStorageModeShared;

    const id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];

    [texture replaceRegion:MTLRegionMake2D(0, 0, width, height)
               mipmapLevel:0
                 withBytes:sheet.pixels.data()
               bytesPerRow:width * 4];

    // the sprites are rasterized larger than they'll usually be drawn,
    // so we need mipmaps to get decent quality when scaling down
    const id<MTLCommandBuffer>      commandBuffer = [commandQueue commandBuffer];
    const id<MTLBlitCommandEncoder> blitEncoder   = [commandBuffer blitCommandEncoder];
    [blitEncoder generateMipmapsForTexture:texture];
    [blitEncoder endEncoding];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];

    return texture;
}
} // namespace

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

    const id<MTLTexture> pieceTexture = create_piece_texture(device, commandQueue);

    state.pieceTextures.textureID = static_cast<std::uint64_t>(
        reinterpret_cast<std::uintptr_t>(pieceTexture));

    NSWindow*     nswin          = glfwGetCocoaWindow(window);
    CAMetalLayer* layer          = [CAMetalLayer layer];
    layer.device                 = device;
    layer.pixelFormat            = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer      = layer;
    nswin.contentView.wantsLayer = YES;

    MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor new];

    while (not glfwWindowShouldClose(window)) {
        @autoreleasepool {
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

    state.pieceTextures = { };
    [pieceTexture release];

    ImGui_ImplMetal_Shutdown();

    glfw_wrapper::shutdown(window, state);

    return EXIT_SUCCESS;
}
