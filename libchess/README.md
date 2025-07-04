# libchess

This directory defines the core `libchess` library, which implements a generic engine backend.

This library provides:
* A board representation (bitboards)
* Move generation (using magic bitboards)
* Parsing and serialization of FEN, algebraic notation, and PGN
* A framework for handling UCI commands and options

Include style is:
```cpp
#include <libchess/board/Square.hpp>
```

## CMake

### Targets

* `libchess::libchess`: library target

## Dependencies

* [magic_enum](https://github.com/Neargye/magic_enum), for easy serialization & stringification of enums
* [inplace_vector](https://github.com/bemanproject/inplace_vector/tree/main), to help avoid dynamic allocations where possible
