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

find_package (Python 3.9 COMPONENTS Interpreter)

set_package_properties (
    Python PROPERTIES URL "https://www.python.org/" DESCRIPTION "Scripting language interpreter"
)

function (check_python_modules_available outputVar)
    if (NOT TARGET Python::Interpreter)
        set ("${outputVar}" FALSE PARENT_SCOPE)
        return ()
    endif ()

    foreach (moduleName IN LISTS ARGN)
        execute_process (
            COMMAND "${Python_EXECUTABLE}" -c "import ${moduleName}" RESULT_VARIABLE module_found
            OUTPUT_QUIET ERROR_QUIET
        )

        if (NOT module_found EQUAL 0)
            message (VERBOSE "Python module ${moduleName} not found")
            set ("${outputVar}" FALSE PARENT_SCOPE)
            return ()
        endif ()
    endforeach ()

    set ("${outputVar}" TRUE PARENT_SCOPE)
endfunction ()

#

configure_file (CTestCustom.cmake "${BenBot_BINARY_DIR}/CTestCustom.cmake" @ONLY)

add_subdirectory (fastchess)
add_subdirectory (perft)
add_subdirectory (position-solver)
add_subdirectory (rampart)
add_subdirectory (unit)

#

cmake_host_system_information (RESULT num_cores QUERY NUMBER_OF_PHYSICAL_CORES)

add_test (
    NAME ben_bot.verify_headers
    COMMAND
        "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target
        libutil_verify_interface_header_sets libchess_verify_interface_header_sets
        libbenbot_verify_interface_header_sets ben_bot_resources_verify_interface_header_sets
        --config "$<CONFIG>" --parallel "${num_cores}"
)

set_tests_properties (ben_bot.verify_headers PROPERTIES RUN_SERIAL TRUE TIMEOUT 0)
