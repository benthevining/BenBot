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
#
# Replaces the sentence giving the engine's Elo in the README with the estimate from a summary
# printed by tests/fastchess/EstimateElo.py. The sentence must start with "The engine's Elo is"
# (or "ELO") & run to the end of its line.
#
# Usage: UpdateReadmeElo.py <readme> <summary file> <version>
# Exits with 1 if the summary has no estimate or the README has no such sentence.

import re
import sys
from pathlib import Path

README_FILE = Path(sys.argv[1])
SUMMARY_FILE = Path(sys.argv[2])
VERSION = sys.argv[3]

ESTIMATE = re.compile(r"\*\*Estimated Elo: (-?\d+) \+/- (\d+)\*\*")
ELO_SENTENCE = re.compile(r"The engine's E(?:LO|lo) is .*$", re.MULTILINE)

estimate = ESTIMATE.search(SUMMARY_FILE.read_text(encoding="utf-8"))

if estimate is None:
    print(f"No Elo estimate in {SUMMARY_FILE}", file=sys.stderr)
    sys.exit(1)

elo, margin = estimate.groups()

sentence = (
    f"The engine's Elo is estimated at {elo} ± {margin} ({VERSION}), from a gauntlet "
    "against Stockfish's `UCI_Elo` levels (see `tests/fastchess/README.md`)."
)

readme = README_FILE.read_text(encoding="utf-8")
updated, count = ELO_SENTENCE.subn(lambda _: sentence, readme, count=1)

if not count:
    print(f"No sentence giving the Elo in {README_FILE}", file=sys.stderr)
    sys.exit(1)

README_FILE.write_text(updated, encoding="utf-8")
