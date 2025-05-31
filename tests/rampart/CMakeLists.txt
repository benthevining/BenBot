# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (rampart)

target_link_libraries (rampart PRIVATE libchess::libchess nlohmann_json::nlohmann_json)

target_sources (rampart PRIVATE main.cpp)

if (NOT TARGET Python::Interpreter)
    return ()
endif ()

# in the template script, we want to expand both variables and generator expressions

file (READ RunRampart.py python_script_content)

set (TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/results")
set (TESTCASES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

string (CONFIGURE "${python_script_content}" python_script_content @ONLY)

set (generated_script "${CMAKE_CURRENT_BINARY_DIR}/RunRampart$<CONFIG>.py")

# cmake-format: off
file (
    GENERATE
    OUTPUT "${generated_script}"
    CONTENT "${python_script_content}"
    TARGET rampart
    FILE_PERMISSIONS
        OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)
# cmake-format: on

set_property (DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS RunRampart.py)

file (GLOB testcase_files LIST_DIRECTORIES false CONFIGURE_DEPENDS
                                                 "${CMAKE_CURRENT_LIST_DIR}/data/*.json"
)

foreach (data_file IN LISTS testcase_files)
    cmake_path (GET data_file STEM filename)

    set (test_name "libchess.rampart.${filename}")

    add_test (NAME "${test_name}" COMMAND Python::Interpreter "${generated_script}" "${data_file}")

    set_tests_properties ("${test_name}" PROPERTIES REQUIRED_FILES "${data_file}")
endforeach ()
