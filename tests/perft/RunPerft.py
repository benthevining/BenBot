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

with open('@CORRECT_RESULTS_FILE@', 'r') as file:
    CORRECT_DATA = json.load(file)

def get_correct_for(depth):
    for object in CORRECT_DATA:
        if object['depth'] == depth:
            return object['results']

    raise ValueError(f"Correct data for depth {depth} not found")

# We stop the test if we get the incorrect number of total nodes, since this
# indicates a total failure of our move generation algorithm. If the stats
# breakdown is incorrect but the total number of nodes is correct, continue
# testing deeper depths until the total number of nodes is wrong.
incorrect_stats = []
passed = []

for depth in range(7):
    results_file = TMP_DIR_PATH / f'{depth}_results.json'

    subprocess.run(['$<TARGET_FILE:perft>', f'{depth}', '--write-json', results_file])

    with open(results_file, 'r') as file:
        result_file_text = json.load(file)

    results = result_file_text['results']

    correct = get_correct_for(depth)

    any_error = False

    for field in ['totalNodes', 'captures', 'castles', 'checks', 'checkmates', 'stalemates', 'en_passants', 'promotions']:
        expected = correct[field]
        reported = results[field]

        if expected != reported:
            if field == 'totalNodes': # wrong number of total nodes reported, exit the test with failure
                print(f'ERROR: Depth {depth} reported incorrect number of nodes! Expected {expected}, got {reported}')
                exit(1)

            # wrong stats reported, report failure but continue on to deeper depths
            print(f'ERROR: Expected {expected} {field}, got {reported}')

            incorrect_stats.append(depth)

            any_error = True

    if not any_error:
        passed.append(depth)

if incorrect_stats:
    failed_depths = list(set(incorrect_stats)) # remove duplicates
    failed_depths.sort()
    print(f'The following depths reported correct total nodes, but incorrect breakdown stats: {failed_depths}')

if passed:
    print(f'The following depths succeeded: {passed}')
