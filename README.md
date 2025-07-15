# BenBot

A UCI chess engine.

* Board representation and move generation is in `libchess/`
* Evaluation and search is in `libbenbot/`
* `ben-bot/` is the actual engine executable

See each subdirectory's readme for details.

Building or using `ben-bot`, `libbenbot` or `libchess` requires CMake and C++23.

## CMake options

* `BENBOT_DOCS`: controls whether docs are built (defaults to off unless this is the top-level project)
* `BENBOT_TESTS`: controls whether tests are built (defaults to off unless this is the top-level project)
