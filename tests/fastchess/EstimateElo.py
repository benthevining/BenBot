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
# This script estimates the engine's Elo by playing a gauntlet with fastchess against
# opponents of known strength, & fitting one rating to all the results.
#
# By default the opponents are Stockfish at several UCI_Elo levels. Stockfish calibrated
# UCI_Elo against versions of the Stash engine that are rated on CCRL Blitz, so the estimate
# is roughly on the CCRL Blitz scale, but only to within about 100 Elo (see
# https://github.com/official-stockfish/Stockfish/pull/4341). Other engines with known
# ratings can be added with --opponent.
#
# The rating is the maximum likelihood fit of the logistic Elo model to the scores against
# every opponent, so a level far above or below the engine still counts, just for less.
#
# Usage: EstimateElo.py <engine> --book <openings file> [options]
# Run with --help for the options. Each match's log & games are written to --out-dir, and with
# --reuse-logs a match whose log already has results isn't played again, so an interrupted
# run can carry on, or more opponents can be added to an earlier run.
#
# Prints a Markdown summary of the results, & exits with 1 if no games were played.

import argparse
import math
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

ELO_PER_NAT = 400 / math.log(10)

DEFAULT_LEVELS = [2200, 2500, 2800, 3100]

# scores further from 50% than this tell us little about the rating, so they get a warning
LOPSIDED_SCORE = 0.9


@dataclass
class Opponent:
    name: str
    elo: float
    cmd: str
    options: list[str]


@dataclass
class Result:
    opponent: Opponent
    wins: int
    draws: int
    losses: int

    @property
    def games(self) -> int:
        return self.wins + self.draws + self.losses

    @property
    def score(self) -> float:
        return (self.wins + 0.5 * self.draws) / self.games


def parse_results(log_text: str) -> Optional[tuple[int, int, int]]:
    # fastchess prints a "Games: ..." line in each results block, from the first engine's
    # point of view; the last one has the final counts
    matches = re.findall(
        r"^Games: \d+, Wins: (\d+), Losses: (\d+), Draws: (\d+)", log_text, re.MULTILINE
    )

    if not matches:
        return None

    wins, losses, draws = (int(count) for count in matches[-1])
    return wins, draws, losses


def expected_score(elo: float, opponent_elo: float) -> float:
    return 1 / (1 + 10 ** ((opponent_elo - elo) / 400))


def estimate_elo(results: list[Result]) -> tuple[float, float]:
    # Returns the rating & its 95% error margin.
    # The maximum likelihood rating is where the total expected score equals the total score
    # actually made, & the expected score only goes up with the rating, so bisect for it.
    results = [result for result in results if result.games]

    games = sum(result.games for result in results)
    points = sum(result.wins + 0.5 * result.draws for result in results)

    if not games:
        raise ValueError("no games were played")

    if points in (0, games):
        raise ValueError("every game was won or every game was lost, so there's no fit")

    def total_expected(elo: float) -> float:
        return sum(
            result.games * expected_score(elo, result.opponent.elo)
            for result in results
        )

    low = min(result.opponent.elo for result in results) - 2000
    high = max(result.opponent.elo for result in results) + 2000

    while high - low > 0.01:
        mid = (low + high) / 2
        if total_expected(mid) < points:
            low = mid
        else:
            high = mid

    elo = (low + high) / 2

    # The error comes from the spread of the scores actually made (so that draws count as
    # less noisy than a win & a loss), divided by how fast the expected score changes with
    # the rating at the fit.
    variance = 0.0
    slope = 0.0
    for result in results:
        score = result.score
        variance += (
            result.wins * (1 - score) ** 2
            + result.draws * (0.5 - score) ** 2
            + result.losses * score**2
        )
        expected = expected_score(elo, result.opponent.elo)
        slope += result.games * expected * (1 - expected) / ELO_PER_NAT

    return elo, 1.96 * math.sqrt(variance) / slope


def log_path(out_dir: Path, opponent: Opponent) -> Path:
    return out_dir / f"vs-{opponent.name}.log"


def play_match(args: argparse.Namespace, opponent: Opponent) -> Path:
    log = log_path(args.out_dir, opponent)
    pgn = args.out_dir / f"vs-{opponent.name}-games.pgn"

    # fastchess appends to these, so a rerun would count the old games again
    log.unlink(missing_ok=True)
    pgn.unlink(missing_ok=True)

    book_format = "epd" if args.book.suffix.lower() == ".epd" else "pgn"

    command = [
        args.fastchess,
        "-engine",
        f"cmd={args.engine}",
        f"name={args.name}",
        "-engine",
        f"cmd={opponent.cmd}",
        f"name={opponent.name}",
        *opponent.options,
        "-each",
        f"tc={args.tc}",
        "option.Hash=32",
        "-rounds",
        str(math.ceil(args.games / 2)),
        "-repeat",
        "-concurrency",
        str(args.concurrency),
        "-maxmoves",
        "400",
        "-resign",
        "movecount=3",
        "score=500",
        "twosided=true",
        "-draw",
        "movenumber=40",
        "movecount=8",
        "score=10",
        "-recover",
        "-openings",
        f"file={args.book}",
        f"format={book_format}",
        "order=random",
        "-pgnout",
        f"file={pgn}",
        "-ratinginterval",
        "10",
    ]

    print(f"Playing {args.games} games against {opponent.name}...", file=sys.stderr)

    with open(log, "w") as file:
        # fastchess writes a config.json to resume from in its working directory
        subprocess.run(
            command,
            stdout=file,
            stderr=subprocess.STDOUT,
            cwd=args.out_dir,
            check=False,
        )

    return log


def absolute(cmd: str) -> str:
    # fastchess runs in the output directory, so relative paths to programs have to be
    # resolved first; plain program names are left to be found on the PATH
    path = Path(cmd)
    return str(path.resolve()) if path.exists() else cmd


def get_opponents(args: argparse.Namespace) -> list[Opponent]:
    opponents = []

    if args.levels:
        if args.stockfish is None:
            raise SystemExit(
                "Stockfish wasn't found: pass --stockfish, or --levels with no values"
            )

        for level in args.levels:
            opponents.append(
                Opponent(
                    name=f"Stockfish-{level}",
                    elo=level,
                    cmd=absolute(args.stockfish),
                    options=[
                        "option.UCI_LimitStrength=true",
                        f"option.UCI_Elo={level}",
                    ],
                )
            )

    for elo, cmd in args.opponent or []:
        opponents.append(
            Opponent(name=Path(cmd).stem, elo=float(elo), cmd=absolute(cmd), options=[])
        )

    if not opponents:
        raise SystemExit("No opponents: pass --levels and/or --opponent")

    return opponents


def format_summary(results: list[Result], args: argparse.Namespace) -> str:
    lines = [
        f"## {args.name} Elo estimate",
        "",
        f"Time control {args.tc}, 1 thread, 32MB hash.",
        "",
        "| Opponent | Rating | Games | W / D / L | Score | Performance |",
        "|---|---|---|---|---|---|",
    ]

    for result in results:
        score = result.score
        if 0 < score < 1:
            performance = f"{result.opponent.elo + ELO_PER_NAT * math.log(score / (1 - score)):.0f}"
        else:
            performance = "-"

        lines.append(
            f"| {result.opponent.name} | {result.opponent.elo:.0f} | {result.games} "
            f"| {result.wins} / {result.draws} / {result.losses} | {100 * score:.1f}% "
            f"| {performance} |"
        )

    lines.append("")

    try:
        elo, margin = estimate_elo(results)
        lines.append(f"**Estimated Elo: {elo:.0f} +/- {margin:.0f}** (95%)")
    except ValueError as error:
        lines.append(f"**No estimate:** {error}.")

    lopsided = [
        result.opponent.name
        for result in results
        if abs(result.score - 0.5) > LOPSIDED_SCORE - 0.5
    ]

    if lopsided:
        lines += [
            "",
            f"The results against {', '.join(lopsided)} are lopsided, so they add little; "
            "opponents closer to the estimate would narrow the error.",
        ]

    return "\n".join(lines) + "\n"


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Estimates an engine's Elo with a fastchess gauntlet against opponents of known strength."
    )
    parser.add_argument("engine", help="the engine to rate")
    parser.add_argument(
        "--book", type=Path, required=True, help="openings file (.pgn or .epd)"
    )
    parser.add_argument(
        "--name", default="BenBot", help="the engine's name in the logs & games"
    )
    parser.add_argument(
        "--stockfish",
        default=shutil.which("stockfish"),
        help="Stockfish executable (default: from PATH)",
    )
    parser.add_argument(
        "--levels",
        type=int,
        nargs="*",
        default=DEFAULT_LEVELS,
        help=f"Stockfish UCI_Elo levels to play, from 1320 to 3190 (default: {' '.join(str(level) for level in DEFAULT_LEVELS)}); "
        "pass no values to play only the --opponent engines",
    )
    parser.add_argument(
        "--opponent",
        nargs=2,
        action="append",
        metavar=("ELO", "CMD"),
        help="another opponent & its rating; can be given more than once",
    )
    parser.add_argument(
        "--games",
        type=int,
        default=200,
        help="games against each opponent (default: 200)",
    )
    parser.add_argument("--tc", default="60+0.6", help="time control (default: 60+0.6)")
    parser.add_argument(
        "--concurrency",
        type=int,
        default=os.cpu_count() or 1,
        help="games played at once (default: the number of cores)",
    )
    parser.add_argument(
        "--fastchess",
        default=shutil.which("fastchess"),
        help="fastchess executable (default: from PATH)",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("logs") / "elo",
        help="where the logs & games are written (default: logs/elo)",
    )
    parser.add_argument(
        "--reuse-logs",
        action="store_true",
        help="don't replay matches whose log in --out-dir already has results",
    )

    args = parser.parse_args(argv)

    if args.fastchess is None:
        parser.error("fastchess wasn't found: pass --fastchess")

    args.engine = absolute(args.engine)
    args.book = args.book.resolve()
    args.out_dir = args.out_dir.resolve()

    return args


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    opponents = get_opponents(args)

    args.out_dir.mkdir(parents=True, exist_ok=True)

    results = []
    for opponent in opponents:
        log = log_path(args.out_dir, opponent)

        counts = None
        if args.reuse_logs and log.exists():
            counts = parse_results(log.read_text())

        if counts is None:
            log = play_match(args, opponent)
            counts = parse_results(log.read_text())

        if counts is None:
            print(f"No results against {opponent.name}: see {log}", file=sys.stderr)
            continue

        results.append(Result(opponent, *counts))

    if not any(result.games for result in results):
        print("No games were played", file=sys.stderr)
        return 1

    print(format_summary(results, args), end="")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
