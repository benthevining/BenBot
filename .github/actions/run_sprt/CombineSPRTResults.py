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
# This script combines the results of an SPRT test that was sharded across several machines.
# Each shard runs fastchess without -sprt, so it plays games until it's stopped, and the
# pentanomial counts from all the shards' logs are summed to compute the overall LLR.
# The LLR is the same normalized pentanomial GSPRT that fastchess uses.
#
# Usage: CombineSPRTResults.py <elo0> <elo1> <final> <log files...>
# where <final> is true if no more games will be played if no bound has been reached.
#
# Prints a Markdown summary, and exits with 0 if the test passed, 1 if it failed or
# didn't reach a bound in the final round, or 2 if more games are needed.

import math
import re
import sys
from pathlib import Path

ALPHA = 0.05
BETA = 0.1

# pentanomial outcomes are the score of a game pair, from 0 to 2 points
PAIR_SCORES = (0.0, 0.25, 0.5, 0.75, 1.0)

# the conversion from normalized Elo to the t-value used by the normalized GSPRT
NELO_PER_T = 800 / math.log(10)

ELO0 = float(sys.argv[1])
ELO1 = float(sys.argv[2])
FINAL = sys.argv[3] == "true"
LOG_FILES = [Path(arg) for arg in sys.argv[4:]]


def get_ptnml(log_file: Path) -> list[int]:
    # fastchess prints its results each rating interval & when the match finishes;
    # the last Ptnml line has the final counts
    counts = [0] * 5

    for line in log_file.read_text().splitlines():
        if match := re.search(r"Ptnml\(0-2\):\s*\[([\d,\s]+)\]", line):
            counts = [int(num) for num in match.group(1).split(",")]

    return counts


ptnml = [0] * 5

for log_file in LOG_FILES:
    ptnml = [total + count for total, count in zip(ptnml, get_ptnml(log_file))]

pairs = sum(ptnml)

lower_bound = math.log(BETA / (1 - ALPHA))
upper_bound = math.log((1 - BETA) / ALPHA)

test_type = "Non-regression" if ELO0 < 0 else "Gainer"

if pairs == 0:
    print(f"{test_type} SPRT didn't finish any games.")
    sys.exit(1 if FINAL else 2)

probs = [count / pairs for count in ptnml]

mean = sum(prob * score for prob, score in zip(probs, PAIR_SCORES))
variance = sum(prob * (score - mean) ** 2 for prob, score in zip(probs, PAIR_SCORES))

if variance == 0:
    # every pair had the same result, so there's nothing to go on yet
    print(f"{test_type} SPRT needs more games.")
    sys.exit(1 if FINAL else 2)

# per-pair standard deviation, in units of a single game's score
sigma = math.sqrt(2 * variance)

score0 = 0.5 + ELO0 / NELO_PER_T * sigma
score1 = 0.5 + ELO1 / NELO_PER_T * sigma

llr = (score1 - score0) * (2 * mean - score0 - score1) / (2 * variance / pairs)


def score_to_elo(score: float) -> float:
    score = min(max(score, 1e-6), 1 - 1e-6)
    return -400 * math.log10(1 / score - 1)


# 95% confidence intervals
error = 1.959964 * math.sqrt(variance / pairs)

elo = score_to_elo(mean)
elo_error = (score_to_elo(mean + error) - score_to_elo(mean - error)) / 2

nelo = (mean - 0.5) / sigma * NELO_PER_T
nelo_error = error / sigma * NELO_PER_T

los = 0.5 * (1 + math.erf((mean - 0.5) / math.sqrt(2 * variance / pairs)))

if llr >= upper_bound:
    status = 0
    print(f"{test_type} SPRT passed!")
elif llr <= lower_bound:
    status = 1
    print(f"{test_type} SPRT failed!")
elif FINAL:
    status = 1
    print(f"{test_type} SPRT stopped before reaching a bound.")
else:
    status = 2
    print(f"{test_type} SPRT hasn't reached a bound yet, playing more games...")

print(f"""
```
Elo: {elo:.2f} +/- {elo_error:.2f}, nElo: {nelo:.2f} +/- {nelo_error:.2f}
LOS: {los * 100:.2f} %
Games: {pairs * 2}, Pairs: {pairs}, Shards: {len(LOG_FILES)}
Ptnml(0-2): [{', '.join(str(count) for count in ptnml)}]
LLR: {llr:.2f} ({lower_bound:.2f}, {upper_bound:.2f}) [{ELO0:.2f}, {ELO1:.2f}]
```""")

sys.exit(status)
