#include <gb/gb.h>
#include <stdint.h>
#include "BackgroundTileMap.h"
#include "BackgroundTiles1.h"
#include "Sprite1.h"
#include "Dave.h"

void gravity(Dave *dave)
{

    uint8_t cantFall = 0;
    uint8_t tileToBeChecked;

    /* Calculate intended velocity. */
    if (dave->gravTimer % 5 == 0)
    {
        if (dave->vertVelocity == 0)
        {
            dave->vertVelocity = 1;
        }
        else if (dave->vertVelocity < 16)
        {
            dave->vertVelocity *= 2;
        }
    }

    dave->gravTimer++;

    /* See if position change according to velocity is possible and move accordingly. */
    /* Are we going down a tile? */
    if (dave->vertVelocity + dave->y > 7)
    {
        /* Are we beneath one or two tiles? */
        if (dave->x != 0)
        {
            /* Two, so check the tile further right. */
            tileToBeChecked = BackgroundTileMap[(((dave->tiley + 1) % 18) * 32) + ((dave->tilex + 1) % 32)];
            /* If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
            if (!(0x01 < tileToBeChecked && (tileToBeChecked < 0x0C || tileToBeChecked == 0x0F)))
            {
                cantFall = 1;
            }
        }

        /* Check the main tile. */
        tileToBeChecked = BackgroundTileMap[(((dave->tiley + 1) % 18) * 32) + dave->tilex];
        /* If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
        if (!(0x01 < tileToBeChecked && (tileToBeChecked < 0x0C || tileToBeChecked == 0x0F)))
        {
            cantFall = 1;
        }

        /* Are we going down TWO tiles? */
        if (!cantFall && dave->vertVelocity > 15)
        {
            /* Are we beneath one or two tiles? */
            if (dave->x != 0)
            {
                /* Two, so check the tile further right. */
                tileToBeChecked = BackgroundTileMap[(((dave->tiley + 2) % 18) * 32) + ((dave->tilex + 1) % 32)];
                /* If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
                if (!(0x01 < tileToBeChecked && (tileToBeChecked < 0x0C || tileToBeChecked == 0x0F)))
                {
                    cantFall = 2;
                }
            }

            /* Check the main tile. */
            tileToBeChecked = BackgroundTileMap[(((dave->tiley + 2) % 18) * 32) + dave->tilex];
            /* If it's a solid tile, then we can't fall! (0x00, 0x01, 0x0C, 0x0D, 0x0E are all solid) */
            if (!(0x01 < tileToBeChecked && (tileToBeChecked < 0x0C || tileToBeChecked == 0x0F)))
            {
                cantFall = 2;
            }
        }

        /* Can we fall? Move as intended. */
        if (cantFall == 0) {
            scroll_sprite(0, 0, dave->vertVelocity);
            /* Adjust the current tile appropriately. */
            if (dave->y + dave->vertVelocity > 15) {
                dave->tiley = (dave->tiley + 2) % 18;
            } else {
                dave->tiley = (dave->tiley + 1) % 18;
            }
            dave->y = (dave->y + dave->vertVelocity) % 8;
        /* Can we only fall less than a tile's worth? Close that gap. */
        } else if (cantFall == 1) {
            uint8_t difference = 7 - dave->y;
            scroll_sprite(0, 0, difference);
            dave->y = 7;
            dave->vertVelocity = 0;
        /* Can we only fall less than two tiles' worth? Close that gap. */
        } else {
            uint8_t difference = 15 - dave->y;
            scroll_sprite(0, 0, difference);
            dave->y = 7;
            dave->tiley = (dave->tiley + 1) % 18;
            dave->vertVelocity = 0;
        }
    }

    else
    {
        /* If we're going down no tiles, then move him down. */
        scroll_sprite(0, 0, dave->vertVelocity);
        dave->y += dave->vertVelocity;
    }
}

int main(void)
{
    uint8_t joypadVal = 0;
    Dave dave;

    /* Initialize Dave. */
    dave.x = 0;
    dave.tilex = 0x0A;
    dave.y = 7;
    dave.tiley = 0x06;
    dave.mvtTimer = 0;
    dave.vertVelocity = 0;
    dave.gravTimer = 0;

    /* Initialize background tile information. */
    set_bkg_data(0, 20, BackgroundTiles1);
    set_bkg_tiles(0, 0, BackgroundTileMapWidth, BackgroundTileMapHeight, BackgroundTileMap);

    /* Initialize sprite tile information. */
    SPRITES_8x16;
    set_sprite_data(0, 20, Sprite1);
    set_sprite_tile(0, 0);
    move_sprite(0, 88, 57);

    /* Initialize display. */
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    while (1)
    {
        /* Handle input. */
        if (joypadVal = joypad())
        {
            if (joypadVal & J_LEFT)
            {
                scroll_bkg(-1, 0);
                dave.x--;
                /* Process tile move. */
                if (dave.x < 0)
                {
                    dave.tilex = (dave.tilex - 1) % 32;
                    dave.x = dave.x % 8;
                }
                else if (dave.x > 7)
                {
                    dave.tilex = (dave.tilex + 1) % 32;
                    dave.x = dave.x % 8;
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
                    dave.tilex = (dave.tilex - 1) % 32;
                    dave.x = dave.x % 8;
                }
                else if (dave.x > 7)
                {
                    dave.tilex = (dave.tilex + 1) % 32;
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
        }

        /* Handle gravity. */
        gravity(&dave);

        wait_vbl_done();
    }
    return 1;
}