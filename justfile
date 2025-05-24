# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================
# Configuration file for the Just command runner: https://github.com/casey/just
# Using Just is optional; even if you don't have it installed, this file serves
# as a convenient reference for this project's most frequently used commands.

set windows-shell := ['cmd.exe', '/c']

[private]
list:
    @{{ just_executable() }} --list --justfile {{ justfile() }}

# Runs pre-commit on all files
[group('lint')]
[no-exit-message]
pre-commit:
    git add .
    pre-commit run

alias pc := pre-commit

# Checks for things like to-do notes, #if 0, etc
[group('lint')]
[no-exit-message]
relint:
    pre-commit run --hook-stage manual relint --all-files

# Updates pre-commit hooks to latest tags
[group('misc')]
pc-update:
    pre-commit autoupdate -j {{ num_cpus() }}

# Runs garbage collection for git & pre-commit
[group('misc')]
gc:
    git gc --aggressive --prune
    pre-commit gc

# Bumps version hard-coded in files & creates a new git tag
[group('misc')]
bump part='major':
    bump-my-version bump --verbose {{ part }}
