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

FetchContent_Declare (
    Catch2
    SYSTEM
    GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
    GIT_TAG v3.8.1
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 3.8.1
)

FetchContent_MakeAvailable (Catch2)

if (catch2_SOURCE_DIR)
    list (APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
elseif (Catch2_DIR)
    list (APPEND CMAKE_MODULE_PATH "${Catch2_DIR}")
endif ()

include (Catch)

#

add_executable (libchess_tests)

target_link_libraries (libchess_tests PRIVATE ben_bot::libchess Catch2::Catch2WithMain)

target_compile_definitions (
    libchess_tests
    PRIVATE
        "BENBOT_LICENSE_HEADER_FILE=\"${BenBot_SOURCE_DIR}/ben-bot/resources/license_header.txt\""
)

file (GLOB_RECURSE sources LIST_DIRECTORIES false CONFIGURE_DEPENDS
                                                  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

target_sources (libchess_tests PRIVATE ${sources})

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${sources})

# cmake-format: off
catch_discover_tests (
        libchess_tests
        EXTRA_ARGS
            --verbosity high
            --warn NoAssertions
            --warn UnmatchedTestSpec
            --colour-mode none
            --reporter console::out=-
        TEST_PREFIX libchess.unit.
        PROPERTIES
            SKIP_REGULAR_EXPRESSION "SKIPPED:"
)

catch_discover_tests (
        libchess_tests
        TEST_SPEC "[!benchmark]"
        EXTRA_ARGS
            --verbosity high
            --warn UnmatchedTestSpec
            --colour-mode none
            --reporter console::out=-
            --reporter cdash::out=-
        TEST_PREFIX libchess.bench.
        PROPERTIES
            SKIP_REGULAR_EXPRESSION "SKIPPED:"
)
# cmake-format: on
