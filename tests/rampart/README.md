# rampart

This directory implements the test cases provided by the [rampart](https://github.com/schnitzi/rampart) project. The test cases consist of JSON files containing a starting FEN position and a list of expected generated moves, each with the move in algebraic notation and the FEN position resulting from making the move.

This directory contains an executable that can be run to generate this JSON data using our move generator, and a Python wrapper script to invoke this executable and verify the results against the correct data. CTest test cases are added to run this wrapper script. Invoking CTest is the recommended way to run these tests.

## CMake

### Targets

* `rampart`: executable that writes rampart-style output for a given position

## Dependencies

### Programs
* Python interpreter, needed for runner wrapper script

### C++ libraries
* [nlohmann_json](https://json.nlohmann.me/), needed by `rampart` for outputting results as JSON
