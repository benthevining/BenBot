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

import chess.engine

logging.basicConfig(level=logging.DEBUG)

parser = argparse.ArgumentParser(
    prog="PositionSolver",
    description="Run BenBot test positions",
    epilog="This script is intended to be invoked by CTest",
)

parser.add_argument("-e", "--engine", required=True, help="Path to engine executable")
parser.add_argument("-l", "--log", required=True, help="Path to engine log file")
parser.add_argument(
    "-t", "--test", required=True, help="Path to testcase data file (1 EPD per line)"
)
parser.add_argument(
    "-i",
    "--index",
    type=int,
    required=True,
    help="Index of line to take from the testcase file",
)

args = parser.parse_args()

TESTCASE_FILE = Path(args.test).resolve()
ENGINE_PATH = Path(args.engine).resolve()
ENGINE_LOG_PATH = Path(args.log).resolve()

with open(TESTCASE_FILE) as file:
    epd_data = file.readlines()[args.index]

board = chess.Board()

operations = board.set_epd(epd_data)

engine = chess.engine.SimpleEngine.popen_uci(ENGINE_PATH, timeout=None)

engine.configure({"Debug Log File": str(ENGINE_LOG_PATH.resolve())})

result = engine.play(board, chess.engine.Limit(depth=operations["depth"]))

engine.quit()

exit_code = engine.returncode.result(timeout=None)

if exit_code != 0:
    print(f"Engine exited with code {exit_code}")

expectedMove = operations["bm"][0]

if result.move == expectedMove:
    print("Passed!")
    exit(0)

print("FAILED!")
print(f"Expected {board.san(expectedMove)}, got {board.san(result.move)}")

exit(1)
