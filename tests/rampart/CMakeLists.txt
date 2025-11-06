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

# NB. even if we don't have a Python interpreter to run the wrapper script, we still want to build
# the rampart executable

FetchContent_Declare (
    nlohmann_json
    SYSTEM
    GIT_REPOSITORY "https://github.com/nlohmann/json.git"
    GIT_TAG v3.12.0
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 3.12.0
)

FetchContent_Declare (
    rampart
    SYSTEM
    GIT_REPOSITORY "https://github.com/schnitzi/rampart.git"
    GIT_TAG origin/master
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable (nlohmann_json rampart)

add_executable (rampart)

target_link_libraries (rampart PRIVATE ben_bot::libchess nlohmann_json::nlohmann_json)

target_sources (rampart PRIVATE main.cpp)

add_feature_info (
    benbot_rampart_testing "TARGET Python::Interpreter" "BenBot rampart movegen testing"
)

if (NOT TARGET Python::Interpreter)
    return ()
endif ()

file (GLOB testcase_files LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${rampart_SOURCE_DIR}/src/main/resources/testcases/*.json"
)

foreach (data_file IN LISTS testcase_files)
    cmake_path (GET data_file STEM filename)

    set (test_name "ben_bot.rampart.${filename}")

    add_test (NAME "${test_name}"
              COMMAND Python::Interpreter "${CMAKE_CURRENT_LIST_DIR}/RunRampart.py"
                      "--test=${data_file}" "--exec=$<TARGET_FILE:rampart>"
    )

    set_tests_properties ("${test_name}" PROPERTIES REQUIRED_FILES "${data_file}")
endforeach ()
