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
#include <catch2/catch_test_macros.hpp>
#include <libbenbot/data-structures/OpeningBook.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <string_view>

static constexpr auto TAGS { "[data-structures][OpeningBook]" };

using ben_bot::OpeningBook;
using chess::game::Position;

namespace notation = chess::notation;

static constexpr std::string_view TEST_BOOK_PGN {
    R"-([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1/2-1/2"]

1. e4 e5 2. Nf3 Nc6 3. Nc3 (3. c3))-"
};

TEST_CASE("Opening book - position not found", TAGS)
{
    static const Position startPos {};

    OpeningBook book;

    REQUIRE(book.get_moves(startPos).empty());

    book.add_from_pgn(TEST_BOOK_PGN);

    REQUIRE(book.get_moves(
                    after_move(startPos,
                        notation::from_alg(startPos, "d4")))
            .empty());

    REQUIRE(book.get_moves(
                    after_move(startPos,
                        notation::from_alg(startPos, "e3")))
            .empty());
}

TEST_CASE("Opening book - finding moves", TAGS)
{
    OpeningBook book;

    book.add_from_pgn(TEST_BOOK_PGN);

    Position startPos;

    const auto firstMove = book.get_moves(startPos);

    REQUIRE(firstMove.size() == 1uz);

    REQUIRE(firstMove.front() == notation::from_alg(startPos, "e4"));

    startPos.make_move(firstMove.front());

    const auto firstResponse = book.get_moves(startPos);

    REQUIRE(firstResponse.size() == 1uz);

    REQUIRE(firstResponse.front() == notation::from_alg(startPos, "e5"));

    startPos.make_move(firstResponse.front());

    const auto secondMove = book.get_moves(startPos);

    REQUIRE(secondMove.size() == 1uz);

    REQUIRE(secondMove.front() == notation::from_alg(startPos, "Nf3"));

    startPos.make_move(secondMove.front());

    const auto secondResponse = book.get_moves(startPos);

    REQUIRE(secondResponse.size() == 1uz);

    REQUIRE(secondResponse.front() == notation::from_alg(startPos, "Nc6"));

    startPos.make_move(secondResponse.front());

    const auto thirdMove = book.get_moves(startPos);

    REQUIRE(thirdMove.size() == 2uz);

    REQUIRE(std::ranges::contains(thirdMove,
        notation::from_alg(startPos, "Nc3")));

    REQUIRE(std::ranges::contains(thirdMove,
        notation::from_alg(startPos, "c3")));
}
