# BenBot

A UCI chess engine.

* Board representation and move generation is in `libchess/`
* Evaluation and search is in `libbenbot/`
* `ben-bot/` is the actual engine executable

See each subdirectory's readme for details.

## Building

Building or using `ben-bot`, `libbenbot` or `libchess` requires CMake and C++23.

All dependencies are fetched via `FetchContent`, so everything should "just work" out of the box.

CMake presets are provided for IDE integration, but are not mandatory.

### CMake options

* `BENBOT_DOCS`: controls whether docs are built (defaults to off unless this is the top-level project)
* `BENBOT_TESTS`: controls whether tests are built (defaults to off unless this is the top-level project)
