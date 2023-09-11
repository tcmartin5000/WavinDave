#ifndef NEIGHBOR_H
#define NEIGHBOR_H

typedef struct Neighbor {
    // X coordinate on map grid.
    uint8_t x;
    // Y coordinate on map grid.
    uint8_t y;
    // Number in the sprite list. If 255, then neighbor does not exist.
    uint8_t spriteNumber;
    // Time to game over in rough seconds (1092 increments of SYS_TIME).
    uint8_t secondsLeft;
    /* At some point this should probably be converted into a linked list of neighbors
       to eliminate performance overhead on removal from the list. Will require some
       major refactoring and dynamic allocation shenanigans. I don't want to do that
       right now. */
    //Neighbor *next;
    
} Neighbor;

#endif