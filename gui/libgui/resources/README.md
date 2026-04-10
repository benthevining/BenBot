# BenBot GUI resources

This directory defines the BenBot GUI resources library.

The main include is:
```cpp
#include <libgui/Resources.hpp>
```

## CMake

### Targets

* `ben_bot::gui_resources`: library target

## Dependencies

* [cmrc](https://github.com/vector-of-bool/cmrc), used to embed resources as static data
* [inkscape](https://inkscape.org/), for converting SVGs to PNGs (for app icon)

[!NOTE]
This library supports being built without `inkscape` present on the host machine. In this case, requesting the app icon resource returns a null `string_view`.
