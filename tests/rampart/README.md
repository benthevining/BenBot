# rampart

This directory implements the test cases provided by the [rampart](https://github.com/schnitzi/rampart) project. The test cases consist of JSON files containing a starting FEN position and a list of expected generated moves, each with the move in algebraic notation and the FEN position resulting from making the move.

This directory contains an executable that can be run to generate this JSON data using our move generator, and a Python wrapper script to invoke this executable and verify the results against the correct data. The `rampart` executable is typically not meant to be run manually; build the `run_rampart` custom target to execute the wrapper script.
