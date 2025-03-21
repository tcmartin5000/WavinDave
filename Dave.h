#ifndef DAVE_H
#define DAVE_H

typedef struct Dave {
    /* Originally planned to be a singular velocity value, hence the signed
       type, I just can't be bothered to look and see if this is safe to change
       or not so it will remain as such. */
    int8_t gravityForce;
    uint8_t jumpForce;
    int8_t x;
    int8_t y;
    uint8_t tilex;
    uint8_t tiley;
    uint8_t physTimer;

} Dave;

#endif