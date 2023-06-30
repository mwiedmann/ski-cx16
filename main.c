#include <cx16.h>
#include <cbm.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "course.h"
#include "utils.h"
#include "wait.h"
#include "sprites.h"

GuyData guyData;

short scroll = 0;
unsigned char lastTileX = 0, lastTileY = 0;

void getCollisionTiles(unsigned char *l0Tile, unsigned char *l1Tile) {
    unsigned char tileX, tileY;
    unsigned long tileAddr;

    // Get the bottom center of the sprite (his feet) for collision
    tileX = (guyData.guyX+8)>>4;
    tileY = (scroll + guyData.guyY + 8)>>4;

    if (tileX == lastTileX && tileY == lastTileY) {
        return;
    }

    lastTileX = tileX;
    lastTileY = tileY;

    // Get tile on L0 guy is "touching"
    tileAddr = L0_MAPBASE_ADDR + (((tileY * MAPBASE_TILE_WIDTH) + tileX) * 2);
    VERA.address = tileAddr;
    VERA.address_hi = tileAddr>>16;
    *l0Tile = VERA.data0;

    // Get tile on L1 guy is "touching"
    tileAddr = L1_MAPBASE_ADDR + (((tileY * MAPBASE_TILE_WIDTH) + tileX) * 2);
    VERA.address = tileAddr;
    VERA.address_hi = tileAddr>>16;
    *l1Tile = VERA.data0;
}

void showTimer(unsigned char mins, unsigned char secs, unsigned char milli) {
    unsigned char msg[24], buf[24];

    sprintf(buf, "%u:%02u.%02u", mins, secs, milli);
    sprintf(msg, "%12s", buf);

    spriteText(msg, 0);
}

void main() {
    unsigned char l0Tile;
    unsigned char l1Tile;
    unsigned char inSnow = 0;
    unsigned short scrollSpeed;
    unsigned char mins, secs, ticks, milli;
    

    init();
    
    while(1) {
        spritesConfig(&guyData);
        drawCourse();
        scroll = 0;
        scrollSpeed = 0;

        VERA.layer0.vscroll = scroll;
        VERA.layer1.vscroll = scroll;

        waitCount(120);

        ticks = 0;
        mins = 0;
        secs = 0;

        showTimer(mins, secs, milli);

        while(1) {
            getCollisionTiles(&l0Tile, &l1Tile);

            // Check for collision with tree bases/stumps/bushes/poles
            if (l1Tile) {
                if (l1Tile == 19 || l1Tile == 20 || l1Tile == 31 || l1Tile == 32 || l1Tile == 48 || l1Tile == 67) {
                    waitCount(180);
                    break;
                }
            }

            // Check if on course or in heavy snow...changes speed
            if (l0Tile == 1 || l0Tile == 6) {
                inSnow = 1;
            } else {
                inSnow = 0;
            }

            move(&guyData, &scrollSpeed, inSnow);

            // Dead if off screen
            if (guyData.guyX > 640) {
                waitCount(180);
                    break;
            }

            scroll+= scrollSpeed>>4;

            VERA.layer0.vscroll = scroll;
            VERA.layer1.vscroll = scroll;

            // Timer
            ticks++;
            
            if (ticks == 60) {
                ticks = 0;
                secs++;
                if (secs == 60) {
                    secs = 0;
                    mins++;
                }
            }

            if (ticks == 15) {
                milli = 25;
                showTimer(mins, secs, milli);
            } else if (ticks == 30) {
                milli = 50;
                showTimer(mins, secs, milli);
            } else if (ticks == 45) {
                milli = 75;
                showTimer(mins, secs, milli);
            } else if (ticks == 0) {
                milli = 0;
                showTimer(mins, secs, milli);
            }

            
            wait();
        }
    }
}