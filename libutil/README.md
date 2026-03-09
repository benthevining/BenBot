# libutil

This library contains generic utilities and platform abstractions used throughout this codebase, but not directly related to chess-specific logic.

Include style is:
```cpp
#include <libutil/Chrono.hpp>
```

## CMake

### Targets

* `ben_bot::libutil`: library target

## Dependencies

* [magic_enum](https://github.com/Neargye/magic_enum), for easy serialization & stringification of enums
* [inplace_vector](https://github.com/bemanproject/inplace_vector/tree/main), to help avoid dynamic allocations where possible
