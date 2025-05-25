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

    for (auto idx = 0uz; idx <= 7uz; ++idx)
        set.set(idx);

    for (auto idx = 56uz; idx <= 63uz; ++idx)
        set.set(idx);

    for (auto idx : { 8uz, 16uz, 24uz, 32uz, 40uz, 48uz,
             15uz, 23uz, 31uz, 39uz, 47uz, 55uz })
        set.set(idx);

    std::println("{:#X}", set.to_ullong());
}
