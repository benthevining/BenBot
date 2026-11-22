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

import importlib.util
import math
import shutil

import pytest

from conftest import DATA_DIR, REPO_ROOT, run_script

SCRIPT = REPO_ROOT / "tests" / "fastchess" / "EstimateElo.py"

LOGS_DIR = DATA_DIR / "elo"

# the script runs its matches only from main(), so it can be imported for its functions
spec = importlib.util.spec_from_file_location("EstimateElo", SCRIPT)
estimate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(estimate)


def result(elo: float, wins: int, draws: int, losses: int):
    opponent = estimate.Opponent(name=f"opponent-{elo}", elo=elo, cmd="", options=[])
    return estimate.Result(opponent, wins, draws, losses)


def test_parse_results_uses_last_block():
    log = (
        "Games: 4, Wins: 1, Losses: 2, Draws: 1, Points: 1.5 (37.50 %)\n"
        "Games: 8, Wins: 6, Losses: 2, Draws: 0, Points: 6.0 (75.00 %)\n"
    )
    assert estimate.parse_results(log) == (6, 0, 2)


def test_parse_results_real_log():
    log = (LOGS_DIR / "vs-Stockfish-2400.log").read_text()
    assert estimate.parse_results(log) == (1, 1, 6)


def test_parse_results_no_games():
    assert estimate.parse_results("Indexing opening suite...\n") is None


def test_one_opponent_matches_performance_rating():
    # a 75% score is 400 * log10(3) above the opponent
    elo, _ = estimate.estimate_elo([result(2000, 60, 30, 10)])
    assert elo == pytest.approx(2000 + 400 * math.log10(3), abs=0.1)


def test_equal_scores_either_side():
    # 75% against 1800 & 25% against 2200 is the same distance from each
    elo, _ = estimate.estimate_elo([result(1800, 70, 10, 20), result(2200, 20, 10, 70)])
    assert elo == pytest.approx(2000, abs=0.1)


def test_lopsided_results_still_count():
    close = [result(2000, 50, 0, 50)]
    elo, _ = estimate.estimate_elo(close)
    assert elo == pytest.approx(2000, abs=0.1)

    # 99% against an opponent 500 below is a performance of about 2300, so it still pulls the
    # estimate up, though it counts for less than the close match
    elo, _ = estimate.estimate_elo(close + [result(1500, 99, 0, 1)])
    assert elo < 2150
    assert elo > 2000


def test_margin_shrinks_with_more_games():
    _, margin = estimate.estimate_elo([result(2000, 40, 20, 40)])
    _, bigger = estimate.estimate_elo([result(2000, 160, 80, 160)])
    assert margin == pytest.approx(2 * bigger, rel=1e-3)


def test_draws_narrow_the_margin():
    _, decisive = estimate.estimate_elo([result(2000, 50, 0, 50)])
    _, drawish = estimate.estimate_elo([result(2000, 25, 50, 25)])
    assert drawish < decisive


def test_one_opponent_margin_matches_normal_approximation():
    # for one opponent at 50%, the trinomial standard error of the score is
    # sqrt(p_win + p_loss) / 2 per game, & the expected score changes by 1 / (4 * ELO_PER_NAT)
    # per Elo, so the margin is 1.96 * 2 * ELO_PER_NAT * sqrt((W + L) / N) / sqrt(N)
    _, margin = estimate.estimate_elo([result(2000, 30, 40, 30)])
    expected = 1.96 * 2 * estimate.ELO_PER_NAT * math.sqrt(0.6) / math.sqrt(100)
    assert margin == pytest.approx(expected, rel=1e-3)


@pytest.mark.parametrize("wins,losses", [(10, 0), (0, 10)])
def test_no_fit_for_clean_sweeps(wins, losses):
    with pytest.raises(ValueError):
        estimate.estimate_elo([result(2000, wins, 0, losses)])


def test_reuses_existing_logs(tmp_path):
    for log in LOGS_DIR.glob("*.log"):
        shutil.copy(log, tmp_path)

    # fastchess is a program that would fail, so the results must come from the logs
    result = run_script(
        SCRIPT,
        "ben_bot",
        "--book",
        "book.pgn",
        "--stockfish",
        "stockfish",
        "--levels",
        "1800",
        "2400",
        "--fastchess",
        "missing-fastchess",
        "--out-dir",
        tmp_path,
        "--reuse-logs",
    )

    assert result.returncode == 0, result.stderr
    assert "| Stockfish-1800 | 1800 | 8 | 6 / 0 / 2 | 75.0% | 1991 |" in result.stdout
    assert "| Stockfish-2400 | 2400 | 8 | 1 / 1 / 6 | 18.8% | 2145 |" in result.stdout
    assert "**Estimated Elo: 2058 +/- 259** (95%)" in result.stdout


def test_no_opponents():
    result = run_script(
        SCRIPT, "ben_bot", "--book", "book.pgn", "--levels", "--fastchess", "fastchess"
    )
    assert result.returncode != 0
    assert "No opponents" in result.stderr
