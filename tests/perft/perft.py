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
from typing import Any


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

    def send_command(self, command: str) -> None:
        self.engine.stdin.write(f"{command}\n")
        self.engine.stdin.flush()

    def readline(self) -> str:
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


def find_correct_result(file_content: Any, depth: int) -> dict[str, int]:
    for obj in file_content["depths"]:
        if obj["depth"] == depth:
            return obj["results"]

    print(f"ERROR! Could not find correct result for depth {depth}")
    exit(1)


def check_result(expected: dict[str, int], actual: dict[str, int]) -> None:
    for key in expected.keys():
        if actual[key] != expected[key]:
            print(f"FAILED! Expected {expected[key]} {key}, got {actual[key]}")
            exit(1)


def run_perft_test(data_file: Path, engine_path: Path, depth: int) -> None:
    with open(data_file) as file:
        file_data = json.load(file)

    starting_fen = file_data["position"]

    print(f"FEN: {starting_fen}", flush=True)

    correct_result = find_correct_result(file_content=file_data, depth=depth)

    print(f"Running perft depth {depth}...", flush=True)

    engine = Engine(engine_path=engine_path, pos_fen=starting_fen)

    result = engine.run_perft(depth)

    check_result(expected=correct_result, actual=result)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="RunPerft",
        description="Run BenBot perft tests",
        epilog="This script is intended to be invoked by CTest",
    )

    parser.add_argument(
        "--test",
        required=True,
        type=lambda p: Path(p).resolve(),
        help="Path to testcase data file",
    )

    parser.add_argument(
        "--engine",
        required=True,
        type=lambda p: Path(p).resolve(),
        help="Path to engine executable",
    )

    parser.add_argument("--depth", required=True, type=int, help="Perft depth")

    return parser.parse_args()


#

if __name__ == "__main__":
    args = parse_args()

    run_perft_test(data_file=args.test, engine_path=args.engine, depth=args.depth)

    print("Succeeded :-)")
    exit(0)
