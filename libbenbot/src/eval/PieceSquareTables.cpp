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
#include <cassert>
#include <cmath> // IWYU pragma: keep - for std::round()
#include <functional>
#include <iterator>
#include <libbenbot/Resources.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libchess/board/Flips.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <nlohmann/json.hpp>
#include <numeric>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::eval {

namespace {
    using Table = PieceSquareTables::Table;
    using chess::board::Pieces;

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto sum_squares(
        chess::board::Bitboard board, const Table& table) noexcept
        -> int
    {
        if constexpr (IsBlack) {
            board = chess::board::flips::vertical(board);
        }

        const auto indices = board.indices();

        return std::transform_reduce(
            indices.begin(), indices.end(),
            0,
            std::plus<void> { },
            [&table](const auto idx) {
                assert(idx < table.size());
                return table.at(idx);
            });
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_pawns(
        const Pieces& pieces, const PieceSquareTables& pst) noexcept -> int
    {
        return sum_squares<IsBlack>(pieces.pawns, pst.pawn);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_knights(
        const Pieces& pieces, const PieceSquareTables& pst) noexcept -> int
    {
        return sum_squares<IsBlack>(pieces.knights, pst.knight);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_bishops(
        const Pieces& pieces, const PieceSquareTables& pst) noexcept -> int
    {
        return sum_squares<IsBlack>(pieces.bishops, pst.bishop);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_rooks(
        const Pieces& pieces, const PieceSquareTables& pst) noexcept -> int
    {
        return sum_squares<IsBlack>(pieces.rooks, pst.rook);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_queens(
        const Pieces& pieces, const PieceSquareTables& pst) noexcept -> int
    {
        return sum_squares<IsBlack>(pieces.queens, pst.queen);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] auto score_king(
        const Pieces& pieces, const float endgameWeight, const PieceSquareTables& pst) noexcept -> int
    {
        auto king = pieces.king;

        assert(king.count() == 1uz);

        if constexpr (IsBlack) {
            king = chess::board::flips::vertical(king);
        }

        const auto idx = king.first();

        const auto middlegameValue = pst.kingMiddlegame.at(idx);
        const auto endgameValue    = pst.kingEndgame.at(idx);

        return static_cast<int>(std::round(static_cast<float>(middlegameValue) * (1.f - endgameWeight)))
             + static_cast<int>(std::round(static_cast<float>(endgameValue) * endgameWeight));
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] auto score_side_pieces(
        const Pieces& pieces, const float endgameWeight, const PieceSquareTables& pst)
        -> int
    {
        return score_pawns<IsBlack>(pieces, pst)
             + score_knights<IsBlack>(pieces, pst)
             + score_bishops<IsBlack>(pieces, pst)
             + score_rooks<IsBlack>(pieces, pst)
             + score_queens<IsBlack>(pieces, pst)
             + score_king<IsBlack>(pieces, endgameWeight, pst);
    }

} // namespace

auto PieceSquareTables::score_piece_placement(
    const Position& position, const float endgameWeight) const
    -> int
{
    const auto [ourScore, theirScore] = [this, &position, endgameWeight] {
        if (position.is_black_to_move()) {
            return std::make_pair(
                score_side_pieces<true>(position.our_pieces(), endgameWeight, *this),
                score_side_pieces<false>(position.their_pieces(), endgameWeight, *this));
        }

        return std::make_pair(
            score_side_pieces<false>(position.our_pieces(), endgameWeight, *this),
            score_side_pieces<true>(position.their_pieces(), endgameWeight, *this));
    }();

    return ourScore - theirScore;
}

using nlohmann::json;
using std::string_view;

inline constexpr string_view TAG_PAWN { "pawn" };
inline constexpr string_view TAG_KNIGHT { "knight" };
inline constexpr string_view TAG_BISHOP { "bishop" };
inline constexpr string_view TAG_ROOK { "rook" };
inline constexpr string_view TAG_QUEEN { "queen" };
inline constexpr string_view TAG_KING { "king" };
inline constexpr string_view TAG_MIDDLEGAME { "middlegame" };
inline constexpr string_view TAG_ENDGAME { "endgame" };

namespace {
    [[nodiscard]] auto serialize_array(const Table& table) -> json
    {
        auto data = json::array();

        std::ranges::copy(
            table, std::back_inserter(data));

        return data;
    }
} // namespace

auto PieceSquareTables::to_string() const -> std::string
{
    json data;

    data[TAG_PAWN]   = serialize_array(pawn);
    data[TAG_KNIGHT] = serialize_array(knight);
    data[TAG_BISHOP] = serialize_array(bishop);
    data[TAG_ROOK]   = serialize_array(rook);
    data[TAG_QUEEN]  = serialize_array(queen);

    json kingData;

    kingData[TAG_MIDDLEGAME] = serialize_array(kingMiddlegame);
    kingData[TAG_ENDGAME]    = serialize_array(kingEndgame);

    data[TAG_KING] = std::move(kingData);

    return data.dump();
}

namespace {
    [[nodiscard]] auto read_array(const json& data) -> Table
    {
        Table table { };

        std::ranges::transform(
            data,
            table.data(),
            [](const json& value) {
                return value.get<Table::value_type>();
            });

        return table;
    }
} // namespace

auto PieceSquareTables::from_string(
    const string_view text) -> PieceSquareTables
{
    const auto parsed = json::parse(text);

    const auto& kingData = parsed.at(TAG_KING);

    return {
        .pawn           = read_array(parsed.at(TAG_PAWN)),
        .knight         = read_array(parsed.at(TAG_KNIGHT)),
        .bishop         = read_array(parsed.at(TAG_BISHOP)),
        .rook           = read_array(parsed.at(TAG_ROOK)),
        .queen          = read_array(parsed.at(TAG_QUEEN)),
        .kingMiddlegame = read_array(kingData.at(TAG_MIDDLEGAME)),
        .kingEndgame    = read_array(kingData.at(TAG_ENDGAME)),
    };
}

auto PieceSquareTables::get_default() -> PieceSquareTables
{
    return from_string(
        resources::get_default_piece_square_tables());
}

} // namespace ben_bot::eval
