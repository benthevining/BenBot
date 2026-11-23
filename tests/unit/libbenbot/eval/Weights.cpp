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

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <set>
#include <string>
#include <string_view>
#include <utility>

inline constexpr auto TAGS { "[eval][Weights]" };

using ben_bot::eval::count_of;
using ben_bot::eval::DEFAULT_WEIGHTS;
using ben_bot::eval::NUM_WEIGHTS;
using ben_bot::eval::offset_of;
using ben_bot::eval::Term;
using ben_bot::eval::TERMS;

TEST_CASE("Weights - term registry", TAGS)
{
    auto offset = 0uz;

    std::set<std::string_view> names;

    for (auto i = 0uz; i < TERMS.size(); ++i) {
        const auto& info = TERMS.at(i);

        INFO(info.name);

        REQUIRE(std::to_underlying(info.term) == i);
        REQUIRE(info.count > 0uz);
        REQUIRE(offset_of(info.term) == offset);
        REQUIRE(count_of(info.term) == info.count);
        REQUIRE(DEFAULT_WEIGHTS.of_term(info.term).size() == info.count);
        REQUIRE(names.insert(info.name).second);

        offset += info.count;
    }

    REQUIRE(offset == NUM_WEIGHTS);
}

TEST_CASE("Weights - accessors", TAGS)
{
    using PieceType = chess::pieces::Type;

    static constexpr auto NUM_SQUARES = chess::board::NUM_SQUARES;

    for (auto type = 0uz; type < 6uz; ++type) {
        const auto pst = DEFAULT_WEIGHTS.pst(static_cast<PieceType>(type));

        REQUIRE(pst.data() == &DEFAULT_WEIGHTS.values.at(offset_of(Term::Pst) + (type * NUM_SQUARES)));
    }

    REQUIRE(DEFAULT_WEIGHTS.material(PieceType::Queen) == DEFAULT_WEIGHTS.values.at(offset_of(Term::Material) + 4uz));
    REQUIRE(DEFAULT_WEIGHTS.get(Term::QueenMobility, 27uz) == DEFAULT_WEIGHTS.values.at(offset_of(Term::QueenMobility) + 27uz));
}

TEST_CASE("Weights - DefaultWeights.hpp is what to_cpp() generates", TAGS)
{
    std::ifstream file { BENBOT_DEFAULT_WEIGHTS_HEADER };

    REQUIRE(file.is_open());

    const std::string contents {
        std::istreambuf_iterator<char> { file }, std::istreambuf_iterator<char> { }
    };

    REQUIRE(DEFAULT_WEIGHTS.to_cpp() == contents);
}
