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

if (NOT TARGET Python::Interpreter)
    return ()
endif ()

# check if the python-chess module can be imported
execute_process (
    COMMAND "${Python_EXECUTABLE}" -c "import chess.engine" RESULT_VARIABLE python_chess_found
    OUTPUT_QUIET ERROR_QUIET
)

if (NOT "${python_chess_found}" EQUAL 0)
    message (FATAL_ERROR "python-chess module not found!")
    return ()
endif ()

file (GLOB testcase_files LIST_DIRECTORIES false CONFIGURE_DEPENDS
                                                 "${CMAKE_CURRENT_LIST_DIR}/data/*.epd"
)

list (APPEND CMAKE_CONFIGURE_DEPENDS ${testcase_files})

foreach (epd_file IN LISTS testcase_files)
    cmake_path (GET epd_file STEM filename)

    file (STRINGS "${epd_file}" epd_lines)

    list (LENGTH epd_lines num_epds)

    math (EXPR num_epds "${num_epds} - 1") # because RANGE includes the end value

    foreach (epd_idx RANGE "${num_epds}")
        add_test (NAME "ben_bot.position_solver.${filename}.${epd_idx}"
                  COMMAND Python::Interpreter "${CMAKE_CURRENT_LIST_DIR}/solver.py" "${epd_file}"
                          "${epd_idx}" "$<TARGET_FILE:ben_bot>"
        )
    endforeach ()
endforeach ()
