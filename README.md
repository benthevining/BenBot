# BenBot

A UCI chess engine.

* Board representation and move generation is in `libchess/`
* Evaluation and search is in `libbenbot/`
* `ben-bot/` is the actual engine executable

See each subdirectory's readme for details.

## Links

* [Lichess profile](https://lichess.org/@/ben-bot)
* [Documentation](http://benthevining.github.io/BenBot/)
* [CDash dashboard](https://my.cdash.org/index.php?project=ben-bot)
* [Latest release](https://github.com/benthevining/BenBot/releases/latest)

## Building

Building or using `ben-bot`, `libbenbot` or `libchess` requires CMake and C++23.

All dependencies are fetched via `FetchContent`, so everything should "just work" out of the box.

CMake presets are provided for IDE integration, but are not mandatory.

Our automated CI & releases include the following builds:

|    OS    | Compiler |          Notes           |
|:--------:|:--------:|:------------------------:|
|  Ubuntu  |  Clang   |                          |
|  Ubuntu  |   GCC    | Requires at least GCC 14 |
| Windows  |  Clang   |                          |
| Windows  |   MSVC   |                          |
|  MacOS   |  Clang   | Builds universal binary  |

### CMake options

* `BENBOT_DOCS`: controls whether docs are built (defaults to off unless this is the top-level project)
* `BENBOT_TESTS`: controls whether tests are built (defaults to off unless this is the top-level project)
