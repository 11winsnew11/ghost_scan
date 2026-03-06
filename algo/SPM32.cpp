#include "SPM32.h"

SplitMix32::SplitMix32(uint32_t seed) : state(seed) {}

uint32_t SplitMix32::next()
{
    uint32_t z = (state += 0x9e3779b9);
    z = (z ^ (z >> 16)) * 0x85ebca6b;
    z = (z ^ (z >> 13)) * 0xc2b2ae35;
    return z ^ (z >> 16);
}

int SplitMix32::nextInRange(int min, int max)
{
    return min + (next() % (max - min + 1));
}
