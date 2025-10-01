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

#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <libchess/util/Logger.hpp>
#include <streambuf>
#include <string>
#include <string_view>

namespace chess::util {

// Credit to Stockfish for this idea:
// https://github.com/official-stockfish/Stockfish/blob/3073d82ccf25a8ab31e0e0215a2a661d0dcdadd7/src/misc.cpp

using std::streambuf;

// This is a streambuf that wraps another streambuf, passing through the output
// and duplicating it into a second streambuf. This is used to duplicate cin and
// cout traffic to a log file for debugging purposes.
struct Tie final : streambuf {
    Tie(streambuf& buffer, streambuf& log) // NOLINT(bugprone-easily-swappable-parameters)
        : buf { buffer }
        , logBuf { log }
    {
    }

    [[nodiscard]] auto buffer() const noexcept -> streambuf* { return &buf; }

private:
    auto sync() -> int override;

    auto overflow(int_type character) -> int_type override;

    auto underflow() -> int_type override { return buf.sgetc(); }

    auto uflow() -> int_type override { return log(buf.sbumpc(), ">> "); }

    [[nodiscard]] auto log(
        int_type         character,
        std::string_view prefix) const -> int_type;

    streambuf& buf;    // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    streambuf& logBuf; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

// NB. virtual member functions defined out-of-line to address -Wweak-vtables

auto Tie::sync() -> int
{
    const bool logSuccess = logBuf.pubsync() == 0;
    const bool bufSuccess = buf.pubsync() == 0;

    return logSuccess and bufSuccess ? 0 : -1;
}

auto Tie::overflow(const int_type character) -> int_type
{
    return log(
        buf.sputc(static_cast<char_type>(character)),
        "<< ");
}

auto Tie::log(
    const int_type         character,
    const std::string_view prefix) const -> int_type
{
    // this is static because there's only one log file shared between
    // two tie objects (one for stdin and one for stdout)
    static int_type last = '\n';

    if (last == '\n') {
        logBuf.sputn(
            prefix.data(),
            static_cast<std::streamsize>(prefix.size()));
    }

    last = logBuf.sputc(static_cast<char_type>(character));

    return last;
}

using std::filesystem::path;
using MaybeError = std::expected<void, std::string>;

// wraps two tie objects (one for stdin & one for stdout) and a file stream
// stdin & stdout traffic is duplicated into the log file
struct Logger final {
    [[nodiscard]] static auto start(const path& logFile) -> MaybeError;

    Logger(const Logger&)            = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&)      = delete;

private:
    Logger() = default;

    ~Logger() { close_log_file(); }

    void close_log_file();

    std::ofstream file;

    Tie in { *std::cin.rdbuf(), *file.rdbuf() };
    Tie out { *std::cout.rdbuf(), *file.rdbuf() };
};

auto Logger::start(const path& logFile) -> MaybeError
{
    static Logger logger;

    logger.close_log_file();

    if (logFile.empty())
        return {};

    logger.file.open(logFile, std::ifstream::out);

    if (not logger.file.is_open()) {
        return std::unexpected(
            std::format(
                "Unable to open log file at path '{}'",
                logFile.string()));
    }

    std::cin.rdbuf(&logger.in);
    std::cout.rdbuf(&logger.out);

    return {};
}

void Logger::close_log_file()
{
    if (not file.is_open())
        return;

    std::cout.rdbuf(out.buffer());
    std::cin.rdbuf(in.buffer());

    file.close();
}

auto start_file_logger(const path& logFile) -> MaybeError
{
    const auto path = absolute(logFile);

    create_directories(path.parent_path());

    return Logger::start(path);
}

} // namespace chess::util
