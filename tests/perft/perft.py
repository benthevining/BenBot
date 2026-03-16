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
from typing import Tuple, Any

#


def find_correct_result(fileContent: Any, depth: int) -> dict[str, int]:
    for obj in fileContent["depths"]:
        if obj["depth"] == depth:
            return obj["results"]

    print(f"ERROR! Could not find correct result for depth {depth}")
    exit(1)


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


def parse_args() -> Tuple[Path, Path, int]:
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
    parser.add_argument("-d", "--depth", required=True, help="Perft depth")

    parsed = parser.parse_args()

    return Path(parsed.test).resolve(), Path(parsed.engine).resolve(), int(parsed.depth)


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

        self.send_command("uci")

        # skip greeting & info/options output
        for line in self.engine.stdout:
            if line.strip() == "uciok":
                break

        self.send_command("ucinewgame")
        self.send_command("isready")

        self.readline()  # "readyok" response

        self.send_command(f"position fen {pos_fen}")

    def send_command(self, command):
        self.engine.stdin.write(f"{command}\n")
        self.engine.stdin.flush()

    def readline(self):
        return self.engine.stdout.readline()

    def run_perft(self, depth: int) -> dict[str, int]:
        self.send_command(f"perft {depth} json")

        self.readline()  # info line

        return json.loads(self.readline())

    def __del__(self):
        self.send_command("quit")

        if self.engine:
            self.engine.stdin.close()
            self.engine.stdout.close()
            self.engine.wait()


#

TESTCASE_FILE, ENGINE_PATH, DEPTH = parse_args()

with open(TESTCASE_FILE) as file:
    FILE_DATA = json.load(file)

startingFEN = FILE_DATA["position"]

print(f"FEN: {startingFEN}", flush=True)

correctResult = find_correct_result(FILE_DATA, DEPTH)

print(f"Running perft depth {DEPTH}...", flush=True)

engine = Engine(engine_path=ENGINE_PATH, pos_fen=startingFEN)

num_failed = 0
num_passed = 0

result = engine.run_perft(DEPTH)

if check_result(correctResult, result):
    print("Succeeded :-)")
    exit(0)
else:
    print("Failed :-(")
    exit(1)
