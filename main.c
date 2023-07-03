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

short scroll = 0, previousScroll = 0, scrollX = 0;
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
    unsigned char course = 0;

    init();
    messageCenter("WELCOME TO SKIING", 13);
    messageCenter("BY MARK WIEDMANN", 14);
    messageCenter("LOADING COURSES...", 16);

    loadCourses();
    waitCount(120);

    while(1) {            
        spritesConfig(&guyData);

        scroll = 0;
        scrollSpeed = 0;

        VERA.layer0.vscroll = scroll;
        VERA.layer1.vscroll = scroll;

        ticks = 0;
        mins = 0;
        secs = 0;
        milli = 0;

        showTimer(mins, secs, milli);

        // Load the top half of the starting course
        drawPartialCourse(course, 0);

        scrollX = guyData.guyX - 160;

        move(&guyData, scrollX, &scrollSpeed, inSnow);
        // timerSpritesMove(scrollX);

        VERA.layer0.vscroll = scroll;
        VERA.layer1.vscroll = scroll;
        VERA.layer0.hscroll = scrollX;
        VERA.layer1.hscroll = scrollX;
        
        message("GET", 6, 33);
        message("READY!", 7, 33);
        waitCount(120);
        message("GO!!!", 6, 33);
        message("      ", 7, 33);

        while(1) {
            getCollisionTiles(&l0Tile, &l1Tile);

            // Check for collision with tree bases/stumps/bushes/poles
            if (l1Tile) {
                if (l1Tile == 19 || l1Tile == 20 || l1Tile == 31 || l1Tile == 32 || l1Tile == 48 || l1Tile == 67) {
                    messageCenter("OUCH!!!", (scroll>>4) + 15);
                    waitCount(180);
                    course = 0;
                    break;
                }
            }

            // Check if on course or in heavy snow...changes speed
            if (l0Tile == 1 || l0Tile == 6) {
                inSnow = 1;
            } else {
                inSnow = 0;
            }

            move(&guyData, scrollX, &scrollSpeed, inSnow);

            // Dead if off screen
            if (guyData.guyX > 640) {
                messageCenter("STAY ON COURSE!!!", (scroll>>4) + 15);
                waitCount(180);
                break;
            }

            scroll+= scrollSpeed>>4;
            // Check max scroll value
            if (scroll > 4095) {
                scroll = scroll - 4095;
            }

            // See if we hit any breakpoints where we need to load more course data
            if (scroll >= 1024 && previousScroll < 1024) {
                drawPartialCourse(course, 1);
            } else if (scroll >= 3072 && previousScroll < 3072) {
                course++;
                if (course == COURSE_COUNT) {
                    course = 0;
                }
                drawPartialCourse(course, 0);
            }

            previousScroll = scroll;

            scrollX = guyData.guyX - 160;

            if (scrollX < 0) {
                scrollX = 0;
            } else if (scrollX > 320) {
                scrollX = 320;
            }
            
            VERA.layer0.vscroll = scroll;
            VERA.layer1.vscroll = scroll;
            VERA.layer0.hscroll = scrollX;
            VERA.layer1.hscroll = scrollX;

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