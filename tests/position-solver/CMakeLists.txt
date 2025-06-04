# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (position_solver)

target_link_libraries (position_solver PRIVATE libchess::libchess)

target_sources (position_solver PRIVATE main.cpp)

if (NOT TARGET Python::Interpreter)
    return ()
endif ()

set (generated_script "${CMAKE_CURRENT_BINARY_DIR}/RunPositionSolver$<CONFIG>.py")

# cmake-format: off
file (
    GENERATE
    OUTPUT "${generated_script}"
    INPUT RunPositionSolver.py
    TARGET position_solver
    FILE_PERMISSIONS
        OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)
# cmake-format: on

set (data_file "${CMAKE_CURRENT_LIST_DIR}/positions.json")
set (test_name "libchess.position_solver")

add_test (NAME "${test_name}" COMMAND Python::Interpreter "${generated_script}" "${data_file}")

set_tests_properties ("${test_name}" PROPERTIES REQUIRED_FILES "${data_file}")
