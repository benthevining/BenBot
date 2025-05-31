# perft

This directory contains a `perft` executable.

The perft function generates all possible moves, plays them all, then generates all possible moves in each resulting position, and so on, to count the total number of move tree nodes reached up to a certain depth.

As the perft values for various chess positions and depths are known, this is useful for debugging move generation. The perft executable can also be timed as a basic form of benchmarking of our move generation functions.

A set of scripts named `RunPerft<Config>.py` will be generated into the top-level build directory, which will run the perft executable and compare the results to the correct results, which are stored in `PerftResults.json`.
