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

TESTCASE_FILES = ['standard']

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

        subprocess.run(['$<TARGET_FILE:rampart>', startFEN, output_file])

        with open(output_file, 'r') as file:
            result_data = json.load(file)

        correct_moves = test_case['expected']
        generated_moves = result_data['generated']

        # print moves in correct_moves not in generated_moves
        # print moves in generated_moves not in correct_moves
        # for moves in both, print incorrect move FEN strings

        if len(correct_moves) != len(generated_moves):
            print('Wrong number of moves generated!')

        test_idx += 1
