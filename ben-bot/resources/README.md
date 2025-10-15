# ben-bot resources

This directory define's the `ben-bot` engine's resources library.

The main include is:
```cpp
#include <ben-bot/Resources.hpp>
```

## CMake

### Options

* `BENBOT_ALWAYS_REBUILD_RESOURCES`: if on, the resources library target is rebuilt every time a build is triggered, so that it always reports the most up-to-date build time. When this option is off, the `libchess`, `libbenbot` or `ben_bot` targets may be rebuilt without rebuilding the resources library, causing `ben_bot`'s `compiler` command output to report a build time earlier than the executable was actually compiled. However, the tradeoff is that when this option is enabled, CMake will never report "no work to do" when rebuilding the `all` target. Defaults to off.

### Targets

* `ben_bot::resources`: library target

## Dependencies

* [cmrc](https://github.com/vector-of-bool/cmrc), used to embed resources as static data
