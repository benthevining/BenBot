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

# needed to fix ABI issues that manifest as linker errors if Catch2 isn't built with C++23
set (CMAKE_CXX_STANDARD 23)

FetchContent_Declare (
    Catch2
    SYSTEM
    GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
    GIT_TAG v3.13.0
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 3.13.0
)

FetchContent_MakeAvailable (Catch2)

if (catch2_SOURCE_DIR)
    list (APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
elseif (Catch2_DIR)
    list (APPEND CMAKE_MODULE_PATH "${Catch2_DIR}")
endif ()

include (Catch)

#

function (add_catch_tests target prefix)
    # cmake-format: off
    catch_discover_tests (
            "${target}"
            EXTRA_ARGS
            --verbosity high
            --warn NoAssertions
            --warn UnmatchedTestSpec
            --colour-mode none
            --reporter console::out=-
            TEST_PREFIX "${prefix}.unit."
            PROPERTIES
                SKIP_REGULAR_EXPRESSION "SKIPPED:"
    )
    # cmake-format: on
endfunction ()

add_subdirectory (libchess)
add_subdirectory (libbenbot)
