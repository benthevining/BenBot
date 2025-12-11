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

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <expected>
#include <format>
#include <libchess/game/Result.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/PGN.hpp>
#include <libchess/util/Strings.hpp>
#include <numeric>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace chess::notation {

auto GameRecord::get_final_position() const -> Position
{
    return std::accumulate(
        moves.begin(), moves.end(),
        startingPosition,
        [](const Position& pos, const Move& move) {
            return after_move(pos, move.move);
        });
}

namespace {

    using std::size_t;
    using std::string;
    using std::string_view;
    using Metadata            = std::unordered_map<std::string, std::string>;
    using Moves               = std::vector<GameRecord::Move>;
    using GameResult          = std::optional<game::Result>;
    using ResultStrOrErrorStr = std::expected<string_view, string_view>;

    using util::strings::int_from_string;
    using util::strings::split_at_first_space_or_newline;
    using util::strings::trim;

    // writes tag key/value pairs into metadata and returns
    // the rest of the PGN text that's left
    [[nodiscard]] auto parse_metadata_tags(
        string_view pgnText, Metadata& metadata)
        -> ResultStrOrErrorStr
    {
        auto openingBracketIdx = pgnText.find('[');

        while (openingBracketIdx != string_view::npos) {
            const auto closingBracketIdx = pgnText.find(']', openingBracketIdx + 1uz);

            if (closingBracketIdx == string_view::npos)
                return std::unexpected { "Invalid PGN: expected ']' following '['" };

            assert(std::cmp_greater(closingBracketIdx, openingBracketIdx));

            // don't include the brackets
            const auto tagText = pgnText.substr(
                openingBracketIdx + 1uz,
                closingBracketIdx - openingBracketIdx - 1uz);

            // NB. we assume that tag keys cannot include spaces
            auto [tagName, tagValue] = util::strings::split_at_first_space(tagText);

            assert(not tagName.empty());
            assert(not tagValue.empty());

            // remove surrounding quotes from tag value
            if (tagValue.front() == '"')
                tagValue.remove_prefix(1uz);

            if (tagValue.back() == '"')
                tagValue.remove_suffix(1uz);

            metadata[string { tagName }] = tagValue;

            pgnText.remove_prefix(closingBracketIdx + 1uz);

            openingBracketIdx = pgnText.find('[');
        }

        return pgnText; // NOLINT
    }

    // writes the content of the block comment to the last move in output
    // and returns the rest of the pgnText after the } that closes this comment
    [[nodiscard]] auto parse_block_comment(
        const string_view pgnText, Moves& output)
        -> ResultStrOrErrorStr
    {
        assert(pgnText.front() == '{');

        const auto closeBracketIdx = pgnText.find('}');

        if (closeBracketIdx == string_view::npos)
            return std::unexpected { "Expected '}' following '{'" };

        if (not output.empty())
            output.back().comment = pgnText.substr(1uz, closeBracketIdx - 1uz);

        return pgnText.substr(closeBracketIdx + 1uz);
    }

    // writes the content of the line comment to the last move in output
    // and returns the rest of the pgnText after the newline that ends this comment
    [[nodiscard]] auto parse_line_comment(
        const string_view pgnText, Moves& output)
        -> string_view
    {
        assert(pgnText.front() == ';');

        const auto newlineIdx = pgnText.find('\n');

        if (newlineIdx == string_view::npos) {
            // assume that a ; comment was the last thing in the file
            if (not output.empty())
                output.back().comment = trim(pgnText.substr(1uz));

            return {};
        }

        if (not output.empty())
            output.back().comment = trim(pgnText.substr(1uz, newlineIdx - 1uz));

        return pgnText.substr(newlineIdx + 1uz);
    }

    // writes the NAG glyph value to the last move in output
    // and returns the rest of the pgnText after the NAG glyph
    [[nodiscard]] auto parse_nag(
        const string_view pgnText, Moves& output)
        -> string_view
    {
        // NB. we're not doing explicit checks for null NAGs here
        // they shouldn't appear in PGN files we parse, but I don't
        // think it's necessary to refuse to parse them

        assert(pgnText.front() == '$');

        const auto [nag, rest] = split_at_first_space_or_newline(pgnText.substr(1uz));

        if (not output.empty()) {
            const auto value = int_from_string<std::uint_least8_t>(trim(nag));

            output.back().nags.emplace_back(static_cast<NAG>(value));
        }

        return rest;
    }

    // parses the move, adds it to the output, and makes the move on the position
    void parse_move(
        Position& position, string_view moveText, Moves& output)
    {
        // move numbers may start with 3. or 3...
        if (const auto lastDotIdx = moveText.rfind('.');
            lastDotIdx != string_view::npos) {
            moveText = moveText.substr(lastDotIdx + 1uz);
        }

        const auto move = from_alg(position, moveText).value();

        position.make_move(move);

        output.emplace_back(move);
    }

    auto parse_variation(
        string_view pgnText, const Position& position, Moves& output)
        -> std::expected<string_view, string>;

    // parses a move list, including nested comments, NAGs, and variations
    // if IsVariation is true, always returns an empty string_view
    // if IsVariation is false (i.e. parsing root PGN), returns text of the game result
    template <bool IsVariation>
    auto parse_moves_internal(
        string_view pgnText,
        Position    position, // intentionally by copy!
        Moves&      output)
        -> ResultStrOrErrorStr
    {
        // With a PGN like: 1. e4 (e3), the move e3 was made from the starting position,
        // not the position after e4. So because Position doesn't have an unmake_move()
        // function, we instead keep a copy of the previous position before parsing each move
        auto lastPos { position };

        while (true) {
            pgnText = trim(pgnText);

            if (pgnText.empty())
                return {};

            switch (pgnText.front()) {
                case '{': {
                    // comment: { continues to }
                    const auto rest = parse_block_comment(pgnText, output);

                    if (not rest.has_value())
                        return std::unexpected { rest.error() };

                    pgnText = rest.value();
                    continue;
                }

                case ';': {
                    // comment: ; continues to end of line
                    pgnText = parse_line_comment(pgnText, output);
                    continue;
                }

                case '$': {
                    // NAG
                    pgnText = parse_nag(pgnText, output);
                    continue;
                }

                case '(': {
                    // variation
                    const auto rest = parse_variation(pgnText, lastPos, output);

                    if (not rest.has_value())
                        return std::unexpected { rest.error() };

                    pgnText = rest.value();
                    continue;
                }

                default: {
                    // either move as SAN or game result string

                    const auto [firstMove, rest] = split_at_first_space_or_newline(pgnText);

                    // tolerate notation such as: 1. e4 e5
                    // in that case, firstMove will be "1." and rest begins with "e4"
                    // this also catches cases such as "3. ... a5": we skip both the "3." and "..." tokens with this check
                    if (firstMove.back() == '.') {
                        pgnText = rest;
                        continue;
                    }

                    if constexpr (not IsVariation) {
                        if (firstMove.contains('-') and trim(rest).empty()) {
                            // we're parsing the end of the move list, this token is the game result
                            return firstMove;
                        }
                    }

                    lastPos = position;

                    parse_move(position, firstMove, output);

                    pgnText = rest;
                }
            }
        }
    }

    // writes the variation to the last move in output
    // and returns the rest of the pgnText after the variation
    [[nodiscard]] auto parse_variation(
        const string_view pgnText,
        const Position&   position,
        Moves&            output)
        -> std::expected<string_view, string>
    {
        assert(pgnText.front() == '(');

        if (output.empty())
            return std::unexpected { "Cannot parse a variation with an empty move list!" };

        return util::strings::find_matching_close_paren(pgnText)
            .and_then([pgnText, &position, &output](const size_t closeParenIdx) {
                return parse_moves_internal<true>(
                    pgnText.substr(1uz, closeParenIdx - 1uz),
                    position,
                    output.back().variations.emplace_back())
                    .and_then([pgnText, closeParenIdx]([[maybe_unused]] const string_view alwaysEmpty) -> ResultStrOrErrorStr {
                        return pgnText.substr(closeParenIdx + 1uz);
                    })
                    .transform_error([](const string_view error) { return string { error }; });
            });
    }

    // writes the parsed moves into output and returns the
    // game result string (the rest of the PGN after the last move)
    [[nodiscard]] auto parse_move_list(
        const string_view pgnText,
        const Position&   position,
        Moves&            output)
        -> ResultStrOrErrorStr
    {
        return parse_moves_internal<false>(pgnText, position, output);
    }

    [[nodiscard]] auto parse_game_result(
        const string_view text, const GameRecord& game)
        -> GameResult
    {
        const auto sepIdx = text.find('-');

        if (sepIdx == string_view::npos)
            return game.get_final_position().get_result();

        if (const auto whiteScore = trim(text.substr(0uz, sepIdx));
            whiteScore == "1") {
            return game::Result::WhiteWon;
        }

        if (const auto blackScore = trim(text.substr(sepIdx + 1uz));
            blackScore == "1") {
            return game::Result::BlackWon;
        }

        return game::Result::Draw;
    }

} // namespace

using GameOrError = std::expected<GameRecord, string_view>;

auto from_pgn(const string_view pgnText) -> GameOrError
{
    GameRecord game;

    return parse_metadata_tags(pgnText, game.metadata)
        .and_then([&game](const string_view afterMeta) -> GameOrError {
            if (const auto posStr = game.metadata.find("FEN");
                posStr != game.metadata.end()) {
                game.startingPosition = from_fen(posStr->second).value_or(Position {});
            }

            return parse_move_list(afterMeta, game.startingPosition, game.moves)
                .and_then([&game](const string_view resultText) -> GameOrError {
                    game.result = parse_game_result(resultText, game);

                    return game;
                });
        });
}

namespace {

    // returns the index in the string of the next line that either
    // starts with or doesn't start with a '[' character
    // returns npos if no such line is found
    template <bool SearchForBracket>
    [[nodiscard, gnu::const]] constexpr auto find_next_line(const string_view text) -> size_t
    {
        size_t lineStart { 0uz };

        while (std::cmp_less(lineStart, text.size())) {
            if constexpr (SearchForBracket) {
                if (text.at(lineStart) == '[')
                    return lineStart;
            } else {
                if (text.at(lineStart) != '[')
                    return lineStart;
            }

            const auto nextNewline = text.find('\n', lineStart);

            if (nextNewline == string_view::npos)
                return string_view::npos;

            lineStart = nextNewline + 1uz;
        }

        return string_view::npos;
    }

} // namespace

auto parse_all_pgns(string_view fileContent) -> std::vector<GameRecord>
{
    std::vector<GameRecord> games;

    fileContent = trim(fileContent);

    while (not fileContent.empty()) {
        // the move text of this PGN starts at the first line not starting in '['
        const auto moveTextStart = find_next_line<false>(fileContent);

        if (moveTextStart == string_view::npos)
            return games;

        // the next PGN after this one is the first line after moveTextStart that starts with a '['
        const auto moveTextToNextPGN = find_next_line<true>(fileContent.substr(moveTextStart + 1uz));

        if (moveTextToNextPGN == string_view::npos) {
            if (const auto game = from_pgn(fileContent))
                games.emplace_back(game.value());

            return games;
        }

        const auto nextPGNStart = moveTextStart + moveTextToNextPGN;

        if (const auto game = from_pgn(
                fileContent.substr(0uz, nextPGNStart))) {
            games.emplace_back(game.value());
        }

        fileContent.remove_prefix(nextPGNStart);
        fileContent = trim(fileContent);
    }

    return games;
}

namespace {

    void write_metadata_item(
        const string_view key, const string_view value,
        string& output)
    {
        output.append(std::format(
            R"([{} "{}"]
)",
            key, value));
    }

    void write_metadata(
        const Metadata& metadata,
        const Position& startingPosition,
        string&         output)
    {
        using namespace std::literals::string_literals; // NOLINT

        // if these tags are present, they must appear before any other tags and in this order
        static const std::array sevenTagRoster {
            "Event"s, "Site"s, "Date"s, "Round"s, "White"s, "Black"s, "Result"s
        };

        for (const auto& tag : sevenTagRoster)
            if (const auto pos = metadata.find(tag); pos != metadata.end())
                write_metadata_item(tag, pos->second, output);

        // write extra metadata tags not part of seven tag roster
        auto otherTags = metadata
                       | std::views::filter([](const auto& it) {
                             return not std::ranges::contains(sevenTagRoster, it.first); // cppcheck-suppress internalAstError
                         });

        for (const auto& [key, value] : otherTags) {
            write_metadata_item(key, value, output);
        }

        static const Position startPos {};

        if (startingPosition != startPos) {
            if (not metadata.contains("FEN"s)) {
                const auto startFEN = to_fen(startingPosition);
                write_metadata_item("FEN", startFEN, output);
            }

            if (not metadata.contains("Setup"s))
                write_metadata_item("Setup", "1", output);
        }
    }

    void write_move_list(
        Position     position,
        const Moves& moves,
        const bool   useBlockComments,
        string&      output)
    {
        // true if we need to insert a move number before Black's next move
        // true for the first move of the game, the first move of a variation,
        // the first move following a variation, or the first move after a comment
        bool writeMoveNumber { true };

        for (const auto& move : moves) {
            if (position.is_white_to_move()) {
                output.append(std::format("{}.{} ",
                    position.fullMoveCounter, to_alg(position, move.move)));
            } else if (writeMoveNumber) {
                output.append(std::format("{}...{} ",
                    position.fullMoveCounter, to_alg(position, move.move)));
            } else {
                output.append(std::format("{} ", to_alg(position, move.move)));
            }

            for (const auto nag : move.nags) {
                // a null NAG has no typographic representation and shouldn't appear in PGN files
                if (nag != NAG::Null)
                    output.append(std::format("${} ", std::to_underlying(nag)));
            }

            // set to false after the first move
            writeMoveNumber = false;

            if (not move.comment.empty()) {
                if (useBlockComments)
                    output.append(std::format("{{{}}} ", move.comment));
                else
                    output.append(std::format("; {}\n", move.comment));

                // print move number after a comment
                writeMoveNumber = true;
            }

            for (const auto& variation : move.variations) {
                output.append("(");

                write_move_list(position, variation, useBlockComments, output);

                if (output.back() == ' ')
                    output.pop_back();

                output.append(") ");

                // we want to print a move number after closing a subvariation
                writeMoveNumber = true;
            }

            position.make_move(move.move);
        }
    }

    void write_game_result(
        const GameResult result, string& output)
    {
        result.transform([&output](const game::Result outcome) {
            switch (outcome) {
                case game::Result::Draw:
                    output.append("1/2-1/2");
                    break;

                case game::Result::WhiteWon:
                    output.append("1-0");
                    break;

                case game::Result::BlackWon:
                    output.append("0-1");
                    break;

                default: std::unreachable();
            }

            return std::monostate {};
        });
    }

} // namespace

auto to_pgn(const GameRecord& game, const bool useBlockComments) -> string
{
    string result;

    write_metadata(game.metadata, game.startingPosition, result);

    result.append(1uz, '\n');

    write_move_list(game.startingPosition, game.moves, useBlockComments, result);

    write_game_result(game.result, result);

    if (not result.empty() and result.back() == ' ')
        result.pop_back();

    return result;
}

} // namespace chess::notation
