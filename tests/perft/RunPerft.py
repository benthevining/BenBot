# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

# This file was configured by CMake! Any changes will be overwritten!

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

    correctNodes = correct['totalNodes']
    reportedNodes = results['totalNodes']

    if correctNodes != reportedNodes:
        print(f'Expected {correctNodes} nodes, got {reportedNodes}')
        any_error = True

    correctCaptures = correct['captures']
    reportedCaptures = results['captures']

    if correctCaptures != reportedCaptures:
        print(f'Expected {correctCaptures} captures, got {reportedCaptures}')
        any_error = True

    correctCastles = correct['castles']
    reportedCastles = results['castles']

    if correctCastles != reportedCastles:
        print(f'Expected {correctCastles} castles, got {reportedCastles}')
        any_error = True

    correctChecks = correct['checks']
    reportedChecks = results['checks']

    if correctChecks != reportedChecks:
        print(f'Expected {correctChecks} checks, got {reportedChecks}')
        any_error = True

    correctMates = correct['checkmates']
    reportedMates = results['checkmates']

    if correctMates != reportedMates:
        print(f'Expected {correctMates} checkmates, got {reportedMates}')
        any_error = True

    correctStalemates = correct['stalemates']
    reportedStalemates = results['stalemates']

    if correctStalemates != reportedStalemates:
        print(f'Expected {correctStalemates} stalemates, got {reportedStalemates}')
        any_error = True

    correctEP = correct['en_passants']
    reportedEP = results['en_passants']

    if correctEP != reportedEP:
        print(f'Expected {correctEP} en passant captures, got {reportedEP}')
        any_error = True

    correctPromotions = correct['promotions']
    reportedPromotions = results['promotions']

    if correctPromotions != reportedPromotions:
        print(f'Expected {correctPromotions} promotions, got {reportedPromotions}')
        any_error = True

    if any_error:
        print(f'Failed on depth {depth}')
        exit(1)
