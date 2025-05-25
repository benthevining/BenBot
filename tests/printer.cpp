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

    set.set();

    std::println("{:#X}", set.to_ullong());
}
