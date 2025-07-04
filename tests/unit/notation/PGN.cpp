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

// Note: metadata tags are not guaranteed to be written in the same order they were parsed
// the "seven tag roster" will always be in the same order, but other tags may appear in
// any order in the output. For this reason, we can only do round tripping tests on PGNs
// that use only the seven tag roster.

#include <catch2/catch_test_macros.hpp>
#include <libchess/game/Result.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/PGN.hpp>
#include <string>
#include <string_view>

static constexpr auto TAGS { "[notation][PGN]" };

using chess::notation::from_pgn;
using chess::notation::parse_all_pgns;
using chess::notation::to_pgn;

TEST_CASE("PGN - block comments", TAGS)
{
    static const std::string pgn {
        R"([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1/2-1/2"]

1.e4 e5 2.Nf3 Nc6 3.Bb5 {This opening is called the Ruy Lopez.} 3...a6 4.Ba4 Nf6 5.O-O Be7 6.Re1 b5 7.Bb3 d6 8.c3 O-O 9.h3 Nb8 10.d4 Nbd7 11.c4 c6 12.cxb5 axb5 13.Nc3 Bb7 14.Bg5 b4 15.Nb1 h6 16.Bh4 c5 17.dxe5 Nxe4 18.Bxe7 Qxe7 19.exd6 Qf6 20.Nbd2 Nxd6 21.Nc4 Nxc4 22.Bxc4 Nb6 23.Ne5 Rae8 24.Bxf7+ Rxf7 25.Nxf7 Rxe1+ 26.Qxe1 Kxf7 27.Qe3 Qg5 28.Qxg5 hxg5 29.b3 Ke6 30.a3 Kd6 31.axb4 cxb4 32.Ra5 Nd5 33.f3 Bc8 34.Kf2 Bf5 35.Ra7 g6 36.Ra6+ Kc5 37.Ke1 Nf4 38.g3 Nxh3 39.Kd2 Kb5 40.Rd6 Kc5 41.Ra6 Nf2 42.g4 Bd3 43.Re6 1/2-1/2)"
    };

    const auto game = from_pgn(pgn);

    REQUIRE(game.metadata.size() == 7uz);

    REQUIRE(game.metadata.at("Event") == "F/S Return Match");
    REQUIRE(game.metadata.at("Site") == "Belgrade, Serbia JUG");
    REQUIRE(game.metadata.at("Date") == "1992.11.04");
    REQUIRE(game.metadata.at("Round") == "29");
    REQUIRE(game.metadata.at("White") == "Fischer, Robert J.");
    REQUIRE(game.metadata.at("Black") == "Spassky, Boris V.");
    REQUIRE(game.metadata.at("Result") == "1/2-1/2");

    REQUIRE(game.result.has_value());
    REQUIRE(*game.result == chess::game::Result::Draw);

    REQUIRE(game.moves.at(4uz).comment == "This opening is called the Ruy Lopez.");

    REQUIRE(to_pgn(game) == pgn);
}

TEST_CASE("PGN - tolerate spaces between move number and move", TAGS)
{
    static const std::string pgn {
        R"([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1/2-1/2"]

1. e4 e5 2. Nf3 Nc6 3. Bb5 {This opening is called the Ruy Lopez.} 3. ... a6 4. Ba4 Nf6 5. O-O Be7 6. Re1 b5 7. Bb3 d6 8. c3 O-O 9. h3 Nb8 10. d4 Nbd7)"
    };

    const auto game = from_pgn(pgn);

    REQUIRE(game.moves.size() == 20uz);

    REQUIRE(game.moves.at(4uz).comment == "This opening is called the Ruy Lopez.");
}

TEST_CASE("PGN - multiline block comments", TAGS)
{
    static const std::string pgn {
        R"([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1/2-1/2"]

1.e4 e5 2.Nf3 Nc6 3.Bb5 {This opening is
called the Ruy Lopez.} 3...a6 4.Ba4 Nf6 5.O-O Be7 6.Re1 b5 7.Bb3 d6 8.c3 O-O 9.h3 Nb8 10.d4 Nbd7 11.c4 c6 12.cxb5 axb5 13.Nc3 Bb7 14.Bg5 b4 15.Nb1 h6 16.Bh4 c5 17.dxe5 Nxe4 18.Bxe7 Qxe7 19.exd6 Qf6 20.Nbd2 Nxd6 21.Nc4 Nxc4 22.Bxc4 Nb6 23.Ne5 Rae8 24.Bxf7+ Rxf7 25.Nxf7 Rxe1+ 26.Qxe1 Kxf7 27.Qe3 Qg5 28.Qxg5 hxg5 29.b3 Ke6 30.a3 Kd6 31.axb4 cxb4 32.Ra5 Nd5 33.f3 Bc8 34.Kf2 Bf5 35.Ra7 g6 36.Ra6+ Kc5 37.Ke1 Nf4 38.g3 Nxh3 39.Kd2 Kb5 40.Rd6 Kc5 41.Ra6 Nf2 42.g4 Bd3 43.Re6 1/2-1/2)"
    };

    const auto game = from_pgn(pgn);

    static const std::string comment {
        R"(This opening is
called the Ruy Lopez.)"
    };

    REQUIRE(game.moves.at(4uz).comment == comment);

    REQUIRE(to_pgn(game) == pgn);
}

TEST_CASE("PGN - line comments", TAGS)
{
    static const std::string pgn {
        R"([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1/2-1/2"]

1.e4 e5 2.Nf3 Nc6 3.Bb5 ; This opening is called the Ruy Lopez.
3...a6 4.Ba4 Nf6 5.O-O Be7 6.Re1 b5 7.Bb3 d6 8.c3 O-O 9.h3 Nb8 10.d4 Nbd7 11.c4 c6 12.cxb5 axb5 13.Nc3 Bb7 14.Bg5 b4 15.Nb1 h6 16.Bh4 c5 17.dxe5 Nxe4 18.Bxe7 Qxe7 19.exd6 Qf6 20.Nbd2 Nxd6 21.Nc4 Nxc4 22.Bxc4 Nb6 23.Ne5 Rae8 24.Bxf7+ Rxf7 25.Nxf7 Rxe1+ 26.Qxe1 Kxf7 27.Qe3 Qg5 28.Qxg5 hxg5 29.b3 Ke6 30.a3 Kd6 31.axb4 cxb4 32.Ra5 Nd5 33.f3 Bc8 34.Kf2 Bf5 35.Ra7 g6 36.Ra6+ Kc5 37.Ke1 Nf4 38.g3 Nxh3 39.Kd2 Kb5 40.Rd6 Kc5 41.Ra6 Nf2 42.g4 Bd3 43.Re6 1/2-1/2)"
    };

    const auto game = from_pgn(pgn);

    REQUIRE(game.metadata.size() == 7uz);

    REQUIRE(game.metadata.at("Event") == "F/S Return Match");
    REQUIRE(game.metadata.at("Site") == "Belgrade, Serbia JUG");
    REQUIRE(game.metadata.at("Date") == "1992.11.04");
    REQUIRE(game.metadata.at("Round") == "29");
    REQUIRE(game.metadata.at("White") == "Fischer, Robert J.");
    REQUIRE(game.metadata.at("Black") == "Spassky, Boris V.");
    REQUIRE(game.metadata.at("Result") == "1/2-1/2");

    REQUIRE(game.result.has_value());
    REQUIRE(*game.result == chess::game::Result::Draw);

    REQUIRE(game.moves.at(4uz).comment == "This opening is called the Ruy Lopez.");

    REQUIRE(to_pgn(game, false) == pgn);
}

TEST_CASE("PGN - NAGs", TAGS)
{
    static const std::string pgn {
        R"([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "*"]

1.e4 $1 $14 e5 $4)"
    };

    const auto game = from_pgn(pgn);

    REQUIRE(game.moves.size() == 2uz);

    {
        const auto& firstMove = game.moves.front();

        REQUIRE(firstMove.nags.size() == 2uz);

        REQUIRE(firstMove.nags.front() == 1u);
        REQUIRE(firstMove.nags.back() == 14u);
    }
    {
        const auto& secondMove = game.moves.back();

        REQUIRE(secondMove.nags.size() == 1uz);

        REQUIRE(secondMove.nags.front() == 4u);
    }

    REQUIRE(to_pgn(game) == pgn);
}

TEST_CASE("PGN - NAG inside a comment", TAGS)
{
    static const std::string pgn {
        R"([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "*"]

1.e4 $1 {$14} 1...e5 $4)"
    };

    const auto game = from_pgn(pgn);

    REQUIRE(game.moves.size() == 2uz);

    {
        const auto& firstMove = game.moves.front();

        REQUIRE(firstMove.nags.size() == 1uz);

        REQUIRE(firstMove.nags.front() == 1u);

        REQUIRE(firstMove.comment == "$14");
    }
    {
        const auto& secondMove = game.moves.back();

        REQUIRE(secondMove.nags.size() == 1uz);

        REQUIRE(secondMove.nags.front() == 4u);
    }

    REQUIRE(to_pgn(game) == pgn);
}

TEST_CASE("PGN - custom starting position", TAGS)
{
    static const std::string pgn {
        R"(
[FEN "5r2/4k3/8/3R2n1/2K5/8/8/8 b - - 0 1"]
[Setup "1"]

1...Ne6 2.Re5)"
    };

    const auto game = from_pgn(pgn);

    REQUIRE(game.moves.size() == 2uz);

    REQUIRE(game.startingPosition == chess::notation::from_fen("5r2/4k3/8/3R2n1/2K5/8/8/8 b - - 0 1"));
}

TEST_CASE("PGN - variations", TAGS)
{
    static const std::string pgn {
        R"([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1/2-1/2"]

1.e4 e5 2.Nf3 Nc6 3.Bb5 (3.d4 exd4) 3...a6)"
    };

    const auto game = from_pgn(pgn);

    REQUIRE(game.moves.size() == 6uz);

    const auto& variations = game.moves.at(4uz).variations;

    REQUIRE(variations.size() == 1uz);

    const auto& variation = variations.front();

    REQUIRE(variation.size() == 2uz);

    REQUIRE(to_pgn(game) == pgn);
}

TEST_CASE("PGN - nested variations", TAGS)
{
    static const std::string pgn {
        R"([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1/2-1/2"]

1.e4 e5 2.Nf3 Nc6 3.Bb5 (3.d4 exd4 4.Nxd4 (4.Bf4) 4...Nxd4) 3...a6)"
    };

    const auto game = from_pgn(pgn);

    REQUIRE(game.moves.size() == 6uz);

    const auto& variations = game.moves.at(4uz).variations;

    REQUIRE(variations.size() == 1uz);

    const auto& variation = variations.front();

    REQUIRE(variation.size() == 4uz);

    const auto& subvariations = variation.at(2uz).variations;

    REQUIRE(subvariations.size() == 1uz);

    REQUIRE(subvariations.front().size() == 1uz);

    REQUIRE(to_pgn(game) == pgn);
}

TEST_CASE("PGN - complex file", TAGS)
{
    static const std::string pgn {
        R"([Event "Buenos Aires Sicilian"]
[Site "Buenos Aires ARG"]
[Date "1994.10.??"]
[Round "8"]
[White "Alexey Shirov"]
[Black "Judit Polgar"]
[Result "0-1"]

1.e4 c5 2.Nf3 e6 3.d4 cxd4 4.Nxd4 Nc6 5.Nc3 d6 6.g4 a6 7.Be3 Nge7 $5 $146 {An unstandard move for the Sicilian. This prepares the knight to move up the board to f5 later in the game.} 8.Nb3 b5 9.f4 Bb7 10.Qf3 $4 g5 $3 {Polgar sees that Shirov has lined up his queen on the a8-h1 diagonal, with a discovered pin of Shirov's e4 pawn by the bishop once the knight on c6 moves, so she decides to pounce right away. Polgar wants to force the f4 pawn to move so she can play Ne5.} 11.fxg5 (11.f5 Ne5 {If Shirov had advanced his f pawn instead of taking on g5, Polgar could have played Ne5 anyway.}) (11.O-O-O {Shirov probably should have simply ignored the pawn motion on the queenside and castled his king to safety, forcing Polgar to take on f4 if she wants to play Ne5. Shirov can prevent this move by recapturing e5 with his queen.} 11...gxf4 12.Qxf4) 11...Ne5 12.Qg2 $2 {Shirov spends a tempo moving his queen, but still keeps it on the dangerous a8-h1 diagonal, keeping his a4 pawn pinned.} (12.Qe2 b4 13.Na4 Bxe4) 12...b4 {Kicking away the knight that is defending the pinned e4 pawn.} 13.Ne2 h5 $3 {Polgar wants to play Nf5, so she wants to force the g4 pawn to move.} 14.gxh5 (14.gxh6 f5 $3 {If Shirov had captured the h5 pawn with en passant, Polgar still could have forced the g4 pawn to move by sacrificing her last remaining kingside pawn.} 15.gxf5 (15.g5 Bxe4) 15...Nxf5) 14...Nf5 15.Bf2 (15.exf5 $3 {If Shirov had sacrificed his queen, he would be left with a somewhat even game, losing his queen for a knight and a bishop.} 15...Bxg2 16.Bxg2 Rc8) 15...Qxg5 $3 16.Na5 (16.Qxg5 Nf3+ 17.Kd1 Nxg5) 16...Ne3 $1 {Now that the knight has vacated the F5 square, Polgar is happy to leave her light-squared bishop hanging and give up the pin of the e4 pawn in order to keep her attack towards the king going.} 17.Qg3 Qxg3 $6 {Polgar probably already wanted to begin simplifying towards a winning endgame.} (17...Nxc2+ 18.Kd1 Qxh5 $3 19.Kxc2 (19.Nxb7 Nxa1) 19...Bxe4+ {Polgar can win the E4 pawn and move her bishop out of danger in exchange for a knight.} (19...Rc8+)) 18.Nxg3 Nxc2+ 19.Kd1 Nxa1 $18 20.Nxb7 b3 $1 {Polgar is sacrificing this pawn to save the knight.} 21.axb3 (21.a3 Nc2) 21...Nxb3 22.Kc2 Nc5 23.Nxc5 dxc5 24.Be1 Nf3 25.Bc3 Nd4+ {Polgar offers to trade her knight for Shirov's dark-squared bishop.} 26.Kd3 Bd6 27.Bg2 Be5 28.Kc4 Ke7 29.Ra1 (29.Kxc5 Rhc8+ 30.Kb4 Rab8+ 31.Ka3 Nc2+ (31...Nb5+ 32.Ka2 Nxc3+ 33.bxc3 Rxc3 $20) 32.Ka2 Bxc3 33.bxc3 Rxc3 $20) 29...Nc6 0-1)"
    };

    const auto game = from_pgn(pgn);

    REQUIRE(to_pgn(game) == pgn);
}

TEST_CASE("PGN - parse_all_pgns() - single PGN", TAGS)
{
    static constexpr std::string_view fileText {
        R"(
[Event "?"]
[Site "?"]
[Date "2013.11.02"]
[Round "1"]
[White "Stockfish"]
[Black "Stockfish"]
[Result "1/2-1/2"]
[Eco "A07"]

1. Nf3 d5 2. g3 c6 3. Bg2 Nf6 4. d3 Bg4 5. h3 Bh5 6. b3 e6 7. Bb2 Qa5+ 8.
Qd2 Qxd2+ 1/2-1/2
)"
    };

    const auto games = parse_all_pgns(fileText);

    REQUIRE(games.size() == 1uz);
}

TEST_CASE("PGN - parse_all_pgns()", TAGS)
{
    static constexpr std::string_view fileText {
        R"(

[Event "?"]
[Site "?"]
[Date "2013.11.02"]
[Round "1"]
[White "Stockfish"]
[Black "Stockfish"]
[Result "1/2-1/2"]
[Eco "A07"]

1. Nf3 d5 2. g3 c6 3. Bg2 Nf6 4. d3 Bg4 5. h3 Bh5 6. b3 e6 7. Bb2 Qa5+ 8.
Qd2 Qxd2+ 1/2-1/2

[Event "?"]
[Site "?"]
[Date "2013.11.02"]
[Round "1"]
[White "Stockfish"]
[Black "Stockfish"]
[Result "1/2-1/2"]
[Eco "E15"]

1. d4 Nf6 2. c4 e6 3. Nf3 b6 4. g3 Ba6 5. Qa4 Bb7 6. Bg2 c5 7. dxc5 Bxc5 8.
O-O Be7 1/2-1/2
[Event "?"]
[Site "?"]
[Date "2013.11.02"]
[Round "1"]
[White "Stockfish"]
[Black "Stockfish"]
[Result "1/2-1/2"]
[Eco "A41"]

1. d4 d6 2. c4 e5 3. d5 f5 4. e4 fxe4 5. Nc3 Nf6 6. Nge2 Bf5 7. Ng3 Bg6 8.
Bg5 Nbd7 1/2-1/2

)"
    };

    const auto games = parse_all_pgns(fileText);

    REQUIRE(games.size() == 3uz);
}
