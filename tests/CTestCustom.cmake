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

#[[
This file was configured by CMake, any changes will be overwritten!

This file is read by CTest when processing build, test & coverage output for dashboard aggregation.
]]

set (CTEST_CUSTOM_TEST_OUTPUT_TRUNCATION middle)

set (CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE 1000000) # 1 MB
set (CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE 1000000) # 1 MB

set (CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS 100)

# this matches forward- or back-slashes
set (slash "[/\\]")

list (APPEND CTEST_CUSTOM_COVERAGE_EXCLUDE @CMAKE_PREFIX_PATH@ "@FETCHCONTENT_BASE_DIR@"
      "${slash}_deps${slash}" "tests${slash}"
)

list (
    APPEND
    CTEST_CUSTOM_WARNING_EXCEPTION
    @CMAKE_PREFIX_PATH@
    "@FETCHCONTENT_BASE_DIR@"
    "${slash}_deps${slash}"
    "bits/sigaction.h"
    "Program Files\\\\Microsoft Visual Studio"
    "libbenbot${slash}resources${slash}__cmrc_ben_bot_resources_internal${slash}"
    "libutil${slash}src${slash}memory${slash}PageAlignedAlloc_Windows.hpp"
    # MSVC warnings about the Visitor class's move operators being implicitly deleted
    "util::Visitor<"
    # warnings about std::getenv() being deprecated/thread-unsafe
    "main.cpp.+getenv"
    "note: 'getenv' has been explicitly marked deprecated here"
)
