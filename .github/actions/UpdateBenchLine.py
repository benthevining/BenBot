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
# Replaces the "Bench: <nodes>" line at the end of a PR description, or adds one if it doesn't have one.
# If the base's bench is given, the paragraph above it says how the bench changed: either
# "No functional change." or e.g. "Bench change: 5727856 → 5450721 (-277135, -4.84%)".
# A previous comparison line directly above the bench line is replaced, so this can be re-run on each push.
# Other lines starting with "Bench:" (e.g. "Bench: 123 -> 456 nodes") are left alone.
#
# Usage: UpdateBenchLine.py <description file> <nodes> [<base nodes>]

import re
import sys
from pathlib import Path

BODY_FILE = Path(sys.argv[1])
NODES = int(sys.argv[2])
BASE_NODES = int(sys.argv[3]) if len(sys.argv) > 3 and sys.argv[3] else None

NO_CHANGE = "No functional change."

BENCH_LINE = re.compile(r"Bench:\s*\d+\s*")
COMPARISON_LINE = re.compile(
    rf"{re.escape(NO_CHANGE)}\s*|Bench change: \d+ → \d+ \([+-]\d+, [+-][\d.]+%\)\s*"
)


def comparison() -> str:
    if NODES == BASE_NODES:
        return NO_CHANGE

    delta = NODES - BASE_NODES
    percent = delta / BASE_NODES * 100 if BASE_NODES else 0.0

    return f"Bench change: {BASE_NODES} → {NODES} ({delta:+d}, {percent:+.2f}%)"


def strip_trailing(lines: list[str], pattern: re.Pattern) -> None:
    # drops any trailing lines matching the pattern & the blank lines before them
    while lines and (not lines[-1].strip() or pattern.fullmatch(lines[-1])):
        lines.pop()


lines = (
    BODY_FILE.read_text(encoding="utf-8").replace("\r\n", "\n").rstrip().splitlines()
)

strip_trailing(lines, BENCH_LINE)
strip_trailing(lines, COMPARISON_LINE)

new_lines = [f"Bench: {NODES}"]

if BASE_NODES is not None:
    new_lines.insert(0, comparison())

for line in new_lines:
    lines += ["", line] if lines else [line]

BODY_FILE.write_text("\n".join(lines) + "\n", encoding="utf-8")
