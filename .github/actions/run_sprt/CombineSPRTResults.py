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
# The LLR is the same normalized pentanomial GSPRT that fastchess uses (see getLLR_normalized()
# in fastchess's sprt.cpp), so a combined test gives the same result as a single fastchess run.
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


def get_llr(ptnml: list[int]) -> float:
    # The generalized log-likelihood ratio, using the maximum likelihood estimates of the
    # pentanomial distribution with a normalized Elo of ELO0 & ELO1 (Section 4.1 of
    # Michel Van den Bergh, "Comments on normalized Elo",
    # https://www.cantate.be/Fishtest/normalized_elo_practical.pdf)

    # like fastchess, replace zero counts with a small value so that the logs are finite
    counts = [count if count else 1e-3 for count in ptnml]
    total = sum(counts)
    probs = [count / total for count in counts]

    def mle(t_star: float) -> list[float]:
        # iterative method, starting from a uniform distribution
        p = [1 / len(PAIR_SCORES)] * len(PAIR_SCORES)

        for _ in range(10):
            mu = sum(p_i * a_i for p_i, a_i in zip(p, PAIR_SCORES))
            sigma = math.sqrt(
                sum(p_i * (a_i - mu) ** 2 for p_i, a_i in zip(p, PAIR_SCORES))
            )

            phi = [
                a_i - 0.5 - 0.5 * t_star * sigma * (1 + ((a_i - mu) / sigma) ** 2)
                for a_i in PAIR_SCORES
            ]

            # solve equation 4.9 for theta: the function is decreasing between these bounds
            low = -1 / max(phi)
            high = -1 / min(phi)

            while high - low > 2e-7:
                theta = (low + high) / 2
                if sum(ph * f / (1 + theta * f) for ph, f in zip(probs, phi)) > 0:
                    low = theta
                else:
                    high = theta

            theta = (low + high) / 2

            new_p = [ph / (1 + theta * f) for ph, f in zip(probs, phi)]
            max_diff = max(abs(new - old) for new, old in zip(new_p, p))
            p = new_p

            if max_diff < 1e-4:
                break

        return p

    # the t-values of the bounds; the sqrt(2) is because the outcomes are pairs of games
    p0 = mle(math.sqrt(2) * ELO0 / NELO_PER_T)
    p1 = mle(math.sqrt(2) * ELO1 / NELO_PER_T)

    return total * sum(
        ph * (math.log(p1_i) - math.log(p0_i)) for ph, p0_i, p1_i in zip(probs, p0, p1)
    )


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

llr = get_llr(ptnml)


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
