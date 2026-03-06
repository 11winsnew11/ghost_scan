#ifndef WYRAND64_H
#define WYRAND64_H

#include <cstdint>

class WyRand64 {
private:
    uint64_t state;
    
public:
    WyRand64(uint64_t seed = 0) : state(seed) {
        if(seed == 0) {
            state = 0x123456789ABCDEFULL;
        }
    }
    
    uint64_t generate64() {
        state += 0x60bee2bee120fc15ULL;
        uint64_t tmp = state;
        tmp = (tmp ^ (tmp >> 32)) * 0xda942042e4dd58b5ULL;
        tmp = (tmp ^ (tmp >> 32)) * 0xda942042e4dd58b5ULL;
        return tmp ^ (tmp >> 32);
    }
    
    uint64_t generate16Digit() {
        uint64_t r = generate64();
        // Fast modulo reduction
        uint64_t result = 1000000000000000ULL;
        result += ((r & 0x7FFFFFFFFFFFFFFF) % 9000000000000000ULL);
        return result;
    }
    
};
#endif