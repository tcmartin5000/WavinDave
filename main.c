#include <gb/gb.h>
#include <stdint.h>
#include "BackgroundTileMap.h"
#include "BackgroundTiles1.h"
#include "Sprite1.h"

void main(void)
{
    uint8_t spriteMvtTimer = 0;
    uint8_t joypadVal = 0;

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
        if (joypadVal = joypad()) {
            if (joypadVal & J_LEFT) {
                scroll_bkg(-1, 0);
                if (spriteMvtTimer < 6) {
                    /* Index of first sprite in walk cycle left. */
                    set_sprite_tile(0, 4);
                    set_sprite_prop(0, S_FLIPX);
                } else if (spriteMvtTimer < 12) {
                    /* Index of second sprite in walk cycle left. */
                    set_sprite_tile(0, 6);
                    set_sprite_prop(0, S_FLIPX);
                } else {
                    /* Index of third sprite in walk cycle left. */
                    set_sprite_tile(0, 8);
                    set_sprite_prop(0, S_FLIPX);
                }
                spriteMvtTimer = (spriteMvtTimer + 1) % 18;
            } else if (joypadVal & J_RIGHT) {
                scroll_bkg(1, 0);
                if (spriteMvtTimer < 6) {
                    /* Index of first sprite in walk cycle right. */
                    set_sprite_tile(0, 4);
                    set_sprite_prop(0, !S_FLIPX);
                } else if (spriteMvtTimer < 12) {
                    /* Index of second sprite in walk cycle right. */
                    set_sprite_tile(0, 6);
                    set_sprite_prop(0, !S_FLIPX);
                } else {
                    /* Index of third sprite in walk cycle right. */
                    set_sprite_tile(0, 8);
                    set_sprite_prop(0, !S_FLIPX);
                }
                spriteMvtTimer = (spriteMvtTimer + 1) % 18;
            } else if (joypadVal == J_UP) {
                if (spriteMvtTimer < 6) {
                    /* Index of first sprite in wave cycle. */
                    set_sprite_tile(0, 10);
                } else {
                    /* Index of second sprite in wave cycle. */
                    set_sprite_tile(0, 12);
                }
                spriteMvtTimer = (spriteMvtTimer + 1) % 12;
            }
        } else {
            spriteMvtTimer = 0;
            set_sprite_tile(0, 0);
        }
        wait_vbl_done();
    }
}
