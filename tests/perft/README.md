# perft

This directory contains a `perft` executable.

The perft function generates all possible moves, plays them all, then generates all possible moves in each resulting position, and so on, to count the total number of move tree nodes reached up to a certain depth.

As the perft values for various chess positions and depths are known, this is useful for debugging move generation. The perft executable can also be timed as a basic form of benchmarking of our move generation functions.

The custom target named `run_perft` can be built to run the `perft` executable from the starting position at various depths and verify the stats it produces against the known correct data in `PerftResults.json`.
