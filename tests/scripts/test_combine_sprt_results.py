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

import math
import re

import pytest

from conftest import ACTIONS_DIR, DATA_DIR, run_script

SCRIPT = ACTIONS_DIR / "run_sprt" / "CombineSPRTResults.py"

SHARD_LOGS = sorted((DATA_DIR / "shards").glob("*.log"))

# logs of complete fastchess SPRT runs with [0, 5] bounds
PASSED_LOGS = sorted((DATA_DIR / "sprt").glob("pass*.log"))
FAILED_LOGS = sorted((DATA_DIR / "sprt").glob("fail*.log"))

LOWER_BOUND = math.log(0.1 / 0.95)
UPPER_BOUND = math.log(0.9 / 0.05)


def combine(elo0, elo1, final, *logs):
    return run_script(SCRIPT, elo0, elo1, "true" if final else "false", *logs)


def last_line(log, prefix: str) -> str:
    return [line for line in log.read_text().splitlines() if line.startswith(prefix)][
        -1
    ]


def ptnml_line(text: str) -> list[int]:
    match = re.search(r"Ptnml\(0-2\): \[([\d, ]+)\]", text)
    return [int(num) for num in match.group(1).split(",")]


def output_line(output: str, prefix: str) -> str:
    return next(line for line in output.splitlines() if line.startswith(prefix))


def llr_of(line: str) -> float:
    return float(line.split()[1])


def approximate_llr(ptnml: list[int], elo0: float, elo1: float) -> float:
    # the simpler approximation of the normalized GSPRT (fishtest's LLR_normalized),
    # which should be close to, but not exactly the same as, the exact MLE used by fastchess
    pairs = sum(ptnml)
    scores = (0, 0.25, 0.5, 0.75, 1)
    mean = sum(count * score for count, score in zip(ptnml, scores)) / pairs
    var = (
        sum(count * (score - mean) ** 2 for count, score in zip(ptnml, scores)) / pairs
    )
    nelo_per_t = 800 / math.log(10)
    s0 = 0.5 + elo0 / nelo_per_t * math.sqrt(2 * var)
    s1 = 0.5 + elo1 / nelo_per_t * math.sqrt(2 * var)
    return (s1 - s0) * (2 * mean - s0 - s1) / (2 * var / pairs)


# --- agreement with fastchess ---


@pytest.mark.parametrize("log", PASSED_LOGS + FAILED_LOGS, ids=lambda log: log.stem)
def test_single_log_matches_fastchess(log):
    result = combine(0, 5, False, log)

    # the same stats as fastchess printed for the same games
    assert output_line(result.stdout, "Elo:") == last_line(log, "Elo:")
    assert output_line(result.stdout, "LOS:") == last_line(log, "LOS:").split(",")[0]
    assert ptnml_line(result.stdout) == ptnml_line(last_line(log, "Ptnml"))

    fastchess_llr = last_line(log, "LLR:")
    assert llr_of(output_line(result.stdout, "LLR:")) == llr_of(fastchess_llr)
    assert output_line(result.stdout, "LLR:").endswith("(-2.25, 2.89) [0.00, 5.00]")

    games = int(re.search(r"Games: (\d+)", last_line(log, "Games:")).group(1))
    assert f"Games: {games}, Pairs: {games // 2}, Shards: 1" in result.stdout


@pytest.mark.parametrize("log", PASSED_LOGS, ids=lambda log: log.stem)
def test_passed(log):
    result = combine(0, 5, False, log)
    assert result.returncode == 0
    assert result.stdout.startswith("Gainer SPRT passed!")


@pytest.mark.parametrize("log", FAILED_LOGS, ids=lambda log: log.stem)
def test_failed(log):
    result = combine(0, 5, False, log)
    assert result.returncode == 1
    assert result.stdout.startswith("Gainer SPRT failed!")


@pytest.mark.parametrize("log", PASSED_LOGS + FAILED_LOGS, ids=lambda log: log.stem)
def test_llr_close_to_approximation(log):
    result = combine(0, 5, False, log)
    llr = llr_of(output_line(result.stdout, "LLR:"))
    approx = approximate_llr(ptnml_line(last_line(log, "Ptnml")), 0, 5)
    assert llr == pytest.approx(approx, abs=0.15)


# --- combining shards ---


def test_real_shards():
    result = combine(-5, 0, False, *SHARD_LOGS)

    assert result.returncode == 2
    assert result.stdout.startswith(
        "Non-regression SPRT hasn't reached a bound yet, playing more games..."
    )

    # each shard's counts are the last Ptnml line in its log
    expected = [0] * 5
    for log in SHARD_LOGS:
        expected = [
            a + b for a, b in zip(expected, ptnml_line(last_line(log, "Ptnml")))
        ]

    assert ptnml_line(result.stdout) == expected
    assert (
        f"Games: {sum(expected) * 2}, Pairs: {sum(expected)}, Shards: 16"
        in result.stdout
    )
    assert "Games: 7358, Pairs: 3679, Shards: 16" in result.stdout
    assert "Elo: -0.19 +/- 6.85, nElo: -0.22 +/- 7.94" in result.stdout
    assert "LOS: 47.84 %" in result.stdout
    assert "LLR: 0.69 (-2.25, 2.89) [-5.00, 0.00]" in result.stdout

    llr = llr_of(output_line(result.stdout, "LLR:"))
    assert llr == pytest.approx(approximate_llr(expected, -5, 0), abs=0.05)


def test_real_shards_final_round():
    result = combine(-5, 0, True, *SHARD_LOGS)
    assert result.returncode == 1
    assert result.stdout.startswith(
        "Non-regression SPRT stopped before reaching a bound."
    )
    assert "LLR: 0.69" in result.stdout


def test_output_format():
    result = combine(-5, 0, False, *SHARD_LOGS)
    lines = result.stdout.splitlines()
    assert lines[1] == ""
    assert lines[2] == "```"
    assert lines[-1] == "```"
    assert [line.split(":")[0] for line in lines[3:-1]] == [
        "Elo",
        "LOS",
        "Games",
        "Ptnml(0-2)",
        "LLR",
    ]


def test_split_log_gives_same_result(write_log):
    # one fastchess run's games split across several shards gives the same result as the whole run
    log = PASSED_LOGS[0]
    total = ptnml_line(last_line(log, "Ptnml"))
    part1 = [count // 3 for count in total]
    part2 = [count // 2 for count in total]
    part3 = [t - a - b for t, a, b in zip(total, part1, part2)]

    shards = [
        write_log(f"Ptnml(0-2): [{', '.join(map(str, part))}], WL/DD Ratio: 1.00\n")
        for part in (part1, part2, part3)
    ]

    whole = combine(0, 5, False, log)
    split = combine(0, 5, False, *shards)

    assert split.returncode == whole.returncode
    assert split.stdout.replace("Shards: 3", "Shards: 1") == whole.stdout


def test_uses_last_ptnml_line(write_log):
    log = write_log("""\
Ptnml(0-2): [1, 2, 3, 4, 5], WL/DD Ratio: 1.00
--------------------------------------------------
Ptnml(0-2): [10, 20, 30, 20, 10], WL/DD Ratio: 1.00
--------------------------------------------------
""")
    result = combine(-5, 0, False, log)
    assert "Ptnml(0-2): [10, 20, 30, 20, 10]" in result.stdout
    assert "Games: 180, Pairs: 90, Shards: 1" in result.stdout


def test_shard_without_games(write_log):
    # e.g. a shard whose build failed, or that was stopped before its first rating interval
    empty = write_log("Started game 1 of 200000 (Refactor vs Baseline)\n")

    with_empty = combine(-5, 0, False, *SHARD_LOGS, empty)
    without = combine(-5, 0, False, *SHARD_LOGS)

    assert with_empty.returncode == without.returncode
    assert with_empty.stdout.replace("Shards: 17", "Shards: 16") == without.stdout


@pytest.mark.parametrize("final", [False, True])
def test_no_games(write_log, final):
    # e.g. fastchess couldn't start, so more rounds won't help
    log = write_log("ninja: build stopped: interrupted by user.\n")
    result = combine(-5, 0, final, log)
    assert result.returncode == 1
    assert (
        result.stdout.strip()
        == "Non-regression SPRT didn't finish any games, check the logs for details."
    )


@pytest.mark.parametrize("final", [False, True])
def test_no_logs(final):
    result = combine(0, 5, final)
    assert result.returncode == 1
    assert (
        result.stdout.strip()
        == "Gainer SPRT didn't finish any games, check the logs for details."
    )


@pytest.mark.parametrize("final,status", [(False, 2), (True, 1)])
def test_all_pairs_same_result(write_log, final, status):
    log = write_log("Ptnml(0-2): [0, 0, 50, 0, 0], WL/DD Ratio: 0.00\n")
    result = combine(0, 5, final, log)
    assert result.returncode == status
    assert result.stdout.strip() == "Gainer SPRT needs more games."


def test_zero_counts(write_log):
    # zero counts in some outcomes mustn't break the LLR (fastchess regularizes them)
    log = write_log("Ptnml(0-2): [0, 0, 100, 150, 200], WL/DD Ratio: 0.00\n")
    result = combine(0, 5, False, log)
    assert result.returncode == 0
    assert result.stdout.startswith("Gainer SPRT passed!")
    assert llr_of(output_line(result.stdout, "LLR:")) > UPPER_BOUND


def test_clear_loss(write_log):
    log = write_log("Ptnml(0-2): [200, 150, 100, 0, 0], WL/DD Ratio: 0.00\n")
    result = combine(-5, 0, False, log)
    assert result.returncode == 1
    assert result.stdout.startswith("Non-regression SPRT failed!")
    assert llr_of(output_line(result.stdout, "LLR:")) < LOWER_BOUND
    assert "LOS: 0.00 %" in result.stdout


def test_even_match(write_log):
    # symmetric results: Elo is exactly 0 and the LLR is halfway between the hypotheses
    log = write_log("Ptnml(0-2): [10, 20, 40, 20, 10], WL/DD Ratio: 1.00\n")
    result = combine(-5, 0, False, log)
    assert result.returncode == 2
    assert "Elo: 0.00 +/- " in result.stdout
    assert "LOS: 50.00 %" in result.stdout
    # at the midpoint of the bounds the LLR is (nearly) 0
    mid = combine(-2.5, 2.5, False, log)
    assert abs(llr_of(output_line(mid.stdout, "LLR:"))) < 0.01
