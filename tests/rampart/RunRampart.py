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

# This file was configured by CMake! Any changes will be overwritten!
# Note for devs: if you're working on the template script in the source tree,
# you'll need to rerun CMake to see the effects.

import json
import subprocess
from pathlib import Path
import sys

def get_move_from_obj(listObj, move): # listObj is a JSON list of objects
    for obj in listObj:
        if obj['move'] == move:
            return obj

    return None

TESTCASE_FILE = Path(sys.argv[1])
TMP_DIR_PATH = Path(sys.argv[2])

test_cases_passed = 0
test_cases_failed = 0

print(f'Running tests from {TESTCASE_FILE}...')

with open(TESTCASE_FILE, 'r') as file:
    testcase_data = json.load(file)

output_dir = TMP_DIR_PATH / TESTCASE_FILE.stem

test_idx = 1

for test_case in testcase_data['testCases']:
    startFEN = test_case['start']['fen']

    output_file = output_dir / f'{test_idx}.json'

    print(f'Running tests on position {startFEN}')
    print(f'Output file: {output_file}')

    subprocess.run(['$<TARGET_FILE:rampart>', startFEN, output_file])

    with open(output_file, 'r') as file:
        result_data = json.load(file)

    correct_moves   = test_case['expected']
    generated_moves = result_data['generated']

    any_errors = False

    for correct_move in correct_moves:
        move = correct_move['move']
        generated_move = get_move_from_obj(generated_moves, move)

        if generated_move is None:
            print(f'ERROR: move {move} was not generated, it should be legal!')
            any_errors = True
            continue

        correctFEN   = correct_move['fen']
        generatedFEN = generated_move['fen']

        if correctFEN != generatedFEN:
            print(f'ERROR: move {move} resulted in incorrect FEN!')
            print(f'Expected {correctFEN}, got {generatedFEN}')
            any_errors = True

        # check for moves in generated_moves not in correct_moves

    for generated_move in generated_moves:
        move = generated_move['move']

        correct_move = get_move_from_obj(correct_moves, move)

        if correct_move is None:
            print(f'ERROR: move {move} was incorrectly generated, it should not be legal!')
            any_errors = True

        if any_errors:
            test_cases_failed += 1
        else:
            test_cases_passed += 1

    test_idx += 1

print(f'{test_cases_passed} test cases passed')
print(f'{test_cases_failed} test cases failed')

if test_cases_failed > 0:
    exit(1)
