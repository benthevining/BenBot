# Documentation

This directory defines the build rules for our documentation.

Library API docs are built using Doxygen. Documentation is written inline in the library header files.

## CMake

### Targets

* `ben_bot_docs`: Builds the library API documentation using Doxygen
* `ben_bot_open_docs`: First builds `ben_bot_docs`, then opens the generated HTML documentation in your default browser

## Dependencies

* [Doxygen](https://www.doxygen.nl/index.html): required to build API docs
* dot, from [graphviz](https://graphviz.org/): optional; needed for generating charts and graphs
