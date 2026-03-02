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
from typing import Tuple

#


def get_move_from_obj(listObj: list[dict], move: str) -> Optional[dict]:
    for obj in listObj:
        if obj["move"] == move:
            return obj

    return None


def parse_args() -> Tuple[Path, Path]:
    parser = argparse.ArgumentParser(
        prog="RunRampart",
        description="Run rampart movegen tests",
        epilog="This script is intended to be invoked by CTest",
    )

    parser.add_argument(
        "-t", "--test", required=True, help="Path to testcase data file"
    )
    parser.add_argument(
        "-e", "--exec", required=True, help="Path to rampart executable"
    )

    parsed = parser.parse_args()

    return Path(parsed.test).resolve(), Path(parsed.exec).resolve()


#


TESTCASE_FILE, RAMPART_PROGRAM = parse_args()

test_cases_passed = 0
test_cases_failed = 0

print(f"Running tests from {TESTCASE_FILE}...", flush=True)

with open(TESTCASE_FILE) as file:
    testcase_data = json.load(file)

test_idx = 1

for test_case in testcase_data["testCases"]:
    startFEN = test_case["start"]["fen"]

    print(f"Running tests on position {startFEN}", flush=True)

    result = subprocess.run(
        [RAMPART_PROGRAM, startFEN],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )

    if result.returncode != 0:
        print(f"Rampart executable exited with code {result.returncode}")
        exit(result.returncode)

    result_data = json.loads(result.stdout)

    correct_moves = test_case["expected"]
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
        correctFEN = correct_move["fen"]

        generatedFEN = generated_move["fen"]
        generatedXFEN = generated_move["xfen"]

        if generatedFEN != correctFEN and generatedXFEN != correctFEN:
            print(f"ERROR: move {move} resulted in incorrect FEN!", flush=True)
            print(f"Expected FEN: {correctFEN}", flush=True)
            print(f"Got FEN: {generatedFEN}", flush=True)
            print(f"Got XFEN: {generatedXFEN}", flush=True)
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

        if any_errors:
            test_cases_failed += 1
        else:
            test_cases_passed += 1

    test_idx += 1

print(f"{test_cases_passed} test cases passed")
print(f"{test_cases_failed} test cases failed")

exit(test_cases_failed)
