# fastchess

Tests that utilize the `fastchess` tool. This tool can perform UCI compliance testing, and we also use it for SPRT testing.

## SPRT testing

SPRT testing works by playing a match between the previous ("baseline") build and the new build. This directory creates two custom targets that you can run to automate this process:
* `sprt_set_baseline`: Builds `ben_bot` and uses this new binary as the baseline for the next SPRT test
* `sprt`: runs SPRT testing with the newest `ben_bot` build and the last baseline that was saved by building `sprt_set_baseline`

Neither of these targets are run by CTest; SPRT testing should be invoked manually.
