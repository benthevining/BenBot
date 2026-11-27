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
* A bracketed result can be followed by a search score in centipawns from White's point of view, as in
  `<FEN> [1.0] 34`. `extract` writes these (see below), and tuning ignores them for now.
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

## Making a dataset from self-play

The tuner can also make its own dataset from BenBot's games against itself, in three steps.

**1. Play the games.** With `BENBOT_TUNER` on and fastchess installed, the `tuner_datagen` target plays `ben_bot`
against itself with fastchess, using the same opening book & adjudication as the SPRT tests:

```sh
cmake --build Builds/clang --config Release --target tuner_datagen
```

The games are written to `logs/datagen/games.pgn` (fastchess adds to it if it's already there). Each game starts
from a random opening, and there's no `-repeat`, since colour-swapped pairs of games would add correlated positions.
These cache variables change it:

| Variable | Default | Description |
| --- | --- | --- |
| `BENBOT_DATAGEN_GAMES` | 10000 | The number of games. |
| `BENBOT_DATAGEN_LIMIT` | `tc=2+0.02` | The limit per move, passed to fastchess's `-each`, such as `tc=2+0.02` or `nodes=20000`. |
| `BENBOT_DATAGEN_SEED` | (random) | The seed for the opening order. |

**2. Sample positions from the games.** `benbot_tuner extract` reads the PGNs and writes a dataset file:

```sh
Builds/clang/tools/tuner/Release/benbot_tuner extract --pgn logs/datagen/games.pgn --output ~/chess-data/selfplay.book
```

* The book moves (the moves with a `{book}` comment) and the next `--skip-plies` plies are skipped.
* Positions where the side to move is in check are dropped, and so are positions where the move played has a mate
  score.
* Of the positions left in each game, `--per-game` are chosen at random, since many positions from the same game are
  correlated. The choice only depends on `--seed` and the PGNs, so the same input always gives the same dataset.
* Each position is labelled with its game's result, and followed by the score of the move played from it (from its
  fastchess comment, such as `{+0.36/14 0.244s, n=123456}`), as in `<FEN> [1.0] 34`. The score isn't used yet, but
  keeping it means a blend of the result & the score can be tried later without playing the games again.
* Unfinished games (result `*`) and games that can't be parsed are skipped.

**3. Make the positions quiet.** `benbot_tuner resolve` runs a small quiescence search on each position, and replaces
it with the position at the end of the search's principal variation, keeping its label & score:

```sh
Builds/clang/tools/tuner/Release/benbot_tuner resolve --data ~/chess-data/selfplay.book --output ~/chess-data/selfplay-resolved.book
```

The search is the tuner's own, not the engine's: it only looks at captures & queen promotions, skips the ones that
lose material by SEE, and stands pat on the current eval (when in check, it searches every move instead). It has no
transposition table, history or time limit, so it always gives the same result. Positions whose PV ends in checkmate
are dropped. Public datasets such as `lichess-big3-resolved` are already quiet, so this is only needed for our own
data.

The resolved file can then be tuned on with `--data` as usual.

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

`benbot_tuner [tune] --data <file> [options]` tunes (`tune` is the default command, so it can be left out):

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

`benbot_tuner extract --pgn <file> [options]` samples positions from games:

| Option | Default | Description |
| --- | --- | --- |
| `--pgn <file>` | (required) | A PGN file. Can be given more than once. |
| `--output <file>` | `extracted.book` | Where to write the positions. |
| `--per-game <n>` | 10 | The most positions to take from each game (0 = all of them). |
| `--skip-plies <n>` | 8 | The number of plies to skip after the book moves. |
| `--seed <n>` | 1 | The seed used to choose the positions. |

`benbot_tuner resolve --data <file> [options]` makes a dataset's positions quiet:

| Option | Default | Description |
| --- | --- | --- |
| `--data <file>` | (required) | A dataset file. Can be given more than once; all of them are written to one output. |
| `--output <file>` | `resolved.book` | Where to write the resolved positions. |
| `--threads <n>` | hardware threads | The number of threads. |

## Using the output

The output file is the whole of `DefaultWeights.hpp`, in the same format that the engine's unit tests check. Copy it
over `libbenbot/include/libbenbot/eval/DefaultWeights.hpp`, rebuild, and run the unit tests. The unit tests that
record exact evals (`tests/unit/libbenbot/eval/RecordedEvals.cpp`) will need their expected values updating, and
the position solver tests may change too. Then test the new weights with an SPRT (see `tests/fastchess/README.md`).

Before trusting a tune, check that the numbers make sense: the tuner prints the tuned material values at the end, and
the endgame pawn should still be worth about 100.
