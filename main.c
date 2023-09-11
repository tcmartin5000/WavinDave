#include <gb/gb.h>
#include <gbdk/font.h>
#include <rand.h>
#include <stdlib.h>
#include <stdint.h>
#include "BackgroundTileMap.h"
#include "BackgroundTiles1.h"
#include "Sprite1.h"
#include "Dave.h"
#include "Neighbor.h"
#include "hUGEDriver.h"

extern const hUGESong_t dave_bgm;
/* First six pointers are window score digits, left to right. Second six are high score digits. */
uint8_t *score_pointers[12];

/* Reads arrays as three ints in length, adds to them increment where necessary. */
void incrementAndDrawScore(uint8_t *scoreArray, uint8_t *highScoreArray, uint8_t increment)
{
    for (uint8_t i = 2; i <= 2; i--)
    {
        // Add the increment to the current segment.
        scoreArray[i] += increment;
        // Have we overflown?
        if (scoreArray[i] > 99)
        {
            // If so, are we at the leftmost segment? Then max the score.
            if (i == 0)
            {
                scoreArray[i] = 99;
                scoreArray[i + 1] = 99;
                scoreArray[i + 2] = 99;
                for (uint8_t j = 0; j < 6; j++)
                {
                    set_vram_byte(score_pointers[j], 0x0A);
                }
            }
            // If not, then set the increment to this segment / 100, mod this segment by 100, draw.
            increment = scoreArray[i] / 100;
            scoreArray[i] %= 100;
            if (scoreArray[i] == 0)
            {
                set_vram_byte(score_pointers[i * 2], 0x01);
                set_vram_byte(score_pointers[i * 2 + 1], 0x01);
            }
            else
            {
                set_vram_byte(score_pointers[i * 2], (scoreArray[i] / 10) + 1);
                set_vram_byte(score_pointers[i * 2 + 1], (scoreArray[i] % 10) + 1);
            }
        }
        // If we've not overflown, then draw the new number.
        else
        {
            set_vram_byte(score_pointers[i * 2], (scoreArray[i] / 10) + 1);
            set_vram_byte(score_pointers[i * 2 + 1], (scoreArray[i] % 10) + 1);
            break;
        }
    }

    // Increment high score.
    if (scoreArray[0] > highScoreArray[0])
    {
        for (uint8_t i = 0; i < 6; i++)
        {
            set_vram_byte(score_pointers[i + 6], get_vram_byte(score_pointers[i]));
        }
        highScoreArray[0] = scoreArray[0];
        highScoreArray[1] = scoreArray[1];
        highScoreArray[2] = scoreArray[2];
    }
    else if (scoreArray[1] > highScoreArray[1] && highScoreArray[0] == scoreArray[0])
    {
        for (uint8_t i = 2; i < 6; i++)
        {
            set_vram_byte(score_pointers[i + 6], get_vram_byte(score_pointers[i]));
        }
        highScoreArray[1] = scoreArray[1];
        highScoreArray[2] = scoreArray[2];
    }
    else if (scoreArray[2] > highScoreArray[2] && highScoreArray[0] == scoreArray[0] && highScoreArray[1] == scoreArray[1])
    {
        for (uint8_t i = 4; i < 6; i++)
        {
            set_vram_byte(score_pointers[i + 6], get_vram_byte(score_pointers[i]));
        }
        highScoreArray[2] = scoreArray[2];
    }
}

uint8_t getBeneaths(Dave *dave, uint8_t index)
{
    /* Tile beneath. */
    switch (index)
    {
    case 0:
        return BackgroundTileMap[(((dave->tiley + 1) % 18) * 32) + dave->tilex];
        break;
        /* Tile beneath and to the right. */
    case 1:
        return BackgroundTileMap[(((dave->tiley + 1) % 18) * 32) + ((dave->tilex + 1) % 32)];
        break;
        /* Tile two beneath. */
    case 2:
        return BackgroundTileMap[(((dave->tiley + 2) % 18) * 32) + dave->tilex];
        break;
        /* Tile two beneath and one to the right. */
    case 3:
        return BackgroundTileMap[(((dave->tiley + 2) % 18) * 32) + ((dave->tilex + 1) % 32)];
        break;
    }

    return 0;
}

void gravity(Dave *dave)
{

    uint8_t cantFall = 0;
    uint8_t tilesBeneath[4] = {getBeneaths(dave, 0), getBeneaths(dave, 1), getBeneaths(dave, 2), getBeneaths(dave, 3)};

    /* Calculate intended velocity. */
    if (dave->physTimer % 8 == 0)
    {
        if (dave->gravityForce == 0)
        {
            dave->gravityForce = 1;
        }
        else if (dave->gravityForce < 16)
        {
            dave->gravityForce *= 2;
        }
    }

    /* See if position change according to velocity is possible and move accordingly. */
    /* Are we going down ONE tile? */
    if (dave->gravityForce + dave->y > 7)
    {
        /* Are we beneath one or two tiles? */
        if (dave->x != 0)
        {
            /* Two, so check the tile further right.
               If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
            if (!(0x27 < tilesBeneath[1] && (tilesBeneath[1] < 0x32 || tilesBeneath[1] == 0x35)))
            {
                cantFall = 1;
            }
        }

        /* Check the main tile.
           If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
        if (!(0x27 < tilesBeneath[0] && (tilesBeneath[0] < 0x32 || tilesBeneath[0] == 0x35)))
        {
            cantFall = 1;
        }

        /* Are we going down TWO tiles? */
        if (!cantFall && dave->gravityForce + dave->y > 15)
        {
            /* Are we beneath one or two tiles? */
            if (dave->x != 0)
            {
                /* Two, so check the tile further right.
                   If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
                if (!(0x27 < tilesBeneath[3] && (tilesBeneath[3] < 0x32 || tilesBeneath[3] == 0x35)))
                {
                    cantFall = 2;
                }
            }

            /* Check the main tile.
               If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
            if (!(0x27 < tilesBeneath[2] && (tilesBeneath[2] < 0x32 || tilesBeneath[2] == 0x35)))
            {
                cantFall = 2;
            }
        }

        /* Can we fall? Move as intended. */
        if (cantFall == 0)
        {
            scroll_sprite(0, 0, dave->gravityForce);
            /* Adjust the current tile appropriately. */
            if (dave->y + dave->gravityForce > 15)
            {
                dave->tiley = (dave->tiley + 2) % 18;
            }
            else
            {
                dave->tiley = (dave->tiley + 1) % 18;
            }
            dave->y = (dave->y + dave->gravityForce) % 8;
            /* Can we only fall less than a tile's worth? Close that gap. */
        }
        else if (cantFall == 1)
        {
            uint8_t difference = 7 - dave->y;
            scroll_sprite(0, 0, difference);
            dave->y = 7;
            dave->gravityForce = 0;
            /* Can we only fall less than two tiles' worth? Close that gap. */
        }
        else
        {
            uint8_t difference = 15 - dave->y;
            scroll_sprite(0, 0, difference);
            dave->y = 7;
            dave->tiley = (dave->tiley + 1) % 18;
            dave->gravityForce = 0;
        }
    }

    else
    {
        /* If we're going down no tiles, then move him down. */
        scroll_sprite(0, 0, dave->gravityForce);
        dave->y += dave->gravityForce;
    }
}

void jump(Dave *dave, uint8_t incrementJump, uint8_t startJump)
{

    dave->physTimer++;

    uint8_t isOnGround = 0;
    uint8_t tilesBeneath[4] = {getBeneaths(dave, 0), getBeneaths(dave, 1), getBeneaths(dave, 2), getBeneaths(dave, 3)};

    /* Are we on the bottom of a tile? */
    if (dave->y == 7)
    {
        /* Are we above one or two tiles? */
        if (dave->x != 0)
        {
            /* Two tiles, check further right one. */
            if (!((0x27 < tilesBeneath[1] && tilesBeneath[1] < 0x32) || tilesBeneath[1] == 0x35))
            {
                isOnGround = 1;
                /* Check main tile now. */
            }
            else if (!((0x27 < tilesBeneath[0] && tilesBeneath[0] < 0x32) || tilesBeneath[0] == 0x35))
            {
                isOnGround = 1;
            }
            /* Check only the main tile. */
        }
        else if (!((0x27 < tilesBeneath[0] && tilesBeneath[0] < 0x32) || tilesBeneath[0] == 0x35))
        {
            isOnGround = 1;
        }
    }

    /* Are we jumping from the ground? */
    if (isOnGround && startJump)
    {
        dave->jumpForce = 4;
        dave->physTimer = 1;
        dave->gravityForce = 0;

        /* Begin SFX by modifying channel 2 register values. */
        /* VVVVAPPP, where V is starting volume, A is decrement or increment, and P is period.
           Apparently writing to this register can cause undefined behavior, but I don't know how
           else to adjust these settings? That's why I'm doing this before 21. We'll see what
           happens.
        */
        NR12_REG = 0b11110010;

        /* Unique to channel 1, set sweep value.
           XPPPNSSS, where P is period, N is isNegative, S is shift per six steps.
        */
        NR10_REG = 0b00100111;

        /* DDLLLLLL, where D is duty value, L is length value. */
        NR11_REG = 0b11001000;

        /* FFFFFFFF, where F is frequency, least significant bits (sets initial). */
        NR13_REG = 0x00;

        /* TLXXXFFF, where T is trigger, L is length on/off, F is frequency, most significant bits. */
        NR14_REG = 0b10000110;

        /* Are we leaning into our jump? */
    }
    else if (incrementJump && dave->jumpForce != 0)
    {
        if (dave->physTimer % 4 == 0)
        {
            dave->jumpForce /= 2;
        }
        /* If not, then fall faster. */
    }
    else if (dave->jumpForce != 0)
    {
        dave->jumpForce /= 2;
    }
    else
    {
        /* There is no need to go up, so we'll have the gravity handler bring us down instead. */
        gravity(dave);
        return;
    }

    /* Scroll the appropriate amount of pixels upward. */
    scroll_sprite(0, 0, -dave->jumpForce);
    /* Adjust tile and position values. */
    dave->y -= dave->jumpForce;
    if (dave->y < -8)
    {
        dave->tiley = (((dave->tiley - 2) % 18) + 18) % 18;
    }
    else if (dave->y < 0)
    {
        dave->tiley = (((dave->tiley - 1) % 18) + 18) % 18;
    }
    /* Negative modulo works stupidly in C so we have to do this. */
    dave->y = ((dave->y % 8) + 8) % 8;
}

void scrollNeighbors(int8_t amount)
{
    for (uint8_t i = 0; i < 9; i++)
    {
        scroll_sprite(i + 1, amount, 0);
    }
}

/* These are hard coded because I was getting some unexplainable bugs otherwise.
   It's not ideal, but it'll do. */
uint8_t spawnNeighborForMap1(Neighbor *neighbors, uint8_t numberNeighbors, uint8_t screenOffset)
{
    /* Don't draw if we're full, but also add some randomness to the draw call. */
    if (numberNeighbors < 9 && sys_time % 255 == 0 && rand() % 3 == 0)
    {
        uint8_t validSpawnFound = 0;
        /* Where in the array the new neighbor should be placed. */
        uint8_t placement = 0;
        uint8_t spawnIndex;

        // Very big potential for error here if I'm off by one. Check here if there's weird behavior.
        do
        {
            // Randomly pull an index from the valid spawn point array.
            spawnIndex = rand() % BackgroundTileMapNumberSpawnPositions;
            // Ensure no duplicates
            validSpawnFound = 1;
            // If there's none right now then it's not a duplicate.
            if (numberNeighbors != 0)
            {
                for (uint8_t i = 0; i < 9; i++)
                {
                    if (neighbors[i].spriteNumber != 255)
                    {
                        if (neighbors[i].x == BackgroundTileMapSpawnXPositions[spawnIndex] && neighbors[i].y == BackgroundTileMapSpawnYPositions[spawnIndex])
                        {
                            validSpawnFound = 0;
                            break;
                        }
                    }
                    else
                    {
                        placement = i;
                    }
                }
            }
        } while (!validSpawnFound);

        // Add to master list.
        Neighbor n;
        n.x = BackgroundTileMapSpawnXPositions[spawnIndex];
        n.y = BackgroundTileMapSpawnYPositions[spawnIndex];
        n.spriteNumber = placement;
        n.secondsLeft = 30;
        neighbors[placement] = n;

        // Draw the sprite and move to the appropriate coordinates. Sprite 0 is the player, hence the + 1.
        set_sprite_tile(placement + 1, 0);
        move_sprite(placement + 1, ((n.x + 1) * 8) - screenOffset, ((n.y + 1) * 8) + 1);
        return 1;
    }
    return 0;
}

void neighborAnimate()
{
    if (sys_time % 40 < 19)
    {
        for (uint8_t i = 0; i < 9; i++)
        {
            set_sprite_tile(i + 1, 2);
        }
    }
    else
    {
        for (uint8_t i = 0; i < 9; i++)
        {
            set_sprite_tile(i + 1, 0);
        }
    }
}

int isScoring(Neighbor *neighborArray, Dave dave)
{
    for (uint8_t i = 0; i < 9; i++)
    {
        /* If coordinates match, give or take one on the x-axis to account for being on either side, give point and clear sprite. */
        if (neighborArray[i].spriteNumber != 255 && (dave.tilex == neighborArray[i].x || dave.tilex == (((neighborArray[i].x - 1) % 32) + 32) % 32) && dave.tiley == neighborArray[i].y)
        {
            neighborArray[i].x = 255;
            neighborArray[i].y = 255;
            move_sprite(neighborArray[i].spriteNumber + 1, 0, 0);
            neighborArray[i].spriteNumber = 255;
            return 1;
        }
    }
    return 0;
}

int isGameOver(Neighbor *neighborArray)
{
    if (sys_time % 60 == 0)
    {
        for (uint8_t i = 0; i < 9; i++)
        {
            if (neighborArray[i].spriteNumber != 255)
            {
                neighborArray[i].secondsLeft--;
                
                if (neighborArray[i].secondsLeft == 0)
                {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int main(void)
{

    /* Initialize RNG. */
    initrand(DIV_REG * sys_time);

    uint8_t joypadVal = 0;
    uint8_t jumpHeld = 0;
    uint8_t jumpPressed = 0;
    uint8_t jumpFreq = 0;
    /* Should be no greater than 9 for hardware reasons. */
    uint8_t numberNeighbors = 0;
    /* At some point this should probably be converted into a linked list of neighbors
       to eliminate performance overhead on removal from the list. Will require some
       major refactoring and dynamic allocation shenanigans. I don't want to do that
       right now. */
    Neighbor neighbors[9];
    for (uint8_t i = 0; i < 9; i++)
    {
        neighbors[i].x = 255;
        neighbors[i].y = 255;
        neighbors[i].spriteNumber = 255;
    }

    /* Set default state. Equivalent to 1UP 000000-HI 000000 */
    const unsigned char DEFAULT_WINDOW_MAP[] = {
        0x02, 0x1F, 0x1A, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x2D,
        0x12, 0x13, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

    const unsigned char GAME_OVER_WINDOW_MAP[] = {
        0x11, 0x0B, 0x17, 0x0F, 0x00, 0x19, 0x20, 0x0F, 0x1C};

    /* 32-bit integers are too big, so we'll use multiple 8-bit instead. Three segments
       of two digits, specifically, to speed up drawing at the expense of memory usage.
    */
    uint8_t score[] = {0, 0, 0};
    uint8_t highScore[] = {0, 0, 0};
    Dave dave;

    /* Initialize hardware registers for sound. */
    NR52_REG = 0x80;
    NR50_REG = 0x77;
    NR51_REG = 0xFF;

    /* Initialize Dave. */
    dave.x = 0;
    dave.tilex = 0x0A;
    dave.y = 7;
    dave.tiley = 0x0E;
    dave.gravityForce = 0;
    dave.physTimer = 0;
    dave.jumpForce = 0;

    /* Initialize text tile data. */
    font_init();
    const font_t hudFont = font_load(font_min);
    font_set(hudFont);

    /* Initialize background tile information. Offset to avoid overwriting font. */
    set_bkg_data(38, 15, BackgroundTiles1);
    set_bkg_tiles(0, 0, BackgroundTileMapWidth, BackgroundTileMapHeight, BackgroundTileMap);

    /* Initialize sprite tile information. */
    SPRITES_8x16;
    set_sprite_data(0, 20, Sprite1);
    set_sprite_tile(0, 0);
    move_sprite(0, 88, 121);

    /* Initialize window data (text, for score display) */
    set_win_tiles(0, 0, 20, 1, DEFAULT_WINDOW_MAP);
    move_win(7, 136);

    /* Initialize window pointers for faster writing later. Possible y value should be 1? No clue. */
    for (uint8_t i = 0; i < 6; i++)
    {
        score_pointers[i] = get_win_xy_addr(i + 4, 0);
        score_pointers[i + 6] = get_win_xy_addr(i + 14, 0);
    }

    /* Initialize music. */
    CRITICAL
    {
        hUGE_init(&dave_bgm);
        add_VBL(hUGE_dosound);
    }
    /* Initialize display. */
    SHOW_BKG;
    SHOW_WIN;
    SHOW_SPRITES;
    DISPLAY_ON;

    uint8_t gameActive = 1;

    while (gameActive)
    {

        /* Spawn in new neighbors if need be. Do this before input so scroll doesn't break. */
        if (spawnNeighborForMap1(neighbors, numberNeighbors, ((dave.tilex * 8) + dave.x) - 80))
        {
            numberNeighbors++;
        };

        /* Handle input. */
        if (joypadVal = joypad())
        {
            if (joypadVal & J_A)
            {
                if (jumpHeld == 0)
                {
                    jumpPressed = 1;
                    jumpFreq = 0;
                    jumpHeld = 1;
                }
                else
                {
                    jumpPressed = 0;
                }
            }
            else
            {
                jumpPressed = 0;
                jumpHeld = 0;
            }

            if (joypadVal & J_LEFT)
            {
                scroll_bkg(-1, 0);
                dave.x--;
                scrollNeighbors(1);
                /* Process tile move. */
                if (dave.x < 0)
                {
                    dave.tilex = (((dave.tilex - 1) % 32) + 32) % 32;
                    dave.x = 7;
                }
                else if (dave.x > 7)
                {
                    dave.tilex = (((dave.tilex + 1) % 32) + 32) % 32;
                    dave.x = 0;
                }

                /* Animate Dave sprite. */
                if (sys_time % 18 < 6)
                {
                    /* Index of first sprite in walk cycle left. */
                    set_sprite_tile(0, 4);
                    set_sprite_prop(0, S_FLIPX);
                }
                else if (sys_time % 18 < 12)
                {
                    /* Index of second sprite in walk cycle left. */
                    set_sprite_tile(0, 6);
                    set_sprite_prop(0, S_FLIPX);
                }
                else
                {
                    /* Index of third sprite in walk cycle left. */
                    set_sprite_tile(0, 8);
                    set_sprite_prop(0, S_FLIPX);
                }
            }
            else if (joypadVal & J_RIGHT)
            {
                scroll_bkg(1, 0);
                dave.x++;
                scrollNeighbors(-1);
                /* Process tile move. */
                if (dave.x < 0)
                {
                    dave.tilex = (((dave.tilex - 1) % 32) + 32) % 32;
                    dave.x = dave.x % 8;
                }
                else if (dave.x > 7)
                {
                    dave.tilex = (((dave.tilex + 1) % 32) + 32) % 32;
                    dave.x = dave.x % 8;
                }

                /* Animate Dave sprite. */
                if (sys_time % 18 < 6)
                {
                    /* Index of first sprite in walk cycle right. */
                    set_sprite_tile(0, 4);
                    set_sprite_prop(0, !S_FLIPX);
                }
                else if (sys_time % 18 < 12)
                {
                    /* Index of second sprite in walk cycle right. */
                    set_sprite_tile(0, 6);
                    set_sprite_prop(0, !S_FLIPX);
                }
                else
                {
                    /* Index of third sprite in walk cycle right. */
                    set_sprite_tile(0, 8);
                    set_sprite_prop(0, !S_FLIPX);
                }
            }
            else if (joypadVal == J_UP)
            {
                if (isScoring(neighbors, dave))
                {
                    incrementAndDrawScore(score, highScore, 1);
                    numberNeighbors--;
                }

                if (sys_time % 12 < 6)
                {
                    /* Index of first sprite in wave cycle. */
                    set_sprite_tile(0, 10);
                }
                else
                {
                    /* Index of second sprite in wave cycle. */
                    set_sprite_tile(0, 12);
                }

                /* Reinitialize RNG, for extra randomness. Very arbitrary but why not.
                   Might backfire, we'll see. */
                initrand(DIV_REG * sys_time);
            }
            else
            {
                set_sprite_tile(0, 0);
            }
        }
        else
        {
            set_sprite_tile(0, 0);
            jumpHeld = 0;
            jumpPressed = 0;
        }

        /* Handle jumping and gravity. */
        jump(&dave, jumpHeld, jumpPressed);

        /* Handle neighbor animations. */
        neighborAnimate();

        // Check game over state
        gameActive = isGameOver(neighbors);

        vsync();
    }
    // Set game over message, good enough to test with.
    set_win_tiles(0, 0, 9, 1, GAME_OVER_WINDOW_MAP);
    return 1;
}