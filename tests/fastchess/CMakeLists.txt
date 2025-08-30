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

# used by the sprt ci workflow to inject the binary from the latest release as the baseline
set (injection_script "${CMAKE_CURRENT_LIST_DIR}/InjectSPRTBaseline.cmake")

file (READ "${injection_script}" script_content)
list (APPEND CMAKE_CONFIGURE_DEPENDS "${injection_script}")

string (CONFIGURE "${script_content}" script_content @ONLY)

file (GENERATE OUTPUT "${BenBot_BINARY_DIR}/InjectSPRTBaseline-$<CONFIG>.cmake"
      CONTENT "${script_content}" TARGET ben_bot NEWLINE_STYLE UNIX
)

#

cmake_host_system_information (RESULT num_cores QUERY NUMBER_OF_PHYSICAL_CORES)

set (openings_file "${BenBot_SOURCE_DIR}/ben-bot/resources/res/book.pgn")

set (logs_dir "${BenBot_SOURCE_DIR}/logs/sprt")

add_custom_command (
    OUTPUT "${logs_dir}/stamp"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${logs_dir}"
    COMMAND "${CMAKE_COMMAND}" -E touch "${logs_dir}/stamp"
    COMMENT "Creating SPRT logs directory..."
    VERBATIM USES_TERMINAL
)

# cmake-format: off
add_custom_target (
    sprt
    COMMAND "${FASTCHESS_PROGRAM}"
        -engine "cmd=$<TARGET_FILE:ben_bot>" name=Refactor
        -engine "cmd=${baseline_binary}" name=Baseline
        -each tc=8+0.08 -rounds 50 -repeat -concurrency "${num_cores}"
        -recover -show-latency
        -openings "file=${openings_file}" format=pgn
        -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05
        -pgnout "file=${logs_dir}/games.pgn" nodes=true nps=true timeleft=true latency=true
        -epdout "file=${logs_dir}/final-positions.epd"
        -log "file=${logs_dir}/sprt.log" engine=true
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:ben_bot>"
    DEPENDS "${logs_dir}/stamp"
    COMMENT "Running SPRT test..."
    VERBATIM USES_TERMINAL
)
# cmake-format: on

add_dependencies (sprt ben_bot)
