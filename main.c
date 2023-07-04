#include <cx16.h>
#include <cbm.h>
#include <string.h>
#include <stdio.h>
#include <joystick.h>

#include "config.h"
#include "course.h"
#include "utils.h"
#include "wait.h"
#include "sprites.h"

GuyData guyData;

short scrollY = 0, previousScroll = 0, scrollX = 0;
unsigned char lastTileX = 0, lastTileY = 0, zoomMode = 1;

void getCollisionTiles(unsigned char *l0Tile, unsigned char *l1Tile) {
    unsigned char tileX, tileY;
    unsigned long tileAddr;

    // Get the bottom center of the sprite (his feet) for collision
    tileX = (guyData.guyX+8)>>4;
    tileY = (scrollY + guyData.guyY + 8)>>4;

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

void showTitle() {
    clearLayers();
    
    // Requires 640 mode
    setZoom(1);

    messageCenter("WELCOME TO SKIING", 6, 13, scrollX, scrollY, 1);
    messageCenter("BY MARK WIEDMANN", 7, 14, scrollX, scrollY, 1);
    messageCenter("LOADING COURSES...", 9, 16, scrollX, scrollY, 1);
}

void pickMode() {
    unsigned char joy;

    zoomMode = 0;

    clearLayers();
    
    // Requires 640 mode
    setZoom(1);

    messageCenter("CHOOSE GRAPHICS MODE", 5, 11, scrollX, scrollY, 1);
    messageCenter("USE JOYSTICK TO SELECT", 6, 12, scrollX, scrollY, 1);

    while (1) {
        joy = joy_read(0);

        if (JOY_UP(joy) || JOY_DOWN(joy)) {
            zoomMode++;
            if (zoomMode == 2) {
                zoomMode = 0;
            }

            waitCount(15);
        }

        if (JOY_BTN_1(joy)) {
            break;
        }

        messageCenter(zoomMode == 0 ? "::320*240::" : "  320*240  ", 7, 14, scrollX, scrollY, 1);
        messageCenter("ZOOMED IN ACTION FEEL", 8, 15, scrollX, scrollY, 1);
        
        messageCenter(zoomMode == 1 ? "::640*480::" : "  640*480  ", 10, 17, scrollX, scrollY, 1);
        messageCenter("WIDER FIELD OF VIEW", 11, 18, scrollX, scrollY, 1);

        wait();
    }
}

void main() {
    unsigned char l0Tile;
    unsigned char l1Tile;
    unsigned char inSnow = 0;
    unsigned short scrollSpeed, scrollLimit, halfScrollLimit;
    unsigned char mins, secs, ticks, milli;
    unsigned char course = 0;

    init();
    showTitle();

    loadCourses();
    // waitCount(120);
  
    while(1) {
        spritesConfig(&guyData, 0, 0); // hide sprites
        pickMode();
        setZoom(zoomMode);

        scrollLimit = zoomMode == 0 ? 320 : 0;
        halfScrollLimit = zoomMode == 0 ? 160 : 0;

        spritesConfig(&guyData, zoomMode, 1); // Show the sprites

        scrollY = 0;
        scrollX = zoomMode == 0 ? guyData.guyX - halfScrollLimit : 0;
        scrollSpeed = 0;

        VERA.layer0.vscroll = scrollY;
        VERA.layer1.vscroll = scrollY;
        VERA.layer0.hscroll = scrollX;
        VERA.layer1.hscroll = scrollX;

        ticks = 0;
        mins = 0;
        secs = 0;
        milli = 0;

        showTimer(mins, secs, milli);

        // Load the top half of the starting course
        drawPartialCourse(course, 0);

        move(&guyData, scrollX, &scrollSpeed, inSnow);
        
        if (zoomMode == 0) {
            message("GET", 6, 13, scrollX, scrollY);
            message("READY!", 7, 13, scrollX, scrollY);
        } else {
            message("GET", 6, 33, scrollX, scrollY);
            message("READY!", 7, 33, scrollX, scrollY);
        }

        waitCount(120);

        if (zoomMode == 0) {
            message("GO!!!", 6, 13, scrollX, scrollY);
            message("      ", 7, 13, scrollX, scrollY);
        } else {
            message("GO!!!", 6, 33, scrollX, scrollY);
            message("      ", 7, 33, scrollX, scrollY);
        } 

        while(1) {
            getCollisionTiles(&l0Tile, &l1Tile);

            // Check for collision with tree bases/stumps/bushes/poles
            if (l1Tile) {
                if (l1Tile == 19 || l1Tile == 20 || l1Tile == 31 || l1Tile == 32 || l1Tile == 48 || l1Tile == 67) {
                    messageCenter("OUCH!!!", 7, 15, scrollX, scrollY, zoomMode);
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
                messageCenter("STAY ON COURSE!!!", 7, 15, scrollX, scrollY, zoomMode);
                waitCount(180);
                break;
            }

            scrollY+= scrollSpeed>>4;
            // Check max scroll value
            if (scrollY > 4095) {
                scrollY = scrollY - 4095;
            }

            // See if we hit any breakpoints where we need to load more course data
            if (scrollY >= 1024 && previousScroll < 1024) {
                drawPartialCourse(course, 1);
            } else if (scrollY >= 3072 && previousScroll < 3072) {
                course++;
                if (course == COURSE_COUNT) {
                    course = 0;
                }
                drawPartialCourse(course, 0);
            }

            previousScroll = scrollY;

            scrollX = guyData.guyX - halfScrollLimit;

            if (scrollX < 0) {
                scrollX = 0;
            } else if (scrollX > scrollLimit) {
                scrollX = scrollLimit;
            }
            
            VERA.layer0.vscroll = scrollY;
            VERA.layer1.vscroll = scrollY;
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