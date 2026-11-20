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
# This script is used to parse the output of fastchess running an SPRT test.
# Pipe this script's output into $GITHUB_STEP_SUMMARY.
# This script isn't strictly necessary, it just provides a convenient summary in the GHA summary.
import re
import sys
from pathlib import Path
from typing import Optional, Tuple

LOG_FILE = Path(sys.argv[1])

with open(LOG_FILE) as file:
    SPRT_OUTPUT = file.readlines()


def find_result_line() -> Optional[str]:
    for line in reversed(SPRT_OUTPUT):
        if re.search(r"SPRT .* completed", line):
            return line

    # no result line means that the test was stopped before reaching a bound
    return None


def get_final_results() -> str:
    # fastchess prints a block starting with "Results of ..." each rating interval;
    # the last one has the final Elo, LLR, game counts, etc.
    start = None
    for idx, line in enumerate(SPRT_OUTPUT):
        if line.startswith("Results of"):
            start = idx

    if start is None:
        return ""

    block = []
    for line in SPRT_OUTPUT[start:]:
        if line.startswith("---"):
            break
        block.append(line.rstrip())

    return "\n".join(block)


RESULT_LINE = find_result_line()

FINAL_RESULTS = get_final_results()

if RESULT_LINE is None and not FINAL_RESULTS:
    raise Exception("No results found in fastchess log")


def get_elos() -> Tuple[int, int]:
    # both the result line and the LLR line contain the bounds, like "[0.00, 5.00]"
    line = RESULT_LINE or next(
        l for l in FINAL_RESULTS.splitlines() if l.startswith("LLR:")
    )

    match = re.search(r"\[\s*(-?[\d.]+)\s*,\s*(-?[\d.]+)\s*\]", line)

    if match is None:
        raise Exception(f"SPRT bounds not found in line: {line}")

    return round(float(match.group(1))), round(float(match.group(2)))


def get_accepted() -> int:
    idx = RESULT_LINE.find("H") + 1

    return int(RESULT_LINE[idx : idx + 1].strip())


elo0, elo1 = get_elos()

test_type = "Non-regression" if elo0 < 0 else "Gainer"

if RESULT_LINE is None:
    passed = False
    print(f"{test_type} SPRT stopped before reaching a bound.")
else:
    passed = get_accepted() != 0
    print(f"{test_type} SPRT {'passed' if passed else 'failed'}!")

if FINAL_RESULTS:
    print(f"\n```\n{FINAL_RESULTS}\n```")

sys.exit(0 if passed else 1)
