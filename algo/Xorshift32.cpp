#include "Xorshift32.h"
#include <chrono>
#include <iostream>
#include <random>

Xorshift32::Xorshift32(uint32_t seed)
{
    if (seed == 0)
    {
        // Gunakan seed acak jika tidak disediakan
        std::random_device rd;
        state = rd();
    }
    else
    {
        state = seed;
    }
}



