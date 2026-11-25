# fastchess

Tests that utilize the `fastchess` tool. This tool can perform UCI compliance testing, and we also use it for SPRT testing.

## SPRT testing

SPRT testing works by playing a match between the previous ("baseline") build and the new build. This directory creates custom targets that you can run to automate this process.

None of these targets are run by CTest; SPRT testing should be invoked manually.

In CI, tagging a PR's title runs an SPRT test of the PR's head against its base commit, and the results are posted as a PR comment:
* `[sprt-gain]` or `[sprt]`: gainer bounds ([0, 5])
* `[sprt-big]`: wider gainer bounds ([0, 10]), for changes expected to gain a lot of Elo; these finish much sooner (wins over `[sprt-gain]` and `[sprt]`)
* `[sprt-nonreg]`: non-regression bounds ([-5, 0]) (wins if the title has more than one tag)

Adding a tag to the title of an existing PR (or switching to a different test type) starts the test; each new push restarts it.

By default the games are played on Linux (x86) runners. Adding `[macos]` to the title as well as one of the tags above (e.g. `fix: ... [sprt-nonreg] [macos]`) plays them on macOS (Apple silicon) runners instead, for changes that only affect behaviour on some platforms (for example, ones that depend on the cache line size, which is 128 bytes on Apple silicon but 64 on x86). `[macos]` on its own does nothing. Adding or removing `[macos]` while a test is running restarts it on the other platform. Public repos only get 5 concurrent macOS jobs, which CI's macOS jobs share, so a macOS test's shards may have to queue. The macOS runners also have 3 cores instead of 4, so each shard plays fewer games at once, and the test gets through fewer games per round.

The PR tests are sharded across 4 runners, in rounds of about 40 minutes. Each shard plays games with a different opening order, without stopping at the SPRT bounds. After each round, the shards' pentanomial results from all the rounds so far are combined (by `.github/actions/run_sprt/CombineSPRTResults.py`), and the test stops if a bound has been reached. The test stops after 16 rounds (about 30000 games, or about 11.5 hours) even if it hasn't reached a bound.

## CMake

### Options

* `BENBOT_SPRT_SHARD`: if `ON`, the SPRT targets play games until they're stopped instead of stopping at the SPRT bounds, so that the results can be combined with other shards'. Defaults to `OFF`.
* `BENBOT_SPRT_SEED`: seed for the SPRT targets' random opening order. If empty (the default), fastchess picks one.

### Targets

* `sprt_set_baseline`: Builds `ben_bot` and uses this new binary as the baseline for the next SPRT test
* `sprt_gain`: runs SPRT testing with gainer bounds
* `sprt_gain_big`: runs SPRT testing with wider gainer bounds
* `sprt_non_regression`: runs SPRT testing with non-regression bounds
* `estimate_elo`: estimates the engine's Elo (see below). Needs Python & Stockfish

## Estimating the Elo

`EstimateElo.py` plays a gauntlet against opponents of known strength & fits one rating to all the results (the maximum likelihood fit of the logistic Elo model). By default the opponents are Stockfish with `UCI_LimitStrength` at `UCI_Elo` 2200, 2500, 2800 & 3100, 200 games each at 60+0.6. The levels should be spread either side of the engine's rating, since a fit to opponents that are all weaker than the engine leans on how the calibration holds up far from 50%. Stockfish calibrated `UCI_Elo` against versions of the Stash engine that are rated on CCRL Blitz, so the estimate is roughly on the CCRL Blitz scale, but only to within about 100 Elo ([Stockfish PR #4341](https://github.com/official-stockfish/Stockfish/pull/4341)). Self-play SPRT gains don't add up to a rating like this, since they overstate how much stronger the engine is against other engines.

```shell
python3 tests/fastchess/EstimateElo.py <path/to/ben_bot> --book <openings.pgn>
```

The main options are `--levels` (Stockfish levels to play, from 1320 to 3190), `--opponent <elo> <cmd>` (another engine with a known rating, can be given more than once), `--games`, `--tc` and `--concurrency`; run it with `--help` for the rest. Each match's log & games go in `logs/elo/`, and `--reuse-logs` skips matches whose log already has results, so an interrupted run can carry on & extra opponents can be added to an earlier run. It prints a Markdown table of the results against each opponent & the estimate with its 95% error margin, and warns when a result is too lopsided to say much, in which case closer levels narrow the error.

With the defaults, a run takes about 5 hours on 8 cores.

The `estimate_elo` workflow runs it in CI & opens a PR that updates the Elo in the main README. It runs for each new release, & can be started by hand on any branch or tag that has the workflow, with other levels, game counts or time controls. Each level is played on its own runner (about 2.5 hours each with the defaults), & a level that hits the time limit still counts the games it finished.

## Dependencies

### Programs
* [fastchess](https://github.com/Disservin/fastchess), for UCI compliance testing and SPRT testing
* [Stockfish](https://stockfishchess.org/), as the opponent for estimating the Elo

## Scripts

* `<BuildDir>/InjectSPRTBaseline-$<CONFIG>.cmake` is generated by CMake and allows you to inject an arbitrary executable path to be used as the baseline for the `sprt` test. Run it with the `BASELINE_BINARY` variable defined to the desired baseline path, for example:
```shell
cmake -D BASELINE_BINARY=ben_bot -P <BuildDir>/InjectSPRTBaseline-Release.cmake
```
