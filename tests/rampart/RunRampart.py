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
from typing import Optional


def get_move_from_obj(list_obj: list[dict], move: str) -> Optional[dict]:
    for obj in list_obj:
        if obj["move"] == move:
            return obj

    return None


def run_rampart_test(test_case_data: dict, rampart_program: Path) -> bool:
    start_fen = test_case_data["start"]["fen"]

    print(f"Running tests on position {start_fen}", flush=True)

    result = subprocess.run(
        [rampart_program, start_fen],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        bufsize=1,
        universal_newlines=True,
        text=True,
    )

    if result.returncode != 0:
        print(f"Rampart executable exited with code {result.returncode}")
        exit(result.returncode)

    output = str(result.stdout)

    json_text = output[output.find("{") : output.rfind("}") + 1]

    print(f"Found JSON output:\n{json_text}")

    result_data = json.loads(json_text)

    correct_moves = test_case_data["expected"]
    generated_moves = result_data["generated"]

    any_errors = False

    for correct_move in correct_moves:
        move = correct_move["move"]
        generated_move = get_move_from_obj(generated_moves, move)

        if generated_move is None:
            print(
                f"ERROR: move {move} was not generated, it should be legal!", flush=True
            )
            any_errors = True
            continue

        # The expected FEN strings are inconsistent with their handling of
        # en passant squares; they may match the strict FEN, or they may
        # match the XFEN, so our test executable produces both and we only
        # fail if neither matches.
        # See this issue: https://github.com/schnitzi/rampart/issues/4
        correct_fen = correct_move["fen"]

        generated_fen = generated_move["fen"]
        generated_xfen = generated_move["xfen"]

        if correct_fen != generated_fen and correct_fen != generated_xfen:
            print(f"ERROR: move {move} resulted in incorrect FEN!", flush=True)
            print(f"Expected FEN: {correct_fen}", flush=True)
            print(f"Got FEN: {generated_fen}", flush=True)
            print(f"Got XFEN: {generated_xfen}", flush=True)
            any_errors = True

    # check for moves in generated_moves not in correct_moves
    for generated_move in generated_moves:
        move = generated_move["move"]

        correct_move = get_move_from_obj(correct_moves, move)

        if correct_move is None:
            print(
                f"ERROR: move {move} was incorrectly generated, it should not be legal!",
                flush=True,
            )
            any_errors = True

    return not any_errors


def run_rampart_tests(test_file: Path, rampart_program: Path) -> None:
    test_cases_passed = 0
    test_cases_failed = 0

    print(f"Running tests from {test_file}...", flush=True)

    with open(test_file) as file:
        testcase_data = json.load(file)

    for test_case in testcase_data["testCases"]:
        if run_rampart_test(test_case_data=test_case, rampart_program=rampart_program):
            test_cases_passed += 1
        else:
            test_cases_failed += 1

    print(f"{test_cases_passed} test cases passed")
    print(f"{test_cases_failed} test cases failed")

    exit(test_cases_failed)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="RunRampart",
        description="Run rampart movegen tests",
        epilog="This script is intended to be invoked by CTest",
    )

    parser.add_argument(
        "--test",
        required=True,
        type=lambda p: Path(p).resolve(),
        help="Path to testcase data file",
    )

    parser.add_argument(
        "--exec",
        required=True,
        type=lambda p: Path(p).resolve(),
        help="Path to rampart executable",
    )

    return parser.parse_args()


#

if __name__ == "__main__":
    args = parse_args()

    run_rampart_tests(test_file=args.test, rampart_program=args.exec)
