# Tests

This directory contains our test suite.

Results are tracked via the [CDash dashboard](https://my.cdash.org/index.php?project=ben-bot).

## CMake

### Options

* `LIBCHESS_TESTS`: this directory is not entered by CMake unless this option is on. Defaults to off unless `libchess` is the top-level CMake project.

### Targets

* `libchess_tests`: unit test executable
* `perft`: perft executable (see `perft/` directory)
* `rampart`: detailed move generation test cases (see `rampart/` directory)
* `position_solver`: simple test position solver executable (see `position-solver/` directory)

## Dependencies

* [Catch2](https://github.com/catchorg/Catch2), unit test framework
* [nlohmann_json](https://json.nlohmann.me/), needed by `perft` and `rampart` for outputting results as JSON
* Python interpreter, needed for perft and rampart wrapper scripts
* [fastchess](https://github.com/Disservin/fastchess), for UCI compliance testing and SPRT testing
