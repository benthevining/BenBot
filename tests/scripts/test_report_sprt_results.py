# ======================================================================================
#
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
#
# ======================================================================================

import pytest

from conftest import ACTIONS_DIR, DATA_DIR, run_script

SCRIPT = ACTIONS_DIR / "run_sprt" / "ReportSPRTResults.py"

PASSED_LOGS = sorted((DATA_DIR / "sprt").glob("pass*.log"))
FAILED_LOGS = sorted((DATA_DIR / "sprt").glob("fail*.log"))
SHARD_LOG = DATA_DIR / "shards" / "round1-shard1.log"


def last_block(log_text: str) -> str:
    # the last "Results of" block, up to the next separator
    lines = log_text.splitlines()
    start = max(idx for idx, line in enumerate(lines) if line.startswith("Results of"))
    end = next(idx for idx in range(start, len(lines)) if lines[idx].startswith("---"))
    return "\n".join(lines[start:end])


@pytest.mark.parametrize("log", PASSED_LOGS, ids=lambda log: log.stem)
def test_passed(log):
    result = run_script(SCRIPT, log)
    assert result.returncode == 0
    assert (
        result.stdout
        == f"Gainer SPRT passed!\n\n```\n{last_block(log.read_text())}\n```\n"
    )


@pytest.mark.parametrize("log", FAILED_LOGS, ids=lambda log: log.stem)
def test_failed(log):
    result = run_script(SCRIPT, log)
    assert result.returncode == 1
    assert (
        result.stdout
        == f"Gainer SPRT failed!\n\n```\n{last_block(log.read_text())}\n```\n"
    )


def test_reports_last_results_block():
    result = run_script(SCRIPT, DATA_DIR / "sprt" / "pass5.log")
    # the fixture has an earlier results block too
    assert "Elo: 54.84 +/- 17.64" in result.stdout
    assert "Elo: 53.37" not in result.stdout
    assert result.stdout.count("Results of") == 1


def test_non_regression(write_log):
    text = (
        (DATA_DIR / "sprt" / "pass1.log")
        .read_text()
        .replace("[0.00, 5.00]", "[-5.00, 0.00]")
    )
    result = run_script(SCRIPT, write_log(text))
    assert result.returncode == 0
    assert result.stdout.startswith("Non-regression SPRT passed!\n")


@pytest.mark.parametrize(
    "log", PASSED_LOGS[:1] + FAILED_LOGS[:1], ids=lambda log: log.stem
)
def test_stopped_before_bound(write_log, log):
    # e.g. the time limit was hit: there's no "SPRT ... completed" line
    text = "".join(
        line
        for line in log.read_text().splitlines(keepends=True)
        if "completed" not in line
    )
    result = run_script(SCRIPT, write_log(text))
    assert result.returncode == 1
    assert result.stdout.startswith("Gainer SPRT stopped before reaching a bound.\n")
    assert last_block(text) in result.stdout


def test_result_line_without_results_block(write_log):
    log = write_log("SPRT ([-5.00, 0.00]) completed - H1 was accepted\n")
    result = run_script(SCRIPT, log)
    assert result.returncode == 0
    assert result.stdout == "Non-regression SPRT passed!\n"


def test_no_results(write_log):
    log = write_log("Started game 1 of 200000 (Refactor vs Baseline)\n")
    result = run_script(SCRIPT, log)
    assert result.returncode != 0
    assert "No results found in fastchess log" in result.stderr


def test_shard_log_without_llr():
    # shards run without -sprt, so their results have no LLR line to get the bounds from
    result = run_script(SCRIPT, SHARD_LOG)
    assert result.returncode != 0
    assert "SPRT bounds not found" in result.stderr
