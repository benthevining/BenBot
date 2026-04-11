# libgui

This library implements the BenBot GUI app. This application provides a board editor, analysis board, PGN game viewer, and a UI for the engine's UCI and search options.

Include style is:
```cpp
#include <libgui/AppUI.hpp>
```

## Design

This library aims to be platform-agnostic. Its public interface is simply a state struct and `initialize()`, `render()`, and `shutdown()` methods. ImGUI is a private dependency.

## CMake

### Targets

* `ben_bot::libgui`: library target

## Dependencies

### C++ libraries

* [imgui](https://github.com/ocornut/imgui), for UI layout/rendering
* [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended), for file chooser dialogs
* [platform-folders](https://github.com/sago007/PlatformFolders/), for finding user documents directory
