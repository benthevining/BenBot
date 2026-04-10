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
from typing import Tuple

LOG_FILE = Path(sys.argv[1])

with open(LOG_FILE) as file:
    SPRT_OUTPUT = file.readlines()


def find_result_line() -> str:
    for line in reversed(SPRT_OUTPUT):
        if re.search(r"SPRT .* completed", line):
            return line

    raise Exception("Result line not found in fastchess log")


RESULT_LINE = find_result_line()


def get_elos() -> Tuple[int, int]:
    comma_idx = RESULT_LINE.find(",")

    return (
        round(float(RESULT_LINE[RESULT_LINE.find("[") + 1 : comma_idx].strip())),
        round(float(RESULT_LINE[comma_idx + 1 : RESULT_LINE.find("]")].strip())),
    )


def get_accepted() -> int:
    idx = RESULT_LINE.find("H") + 1

    return int(RESULT_LINE[idx : idx + 1].strip())


elo0, elo1 = get_elos()

test_type = "Non-regression" if elo0 < 0 else "Gainer"

accepted = get_accepted()

if accepted == 0:
    print(f"{test_type} SPRT failed!")
    exit(1)

print(f"{test_type} SPRT passed!")
exit(0)
