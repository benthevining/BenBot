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

from pathlib import Path
import sys
import re
from enum import Enum

LOG_FILE = Path(sys.argv[1])

with open(LOG_FILE, 'r') as file:
    SPRT_OUTPUT = file.readlines()

# finds the indices of the last 2 '-----------------' lines in the output
# the final results report is in the fenced section between these 2 lines
def find_last_two_fences():
    indices = []

    for i in range(len(SPRT_OUTPUT) - 1, -1, -1):
        if re.search(r"(-)+", SPRT_OUTPUT[i]):
            indices.append(i)
            if len(indices) == 2:
                break

    return sorted(indices)

def get_final_results_report():
    start, end = find_last_two_fences()

    return SPRT_OUTPUT[start+1:end]

def replace_pairs(old_values, new_values, string):
    for old, new in zip(old_values, new_values):
        string.replace(old, new)

    return string

# returns the first string that matches the given regex
def find_first_match(regex, strings):
    return next(str for str in strings if re.search(regex, str))

class EmojiType(Enum):
    NEUTRAL = 1,
    POSITIVE = 2,
    NEGATIVE = 3

def get_emoji(type):
    match type:
        case EmojiType.NEUTRAL: return '🟰'
        case EmojiType.POSITIVE: return '✅'
        case EmojiType.NEGATIVE: return '❌'

RESULTS = get_final_results_report()

def get_elo():
    elo_line = find_first_match('Elo:', RESULTS)

    first_space_idx = elo_line.find(' ')
    second_space_idx = elo_line.find(' ', first_space_idx + 1)

    return float(
        elo_line[first_space_idx:second_space_idx]
    )

def get_elo_emoji(elo):
    if abs(elo) <= 1:
        return get_emoji(EmojiType.NEUTRAL)

    if elo > 0:
        return get_emoji(EmojiType.POSITIVE)

    return get_emoji(EmojiType.NEGATIVE)

def get_result_breakdown():
    games_line = find_first_match('Games:', RESULTS)

    second_space_idx = games_line.find(' ',
                            games_line.find(' ') + 1)

    after_second_space = games_line[second_space_idx+1:]

    # string format is:
    # Wins: W, Losses: L, Draws: D, Points: 50.5 (50.50 %)

    wins_comma_idx = after_second_space.find(',')

    num_wins = int(
        after_second_space[after_second_space.find(' ')+1:wins_comma_idx]
    )

    after_wins = (after_second_space[after_second_space.find(' ', wins_comma_idx + 1):]).lstrip()

    losses_comma_idx = after_wins.find(',')

    num_losses = int(
        after_wins[after_wins.find(' ')+1:losses_comma_idx]
    )

    after_losses = (after_wins[after_wins.find(' ', losses_comma_idx + 1):]).lstrip()

    draws_comma_idx = after_losses.find(',')

    num_draws = int(
        after_losses[after_losses.find(' ')+1:draws_comma_idx]
    )

    after_draws = (after_losses[after_losses.find(' ', draws_comma_idx+1):]).lstrip()

    pcnt = float(
        after_draws[after_draws.find('(')+1:after_draws.find('%')]
    )

    return (
        num_wins, num_losses, num_draws, pcnt
    )

def get_wins_emoji(wins):
    if wins in range(45, 55):
        return get_emoji(EmojiType.NEUTRAL)

    if wins > 50:
        return get_emoji(EmojiType.POSITIVE)

    return get_emoji(EmojiType.NEGATIVE)

def get_losses_emoji(losses):
    if losses in range(45, 55):
        return get_emoji(EmojiType.NEUTRAL)

    if losses > 50:
        return get_emoji(EmojiType.NEGATIVE)

    return get_emoji(EmojiType.POSITIVE)

def get_draws_emoji(draws, losses, wins):
    if draws < losses:
        return get_emoji(EmojiType.NEGATIVE)

    if draws > wins:
        return get_emoji(EmojiType.POSITIVE)

    return get_emoji(EmojiType.NEUTRAL)

def get_pcnt_emoji(pcnt):
    if pcnt in range(0, 65):
        return get_emoji(EmojiType.NEGATIVE)

    if pcnt in range(65, 85):
        return get_emoji(EmojiType.NEUTRAL)

    return get_emoji(EmojiType.POSITIVE)

#

SCRIPT_DIR = Path(__file__).resolve().parent

with open(f'{SCRIPT_DIR}/sprt-results.md', 'r') as file:
    MD_TEMPLATE_TEXT = file.read()

elo = get_elo()

num_wins, num_losses, num_draws, pcnt = get_result_breakdown()

print(
    replace_pairs(
        ['%ELO%', '%ELO_EMOJI%',
         '%WINS%', '%WINS_EMOJI%',
         '%LOSSES%', '%LOSSES_EMOJI%',
         '%DRAWS%', '%DRAWS_EMOJI%',
         '%PCNT%', '%PCNT_EMOJI%'],
        [f'{elo}', get_elo_emoji(elo),
         f'{num_wins}', get_wins_emoji(num_wins),
         f'{num_losses}', get_losses_emoji(num_losses),
         f'{num_draws}', get_draws_emoji(num_draws, num_losses, num_wins),
         f'{pcnt}%', get_pcnt_emoji(pcnt)],
        MD_TEMPLATE_TEXT
    )
)
