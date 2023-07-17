#include <gb/gb.h>
#include <stdint.h>
#include "BackgroundTileMap.h"
#include "BackgroundTiles1.h"
#include "Sprite1.h"

void main(void)
{
    uint8_t spriteMvtTimer = 0;

    /* Initialize background tile information. */
    set_bkg_data(0, 20, BackgroundTiles1);
    set_bkg_tiles(0, 0, BackgroundTileMapWidth, BackgroundTileMapHeight, BackgroundTileMap);

    /* Initialize sprite tile information. */
    SPRITES_8x16;
    set_sprite_data(0, 20, Sprite1);
    set_sprite_tile(0, 0);
    move_sprite(0, 88, 121);

    /* Initialize display. */
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    while (1) {
        if (joypad()) {
            switch (joypad())
            {
            case J_LEFT:
                scroll_bkg(-1, 0);
                if (spriteMvtTimer < 6) {
                    set_sprite_tile(0, 10);
                } else if (spriteMvtTimer < 12) {
                    set_sprite_tile(0, 12);
                } else {
                    set_sprite_tile(0, 14);
                }
                spriteMvtTimer = (spriteMvtTimer + 1) % 18;
                break;
            case J_RIGHT:
                scroll_bkg(1, 0);
                if (spriteMvtTimer < 6) {
                    set_sprite_tile(0, 4);
                } else if (spriteMvtTimer < 12) {
                    set_sprite_tile(0, 6);
                } else {
                    set_sprite_tile(0, 8);
                }
                spriteMvtTimer = (spriteMvtTimer + 1) % 18;
                break;
            case J_UP:
                if (spriteMvtTimer < 6) {

                    set_sprite_tile(0, 16);
                } else {
                    set_sprite_tile(0, 18);
                }
                spriteMvtTimer = (spriteMvtTimer + 1) % 12;
            }
        }
        else {
            spriteMvtTimer = 0;
            set_sprite_tile(0, 0);
        }
        wait_vbl_done();
    }
}
