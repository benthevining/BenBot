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
    @ingroup benbot
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

/** Returns the engine's opening book as a PGN text. */
[[nodiscard]] string_view get_opening_book_pgn_text();

/** Returns the default set of bench positions as an EPD text. */
[[nodiscard]] string_view get_bench_epd_text();

/** Returns the engine's ASCII logo. */
[[nodiscard]] string_view get_ascii_logo();

/** Returns the engine's version string. */
[[nodiscard, gnu::const]] string_view get_version_string();

/** Returns the name of the compiler used to build the engine. */
[[nodiscard, gnu::const]] string_view get_compiler_name();

/** Returns the version of the compiler used to build the engine. */
[[nodiscard, gnu::const]] string_view get_compiler_version();

/** Returns the name of the target system that the engine was built for. */
[[nodiscard, gnu::const]] string_view get_system_name();

/** Returns the name of the build configuration that the engine was built for. */
[[nodiscard, gnu::const]] string_view get_build_config();

/** Returns the build time (of the resources library) as a string. */
[[nodiscard]] std::string get_build_time();

/// @}

} // namespace ben_bot::resources
