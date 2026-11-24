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

from conftest import ACTIONS_DIR, run_script

SCRIPT = ACTIONS_DIR / "UpdateReadmeElo.py"

SUMMARY = """## BenBot Elo estimate

| Opponent | Rating | Games | W / D / L | Score | Performance |
|---|---|---|---|---|---|
| Stockfish-1800 | 1800 | 8 | 6 / 0 / 2 | 75.0% | 1991 |

**Estimated Elo: 2058 +/- 259** (95%)
"""

NEW_SENTENCE = (
    "The engine's Elo is estimated at 2058 ± 259 (v1.2.3), from a gauntlet against "
    "Stockfish's `UCI_Elo` levels (see `tests/fastchess/README.md`)."
)


@pytest.fixture
def update(tmp_path):
    # writes the README & summary to files, runs the script on them & returns the result & README
    readme_file = tmp_path / "README.md"
    summary_file = tmp_path / "summary.md"

    def run(readme: str, summary: str = SUMMARY):
        readme_file.write_text(readme, encoding="utf-8")
        summary_file.write_text(summary, encoding="utf-8")
        result = run_script(SCRIPT, readme_file, summary_file, "v1.2.3")
        return result, readme_file.read_text(encoding="utf-8")

    return run


def test_replaces_old_sentence(update):
    readme = (
        "## Strength\n\n"
        "We are using a handcrafted evaluation. The engine's ELO is approximately 1700-1800.\n\n"
        "## Next\n"
    )
    result, updated = update(readme)
    assert result.returncode == 0, result.stderr
    assert updated == (
        "## Strength\n\n"
        f"We are using a handcrafted evaluation. {NEW_SENTENCE}\n\n"
        "## Next\n"
    )


def test_replaces_its_own_sentence(update):
    readme = (
        "Intro. The engine's Elo is estimated at 1900 ± 80 (v1.0.0), from a gauntlet "
        "against Stockfish's `UCI_Elo` levels (see `tests/fastchess/README.md`).\nMore.\n"
    )
    result, updated = update(readme)
    assert result.returncode == 0, result.stderr
    assert updated == f"Intro. {NEW_SENTENCE}\nMore.\n"


def test_no_estimate(update):
    readme = "The engine's Elo is approximately 1700.\n"
    result, updated = update(
        readme, "**No estimate:** every game was won or every game was lost.\n"
    )
    assert result.returncode == 1
    assert "No Elo estimate" in result.stderr
    assert updated == readme


def test_no_sentence(update):
    readme = "A UCI chess engine.\n"
    result, updated = update(readme)
    assert result.returncode == 1
    assert "No sentence" in result.stderr
    assert updated == readme
