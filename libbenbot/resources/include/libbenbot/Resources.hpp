/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

#pragma once

#include <string>
#include <string_view>

/** @defgroup benbot_data Resources
    Binary data embedded into the engine executable.
    @ingroup libbenbot
 */

/** @file
    This file defines functions for accessing data embedded into the BenBot executable.
    @ingroup benbot_data
 */

/** This namespace contains resource data embedded into the engine executable.
    @ingroup benbot_data
 */
namespace ben_bot::resources {

using std::string_view;

/// @ingroup benbot_data
/// @{

/** Returns the default set of bench positions as an EPD text. */
[[nodiscard]] auto get_bench_epd_text() -> string_view;

/** Returns the engine's ASCII logo. */
[[nodiscard]] auto get_ascii_logo() -> string_view;

/** Returns the engine's version string. */
[[nodiscard, gnu::const]] auto get_version_string() -> string_view;

/** Returns the name of the compiler used to build the engine. */
[[nodiscard, gnu::const]] auto get_compiler_name() -> string_view;

/** Returns the version of the compiler used to build the engine. */
[[nodiscard, gnu::const]] auto get_compiler_version() -> string_view;

/** Returns the name of the target system that the engine was built for. */
[[nodiscard, gnu::const]] auto get_system_name() -> string_view;

/** Returns the name of the build configuration that the engine was built for. */
[[nodiscard, gnu::const]] auto get_build_config() -> string_view;

/** Returns the build time (of the resources library) as a string. */
[[nodiscard]] auto get_build_time() -> std::string;

/// @}

} // namespace ben_bot::resources
