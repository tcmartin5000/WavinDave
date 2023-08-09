#include <gb/gb.h>
#include <stdint.h>
#include "BackgroundTileMap.h"
#include "BackgroundTiles1.h"
#include "Sprite1.h"
#include "Dave.h"
#include "hUGEDriver.h"

extern const hUGESong_t dave_bgm;

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
            if (!(0x01 < tilesBeneath[1] && (tilesBeneath[1] < 0x0C || tilesBeneath[1] == 0x0F)))
            {
                cantFall = 1;
            }
        }

        /* Check the main tile.
           If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
        if (!(0x01 < tilesBeneath[0] && (tilesBeneath[0] < 0x0C || tilesBeneath[0] == 0x0F)))
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
                if (!(0x01 < tilesBeneath[3] && (tilesBeneath[3] < 0x0C || tilesBeneath[3] == 0x0F)))
                {
                    cantFall = 2;
                }
            }

            /* Check the main tile.
               If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
            if (!(0x01 < tilesBeneath[2] && (tilesBeneath[2] < 0x0C || tilesBeneath[2] == 0x0F)))
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
            if (!((0x01 < tilesBeneath[1] && tilesBeneath[1] < 0x0C) || tilesBeneath[1] == 0x0F))
            {
                isOnGround = 1;
                /* Check main tile now. */
            }
            else if (!((0x01 < tilesBeneath[0] && tilesBeneath[0] < 0x0C) || tilesBeneath[0] == 0x0F))
            {
                isOnGround = 1;
            }
            /* Check only the main tile. */
        }
        else if (!((0x01 < tilesBeneath[0] && tilesBeneath[0] < 0x0C) || tilesBeneath[0] == 0x0F))
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

int main(void)
{
    uint8_t joypadVal = 0;
    uint8_t jumpHeld = 0;
    uint8_t jumpPressed = 0;
    uint8_t jumpFreq = 0;
    Dave dave;

    /* Initialize hardware registers. */
    NR52_REG = 0x80;
    NR50_REG = 0x77;
    NR51_REG = 0xFF;

    /* Initialize Dave. */
    dave.x = 0;
    dave.tilex = 0x0A;
    dave.y = 7;
    dave.tiley = 0x06;
    dave.mvtTimer = 0;
    dave.gravityForce = 0;
    dave.physTimer = 0;
    dave.jumpForce = 0;

    /* Initialize background tile information. */
    set_bkg_data(0, 20, BackgroundTiles1);
    set_bkg_tiles(0, 0, BackgroundTileMapWidth, BackgroundTileMapHeight, BackgroundTileMap);

    /* Initialize sprite tile information. */
    SPRITES_8x16;
    set_sprite_data(0, 20, Sprite1);
    set_sprite_tile(0, 0);
    move_sprite(0, 88, 57);

    /* Initialize music. 
    CRITICAL {
        hUGE_init(&dave_bgm);
        add_VBL(hUGE_dosound);
    } */
    /* Initialize display. */
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    while (1)
    {
        wait_vbl_done();

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
                if (dave.mvtTimer < 6)
                {
                    /* Index of first sprite in walk cycle left. */
                    set_sprite_tile(0, 4);
                    set_sprite_prop(0, S_FLIPX);
                }
                else if (dave.mvtTimer < 12)
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
                dave.mvtTimer = (dave.mvtTimer + 1) % 18;
            }
            else if (joypadVal & J_RIGHT)
            {
                scroll_bkg(1, 0);
                dave.x++;
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
                if (dave.mvtTimer < 6)
                {
                    /* Index of first sprite in walk cycle right. */
                    set_sprite_tile(0, 4);
                    set_sprite_prop(0, !S_FLIPX);
                }
                else if (dave.mvtTimer < 12)
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
                dave.mvtTimer = (dave.mvtTimer + 1) % 18;
            }
            else if (joypadVal == J_UP)
            {
                if (dave.mvtTimer < 6)
                {
                    /* Index of first sprite in wave cycle. */
                    set_sprite_tile(0, 10);
                }
                else
                {
                    /* Index of second sprite in wave cycle. */
                    set_sprite_tile(0, 12);
                }
                dave.mvtTimer = (dave.mvtTimer + 1) % 12;
            }
        }
        else
        {
            dave.mvtTimer = 0;
            set_sprite_tile(0, 0);
            jumpHeld = 0;
            jumpPressed = 0;
        }

        /* Handle jumping and gravity. */
        jump(&dave, jumpHeld, jumpPressed);

    }
    return 1;
}