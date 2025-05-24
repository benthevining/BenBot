# Tests

This directory contains our test suite.

## CMake

### Options

* `LIBCHESS_TESTS`: this directory is not entered by CMake unless this option is on. Defaults to off unless `libchess` is the top-level CMake project.

### Targets

* `libchess_tests`: unit test executable

## Dependencies

* [Catch2](https://github.com/catchorg/Catch2), unit test framework
