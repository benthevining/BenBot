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

SCRIPT = ACTIONS_DIR / "UpdateBenchLine.py"


@pytest.fixture
def update(tmp_path):
    # writes the body to a file, runs the script on it & returns the updated body
    body_file = tmp_path / "body.md"

    def run(body: str, nodes, base_nodes=None, *, newline="\n") -> str:
        if body is not None:
            body_file.write_bytes(body.replace("\n", newline).encode())
        args = [body_file, nodes] + ([] if base_nodes is None else [base_nodes])
        result = run_script(SCRIPT, *args)
        assert result.returncode == 0, result.stderr
        return body_file.read_text(encoding="utf-8")

    return run


def test_empty_body(update):
    assert update("", 123) == "Bench: 123\n"


def test_whitespace_body(update):
    assert update("\n  \n\n", 123) == "Bench: 123\n"


def test_appends(update):
    assert update("Some change.\n", 123) == "Some change.\n\nBench: 123\n"


def test_replaces(update):
    assert update("Some change.\n\nBench: 100\n", 123) == "Some change.\n\nBench: 123\n"


def test_replaces_several_trailing_lines(update):
    body = "Some change.\n\nBench: 100\nBench: 200\n\n\n"
    assert update(body, 123) == "Some change.\n\nBench: 123\n"


def test_crlf(update):
    assert (
        update("Line 1\nLine 2\n\nBench: 1\n", 123, newline="\r\n")
        == "Line 1\nLine 2\n\nBench: 123\n"
    )


def test_leaves_other_bench_lines(update):
    body = "Bench: 123 -> 456 nodes\n\nBench: 7 was the old value\n"
    assert update(body, 999) == body + "\nBench: 999\n"


def test_bench_line_in_middle_left_alone(update):
    body = "Bench: 5\n\nMore text.\n"
    assert update(body, 6) == "Bench: 5\n\nMore text.\n\nBench: 6\n"


def test_unicode(update):
    body = "Speeds up → movegen 🚀\n"
    assert update(body, 1) == body + "\nBench: 1\n"
