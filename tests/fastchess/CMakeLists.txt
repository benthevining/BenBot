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

find_program (FASTCHESS_PROGRAM NAMES fastchess fast-chess DOC "fastchess CLI executable")

if (NOT FASTCHESS_PROGRAM)
    message (VERBOSE "fastchess not found, not creating SPRT targets")
    return ()
endif ()

add_test (NAME ben_bot.uci_compliance COMMAND "${FASTCHESS_PROGRAM}" --compliance
                                              $<TARGET_FILE:ben_bot>
)

#

set (baseline_binary "$<TARGET_FILE_DIR:ben_bot>/last-$<CONFIG>")

add_custom_target (
    sprt_set_baseline COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:ben_bot>"
                              "${baseline_binary}"
    COMMENT "Saving baseline binary for SPRT testing..." VERBATIM USES_TERMINAL
)

add_dependencies (sprt_set_baseline ben_bot)

cmake_host_system_information (RESULT num_cores QUERY NUMBER_OF_PHYSICAL_CORES)

set (openings_file "${BenBot_SOURCE_DIR}/ben-bot/resources/book.pgn")

# cmake-format: off
add_custom_target (
    sprt
    COMMAND "${FASTCHESS_PROGRAM}"
        -engine "cmd=$<TARGET_FILE:ben_bot>" name=Refactor
        -engine "cmd=${baseline_binary}" name=Baseline
        -each tc=8+0.08 -rounds 50 -repeat -concurrency "${num_cores}" -recover
        -openings "file=${openings_file}" format=pgn
        -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:ben_bot>"
    COMMENT "Running SPRT test..."
    VERBATIM USES_TERMINAL
)
# cmake-format: on

add_dependencies (sprt ben_bot)
