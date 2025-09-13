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

list (APPEND CTEST_CUSTOM_COVERAGE_EXCLUDE # cmake-format: sortable
      "@CMAKE_PREFIX_PATH@" "@FETCHCONTENT_BASE_DIR@" "(_cmrc)+"
)

list (
    APPEND
    CTEST_CUSTOM_ERROR_EXCEPTION
    # cmake-format: sortable
    "(information:)+"
    "error: syntax error [syntaxError]" # cppcheck false-positives for new syntax it can't parse
    "(style:)+" # style warnings shouldn't be a hard error
)

list (APPEND CTEST_CUSTOM_WARNING_MATCH # cmake-format: sortable
      "(style:)+"
)

list (APPEND CTEST_CUSTOM_WARNING_EXCEPTION # cmake-format: sortable
      "@CMAKE_PREFIX_PATH@" "@FETCHCONTENT_BASE_DIR@" "(note:)+"
)
