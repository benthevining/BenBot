/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <cassert>
#include <format>
#include <libchess/notation/PGN.hpp>
#include <libchess/util/Strings.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace chess::notation {

namespace {

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
                throw std::invalid_argument {
                    "Invalid PGN: expected ']' following '['"
                };
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

} // namespace

GameRecord from_pgn(std::string_view pgnText)
{
    GameRecord game;

    pgnText = parse_metadata_tags(pgnText, game.metadata);

    // parse move list

    pgnText = util::trim(pgnText);

    return game;
}

std::string to_pgn(const GameRecord& game)
{
}

} // namespace chess::notation
