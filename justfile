# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================
# Configuration file for the Just command runner: https://github.com/casey/just
# Using Just is optional; even if you don't have it installed, this file serves
# as a convenient reference for this project's most frequently used commands.

set windows-shell := ['cmd.exe', '/c']

# NB. the require() function seems to have trouble escaping spaces on Windows correctly

PYTHON := if os_family() == 'windows' { 'python3' } else { require('python3') }

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

# Runs clang-tidy on all source files (might take a few minutes)
[group('lint')]
[no-exit-message]
clang-tidy: cmake-config
    cmake -E make_directory {{ justfile_directory() }}/logs
    pre-commit run --hook-stage manual clang-tidy --all-files

# Checks for things like to-do notes, #if 0, etc
[group('lint')]
[no-exit-message]
relint:
    pre-commit run --hook-stage manual relint --all-files

# Updates pre-commit hooks to latest tags
[group('misc')]
pc-update:
    pre-commit autoupdate -j {{ num_cpus() }}

# Removes all temporary files (but not the build tree)
[group('misc')]
clean:
    git clean -fxd -e .envrc.user -e .env -e CMakeUserPresets.json -e Builds
    cmake -E chdir {{ justfile_directory() }}/Builds/ninja ninja -t cleandead

# Runs garbage collection for git & pre-commit
[group('misc')]
gc:
    git gc --aggressive --prune
    pre-commit gc

# Bumps version hard-coded in files & creates a new git tag
[group('misc')]
bump part='major':
    bump-my-version bump --verbose {{ part }}

# Configures CMake with Ninja
[group('cmake')]
cmake:
    cmake --preset ninja

# Runs ccmake for interactive cache editing
[group('cmake')]
cmake-cache: cmake-config
    ccmake -S {{ justfile_directory() }} -B {{ justfile_directory() }}/Builds/ninja

# Runs the CTest dashboard
[group('test')]
[no-exit-message]
[working-directory('Builds/ninja')]
cdash config='Debug': cmake-config
    ctest -D Nightly -C {{ config }}

# Builds & runs the unit tests
[group('test')]
[no-exit-message]
check config='Debug': (cmake-build config)
    -ctest --preset ninja -C {{ config }} -j {{ num_cpus() }}

# Builds & opens the docs (requires Doxygen)
[group('docs')]
docs: cmake-config
    cmake --build --preset ninja --target mtm_open_docs

# Configures & opens the Xcode project
[group('cmake')]
[macos]
xcode:
    cmake --preset xcode
    cmake --open {{ justfile_directory() }}/Builds/xcode

# Build the Xcode project from the command line
[group('cmake')]
[macos]
build-xcode config='Debug':
    scripts/BuildXcode.sh {{ config }}

# Configures & opens the Visual Studio project
[group('cmake')]
[windows]
vs:
    cmake --preset vs
    cmake --open {{ justfile_directory() }}/Builds/vs

vs_config_command := if path_exists('Builds/vs') == 'true' { '-E true' } else { '--preset vs' }

# Build the Visual Studio project from the command line
[group('cmake')]
[windows]
build-vs config='Debug':
    cmake {{ vs_config_command }}
    cmake --build --preset vs --config {{ config }}

#

cmake_config_command := if path_exists('Builds/ninja') == 'true' { '-E true' } else { '--preset ninja' }

[private]
cmake-config:
    cmake {{ cmake_config_command }}

[private]
cmake-build config: cmake-config
    cmake --build --preset ninja --config {{ config }} --jobs {{ num_cpus() }}
