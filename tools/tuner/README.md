# Eval tuner

`benbot_tuner` tunes BenBot's eval weights with Texel's method: it fits the weights so that a sigmoid of the eval
predicts the results of the games that a set of positions came from. It starts from the current default weights and
writes the tuned weights out as a new `libbenbot/include/libbenbot/eval/DefaultWeights.hpp`.

## Building

The tuner isn't built by default. Turn on the `BENBOT_TUNER` CMake option:

```sh
cmake --preset clang -D BENBOT_TUNER=ON
cmake --build Builds/clang --config Release --target benbot_tuner
```

Always use a Release build; tuning in Debug is very slow. With `BUILD_TESTING` on, this also builds the tuner's unit
tests (`tuner_tests`, in `tests/unit/tuner/`).

## Data

Each line of a dataset file is a position followed by the result of its game, from White's point of view. Both common
formats are accepted, and can be mixed:

```
rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1 [1.0]
r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - c9 "1/2-1/2";
```

* The position is a FEN; the move counters are optional, as in EPD. Anything between the FEN & the result (such as
  EPD opcodes like `c9`) is ignored.
* The result is a number in `[0, 1]` in brackets (`[1.0]`, `[0.5]`, `[0.0]`), a quoted `"1-0"`, `"1/2-1/2"` or `"0-1"`,
  or one of those unquoted as the last word on the line.
* Empty lines and lines starting with `#` are skipped. Lines that can't be parsed are counted, the first few are
  printed, and they're skipped.
* Positions where the side to move is in check are dropped, and so are repeats of a position seen earlier (in any of
  the files), by Zobrist hash.

Commonly used datasets are zurichess's `quiet-labeled.epd` (about 725k positions) and `lichess-big3-resolved.book`
(about 7M). Keep datasets out of the repo.

Each position is traced once while loading (see `libbenbot/include/libbenbot/eval/Trace.hpp`), and only the non-zero
coefficients of its tunable weights are kept, so the board itself is never needed again. Every position is checked as
it's loaded: the eval rebuilt from its trace must equal `evaluate()` exactly, or the tuner stops and names the
position.

About 10% of the positions (`--validation`) are held out from training at random, and the loss on them is printed
alongside the training loss.

## How it tunes

* The loss is the mean squared error between the result and `1 / (1 + exp(-K * eval / 400))`, or cross-entropy with
  `--loss ce`. The eval here is from White's point of view, without the engine's final rounding.
* K is fitted once, to the training positions with the default weights (a golden-section search), and is then fixed
  for the whole run. This keeps the tuned eval on about the same centipawn scale as the current one, which matters
  because the search's margins are in centipawns. Use `--k` to set it instead.
* The optimiser is Adam over the whole training set each epoch. Each step moves a weight by about `--lr` centipawns.
* Only the terms marked `tunable` in `TERMS` (`libbenbot/include/libbenbot/eval/Weights.hpp`) change. The others
  (the endgame conversion helpers) keep their default values, and are treated as a fixed part of each position's eval.
* Each piece's material value, piece square table and mobility table are redundant: adding the same amount to every
  entry of a table and taking it off the material doesn't change any eval, so the split between them drifts as it
  tunes. When the weights are written out, each table's average (over the squares that the piece can stand on, so
  ranks 2 to 7 for pawns) is moved into the material, separately for the middlegame & endgame. The tables then average
  0, and the material is the value of a piece on an average square with an average number of moves. This doesn't
  change any eval (the king's table is only shifted, since each side always has one king).
* `--l2` adds a penalty of `x * (w - w0)^2` for each weight's distance from its starting value. With a small `x`, this
  keeps weights that the data says little about (such as a white king on a7 in the middlegame) from running off to
  extreme values, while hardly moving the others. The losses printed don't include it.
* The weights are kept as floating point numbers while tuning, and are rounded to the nearest integer when they're
  written out. The loss with the rounded weights is printed at the end, so you can check that rounding didn't cost
  anything.

## Options

| Option | Default | Description |
| --- | --- | --- |
| `--data <file>` | (required) | A dataset file. Can be given more than once. |
| `--epochs <n>` | 1000 | The maximum number of epochs. |
| `--lr <x>` | 1 | The learning rate, in centipawns per epoch. |
| `--lr-drop-every <n>` | 1000 | Halve the learning rate every `n` epochs (0 = never). |
| `--threads <n>` | hardware threads | The number of threads used to load & to compute the loss and gradient. |
| `--validation <x>` | 0.1 | The fraction of positions held out for validation, in `[0, 1)`. |
| `--output <file>` | `DefaultWeights.hpp` | Where to write the tuned weights. |
| `--save-every <n>` | 100 | Also write the weights every `n` epochs (0 = only at the end). |
| `--loss <mse\|ce>` | `mse` | Mean squared error or cross-entropy. |
| `--k <x>` | fitted | Use this K instead of fitting it. |
| `--l2 <x>` | 0 | Pull each weight towards its starting value with a penalty of `x * (w - w0)^2`. On zurichess, `1e-10` keeps rarely seen weights sane without costing any validation loss. |
| `--patience <n>` | 200 | Stop early if the validation loss hasn't improved for `n` epochs (0 = never). |
| `--seed <n>` | 1 | The seed used to choose the validation positions. |
| `--limit <n>` | 0 (all) | Only load the first `n` positions that pass the filters. |

Options can also be written as `--name=value`. For example:

```sh
Builds/clang/tools/tuner/Release/benbot_tuner --data ~/chess-data/quiet-labeled.epd --epochs 2000 --output tuned.hpp
```

## Using the output

The output file is the whole of `DefaultWeights.hpp`, in the same format that the engine's unit tests check. Copy it
over `libbenbot/include/libbenbot/eval/DefaultWeights.hpp`, rebuild, and run the unit tests. The unit tests that
record exact evals (`tests/unit/libbenbot/eval/RecordedEvals.cpp`) will need their expected values updating, and
the position solver tests may change too. Then test the new weights with an SPRT (see `tests/fastchess/README.md`).

Before trusting a tune, check that the numbers make sense: the tuner prints the tuned material values at the end, and
the endgame pawn should still be worth about 100.
