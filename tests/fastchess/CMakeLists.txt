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

set (openings_file "${CMAKE_CURRENT_LIST_DIR}/book.pgn")

set (logs_dir "${BenBot_SOURCE_DIR}/logs/sprt")

add_custom_command (
    OUTPUT "${logs_dir}/stamp"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${logs_dir}"
    COMMAND "${CMAKE_COMMAND}" -E touch "${logs_dir}/stamp"
    COMMENT "Creating SPRT logs directory..."
    VERBATIM USES_TERMINAL
)

function (add_sprt_test name elo0 elo1 description)
    set (pgn_out "${logs_dir}/${name}-games.pgn")
    set (epd_out "${logs_dir}/${name}-final-positions.epd")
    set (logfile "${logs_dir}/${name}-sprt.log")

    # cmake-format: off
    add_custom_target (
        "${name}"
        COMMAND "${FASTCHESS_PROGRAM}"
            -engine "cmd=$<TARGET_FILE:ben_bot>" name=Refactor
            -engine "cmd=${baseline_binary}" name=Baseline
            -each tc=8+0.08 option.Hash=32
            -rounds 100000 -repeat
            -sprt "elo0=${elo0}" "elo1=${elo1}" alpha=0.05 beta=0.1
            -ratinginterval 10
            -maxmoves 400
            -resign movecount=3 score=500 twosided=true
            -draw movenumber=40 movecount=8 score=10
            -concurrency "${num_cores}"
            -recover -show-latency
            -openings "file=${openings_file}" format=pgn order=random
            -pgnout "file=${pgn_out}" nodes=true nps=true timeleft=true latency=true
            -epdout "file=${epd_out}"
            -log "file=${logfile}" level=warn engine=true
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:ben_bot>"
        BYPRODUCTS "${pgn_out}" "${epd_out}" "${logfile}"
        DEPENDS "${logs_dir}/stamp" "${openings_file}" "${baseline_binary}" ben_bot::ben_bot
        COMMENT "Running SPRT ${description} test..."
        VERBATIM USES_TERMINAL
    )
    # cmake-format: on

    add_dependencies ("${name}" ben_bot)
endfunction ()

add_sprt_test (sprt_gain 0 10 "gainer")
add_sprt_test (sprt_non_regression -10 0 "non-regression")
