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
#include <format>
#include <libchess/game/Result.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/PGN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/util/Strings.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace chess::notation {

Position GameRecord::get_final_position() const
{
    auto position { startingPosition };

    for (const auto& move : moves)
        position.make_move(move.move);

    return position;
}

namespace {

    using std::size_t;
    using std::string;
    using std::string_view;
    using Metadata   = std::unordered_map<std::string, std::string>;
    using Moves      = std::vector<GameRecord::Move>;
    using GameResult = std::optional<game::Result>;

    using util::int_from_string;
    using util::split_at_first_space_or_newline;

    // writes tag key/value pairs into metadata and returns
    // the rest of the PGN text that's left
    [[nodiscard]] string_view parse_metadata_tags(
        string_view pgnText,
        Metadata&   metadata)
    {
        auto openingBracketIdx = pgnText.find('[');

        while (openingBracketIdx != string_view::npos) {
            const auto closingBracketIdx = pgnText.find(']', openingBracketIdx + 1uz);

            if (closingBracketIdx == string_view::npos) {
                throw std::invalid_argument { "Invalid PGN: expected ']' following '['" };
            }

            assert(closingBracketIdx > openingBracketIdx);

            // don't include the brackets
            const auto tagText = pgnText.substr(
                openingBracketIdx + 1uz,
                closingBracketIdx - openingBracketIdx - 1uz);

            // we assume that tag keys cannot include spaces
            const auto spaceIdx = tagText.find(' ');

            if (spaceIdx == string_view::npos) {
                throw std::invalid_argument {
                    std::format("Expected space in PGN tag key/value text: '{}'", tagText)
                };
            }

            const auto tagName  = tagText.substr(0uz, spaceIdx);
            auto       tagValue = tagText.substr(spaceIdx + 1uz);

            assert(! tagName.empty());
            assert(! tagValue.empty());

            // remove surrounding quotes from tag value
            if (tagValue.front() == '"')
                tagValue.remove_prefix(1uz);

            if (tagValue.back() == '"')
                tagValue.remove_suffix(1uz);

            metadata[std::string { tagName }] = tagValue;

            pgnText.remove_prefix(closingBracketIdx + 1uz);

            openingBracketIdx = pgnText.find('[');
        }

        return pgnText; // NOLINT
    }

    // writes the content of the block comment to the last move in output
    // and returns the rest of the pgnText after the } that closes this comment
    [[nodiscard]] string_view parse_block_comment(
        const string_view pgnText, Moves& output)
    {
        assert(pgnText.front() == '{');

        const auto closeBracketIdx = pgnText.find('}');

        if (closeBracketIdx == string_view::npos) {
            throw std::invalid_argument { "Expected '}' following '{'" };
        }

        if (! output.empty())
            output.back().comment = pgnText.substr(1uz, closeBracketIdx - 1uz);

        return pgnText.substr(closeBracketIdx + 1uz);
    }

    // writes the content of the line comment to the last move in output
    // and returns the rest of the pgnText after the newline that ends this comment
    [[nodiscard]] string_view parse_line_comment(
        const string_view pgnText, Moves& output)
    {
        assert(pgnText.front() == ';');

        const auto newlineIdx = pgnText.find('\n');

        if (newlineIdx == string_view::npos) {
            // assume that a ; comment was the last thing in the file
            if (! output.empty())
                output.back().comment = util::trim(pgnText.substr(1uz));

            return {};
        }

        if (! output.empty())
            output.back().comment = util::trim(pgnText.substr(1uz, newlineIdx - 1uz));

        return pgnText.substr(newlineIdx + 1uz);
    }

    // writes the NAG glyph value to the last move in output
    // and returns the rest of the pgnText after the NAG glyph
    [[nodiscard]] string_view parse_nag(
        const string_view pgnText, Moves& output)
    {
        assert(pgnText.front() == '$');

        auto [nag, rest] = split_at_first_space_or_newline(pgnText.substr(1uz));

        if (! output.empty()) {
            const auto value = int_from_string<std::uint_least8_t>(util::trim(nag));

            output.back().nags.emplace_back(value);
        }

        return rest;
    }

    // parses the move, adds it to the output, and makes the move on the position
    void parse_move(
        Position& position, string_view moveText, Moves& output)
    {
        // move numbers may start with 3. or 3...
        const auto lastDotIdx = moveText.rfind('.');

        if (lastDotIdx != string_view::npos)
            moveText = moveText.substr(lastDotIdx + 1uz);

        const auto move = from_alg(position, moveText);

        position.make_move(move);

        output.emplace_back(move);
    }

    string_view parse_variation(
        string_view pgnText, const Position& position, Moves& output);

    // parses a move list, including nested comments, NAGs, and variations
    // if IsVariation is true, always returns an empty string_view
    // if IsVariation is false (i.e. parsing root PGN), returns text of the game result
    template <bool IsVariation>
    string_view parse_moves_internal(
        string_view pgnText,
        Position    position, // intentionally by copy!
        Moves&      output)
    {
        // With a PGN like: 1. e4 (e3), the move e3 was made from the starting position,
        // not the position after e4. So because Position doesn't have an unmake_move()
        // function, we instead keep a copy of the previous position before parsing each move
        auto lastPos { position };

        while (true) {
            pgnText = util::trim(pgnText);

            if (pgnText.empty())
                return {};

            switch (pgnText.front()) {
                case '{': {
                    // comment: { continues to }
                    pgnText = parse_block_comment(pgnText, output);
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
                    pgnText = parse_variation(pgnText, lastPos, output);
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

                    if constexpr (! IsVariation) {
                        if (firstMove.contains('-') && util::trim(rest).empty()) {
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
    [[nodiscard]] string_view parse_variation(
        const string_view pgnText,
        const Position&   position,
        Moves&            output)
    {
        assert(pgnText.front() == '(');

        if (output.empty()) {
            throw std::invalid_argument { "Cannot parse a variation with an empty move list!" };
        }

        const auto closeParenIdx = util::find_matching_close_paren(pgnText);

        auto& variation = output.back().variations.emplace_back();

        parse_moves_internal<true>(
            pgnText.substr(1uz, closeParenIdx - 1uz),
            position, variation);

        return pgnText.substr(closeParenIdx + 1uz);
    }

    // writes the parsed moves into output and returns the
    // game result string (the rest of the PGN after the last move)
    [[nodiscard]] string_view parse_move_list(
        const string_view pgnText,
        const Position&   position,
        Moves&            output)
    {
        return parse_moves_internal<false>(
            pgnText, position, output);
    }

    [[nodiscard]] GameResult parse_game_result(
        const string_view text, const GameRecord& game)
    {
        const auto sepIdx = text.find('-');

        if (sepIdx == string_view::npos)
            return game.get_final_position().get_result();

        const auto whiteScore = util::trim(text.substr(0uz, sepIdx));
        const auto blackScore = util::trim(text.substr(sepIdx + 1uz));

        if (whiteScore == "1")
            return game::Result::WhiteWon;

        if (blackScore == "1")
            return game::Result::BlackWon;

        return game::Result::Draw;
    }

} // namespace

GameRecord from_pgn(string_view pgnText)
{
    GameRecord game;

    pgnText = parse_metadata_tags(pgnText, game.metadata);

    if (const auto pos = game.metadata.find("FEN");
        pos != game.metadata.end()) {
        game.startingPosition = from_fen(pos->second);
    }

    const auto resultText = parse_move_list(
        pgnText, game.startingPosition, game.moves);

    game.result = parse_game_result(resultText, game);

    return game;
}

namespace {

    // returns the index in the string of the next line that either
    // starts with or doesn't start with a '[' character
    // returns npos if no such line is found
    template <bool SearchForBracket>
    [[nodiscard]] size_t find_next_line(
        const string_view text)
    {
        size_t lineStart { 0uz };

        while (lineStart < text.size()) {
            if constexpr (SearchForBracket) {
                if (text[lineStart] == '[')
                    return lineStart;
            } else {
                if (text[lineStart] != '[')
                    return lineStart;
            }

            const auto nextNewline = text.find('\n', lineStart + 1uz);

            if (nextNewline == string_view::npos)
                return string_view::npos;

            lineStart = nextNewline + 1uz;
        }

        return string_view::npos;
    }

} // namespace

std::vector<GameRecord> parse_all_pgns(string_view fileContent)
{
    std::vector<GameRecord> games;

    while (true) {
        fileContent = util::trim(fileContent);

        if (fileContent.empty())
            return games;

        // the move text of this PGN starts at the first line not starting in '['
        const auto moveTextStart = find_next_line<false>(fileContent);

        if (moveTextStart == string_view::npos)
            return games;

        // the next PGN after this one is the first line after moveTextStart that starts with a '['
        const auto moveTextToNextPGN = find_next_line<true>(fileContent.substr(moveTextStart));

        auto thisPGN = fileContent;

        if (moveTextToNextPGN == string_view::npos) {
            fileContent = {}; // so that we exit the loop
        } else {
            const auto nextPGNStart = moveTextStart + moveTextToNextPGN;

            thisPGN = fileContent.substr(0uz, nextPGNStart);

            fileContent.remove_prefix(nextPGNStart);
        }

        try {
            games.emplace_back(from_pgn(thisPGN));
        } catch (...) { // NOLINT
        }
    }
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

        for (const auto& tag : sevenTagRoster) {
            const auto pos = metadata.find(tag);

            if (pos != metadata.end())
                write_metadata_item(tag, pos->second, output);
        }

        for (const auto& [key, value] : metadata)
            if (! std::ranges::contains(sevenTagRoster, key))
                write_metadata_item(key, value, output);

        const Position startPos {};

        if (startingPosition != startPos) {
            if (! metadata.contains("FEN"s)) {
                const auto startFEN = to_fen(startingPosition);
                write_metadata_item("FEN", startFEN, output);
            }

            if (! metadata.contains("Setup"s))
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
            if (position.sideToMove == pieces::Color::White) {
                output.append(std::format("{}.{} ",
                    position.fullMoveCounter, to_alg(position, move.move)));
            } else {
                if (writeMoveNumber) {
                    output.append(std::format("{}...{} ",
                        position.fullMoveCounter, to_alg(position, move.move)));
                } else {
                    output.append(std::format("{} ", to_alg(position, move.move)));
                }
            }

            for (const auto nag : move.nags)
                output.append(std::format("${} ", nag));

            // set to false after the first move
            writeMoveNumber = false;

            if (! move.comment.empty()) {
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
        if (! result.has_value())
            return;

        switch (*result) {
            case game::Result::Draw:
                output.append("1/2-1/2");
                return;

            case game::Result::WhiteWon:
                output.append("1-0");
                return;

            default: // Black won
                output.append("0-1");
        }
    }

} // namespace

string to_pgn(const GameRecord& game, const bool useBlockComments)
{
    string result;

    write_metadata(game.metadata, game.startingPosition, result);

    result.append("\n");

    write_move_list(game.startingPosition, game.moves, useBlockComments, result);

    write_game_result(game.result, result);

    if (! result.empty() && result.back() == ' ')
        result.pop_back();

    return result;
}

} // namespace chess::notation
