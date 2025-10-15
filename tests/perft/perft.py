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
import argparse
import json
import subprocess
from pathlib import Path
from typing import Tuple

#


def get_result_lines(stdout) -> list[str]:
    lines = []

    for line in stdout:
        if ":" in line:
            lines.append(line)

            # a bit hacky, but we need to check for the end of the perft output
            # or we'll spin forever waiting for EOF from the engine's stdout
            if line.split(":", 1)[0] == f"info string Stalemates":
                break

    return lines


def get_value_for_key(lines: list[str], key: str) -> int:
    for line in lines:
        this_key, value = line.split(":", 1)

        if this_key == f"info string {key}":
            return int(value.strip())

    raise Exception(f"Result does not contain key {key}")


def get_result(lines: list[str]) -> dict[str, int]:
    return {
        "captures": get_value_for_key(lines, "Captures"),
        "castles": get_value_for_key(lines, "Castles"),
        "checkmates": get_value_for_key(lines, "Checkmates"),
        "checks": get_value_for_key(lines, "Checks"),
        "en_passants": get_value_for_key(lines, "En passant captures"),
        "promotions": get_value_for_key(lines, "Promotions"),
        "stalemates": get_value_for_key(lines, "Stalemates"),
        "totalNodes": get_value_for_key(lines, "Nodes"),
    }


def check_result(expected: dict[str, int], actual: dict[str, int]) -> bool:
    for key in (
        "totalNodes",
        "captures",
        "castles",
        "checkmates",
        "checks",
        "en_passants",
        "promotions",
        "stalemates",
    ):
        if actual[key] != expected[key]:
            print(f"FAILED! Expected {expected[key]} {key}, got {actual[key]}")
            return False

    return True


def parse_args() -> Tuple[Path, Path]:
    parser = argparse.ArgumentParser(
        prog="RunPerft",
        description="Run BenBot perft tests",
        epilog="This script is intended to be invoked by CTest",
    )

    parser.add_argument(
        "-t", "--test", required=True, help="Path to testcase data file"
    )
    parser.add_argument(
        "-e", "--engine", required=True, help="Path to engine executable"
    )

    parsed = parser.parse_args()

    return Path(parsed.test).resolve(), Path(parsed.engine).resolve()


#


class Engine:
    def __init__(self, engine_path: Path, pos_fen: str):
        self.engine = subprocess.Popen(
            engine_path,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            bufsize=1,
            universal_newlines=True,
            text=True,
        )

        self.engine.stdin.write("uci\n")

        # skip greeting & info/options output
        for line in self.engine.stdout:
            if line.strip() == "uciok":
                break

        self.engine.stdin.write("ucinewgame\n")
        self.engine.stdin.write("isready\n")

        self.engine.stdout.readline()  # "readyok" response

        self.engine.stdin.write(f"position fen {pos_fen}\n")

    def run_perft(self, depth: int):
        self.engine.stdin.write(f"perft {depth}\n")

        return get_result(get_result_lines(self.engine.stdout))

    def quit(self):
        self.engine.communicate("quit\n", timeout=15)
        self.engine.kill()
        self.engine.communicate()


#

TESTCASE_FILE, ENGINE_PATH = parse_args()

with open(TESTCASE_FILE) as file:
    CORRECT_DATA = json.load(file)

startingFEN = CORRECT_DATA["position"]

print(f"Running tests for position {startingFEN}", flush=True)

engine = Engine(engine_path=ENGINE_PATH, pos_fen=startingFEN)

num_failed = 0
num_passed = 0

for depthObj in CORRECT_DATA["depths"]:
    depth = depthObj["depth"]
    print(f"Running perft depth {depth}...", flush=True)

    result = engine.run_perft(depth)

    if check_result(depthObj["results"], result):
        num_passed = num_passed + 1
    else:
        num_failed = num_failed + 1

engine.quit()

print(f"{num_passed} depths passed")
print(f"{num_failed} depths failed")

exit(num_failed)
