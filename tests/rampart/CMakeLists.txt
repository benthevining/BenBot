# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

add_executable (rampart)

target_link_libraries (rampart PRIVATE libchess::libchess nlohmann_json::nlohmann_json)

target_sources (rampart PRIVATE main.cpp)

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

if (NOT TARGET Python::Interpreter)
    return ()
endif ()

add_custom_target (
    run_rampart
    COMMAND Python::Interpreter "${generated_script}"
    BYPRODUCTS "${TMP_DIR}"
    COMMENT "Running rampart tests..."
    USES_TERMINAL COMMAND_EXPAND_LISTS
    SOURCES RunRampart.py
)

add_dependencies (run_rampart rampart)

add_test (NAME libchess.rampart COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target
                                        run_rampart --config "$<CONFIG>"
)
