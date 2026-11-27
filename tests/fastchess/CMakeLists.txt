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

add_feature_info (benbot_sprt_testing FASTCHESS_PROGRAM "BenBot SPRT tests using fastchess program")

if (NOT FASTCHESS_PROGRAM)
    return ()
endif ()

add_test (NAME ben_bot.uci_compliance COMMAND "${FASTCHESS_PROGRAM}" --compliance
                                              "$<TARGET_FILE:ben_bot>"
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

set (book_name "8moves_v3.pgn")
set (openings_file "${CMAKE_CURRENT_BINARY_DIR}/${book_name}")
set (download_script "${CMAKE_CURRENT_LIST_DIR}/DownloadBook.cmake")

add_custom_command (
    OUTPUT "${openings_file}"
    COMMAND "${CMAKE_COMMAND}" -D "BOOK_NAME=${book_name}" -P "${download_script}"
    DEPENDS "${download_script}"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${book_name}.zip"
    COMMENT "Downloading openings book for SPRT tests..."
    VERBATIM USES_TERMINAL
)

#

# engines don't ponder, so only one engine per game is thinking at a time
cmake_host_system_information (RESULT num_cores QUERY NUMBER_OF_LOGICAL_CORES)

set (logs_dir "${BenBot_SOURCE_DIR}/logs/sprt")

# used by the sprt pr ci workflow to shard SPRT tests across several machines. Each shard plays
# games with a different opening order until it's stopped, and the workflow combines the results
option (BENBOT_SPRT_SHARD "Play SPRT games without stopping at the SPRT bounds" OFF)
set (BENBOT_SPRT_SEED "" CACHE STRING "Seed for the SPRT tests' random opening order")

mark_as_advanced (BENBOT_SPRT_SHARD BENBOT_SPRT_SEED)

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

    if (BENBOT_SPRT_SHARD)
        set (sprt_args "")
    else ()
        set (sprt_args -sprt "elo0=${elo0}" "elo1=${elo1}" alpha=0.05 beta=0.1)
    endif ()

    if (BENBOT_SPRT_SEED STREQUAL "")
        set (seed_args "")
    else ()
        set (seed_args -srand "${BENBOT_SPRT_SEED}")
    endif ()

    # cmake-format: off
    add_custom_target (
        "${name}"
        COMMAND "${FASTCHESS_PROGRAM}"
            -engine "cmd=$<TARGET_FILE:ben_bot>" name=Refactor
            -engine "cmd=${baseline_binary}" name=Baseline
            -each tc=8+0.08 option.Hash=32
            -rounds 100000 -repeat
            ${sprt_args} ${seed_args}
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

add_sprt_test (sprt_gain 0 5 "gainer")
add_sprt_test (sprt_gain_big 0 10 "big gainer")
add_sprt_test (sprt_non_regression -5 0 "non-regression")

#

# self-play games for the eval tuner's own data (see tools/tuner/README.md). There's no -repeat,
# since colour-swapped pairs of games would add correlated positions
if (BENBOT_TUNER)
    set (BENBOT_DATAGEN_GAMES 10000 CACHE STRING "Number of games tuner_datagen plays")
    set (BENBOT_DATAGEN_LIMIT "tc=2+0.02"
         CACHE STRING "tuner_datagen's limit per move, such as tc=2+0.02 or nodes=20000"
    )
    set (BENBOT_DATAGEN_SEED "" CACHE STRING "Seed for tuner_datagen's random opening order")

    mark_as_advanced (BENBOT_DATAGEN_GAMES BENBOT_DATAGEN_LIMIT BENBOT_DATAGEN_SEED)

    set (datagen_dir "${BenBot_SOURCE_DIR}/logs/datagen")
    set (datagen_pgn "${datagen_dir}/games.pgn")
    set (datagen_log "${datagen_dir}/datagen.log")

    if (BENBOT_DATAGEN_SEED STREQUAL "")
        set (datagen_seed_args "")
    else ()
        set (datagen_seed_args -srand "${BENBOT_DATAGEN_SEED}")
    endif ()

    # the PGN's move comments have the book moves & each engine move's score, which benbot_tuner
    # extract reads. fastchess adds to the PGN if it already exists
    # cmake-format: off
    add_custom_target (
        tuner_datagen
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${datagen_dir}"
        COMMAND "${FASTCHESS_PROGRAM}"
            -engine "cmd=$<TARGET_FILE:ben_bot>" name=BenBot1
            -engine "cmd=$<TARGET_FILE:ben_bot>" name=BenBot2
            -each "${BENBOT_DATAGEN_LIMIT}" option.Hash=16
            -rounds "${BENBOT_DATAGEN_GAMES}" -games 1
            ${datagen_seed_args}
            -ratinginterval 100
            -maxmoves 400
            -resign movecount=3 score=500 twosided=true
            -draw movenumber=40 movecount=8 score=10
            -concurrency "${num_cores}"
            -recover
            -openings "file=${openings_file}" format=pgn order=random
            -pgnout "file=${datagen_pgn}" nodes=true
            -log "file=${datagen_log}" level=warn
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:ben_bot>"
        BYPRODUCTS "${datagen_pgn}" "${datagen_log}"
        DEPENDS "${openings_file}" ben_bot::ben_bot
        COMMENT "Playing self-play games for the eval tuner..."
        VERBATIM USES_TERMINAL
    )
    # cmake-format: on

    add_dependencies (tuner_datagen ben_bot)
endif ()

#

if (NOT TARGET Python::Interpreter)
    return ()
endif ()

# plays a gauntlet against Stockfish at several UCI_Elo levels & estimates ben_bot's Elo. Not run by
# CTest, since it takes hours; run EstimateElo.py directly for the other options
# cmake-format: off
add_custom_target (
    estimate_elo
    COMMAND Python::Interpreter "${CMAKE_CURRENT_LIST_DIR}/EstimateElo.py"
        "$<TARGET_FILE:ben_bot>"
        --book "${openings_file}"
        --fastchess "${FASTCHESS_PROGRAM}"
        --out-dir "${BenBot_SOURCE_DIR}/logs/elo"
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:ben_bot>"
    DEPENDS "${openings_file}" ben_bot::ben_bot
    COMMENT "Estimating ben_bot's Elo..."
    VERBATIM USES_TERMINAL
)
# cmake-format: on

add_dependencies (estimate_elo ben_bot)
