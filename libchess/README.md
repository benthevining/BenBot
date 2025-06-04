# libchess

This directory defines the core `libchess` library, which implements the engine's backend. This library provides a board representation and move generation, parsing and serialization of FEN and algebraic notation, as well as a framework for handling UCI commands and options.

Include style is:
```cpp
#include <libchess/board/Square.hpp>
```

## CMake

### Targets

* `libchess::libchess`: library target

## Dependencies

* [magic_enum](https://github.com/Neargye/magic_enum), for easy serialization & stringification of enums
