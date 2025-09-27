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

/** @file
    This file defines the custom command struct used by the engine to implement
    non-standard UCI commands.

    @ingroup benbot
 */

#pragma once

#include <functional>
#include <string_view>
#include <utility>

namespace ben_bot {

using std::string_view;

/** A custom UCI command that the engine can respond to.
    @ingroup benbot
 */
struct CustomCommand final {
    using Callback = std::function<void(string_view)>;

    /** The name of the command.
        This is the token the user should type in the CLI to execute the command.
     */
    string_view name;

    /** Function object that will be called when the command is executed.
        This callback will receive the rest of the command line as its argument.
     */
    Callback action;

    /** Brief description of this command. This will be shown in the engine's help output. */
    string_view description;

    /** A brief string to provide some documentation for the command's arguments.
        This will be shown in the engine's help output.
        For example, if the command expects a single filepath argument, this help string
        might be ``<path>``.
     */
    string_view argsHelp;

    /** Wraps a callback taking no arguments into a ``Callback`` for a command. */
    [[nodiscard]] static auto void_cb(std::function<void()>&& func) -> Callback
    {
        return [callback = std::move(func)]([[maybe_unused]] const string_view args) {
            callback();
        };
    }
};

} // namespace ben_bot
