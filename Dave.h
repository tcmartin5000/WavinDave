#ifndef DAVE_H
#define DAVE_H

typedef struct Dave {
    uint8_t mvtTimer;
    /* Originally planned to be a singular velocity value, hence the signed
       type, I just can't be bothered to look and see if this is safe to change
       or not so it will remain as such. */
    int8_t gravityForce;
    uint8_t jumpForce;
    int8_t x;
    int8_t y;
    int8_t tilex;
    int8_t tiley;
    uint8_t physTimer;
    uint8_t jumpLength;
} Dave;

#endif