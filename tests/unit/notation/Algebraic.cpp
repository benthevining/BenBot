/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <libchess/notation/Algebraic.hpp>

static constexpr auto TAGS { "[notation][Algebraic]" };

using chess::notation::from_alg;
using chess::notation::to_alg;

namespace match = Catch::Matchers;
