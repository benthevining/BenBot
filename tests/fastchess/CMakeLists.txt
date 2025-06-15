# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

find_program (FASTCHESS_PROGRAM NAMES fastchess fast-chess DOC "fastchess CLI executable")

if (NOT FASTCHESS_PROGRAM)
    return ()
endif ()

add_test (NAME libchess.uci_compliance COMMAND "${FASTCHESS_PROGRAM}" --compliance
                                               $<TARGET_FILE:ben_bot>
)
