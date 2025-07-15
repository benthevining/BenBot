# perft

The perft function generates all possible moves, plays them all, then generates all possible moves in each resulting position, and so on, to count the total number of move tree nodes reached up to a certain depth.

As the perft values for various chess positions and depths are known, this is useful for debugging move generation. The perft executable can also be timed as a basic form of benchmarking of our move generation functions.

The wrapper script in this directory invokes the `ben-bot` executable to run its perft function, then compares the output to the known stats contained in the `data/` files. Each data file is one CTest test case.
