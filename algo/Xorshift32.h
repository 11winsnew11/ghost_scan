#ifndef XORSHIFT32_H
#define XORSHIFT32_H

#include <cstdint>
#include <vector>

class Xorshift32
{
private:
    uint32_t state;

public:
    // Constructor dengan seed default menggunakan waktu
    Xorshift32(uint32_t seed = 0);
    void seed(uint32_t newSeed)
    { // Tambahkan metode ini
        state = newSeed;
    }

    uint32_t getState() const
    {
        return state;
    }

    uint32_t next32Variant(uint32_t shift1, uint32_t shift2, uint32_t shift3)
    {
        uint32_t result = state;
        result ^= result << shift1;
        result ^= result >> shift2;
        result ^= result << shift3;
        return result;
    }

    // Fungsi untuk mengupdate state dengan parameter shift tertentu
    void updateStateWithVariant(uint32_t shift1, uint32_t shift2, uint32_t shift3)
    {
        state ^= state << shift1;
        state ^= state >> shift2;
        state ^= state << shift3;
    }

    uint32_t generate32()
    {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }
};

#endif // XORSHIFT32_H