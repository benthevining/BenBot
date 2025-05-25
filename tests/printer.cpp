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

    for (size_t idx : { 1uz, 9uz, 17uz, 25uz, 33uz, 41uz, 49uz, 57uz })
        set.set(idx + 5uz);

    std::println("{:#X}", set.to_ullong());
}
