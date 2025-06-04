/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <cassert>
#include <format>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/game/Result.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/PGN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/util/Strings.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace chess::notation {

namespace {

    using game::Position;

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
        std::string_view                              pgnText,
        std::unordered_map<std::string, std::string>& metadata)
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

    // writes the parsed moves into outputIt and returns
    // the parsed game result
    [[nodiscard]] GameResult parse_move_list(
        std::string_view                pgnText,
        std::output_iterator<Move> auto outputIt)
    {
        Position position {};

        while (! pgnText.empty()) {
            pgnText = util::trim(pgnText);

            // comment: { continues to }
            if (pgnText.front() == '{') {
                const auto closeBracketIdx = pgnText.find('}');

                if (closeBracketIdx == std::string_view::npos) {
                    throw std::invalid_argument {
                        "Expected '}' following '{'"
                    };
                }

                pgnText.remove_prefix(closeBracketIdx + 1uz);

                continue;
            }

            // comment: ; continues to end of line
            if (pgnText.front() == ';') {
                const auto newlineIdx = pgnText.find('\n');

                if (newlineIdx == std::string_view::npos) {
                    // assume that a ; comment was the last thing in the file
                    return parse_game_result({}, position);
                }

                pgnText.remove_prefix(newlineIdx + 1uz);

                continue;
            }

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
            *outputIt = move;

            pgnText = rest;
        }

        return parse_game_result({}, position);
    }

} // namespace

GameRecord from_pgn(std::string_view pgnText)
{
    GameRecord game;

    pgnText = parse_metadata_tags(pgnText, game.metadata);

    game.result = parse_move_list(pgnText, std::back_inserter(game.moves));

    return game;
}

std::string to_pgn(const GameRecord& game)
{
    std::string result;

    for (const auto& [key, value] : game.metadata) {
        result.append(std::format(
            R"([{} "{}"]\n)", key, value));
    }

    result.append("\n");

    Position position {};

    for (const auto& move : game.moves) {
        if (position.sideToMove == pieces::Color::White)
            result.append(std::format("{}.{} ", position.fullMoveCounter, to_alg(position, move)));
        else
            result.append(std::format("{} ", to_alg(position, move)));

        position.make_move(move);
    }

    if (game.result.has_value()) {
        switch (*game.result) {
            case game::Result::Draw:
                result.append("1/2-1/2");
                break;

            case game::Result::WhiteWon:
                result.append("1-0");
                break;

            default: // Black won
                result.append("0-1");
        }
    } else {
        result.append("*");
    }

    return result;
}

} // namespace chess::notation
