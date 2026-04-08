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


def run_position_solver_test(
    data_file: Path, engine_path: Path, log_file: Path, index: int
) -> None:
    logging.basicConfig(level=logging.DEBUG)

    with open(data_file) as file:
        epd_data = file.readlines()[index]

    board = chess.Board()

    operations = board.set_epd(epd_data)

    print(f"Position ID: {operations['id']}")
    print(f"FEN: {board.fen()}", flush=True)

    engine = chess.engine.SimpleEngine.popen_uci(str(engine_path), timeout=None)

    engine.configure({"Debug Log File": str(log_file)})

    result = engine.play(board, chess.engine.Limit(depth=operations["depth"]))

    engine.quit()

    exit_code = engine.returncode.result()

    if exit_code != 0:
        print(f"Engine exited with code {exit_code}", flush=True)

    expected_move = operations["bm"][0]

    if result.move == expected_move:
        print("Passed!")
        exit(0)

    print("FAILED!")
    print(f"Expected {board.san(expected_move)}, got {board.san(result.move)}")

    print(board.unicode(borders=True, empty_square=" "))

    exit(1)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="PositionSolver",
        description="Run BenBot test positions",
        epilog="This script is intended to be invoked by CTest",
    )

    parser.add_argument(
        "--engine",
        required=True,
        type=lambda p: Path(p).resolve(),
        help="Path to engine executable",
    )

    parser.add_argument(
        "--log",
        required=True,
        type=lambda p: Path(p).resolve(),
        help="Path to engine log file",
    )

    parser.add_argument(
        "--test",
        required=True,
        type=lambda p: Path(p).resolve(),
        help="Path to testcase data file (1 EPD per line)",
    )

    parser.add_argument(
        "--index",
        type=int,
        required=True,
        help="Index of line to take from the testcase file",
    )

    return parser.parse_args()


#

if __name__ == "__main__":
    args = parse_args()

    run_position_solver_test(
        data_file=args.test,
        engine_path=args.engine,
        index=args.index,
        log_file=args.log,
    )
