# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

# This file was configured by CMake! Any changes will be overwritten!
# Note for devs: if you're working on the template script in the source tree,
# you'll need to rerun CMake to see the effects.

import json
import subprocess
from pathlib import Path

TMP_DIR_PATH = Path('@TMP_DIR@')
CORRECT_FILES_DIR = Path('@TESTCASES_DIR@')

# TODO: famous, pawns, taxing
TESTCASE_FILES = ['castling', 'checkmates', 'promotions', 'stalemates', 'standard']

def get_move_from_obj(obj):
    for key, value in list(obj):
        if key == 'move':
            return value

    raise ValueError(f'\"move\" key not found in object: {obj}')

test_cases_passed = 0
test_cases_failed = 0

for testcase_file in TESTCASE_FILES:
    print(f'Running tests from {testcase_file}.json...')

    correct_file_path = CORRECT_FILES_DIR / f'{testcase_file}.json'

    with open(correct_file_path, 'r') as file:
        testcase_data = json.load(file)

    output_dir = TMP_DIR_PATH / testcase_file

    test_idx = 1

    for test_case in testcase_data['testCases']:
        startFEN = test_case['start']['fen']

        output_file = output_dir / f'{test_idx}.json'

        print(f'Running tests on position {startFEN}')
        print(f'Output file: {output_file}')

        subprocess.run(['$<TARGET_FILE:rampart>', startFEN, output_file])

        with open(output_file, 'r') as file:
            result_data = json.load(file)

        correct_moves   = frozenset(frozenset(d.items()) for d in test_case['expected'])
        generated_moves = frozenset(frozenset(d.items()) for d in result_data['generated'])

        any_errors = False

        # print moves in correct_moves not in generated_moves
        for missing_move in correct_moves.difference(generated_moves):
            print(f'ERROR! Move {get_move_from_obj(missing_move)} was not generated (or resulting FEN is wrong)')
            any_errors = True

        # print moves in generated_moves not in correct_moves
        for incorrect_move in generated_moves.difference(correct_moves):
            print(f'ERROR! Move {get_move_from_obj(incorrect_move)} was incorrectly generated (or resulting FEN is wrong)')
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
