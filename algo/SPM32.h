#ifndef SPLITMIX32_H
#define SPLITMIX32_H

#include <cstdint> // Diperlukan untuk uint32_t

class SplitMix32
{
private:
    uint32_t state;

public:
    // Konstruktor
    SplitMix32(uint32_t seed);

    // Method untuk menghasilkan angka acak raw 32-bit
    uint32_t next();

    // Method utilitas untuk rentang angka [min, max]
    int nextInRange(int min, int max);

    // Method utilitas untuk float [0.0, 1.0]
    float nextFloat();
};

#endif // SPLITMIX32_H