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

import subprocess
import sys
from pathlib import Path


def run_oneshot_cli_test_suite(commands: list[str], benbot_path: Path):
    num_passed = 0
    num_failed = 0

    def run_benbot_command(command: str):
        nonlocal num_passed
        nonlocal num_failed

        print(f"Running command: '{command}'")

        process = subprocess.run(
            [benbot_path] + command.split(" ") + ["--no-loop"],
            capture_output=True,
            text=True,
        )

        if process.returncode == 0:
            num_passed += 1
            return

        print(process.stdout)
        print(process.stderr)
        print(f"Process failed with return code {process.returncode}")
        num_failed += 1

    for cmd in commands:
        run_benbot_command(cmd)

    print(f"{num_passed} tests passed")
    print(f"{num_failed} tests failed")

    exit(num_failed)


TEST_COMMANDS = [
    "go nodes 1000",
    "go depth 10",
    "perft 4 json",
    "go movetime 1000",
    "go wtime 8000 btime 8000 winc 500 binc 500",
    "go wtime 1000 btime 1000 winc 0 binc 0",
    "go wtime 1000 btime 1000 winc 0 binc 0 movestogo 5",
    "go movetime 200",
    "go nodes 20000 searchmoves e2e4 d2d4",
    "showpos",
    "compiler",
    "uci",
]

run_oneshot_cli_test_suite(TEST_COMMANDS, Path(sys.argv[1]))
