# ben-bot

This directory contains the `ben_bot` executable. This executable is a command line UCI engine.

Include style is:
```cpp
#include <ben-bot/Engine.hpp>
```

This directory's code is essentially a thin wrapper adapting the `libbenbot` search logic to the `uci::EngineBase` class provided by `libchess`.

The engine supports several non-standard UCI commands. Type `help` for a list of them.

## CMake

### Targets

* `ben_bot::ben_bot`: executable target
