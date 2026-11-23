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

# The eval tuner. This is only built if BENBOT_TUNER is on. See README.md.

# the tuner's maths & dataset code, shared by the tuner & its unit tests
add_library (benbot_tuner_lib STATIC)

target_compile_features (benbot_tuner_lib PUBLIC cxx_std_23)

target_link_libraries (benbot_tuner_lib PUBLIC ben_bot::libbenbot)

file (GLOB_RECURSE tuner_headers LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/tuner/*.hpp"
)

file (GLOB_RECURSE tuner_sources LIST_DIRECTORIES false CONFIGURE_DEPENDS
                                                        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

target_sources (
    benbot_tuner_lib PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${tuner_headers}
    PRIVATE ${tuner_sources}
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${tuner_headers} ${tuner_sources})

add_library (ben_bot::tuner_lib ALIAS benbot_tuner_lib)

# the command-line tool
add_executable (benbot_tuner main.cpp)

target_compile_features (benbot_tuner PRIVATE cxx_std_23)

target_link_libraries (benbot_tuner PRIVATE ben_bot::tuner_lib)

add_executable (ben_bot::tuner ALIAS benbot_tuner)
