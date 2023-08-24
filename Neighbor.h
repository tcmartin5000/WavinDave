#ifndef NEIGHBOR_H
#define NEIGHBOR_H

typedef struct Neighbor {
    uint8_t x;
    uint8_t y;
    uint8_t spriteNumber;
    /* At some point this should probably be converted into a linked list of neighbors
       to eliminate performance overhead on removal from the list. Will require some
       major refactoring and dynamic allocation shenanigans. I don't want to do that
       right now. */
    //Neighbor *next;
    
} Neighbor;

#endif