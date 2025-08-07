# Tests

This directory contains our test suite.

Results are tracked via the [CDash dashboard](https://my.cdash.org/index.php?project=ben-bot).

## CMake

### Options

* `BENBOT_TESTS`: this directory is not entered by CMake unless this option is on. Defaults to off unless `BenBot` is the top-level CMake project.

### Targets

* `libchess_tests`: unit test executable
* `rampart`: detailed move generation test cases

## Dependencies

### Programs
* Python interpreter, needed for wrapper scripts
* [fastchess](https://github.com/Disservin/fastchess), for UCI compliance testing and SPRT testing

### C++ libraries
* [Catch2](https://github.com/catchorg/Catch2), unit test framework
* [nlohmann_json](https://json.nlohmann.me/), needed by `rampart` for outputting results as JSON

### Python libraries
* [python-chess](https://python-chess.readthedocs.io/en/latest/index.html), used by the position solver wrapper script
