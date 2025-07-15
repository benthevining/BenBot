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

## Design goals

This library aims to avoid using global state as much as possible. The one exception to this is the magic bitboards move generation, which does require some constant arrays to be initialized at program startup. Aside from this, however, the board position is entirely object-oriented, there is no global `make_move()` function.

Any code produced in the development of `BenBot` that could potentially be useful in other chess programs is put into this library. This library aims to be useful as a basis for any new engine project, or perhaps even chess GUI programs.

## CMake

### Targets

* `ben_bot::libchess`: library target

## Dependencies

* [magic_enum](https://github.com/Neargye/magic_enum), for easy serialization & stringification of enums
* [inplace_vector](https://github.com/bemanproject/inplace_vector/tree/main), to help avoid dynamic allocations where possible
