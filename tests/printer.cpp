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

    // for (auto idx : { 58uz, 61uz })
    set.set(60uz);

    std::println("{:#X}", set.to_ullong());
}
