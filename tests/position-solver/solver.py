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
import logging
from pathlib import Path
from typing import Tuple

import chess.engine

#


def parse_args() -> Tuple[Path, Path, Path, int]:
    parser = argparse.ArgumentParser(
        prog="PositionSolver",
        description="Run BenBot test positions",
        epilog="This script is intended to be invoked by CTest",
    )

    parser.add_argument(
        "-e", "--engine", required=True, help="Path to engine executable"
    )
    parser.add_argument("-l", "--log", required=True, help="Path to engine log file")
    parser.add_argument(
        "-t",
        "--test",
        required=True,
        help="Path to testcase data file (1 EPD per line)",
    )
    parser.add_argument(
        "-i",
        "--index",
        type=int,
        required=True,
        help="Index of line to take from the testcase file",
    )

    parsed = parser.parse_args()

    return (
        Path(parsed.test).resolve(),
        Path(parsed.engine).resolve(),
        Path(parsed.log).resolve(),
        parsed.index,
    )


#

logging.basicConfig(level=logging.DEBUG)

TESTCASE_FILE, ENGINE_PATH, ENGINE_LOG_PATH, index = parse_args()

with open(TESTCASE_FILE) as file:
    epd_data = file.readlines()[index]

board = chess.Board()

operations = board.set_epd(epd_data)

print(f"Testing position {board.fen()}", flush=True)

engine = chess.engine.SimpleEngine.popen_uci(str(ENGINE_PATH), timeout=None)

engine.configure({"Debug Log File": str(ENGINE_LOG_PATH.resolve())})

result = engine.play(board, chess.engine.Limit(depth=operations["depth"]))

engine.quit()

exit_code = engine.returncode.result()

if exit_code != 0:
    print(f"Engine exited with code {exit_code}", flush=True)

expectedMove = operations["bm"][0]

if result.move == expectedMove:
    print("Passed!")
    exit(0)

print("FAILED!")
print(f"Expected {board.san(expectedMove)}, got {board.san(result.move)}")

print(board.unicode(borders=True, empty_square=" "))

exit(1)
