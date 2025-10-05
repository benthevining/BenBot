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

find_package (Python 3.10 COMPONENTS Interpreter)

if (NOT TARGET Python::Interpreter)
    message (VERBOSE "Python interpreter not found, not adding all test cases")
endif ()

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

        if (NOT "${module_found}" EQUAL 0)
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
# add_subdirectory (matetrack)
add_subdirectory (perft)
add_subdirectory (position-solver)
add_subdirectory (rampart)
add_subdirectory (unit)

add_test (NAME ben_bot.bench.engine_nps COMMAND ben_bot::ben_bot bench --no-loop
          CONFIGURATIONS Release
)

set_tests_properties (ben_bot.bench.engine_nps PROPERTIES RUN_SERIAL TRUE)
