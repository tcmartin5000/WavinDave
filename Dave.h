#ifndef DAVE_H
#define DAVE_H

typedef struct Dave {
    uint8_t mvtTimer;
    int8_t vertVelocity;
    int8_t x;
    int8_t y;
    uint8_t tilex;
    uint8_t tiley;
    uint8_t gravTimer;
} Dave;

#endif