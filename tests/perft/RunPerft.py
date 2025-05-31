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
import sys

TMP_DIR_PATH = Path('@TMP_DIR@')
TESTCASE_FILE = Path(sys.argv[1])

with open(TESTCASE_FILE, 'r') as file:
    CORRECT_DATA = json.load(file)

num_failed = 0
num_passed = 0

startingFEN = CORRECT_DATA['position']

print(f'Running tests for position {startingFEN}')

output_dir = TMP_DIR_PATH / TESTCASE_FILE.stem

for depthObj in CORRECT_DATA['depths']:
    depth = depthObj['depth']
    correctResultObj = depthObj['results']

    results_file = output_dir / f'depth_{depth}.json'

    subprocess.run(['$<TARGET_FILE:perft>', f'{depth}', '--fen', startingFEN, '--write-json', results_file])

    with open(results_file, 'r') as file:
        result_data = json.load(file)

    generatedResultObj = result_data['results']

    anyError = False

    for field in ['totalNodes', 'captures', 'castles', 'checks', 'checkmates', 'stalemates', 'en_passants', 'promotions']:
        expected = correctResultObj[field]
        reported = generatedResultObj[field]

        # We stop the test if we get the incorrect number of total nodes, since this
        # indicates a total failure of our move generation algorithm. If the stats
        # breakdown is incorrect but the total number of nodes is correct, continue
        # testing deeper depths until the total number of nodes is wrong.

        if expected != reported:
            if field == 'totalNodes': # wrong number of total nodes reported, exit the test with failure
                print(f'FATAL ERROR: Depth {depth} reported incorrect number of nodes! Expected {expected}, got {reported}')
                exit(1)

            # wrong stats reported, report failure but continue on to deeper depths
            print(f'ERROR: Expected {expected} {field}, got {reported}')
            anyError = True

        if anyError:
            num_failed += 1
        else:
            num_passed += 1

print(f'{num_passed} test cases passed')
print(f'{num_failed} test cases failed (correct total nodes, incorrect stats breakdown)')

if num_failed > 0:
    exit(1)
