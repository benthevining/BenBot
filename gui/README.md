# BenBot GUI

This directory contains the BenBot ImGUI app.

This application provides a board editor, analysis board, and PGN game viewer, as well as a UI interface for the engine's UCI and search options.

## Design

The application UI is implemented in the `libgui/` library, which aims to be platform agnostic and only makes ImGUI calls.

One of several main files can be chosen to use a specific rendering backend to create a window and run the application event loop.
On MacOS we use Metal; on other platforms we use Vulkan.
Both of these renderers require the glfw library for windowing and event input.

## CMake

### Targets

* `benbot_gui`: GUI application executable

## Dependencies

### C++ libraries

* [imgui](https://github.com/ocornut/imgui), for UI layout/rendering
* [glfw](https://github.com/glfw/glfw), for windowing
* [stb](https://github.com/nothings/stb), for image loading
* [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended), for file chooser dialogs

#### Rendering backends

* [Metal](https://developer.apple.com/metal/), on Apple platforms
* [Vulkan](https://vulkan.lunarg.com/sdk/home), on other platforms
* [gtk](https://www.gtk.org/) is also required on Linux

### Programs

* [inkscape](https://inkscape.org/), for converting SVGs to PNGs (for app icon)
* [ImageMagick](https://imagemagick.org/#gsc.tab=0), needed on Windows for generating app icon
