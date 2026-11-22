# CI script tests

[pytest](https://docs.pytest.org/) tests for the Python scripts used by the CI workflows:

* `.github/actions/UpdateBenchLine.py`, which keeps the bench line at the end of PR descriptions up to date
* `.github/actions/run_sprt/CombineSPRTResults.py`, which combines the results of sharded SPRT tests
* `.github/actions/run_sprt/ReportSPRTResults.py`, which summarizes a fastchess SPRT log
* `tests/fastchess/EstimateElo.py`, which estimates the engine's Elo from a gauntlet

These aren't registered with CTest, because they don't test the engine. To run them:

```sh
pip install -r tests/scripts/requirements.txt
python3 -m pytest tests/scripts
```

The `script_tests` workflow runs them when the scripts or tests change.

The logs in `data/` are real fastchess logs, trimmed to the lines that the scripts read.
`data/sprt/` has complete SPRT runs with `[0, 5]` bounds, which check that `CombineSPRTResults.py`
gives the same Elo, LOS & LLR as fastchess. `data/shards/` has the 16 shard logs of a sharded
`[-5, 0]` SPRT of the engine against itself. `data/elo/` has the logs of a short gauntlet against
Stockfish at `UCI_Elo` 1800 & 2400.
