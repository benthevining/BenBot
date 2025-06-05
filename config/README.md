# Configuration files

This directory contains configuration files for miscellaneous tools used by `pre-commit` hooks, `just` recipes, etc.

If tools have a command line option for the location of the configuration file, then the file is placed in this
directory, to try to keep the repository root as clean as possible. Only when tools don't support such an option do we
place configuration files in the repository root.

# Environment variables

Various CMake variables are initialized by environment variables. They frequently have the same name, but not always.
Environment variables are typically used for injecting secrets or user-specific credentials.

## direnv

`direnv` is recommended for setting project-specific environment variables when working in this project. The `.envrc`
file in the project root sets a few default variables, and it loads the following user-specific files, if present:

* `.env` - `export` statements only
* `.envrc.user` - can use `direnv` stdlib functions

Both files are git ignored. Both files can override default values set by the project's `.envrc`.

Unfortunately there is no Windows support yet.

## List of useful environment variables

* `LICHESS_BOT_TOKEN`: Lichess OAuth token
