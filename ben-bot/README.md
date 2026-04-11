# BenBot

This directory contains the `ben_bot` executable. This executable is a command line UCI engine.

## CMake

### Options

* `BENBOT_IPO`: controls whether interprocedural optimization is used when compiling `ben_bot`. Defaults to on if IPO is supported, and force-set to off if unsupported.
* `BENBOT_CODESIGN_ID`: codesign ID used for signing `ben_bot`. Initialized by the environment variable of the same name.

### Targets

* `ben_bot::ben_bot`: executable target
