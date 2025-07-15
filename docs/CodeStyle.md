# BenBot code style guidelines

The `BenBot` source code is written to be modern, understandable, and efficient.
Our coding style is enforced by tools such as `clang-format`, `clang-tidy`, etc.,
and therefore many of our guidelines are formalized in the configuration files
for these tools. However, this file provides some additional guidelines useful for
contributors to the `BenBot` project.

## C++

* Prefer modern facilities: use `auto`, `using`, etc.
* West const. `const auto* foo` or `auto* const foo`.
* For boolean operators `&&`, `||` and `!`, we prefer the alternate tokens `and`, `or`, and `not`. This makes the code more readable. `!` is much easier to miss than `not`.
* Prefer to use standard library mechanisms over reinventing the wheel.

## CMake

* CMake code **is code**. The same level of care should be taken maintaining our CMake scripts as our C++ sources.
* Never hard-code in the project something that isn't absolutely **essential** for the project to build correctly. Most build settings can be injected via toolchain files, CMake presets, or other mechanisms.

## Other files

* Every configuration file should have at least a brief explanatory comment mentioning what it is for, if the file format allows for comments.
* When adding new file types, add any relevant linting/formatting hooks to `.pre-commit-config.yaml`.
