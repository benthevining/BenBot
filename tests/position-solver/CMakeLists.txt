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

check_python_modules_available (deps_available chess.engine)

add_feature_info (benbot_pos_solver_testing deps_available "BenBot position solver testing")

if (NOT deps_available)
    return ()
endif ()

file (GLOB testcase_files LIST_DIRECTORIES false CONFIGURE_DEPENDS
                                                 "${CMAKE_CURRENT_LIST_DIR}/data/*.epd"
)

set_property (DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${testcase_files})

foreach (epd_file IN LISTS testcase_files)
    cmake_path (GET epd_file STEM filename)

    file (STRINGS "${epd_file}" epd_lines)
    list (LENGTH epd_lines num_epds)
    math (EXPR num_epds "${num_epds} - 1") # because RANGE includes the end value

    foreach (epd_idx RANGE "${num_epds}")
        set (test_name "ben_bot.position_solver.${filename}.${epd_idx}")

        set (engine_log "${BenBot_SOURCE_DIR}/logs/position-solver/${filename}-${epd_idx}.log")

        add_test (
            NAME "${test_name}"
            COMMAND Python::Interpreter "${CMAKE_CURRENT_LIST_DIR}/solver.py" "--test=${epd_file}"
                    "--index=${epd_idx}" "--engine=$<TARGET_FILE:ben_bot>" "--log=${engine_log}"
        )

        # cmake-format: off
        set_tests_properties (
            "${test_name}" PROPERTIES ATTACHED_FILES_ON_FAIL "${engine_log}"
                                      RESOURCE_LOCK BenBotPositionSolver
        )
        # cmake-format: on

        set_property (DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${engine_log}")
    endforeach ()
endforeach ()
