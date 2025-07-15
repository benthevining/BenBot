# Position solver

This directory contains a set of EPD files describing a test position with a known best move. The Python script in this directory executes the `ben-bot` executable in the test position and checks that it outputs the best move.

Each EPD entry must have the following operations:
* `bm`: best move, in algebraic (SAN) notation
* `depth`: search depth. It is desirable to set this as low as possible, while still getting the correct answer. When adding new test positions, a good strategy is to start with depth 4; increase this if the test fails (if it still fails with a depth of 6, we likely won't find the move at any reasonable depth), and decrease this if the test passes.

Additional operations may be specified.

The test case data files contain positions from the following test suites:
* [Bratko-Kopec Test](https://www.chessprogramming.org/Bratko-Kopec_Test)
* [CCR One Hour Test](https://www.chessprogramming.org/CCR_One_Hour_Test)
* [Kaufman Test](https://www.chessprogramming.org/Kaufman_Test)
* [Louguet Chess Test II](https://www.chessprogramming.org/LCT_II) (LCT)

## Dependencies

* [python-chess](https://python-chess.readthedocs.io/en/latest/index.html), used by the wrapper script
