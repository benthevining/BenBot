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


# --- without the base's bench ---


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


def test_empty_base_arg(update):
    # the workflow passes an empty string when the base's bench isn't known
    assert update("Text.\n", 5, "") == "Text.\n\nBench: 5\n"


# --- with the base's bench ---


def test_no_change(update):
    assert update("Refactor.\n", 5727856, 5727856) == (
        "Refactor.\n\nNo functional change.\n\nBench: 5727856\n"
    )


def test_no_change_empty_body(update):
    assert update("", 5, 5) == "No functional change.\n\nBench: 5\n"


def test_decrease(update):
    assert update("Prune more.\n", 5450721, 5727856) == (
        "Prune more.\n\nBench change: 5727856 → 5450721 (-277135, -4.84%)\n\nBench: 5450721\n"
    )


def test_increase(update):
    assert update("", 1100, 1000) == (
        "Bench change: 1000 → 1100 (+100, +10.00%)\n\nBench: 1100\n"
    )


def test_tiny_change(update):
    assert update("", 10000001, 10000000).startswith(
        "Bench change: 10000000 → 10000001 (+1, +0.00%)\n"
    )


def test_percentage_is_independent_check(update):
    base, new = 4123457, 4398765
    percent = (new - base) / base * 100
    assert f"({new - base:+d}, {percent:+.2f}%)" in update("", new, base)


# --- re-running on later pushes ---


BODY = "## Summary\n\nDoes a thing.\n\n- point 1\n- point 2\n"


def test_rerun_same_values_is_stable(update):
    once = update(BODY, 10, 12)
    assert update(once, 10, 12) == once


@pytest.mark.parametrize(
    "first,second",
    [
        ((10, 10), (11, 10)),
        ((11, 10), (10, 10)),
        ((11, 10), (12, 10)),
        ((10, 10), (10, 10)),
    ],
)
def test_rerun_replaces_comparison(update, first, second):
    once = update(BODY, *first)
    twice = update(once, *second)
    assert twice == update(BODY, *second)
    assert twice.startswith(BODY)
    assert twice.count("Bench") == (1 if second[0] == second[1] else 2)


def test_rerun_without_base_removes_comparison(update):
    # a stale comparison is worse than none
    once = update(BODY, 11, 10)
    assert update(once, 12) == BODY + "\nBench: 12\n"


def test_rerun_after_old_bench_line(update):
    # PRs from before the comparison was added only have the bench line
    assert update(BODY + "\nBench: 9\n", 10, 10) == update(BODY, 10, 10)


def test_rerun_after_user_edit(update):
    once = update(BODY, 11, 10)
    edited = once.replace("Does a thing.", "Does a better thing.")
    assert update(edited, 11, 10) == edited


def test_custom_comparison_lines_left_alone(update):
    # only the script's own format is replaced
    body = "Text.\n\nBench change: lots\n"
    assert update(body, 5, 5) == body + "\nNo functional change.\n\nBench: 5\n"


def test_comparison_above_text_left_alone(update):
    body = "No functional change.\n\nExcept for this.\n"
    assert update(body, 5, 5) == body + "\nNo functional change.\n\nBench: 5\n"
