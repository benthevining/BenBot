# Unit tests

This directory contains unit tests, which are written using Catch2.

## CMake

### Targets

* `libbenbot_tests`: unit test executable for `libbenbot`
* `libchess_tests`: unit test executable for `libchess`
* `tuner_tests`: unit test executable for the eval tuner (only if `BENBOT_TUNER` is on)

## Dependencies

### C++ libraries
* [Catch2](https://github.com/catchorg/Catch2), unit test framework
