# Position solver

This directory contains a simple executable that reads test positions with known best moves from EPD files, and exits with an error if the `ben_bot` search algorithm doesn't find the correct move.

Each EPD file may contain multiple positions, and will be executed as one CTest test case.

Each EPD entry must have the following operations:
* `bm`: best move, in algebraic (SAN) notation
* `depth`: search depth. It is desirable to set this as low as possible, while still getting the correct answer. When adding new test positions, a good strategy is to start with depth 4; increase this if the test fails (if it still fails with a depth of 6, we likely won't find the move at any reasonable depth), and decrease this if the test passes.
* `id`: an identifier for the position, which is printed along with the position's FEN string if a test case fails. May also be an explanatory comment describing the position.

Additional operations may be specified.

The test case data files contain positions from the following test suites:
* [Bratko-Kopec Test](https://www.chessprogramming.org/Bratko-Kopec_Test)
* [CCR One Hour Test](https://www.chessprogramming.org/CCR_One_Hour_Test)
* [Kaufman Test](https://www.chessprogramming.org/Kaufman_Test)
* [Louguet Chess Test II](https://www.chessprogramming.org/LCT_II) (LCT)
