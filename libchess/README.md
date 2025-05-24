# libchess

This directory defines the core `libchess` library, which implements the engine's backend.

Include style is:
```cpp
#include <libchess/board/Square.hpp>
```

## CMake

### Targets

* `libchess::libchess`: library target

## Dependencies

* [magic_enum](https://github.com/Neargye/magic_enum), for easy serialization & stringification of enums
