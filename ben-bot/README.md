# ben-bot

This directory contains the `ben-bot` executable. This executable is a command line UCI engine.

The engine supports several non-standard UCI commands. Type `help` for a list of them.

## CMake

### Targets

* `ben_bot::resources`: resource library target
* `ben_bot::ben_bot`: executable target

## Dependencies

* [cmrc](https://github.com/vector-of-bool/cmrc), used for embedding the opening book data
