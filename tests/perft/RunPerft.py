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
            print(f'Expected {expected} {field}, got {reported}')
            any_error = True

    if any_error:
        print(f'Failed on depth {depth}')
        exit(1)
