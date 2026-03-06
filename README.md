# BenBot

A UCI chess engine.

This is a command-line chess playing program, designed to be used with a chess GUI.

## Strength

We are currently using a handcrafted evaluation, and our search function is still in its early days. The engine's ELO is approximately 1700-1800.

## Repository layout

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

Notes on building:
* If using `gcc`, at least version 14 is required

Our automated CI & releases include the following builds:

|    OS    | Compiler |
|:--------:|:--------:|
|  Ubuntu  |  Clang   |
|  Ubuntu  |   GCC    |
| Windows  |  Clang   |
| Windows  |   MSVC   |
|  MacOS   |  Clang   |

### CMake options

* `BENBOT_DOCS`: controls whether docs are built (defaults to off unless this is the top-level project)
* `BENBOT_TESTS`: controls whether tests are built (defaults to off unless this is the top-level project)

## Thanks

Special thanks to the following open source chess projects, which have provided guidance and inspiration (in no particular order):
* [Stockfish](https://github.com/official-stockfish/Stockfish)
* [Ethereal](https://github.com/AndyGrant/Ethereal)
* [Stormphrax](https://github.com/Ciekce/Stormphrax)
* [Coding Adventure Chess Bot](https://github.com/SebLague/Chess-Coding-Adventure)
