/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <bitset>
#include <print>

int main()
{
    std::bitset<64uz> set;

    for (size_t idx = 48uz; idx <= 55uz; ++idx)
        set.set(idx);

    std::println("{:#X}", set.to_ullong());
}
