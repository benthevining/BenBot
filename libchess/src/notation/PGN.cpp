/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <format>
#include <iterator>
#include <libchess/game/Result.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/PGN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/util/Strings.hpp>
#include <optional>
#include <span>
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

    using Metadata   = std::unordered_map<std::string, std::string>;
    using GameResult = std::optional<game::Result>;

    [[nodiscard]] std::pair<std::string_view, std::string_view>
    split_at_first_space_or_newline(
        const std::string_view input)
    {
        const auto spaceIdx   = input.find(' ');
        const auto newLineIdx = input.find('\n');

        const auto firstDelimIdx = std::min(spaceIdx, newLineIdx);

        if (firstDelimIdx == std::string_view::npos) {
            return { input, {} };
        }

        return {
            input.substr(0uz, firstDelimIdx),
            input.substr(firstDelimIdx + 1uz)
        };
    }

    // writes tag key/value pairs into metadata and returns
    // the rest of the PGN text that's left
    [[nodiscard]] std::string_view parse_metadata_tags(
        std::string_view pgnText,
        Metadata&        metadata)
    {
        auto openingBracketIdx = pgnText.find('[');

        while (openingBracketIdx != std::string_view::npos) {
            const auto closingBracketIdx = pgnText.find(']', openingBracketIdx + 1uz);

            if (closingBracketIdx == std::string_view::npos) {
                throw std::invalid_argument { "Invalid PGN: expected ']' following '['" };
            }

            assert(closingBracketIdx > openingBracketIdx);

            // don't include the brackets
            const auto tagText = pgnText.substr(
                openingBracketIdx + 1uz,
                closingBracketIdx - openingBracketIdx - 1uz);

            // we assume that tag keys cannot include spaces
            const auto spaceIdx = tagText.find(' ');

            if (spaceIdx == std::string_view::npos) {
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

    [[nodiscard]] GameResult parse_game_result(
        const std::string_view text, const Position& finalPosition)
    {
        const auto sepIdx = text.find('-');

        if (sepIdx == std::string_view::npos)
            return finalPosition.get_result();

        const auto whiteScore = util::trim(text.substr(0uz, sepIdx));
        const auto blackScore = util::trim(text.substr(sepIdx + 1uz));

        if (whiteScore == "1")
            return game::Result::WhiteWon;

        if (blackScore == "1")
            return game::Result::BlackWon;

        return game::Result::Draw;
    }

    using Moves = std::vector<GameRecord::Move>;

    // writes the content of the block comment to the last move in output
    // and returns the rest of the pgnText after the } that closes this comment
    [[nodiscard]] std::string_view parse_block_comment(
        const std::string_view pgnText, Moves& output)
    {
        // first char in pgnText is {

        const auto closeBracketIdx = pgnText.find('}');

        if (closeBracketIdx == std::string_view::npos) {
            throw std::invalid_argument { "Expected '}' following '{'" };
        }

        if (! output.empty())
            output.back().comment = pgnText.substr(1uz, closeBracketIdx - 1uz);

        return pgnText.substr(closeBracketIdx + 1uz);
    }

    // writes the content of the line comment to the last move in output
    // and returns the rest of the pgnText after the newline that ends this comment
    [[nodiscard]] std::string_view parse_line_comment(
        const std::string_view pgnText, Moves& output)
    {
        // first char in pgnText is ;

        const auto newlineIdx = pgnText.find('\n');

        if (newlineIdx == std::string_view::npos) {
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
    [[nodiscard]] std::string_view parse_nag(
        const std::string_view pgnText, Moves& output)
    {
        // first char in pgnText is $

        auto [nag, rest] = split_at_first_space_or_newline(pgnText.substr(1uz));

        if (! output.empty()) {
            nag = util::trim(nag);

            std::uint_least8_t value { 0 };

            std::from_chars(nag.data(), nag.data() + nag.length(), value);

            output.back().nag = value;
        }

        return rest;
    }

    // writes the parsed moves into output and returns the parsed game result
    [[nodiscard]] GameResult parse_move_list(
        std::string_view pgnText,
        Position         position,
        Moves&           output)
    {
        while (! pgnText.empty()) {
            pgnText = util::trim(pgnText);

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

                default: {
                    auto [firstMove, rest] = split_at_first_space_or_newline(pgnText);

                    if (util::trim(rest).empty()) {
                        // the PGN string is exhausted, so this last substring is the game result, not a move
                        return parse_game_result(firstMove, position);
                    }

                    // move numbers may start with 3. or 3...
                    const auto lastDotIdx = firstMove.rfind('.');

                    if (lastDotIdx != std::string_view::npos)
                        firstMove = firstMove.substr(lastDotIdx + 1uz);

                    const auto move = from_alg(position, firstMove);

                    position.make_move(move);

                    output.emplace_back(move);

                    pgnText = rest;
                }
            }
        }

        return parse_game_result({}, position);
    }

} // namespace

GameRecord from_pgn(std::string_view pgnText)
{
    GameRecord game;

    pgnText = parse_metadata_tags(pgnText, game.metadata);

    if (const auto pos = game.metadata.find("FEN");
        pos != game.metadata.end()) {
        game.startingPosition = from_fen(pos->second);
    }

    game.result = parse_move_list(
        pgnText, game.startingPosition, game.moves);

    return game;
}

namespace {

    void write_metadata_item(
        const std::string_view key, const std::string_view value,
        std::string& output)
    {
        output.append(std::format(
            R"([{} "{}"]
)",
            key, value));
    }

    void write_metadata(
        const Metadata& metadata,
        const Position& startingPosition,
        std::string&    output)
    {
        using namespace std::literals::string_literals; // NOLINT

        // if these tags are present, they must appear before any other tags and in this order
        static constexpr std::array sevenTagRoster {
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

        static constexpr Position startPos {};

        if (! metadata.contains("FEN"s) && startingPosition != startPos) {
            const auto startFEN = to_fen(startingPosition);
            write_metadata_item("FEN", startFEN, output);
        }
    }

    void write_move_list(
        Position                                position,
        const std::span<const GameRecord::Move> moves,
        const bool                              useBlockComments,
        std::string&                            output)
    {
        // true if we need to insert a move number before Black's next move
        // true for the fist move of the game and the first move after a comment
        bool firstMove { true };

        for (const auto& move : moves) {
            if (position.sideToMove == pieces::Color::White) {
                output.append(std::format("{}.{} ",
                    position.fullMoveCounter, to_alg(position, move.move)));
            } else {
                if (firstMove) {
                    output.append(std::format("{}...{} ",
                        position.fullMoveCounter, to_alg(position, move.move)));
                } else {
                    output.append(std::format("{} ", to_alg(position, move.move)));
                }
            }

            if (move.nag.has_value())
                output.append(std::format("${} ", *move.nag));

            position.make_move(move.move);
            firstMove = false;

            if (! move.comment.empty()) {
                if (useBlockComments)
                    output.append(std::format("{{{}}} ", move.comment));
                else
                    output.append(std::format("; {}\n", move.comment));

                firstMove = true;
            }
        }
    }

    void write_game_result(
        const GameResult result, std::string& output)
    {
        if (! result.has_value()) {
            output.append("*");
            return;
        }

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

std::string to_pgn(const GameRecord& game, const bool useBlockComments)
{
    std::string result;

    write_metadata(game.metadata, game.startingPosition, result);

    result.append("\n");

    write_move_list(game.startingPosition, game.moves, useBlockComments, result);

    write_game_result(game.result, result);

    return result;
}

} // namespace chess::notation
