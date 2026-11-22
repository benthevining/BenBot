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

/** @file
    This file defines the eval weights and the registry of eval terms.
    @ingroup eval
 */

#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <libbenbot/eval/TaperedScore.hpp>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::eval {

using std::size_t;
using PieceType = chess::pieces::Type;

/** Identifies a group of eval weights.
    The order of this enum must match the order of ``TERMS``.

    @ingroup eval
 */
enum class Term : std::uint8_t {
    Material,                ///< Pawn, knight, bishop, rook & queen values.
    Pst,                     ///< Piece square tables: [piece type][square], from White's point of view.
    PassedPawn,              ///< Passed pawn bonus, indexed by squares from promoting minus 1.
    PasserProtectedByPasser, ///< Bonus for each passed pawn defending a passed pawn.
    PasserProtectedByPawn,   ///< Bonus for each non-passed pawn defending a passed pawn.
    IsolatedPawn,            ///< Bonus (penalty) for each isolated pawn.
    DoubledPawn,             ///< Bonus (penalty) for each file with more than one of our pawns.
    BackwardPawn,            ///< Bonus (penalty) for each backward pawn.
    PawnChain,               ///< Bonus for each pawn defended by a friendly pawn (counted per defender).
    RookBehindPasser,        ///< Bonus for our rook behind our passer (penalty for their rook behind it).
    KingEscortsPasser,       ///< Bonus per square that our king is closer than 7 to our passer.
    EnemyKingBlocksPasser,   ///< Bonus (penalty) for the enemy king on our passer's promotion square.
    RookHalfOpenFile,        ///< Bonus for each rook on a half-open file.
    RookOpenFile,            ///< Bonus for each rook on an open file.
    ConnectedRooksOpenFile,  ///< Bonus for each open file with two of our rooks on it.
    KingHalfOpenFile,        ///< Bonus (penalty) for the king on a half-open file.
    KingOpenLine,            ///< Bonus (penalty) for the king on an open file or a pawnless diagonal.
    KingStranded,            ///< Bonus (penalty) for the king on the d/e file with no castling rights.
    KnightTropism,           ///< Bonus (penalty) per square that an enemy knight is closer than 6 to the king.
    QueenTropism,            ///< Bonus (penalty) per square that an enemy queen is closer than 7 to the king.
    KnightMobility,          ///< Knight mobility, indexed by the number of safe squares.
    BishopMobility,          ///< Bishop mobility, indexed by the number of safe squares.
    RookMobility,            ///< Rook mobility, indexed by the number of safe squares.
    QueenMobility,           ///< Queen mobility, indexed by the number of safe squares.
    KingAreaControl,         ///< Bonus per square around each king that we control more of than they do.
    CenterControl,           ///< Bonus per center square controlled.
    Space,                   ///< Bonus per square behind our pawns that we control more of than they do.
    PawnThreat,              ///< Bonus for each enemy piece attacked by one of our pawns.
    MinorOnRook,             ///< Bonus for each enemy rook attacked by one of our knights or bishops.
    MinorOnQueen,            ///< Bonus for each enemy queen attacked by one of our knights or bishops.
    RookOnQueen,             ///< Bonus for each enemy queen attacked by one of our rooks.
    Hanging,                 ///< Bonus for each hanging enemy piece or pawn.
    LastPiece,               ///< Bonus for having a non-pawn piece left when the opponent has none.
    MopUpEdge,               ///< Mop-up bonus per square that the enemy king is from the center.
    MopUpKingDistance        ///< Mop-up bonus per square that the kings are closer than 14.
};

/** How a term's weights enter the eval.
    @ingroup eval
 */
enum class Method : std::uint8_t {
    Linear ///< The eval is the sum of coefficient times weight.
};

/** Describes one group of eval weights.
    @ingroup eval
 */
struct TermInfo final {
    /** The term's name. */
    std::string_view name;

    /** The term. */
    Term term { };

    /** The number of weights that this term owns. */
    size_t count { 1uz };

    /** False for terms that a tuner shouldn't change (such as the
        endgame conversion helpers).
     */
    bool tunable { true };

    /** How the term's weights enter the eval. */
    Method method { Method::Linear };
};

/** The registry of all eval terms, in the same order as the ``Term`` enum.
    The weights are stored in this order in one flat array.

    @ingroup eval
 */
inline constexpr std::array TERMS {
    TermInfo { .name = "Material", .term = Term::Material, .count = 5uz },
    TermInfo { .name = "Pst", .term = Term::Pst, .count = 6uz * chess::board::NUM_SQUARES },
    TermInfo { .name = "PassedPawn", .term = Term::PassedPawn, .count = 6uz },
    TermInfo { .name = "PasserProtectedByPasser", .term = Term::PasserProtectedByPasser },
    TermInfo { .name = "PasserProtectedByPawn", .term = Term::PasserProtectedByPawn },
    TermInfo { .name = "IsolatedPawn", .term = Term::IsolatedPawn },
    TermInfo { .name = "DoubledPawn", .term = Term::DoubledPawn },
    TermInfo { .name = "BackwardPawn", .term = Term::BackwardPawn },
    TermInfo { .name = "PawnChain", .term = Term::PawnChain },
    TermInfo { .name = "RookBehindPasser", .term = Term::RookBehindPasser },
    TermInfo { .name = "KingEscortsPasser", .term = Term::KingEscortsPasser },
    TermInfo { .name = "EnemyKingBlocksPasser", .term = Term::EnemyKingBlocksPasser },
    TermInfo { .name = "RookHalfOpenFile", .term = Term::RookHalfOpenFile },
    TermInfo { .name = "RookOpenFile", .term = Term::RookOpenFile },
    TermInfo { .name = "ConnectedRooksOpenFile", .term = Term::ConnectedRooksOpenFile },
    TermInfo { .name = "KingHalfOpenFile", .term = Term::KingHalfOpenFile },
    TermInfo { .name = "KingOpenLine", .term = Term::KingOpenLine },
    TermInfo { .name = "KingStranded", .term = Term::KingStranded },
    TermInfo { .name = "KnightTropism", .term = Term::KnightTropism },
    TermInfo { .name = "QueenTropism", .term = Term::QueenTropism },
    TermInfo { .name = "KnightMobility", .term = Term::KnightMobility, .count = 9uz },
    TermInfo { .name = "BishopMobility", .term = Term::BishopMobility, .count = 14uz },
    TermInfo { .name = "RookMobility", .term = Term::RookMobility, .count = 15uz },
    TermInfo { .name = "QueenMobility", .term = Term::QueenMobility, .count = 28uz },
    TermInfo { .name = "KingAreaControl", .term = Term::KingAreaControl },
    TermInfo { .name = "CenterControl", .term = Term::CenterControl },
    TermInfo { .name = "Space", .term = Term::Space },
    TermInfo { .name = "PawnThreat", .term = Term::PawnThreat },
    TermInfo { .name = "MinorOnRook", .term = Term::MinorOnRook },
    TermInfo { .name = "MinorOnQueen", .term = Term::MinorOnQueen },
    TermInfo { .name = "RookOnQueen", .term = Term::RookOnQueen },
    TermInfo { .name = "Hanging", .term = Term::Hanging },
    TermInfo { .name = "LastPiece", .term = Term::LastPiece, .tunable = false },
    TermInfo { .name = "MopUpEdge", .term = Term::MopUpEdge, .tunable = false },
    TermInfo { .name = "MopUpKingDistance", .term = Term::MopUpKingDistance, .tunable = false }
};

/** The number of eval terms.
    @ingroup eval
 */
inline constexpr auto NUM_TERMS = TERMS.size();

namespace detail {

    [[nodiscard]] consteval auto compute_term_offsets() -> std::array<size_t, NUM_TERMS + 1uz>
    {
        std::array<size_t, NUM_TERMS + 1uz> offsets { };

        for (auto i = 0uz; i < NUM_TERMS; ++i) {
            // the registry must list the terms in the same order as the enum
            if (std::to_underlying(TERMS.at(i).term) != i)
                throw "TERMS must be in the same order as the Term enum";

            offsets.at(i + 1uz) = offsets.at(i) + TERMS.at(i).count;
        }

        return offsets;
    }

    inline constexpr auto TERM_OFFSETS = compute_term_offsets();

} // namespace detail

/** The total number of eval weights.
    @ingroup eval
 */
inline constexpr auto NUM_WEIGHTS = detail::TERM_OFFSETS.back();

/** Returns the index of the given term's first weight in the flat weight array.
    @ingroup eval
 */
[[nodiscard, gnu::const]] constexpr auto offset_of(const Term term) noexcept -> size_t
{
    return detail::TERM_OFFSETS[std::to_underlying(term)];
}

/** Returns the number of weights that the given term owns.
    @ingroup eval
 */
[[nodiscard, gnu::const]] constexpr auto count_of(const Term term) noexcept -> size_t
{
    return TERMS[std::to_underlying(term)].count;
}

/** All the eval weights, stored in one flat array in the order given by ``TERMS``.
    The engine's eval uses ``DEFAULT_WEIGHTS`` (in ``DefaultWeights.hpp``), and a
    tuner can treat ``values`` as one parameter vector.

    @ingroup eval
 */
struct Weights final {
    /** The weights, in the order given by ``TERMS``. */
    std::array<TaperedScore, NUM_WEIGHTS> values { };

    /** Returns the weight at ``index`` within the given term. */
    [[nodiscard]] constexpr auto get(const Term term, const size_t index = 0uz) const noexcept -> TaperedScore
    {
        assert(index < count_of(term));

        return values[offset_of(term) + index];
    }

    /** Returns the weight at ``index`` within the given term. */
    [[nodiscard]] constexpr auto get(const Term term, const size_t index = 0uz) noexcept -> TaperedScore&
    {
        assert(index < count_of(term));

        return values[offset_of(term) + index];
    }

    /** Returns all the weights of the given term. */
    [[nodiscard]] constexpr auto of_term(const Term term) const noexcept -> std::span<const TaperedScore>
    {
        return std::span { values }.subspan(offset_of(term), count_of(term));
    }

    /** Returns the material value of a pawn, knight, bishop, rook or queen. */
    [[nodiscard]] constexpr auto material(const PieceType type) const noexcept -> TaperedScore
    {
        return get(Term::Material, static_cast<size_t>(type));
    }

    /** Returns the piece square table for the given piece type.
        Squares are indexed from White's point of view.
     */
    [[nodiscard]] constexpr auto pst(const PieceType type) const noexcept
        -> std::span<const TaperedScore, chess::board::NUM_SQUARES>
    {
        return std::span { values }
            .subspan(offset_of(Term::Pst) + (static_cast<size_t>(type) * chess::board::NUM_SQUARES))
            .first<chess::board::NUM_SQUARES>();
    }

    /** Returns the piece square table for the given piece type.
        Squares are indexed from White's point of view.
     */
    [[nodiscard]] constexpr auto pst(const PieceType type) noexcept
        -> std::span<TaperedScore, chess::board::NUM_SQUARES>
    {
        return std::span { values }
            .subspan(offset_of(Term::Pst) + (static_cast<size_t>(type) * chess::board::NUM_SQUARES))
            .first<chess::board::NUM_SQUARES>();
    }

    /** Returns the C++ source of ``DefaultWeights.hpp``, with these weights as the defaults.
        A tuner can use this to write out its results.
     */
    [[nodiscard]] auto to_cpp() const -> std::string;

    /** Compares two sets of weights for equality. */
    [[nodiscard]] constexpr auto operator==(const Weights&) const noexcept -> bool = default;
};

} // namespace ben_bot::eval
