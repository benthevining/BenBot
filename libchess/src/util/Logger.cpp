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

using std::streambuf;

struct Tie final : streambuf {
    Tie(streambuf& buffer, streambuf& log)
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

auto Tie::sync() -> int
{
    logBuf.pubsync();
    return buf.pubsync();
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
    static int_type last = '\n';

    if (last == '\n') {
        logBuf.sputn(
            prefix.data(),
            static_cast<std::streamsize>(prefix.size()));
    }

    return last = logBuf.sputc(static_cast<char_type>(character));
}

using MaybeError = std::expected<void, std::string>;

struct Logger final {
    [[nodiscard]] static auto start(const std::filesystem::path& logFile) -> MaybeError
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

    Logger(const Logger&)            = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&)      = delete;

private:
    Logger() = default;

    ~Logger() { close_log_file(); }

    void close_log_file()
    {
        if (not file.is_open())
            return;

        std::cout.rdbuf(out.buffer());
        std::cin.rdbuf(in.buffer());

        file.close();
    }

    std::ofstream file;

    Tie in { *std::cin.rdbuf(), *file.rdbuf() };
    Tie out { *std::cout.rdbuf(), *file.rdbuf() };
};

auto start_file_logger(const std::filesystem::path& logFile) -> MaybeError
{
    const auto path = absolute(logFile);

    create_directories(path.parent_path());

    return Logger::start(path);
}

} // namespace chess::util
