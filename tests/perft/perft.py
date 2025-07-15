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

import json
from pathlib import Path
import sys
import subprocess

TESTCASE_FILE = Path(sys.argv[1])
ENGINE_PATH = Path(sys.argv[2])

with open(TESTCASE_FILE, 'r') as file:
    CORRECT_DATA = json.load(file)

startingFEN = CORRECT_DATA['position']

print(f'Running tests for position {startingFEN}')

engine = subprocess.Popen(
    ENGINE_PATH,
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    bufsize=1,
    universal_newlines=True,
    text=True
)

engine.stdin.write('uci\n')
engine.stdin.write('ucinewgame\n')
engine.stdin.write(f'position {startingFEN}\n')

# skip greeting & info/options output
for line in engine.stdout:
    if line.strip() == 'uciok':
        break

def get_result_lines(stdout):
    lines = []

    for line in stdout:
        if ':' in line:
            lines.append(line)

            # a bit hacky, but we need to check for the end of the perft output
            # or we'll spin forever waiting for EOF from the engine's stdout
            if line.split(' ', 1)[0] == 'Stalemates:':
                break
        elif lines:
            break

    return lines

def get_value_for_key(lines, key):
    for line in lines:
        thisKey, value = line.split(':', 1)

        if thisKey == key:
            return int(value.strip())

    raise Exception(f'Result does not contain key {key}')

def get_result(lines):
    return {
        'captures': get_value_for_key(lines, 'Captures'),
        'castles': get_value_for_key(lines, 'Castles'),
        'checkmates': get_value_for_key(lines, 'Checkmates'),
        'checks': get_value_for_key(lines, 'Checks'),
        'en_passants': get_value_for_key(lines, 'En passant captures'),
        'promotions': get_value_for_key(lines, 'Promotions'),
        'stalemates': get_value_for_key(lines, 'Stalemates'),
        'totalNodes': get_value_for_key(lines, 'Nodes')
    }

def check_result(expected, actual):
    for key in 'totalNodes', 'captures', 'castles', 'checkmates', 'checks', 'en_passants', 'promotions', 'stalemates':
        if actual[key] != expected[key]:
            print(f'FAILED! Expected {expected[key]} {key}, got {actual[key]}')
            return False

    return True

num_failed = 0
num_passed = 0

for depthObj in CORRECT_DATA['depths']:
    depth = depthObj['depth']
    print(f'Running perft depth {depth}...')

    engine.stdin.write(f'perft {depth}\n')

    result = get_result(
        get_result_lines(engine.stdout)
    )

    correct = depthObj['results']

    if check_result(depthObj['results'], result):
        num_passed = num_passed + 1
    else:
        num_failed = num_failed + 1

engine.communicate('quit\n', timeout=15)
engine.kill()
engine.communicate()

print(f'{num_passed} depths passed')
print(f'{num_failed} depths failed')

sys.exit(num_failed)
