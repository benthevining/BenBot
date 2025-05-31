# Tests

This directory contains our test suite.

## CMake

### Options

* `LIBCHESS_TESTS`: this directory is not entered by CMake unless this option is on. Defaults to off unless `libchess` is the top-level CMake project.
* `PERFT_DEPTH`: an integer value that controls the maximum depth that the `run_perft` script will run up to. Larger values exponentially increase running time, but provide better test coverage.

### Targets

* `libchess_tests`: unit test executable
* `play_chess`: simple CLI chess game (see `cli/` directory)
* `perft`: perft executable (see `perft/` directory)
  * use the `run_perft` custom target to automatically run all test cases
* `rampart`: detailed move generation test cases (see `rampart/` directory)
  * use the `run_rampart` custom target to automatically run all test cases

## Dependencies

* [Catch2](https://github.com/catchorg/Catch2), unit test framework
* [nlohmann_json](https://json.nlohmann.me/), needed by `perft` and `rampart` for outputting results as JSON
* Python interpreter, needed for `run_perft` and `run_rampart` wrapper scripts
