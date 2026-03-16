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

add_feature_info (
    benbot_e2e_testing "TARGET Python::Interpreter" "BenBot end-to-end & integration testing"
)

if (NOT TARGET Python::Interpreter)
    return ()
endif ()

add_test (NAME ben_bot.one_shot_clis
          COMMAND Python::Interpreter "${CMAKE_CURRENT_LIST_DIR}/OneShotCLIs.py"
                  "$<TARGET_FILE:ben_bot>"
)

add_test (NAME ben_bot.e2e COMMAND Python::Interpreter "${CMAKE_CURRENT_LIST_DIR}/e2e.py"
                                   "$<TARGET_FILE:ben_bot>"
)

set_tests_properties (
    ben_bot.one_shot_clis ben_bot.e2e PROPERTIES REQUIRED_FILES "$<TARGET_FILE:ben_bot>"
)
