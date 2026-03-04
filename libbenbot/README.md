# libbenbot

This library defines BenBot's evaluation and search logic. This is implemented as a library to make it easy to embed the engine into other applications.

Include style is:
```cpp
#include <libbenbot/search/Search.hpp>
```

## Design goals

This library avoids global state and encapsulates the resources needed to perform a search into a context object. Its interface aims to support both sequential playing of entire games, or one-off searches from a set position.

The core search logic does not depend on the UCI protocol; the `ben-bot/` directory contains code that adapts this library into the UCI wrapper.

## CMake

### Targets

* `ben_bot::libbenbot`: library target

## Dependencies

* [termcolor](https://github.com/ikalnytskyi/termcolor), for printing colored terminal output
