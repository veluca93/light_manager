#ifndef RAND_HPP
#define RAND_HPP

#include <Arduino.h>

class rng {
    uint8_t seed;
public:
    uint8_t operator()() {
        seed ^= seed << 7;
        seed ^= seed >> 5;
        seed ^= seed << 3;
        return seed;
    }

    rng(uint8_t seed): seed(seed) {}
};
#endif
