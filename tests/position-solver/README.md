# Position solver

This directory contains a simple executable that reads test positions with known best moves from EPD files, and exits with an error if the `ben_bot` search algorithm doesn't find the correct move.

Each EPD file may contain multiple positions, and will be executed as one CTest test case.

Each EPD entry must have the following operations:
* `bm`: best move, in algebraic (SAN) notation
* `depth`: search depth. It is desirable to set this as low as possible, while still getting the correct answer.
* `comment`: explanatory comment describing the position
