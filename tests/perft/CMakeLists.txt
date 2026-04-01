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

add_feature_info (benbot_perft_testing "TARGET Python::Interpreter" "BenBot perft testing")

if (NOT TARGET Python::Interpreter)
    return ()
endif ()

file (GLOB testcase_files LIST_DIRECTORIES false CONFIGURE_DEPENDS
                                                 "${CMAKE_CURRENT_LIST_DIR}/data/*.json"
)

foreach (data_file IN LISTS testcase_files)
    cmake_path (GET data_file STEM filename)

    file (READ "${data_file}" file_content)

    string (JSON num_depths LENGTH "${file_content}" depths)

    # because foreach(RANGE) includes the boundary value
    math (EXPR num_depths "${num_depths} - 1")

    foreach (depth_idx RANGE 0 "${num_depths}")
        # cmake-format: off
        string (
            JSON
            depth GET
            "${file_content}"
            depths "${depth_idx}" depth
        )
        # cmake-format: on

        set (test_name "ben_bot.perft.${filename}.${depth}")

        add_test (
            NAME "${test_name}"
            COMMAND Python::Interpreter "${CMAKE_CURRENT_LIST_DIR}/perft.py" "--test=${data_file}"
                    "--engine=$<TARGET_FILE:ben_bot>" "--depth=${depth}"
        )

        set_tests_properties ("${test_name}" PROPERTIES REQUIRED_FILES "$<TARGET_FILE:ben_bot>")
    endforeach ()

    set_property (DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${data_file}")
endforeach ()
