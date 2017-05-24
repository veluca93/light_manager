#ifndef RAND_HPP
#define RAND_HPP

#include <Arduino.h>

class rng {
    uint16_t x, y;
public:
    uint16_t operator()() {
        uint16_t t = x^(x<<5);
        x = y;
        return y = (y^(y>>1)) ^ (t^(t>>3));
    }

    rng(uint8_t seed) {
        y = x = seed | ((uint16_t)seed<<8);
    }
};
#endif
