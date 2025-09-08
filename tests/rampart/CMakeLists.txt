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

if (NOT TARGET Python::Interpreter)
    return ()
endif ()

set (generated_script "${CMAKE_CURRENT_BINARY_DIR}/RunRampart$<CONFIG>.py")

# cmake-format: off
file (
    GENERATE
    OUTPUT "${generated_script}"
    INPUT RunRampart.py
    TARGET rampart
    FILE_PERMISSIONS
        OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)
# cmake-format: on

file (GLOB testcase_files LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${rampart_SOURCE_DIR}/src/main/resources/testcases/*.json"
)

foreach (data_file IN LISTS testcase_files)
    cmake_path (GET data_file STEM filename)

    set (test_name "ben_bot.rampart.${filename}")

    add_test (NAME "${test_name}" COMMAND Python::Interpreter "${generated_script}" "${data_file}"
                                          "${CMAKE_CURRENT_BINARY_DIR}/results/$<CONFIG>"
    )

    set_tests_properties ("${test_name}" PROPERTIES REQUIRED_FILES "${data_file}")
endforeach ()
