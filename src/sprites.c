#include <cx16.h>
#include <joystick.h>

#include "sprites.h"
#include "config.h"
#include "utils.h"

// While trying to turn left/right, how many frames before the X move changes
#define FRAMES_PER_GUY_TURN 8

// If the player is not at max speed, it takes a few frames to accelerate
#define FRAMES_TO_ACCELERATE 20

signed char pushCount = 0, pushDir = 0, scrollCount = 0;

void spriteText(char* msg, unsigned char row) {
    unsigned short i, tile;

    for (i=0; i<SPRITE_TEXT_COUNT; i++) {
        if (msg[i] == 0) {
            break;
        }

        // Point to Sprite
        VERA.address = SPRITE_TEXT_ADDR + (row * SPRITE_TEXT_COUNT * 8) + (i * 8);
        VERA.address_hi = (SPRITE_TEXT_ADDR + (row * SPRITE_TEXT_COUNT * 8) + (i * 8))>>16;
        // Set the Increment Mode, turn on bit 4
        VERA.address_hi |= 0b10000;
        
        tile = getTextTile(msg[i]);

        VERA.data0 = (TILEBASE_ADDR + (tile * 256))>>5;
        VERA.data0 = 0b10000000 | (TILEBASE_ADDR + (tile * 256))>>13;
    }
}

void timerSprites(unsigned char zoomMode, unsigned char show) {
    unsigned char x, y;

    // Point to Sprite 2
    VERA.address = SPRITE_TEXT_ADDR;
    VERA.address_hi = SPRITE_TEXT_ADDR>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Configure Sprite 1

    for (y=0; y<2; y++) {
        for (x=0; x<12; x++) {
            // Graphic address bits 12:5
            VERA.data0 = TILESET_DIGITS_ADDR>>5;
            // 256 color mode, and graphic address bits 16:13
            VERA.data0 = 0b10000000 | TILESET_DIGITS_ADDR>>13;
            VERA.data0 = (zoomMode == 0 ? SPRITE_TEXT_X_320 : SPRITE_TEXT_X_640) + (x*16);
            VERA.data0 = ((zoomMode == 0 ? SPRITE_TEXT_X_320 : SPRITE_TEXT_X_640) + (x*16))>>8;
            VERA.data0 = SPRITE_TEXT_Y + (y*16);
            VERA.data0 = (SPRITE_TEXT_Y + (y*16))>>8;
            VERA.data0 = show ? 0b00001100 : 0; // Z-Depth=3 (or 0 to hide)
            VERA.data0 = 0b01010000; // 16x16 pixel image
        }
    }

    spriteText("     0:00.00", 0);
    spriteText("            ", 1);
}

void shadowSprite(GuyData *guyData, unsigned char show) {
    unsigned long spriteGraphicAddress = TILEBASE_ADDR + (108 * 256);
    VERA.address = SPRITE_SHADOW_ADDR;
    VERA.address_hi = SPRITE_SHADOW_ADDR>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Graphic address bits 12:5
    VERA.data0 = spriteGraphicAddress>>5;
    // 256 color mode, and graphic address bits 16:13
    VERA.data0 = 0b10000000 | spriteGraphicAddress>>13;
    VERA.data0 = guyData->guyX;
    VERA.data0 = guyData->guyX>>8;
    VERA.data0 = guyData->guyY+64;
    VERA.data0 = (guyData->guyY+64)>>8;
    VERA.data0 = show ? 0b00001000 : 0;
    VERA.data0 = 0b01010000; // 16x16 pixel image
}

void spritesConfig(GuyData *guyData, unsigned char zoomMode, unsigned char show) {
    // VRAM address for sprite 1 (this is fixed)
    unsigned long spriteGraphicAddress = TILEBASE_ADDR + (72 * 256);
    guyData->guyX = 480;
    guyData->guyY = 32;
    guyData->guyMoveX = 0;
    guyData->guyMoveY = 0;
    guyData->jumping = 0;
    guyData->jumpCount = 0;

    // Point to Sprite 1
    VERA.address = SPRITE1_ADDR;
    VERA.address_hi = SPRITE1_ADDR>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Configure Sprite 1

    // Graphic address bits 12:5
    VERA.data0 = spriteGraphicAddress>>5;
    // 256 color mode, and graphic address bits 16:13
    VERA.data0 = 0b10000000 | spriteGraphicAddress>>13;
    VERA.data0 = guyData->guyX;
    VERA.data0 = guyData->guyX>>8;
    VERA.data0 = guyData->guyY;
    VERA.data0 = guyData->guyY>>8;
    VERA.data0 = show ? 0b00001000 : 0; // Z-Depth=2 (or 0 to hide)
    VERA.data0 = 0b01010000; // 16x16 pixel image

    timerSprites(zoomMode, show);
}

void moveSprite(unsigned long spriteAddr, unsigned short x, unsigned short y) {
    // Update Sprite 1 X/Y Position
    // Point to Sprite 1 byte 2
    VERA.address = spriteAddr+2;
    VERA.address_hi = spriteAddr>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;
    
    // Set the X and Y values
    VERA.data0 = x;
    VERA.data0 = x>>8;
    VERA.data0 = y;
    VERA.data0 = y>>8;
}

void move(GuyData *guyData, short scrollX, unsigned short *scrollSpeed, unsigned char inSnow) {
    unsigned char joy;
    unsigned short scrollInc = 0, scrollMax;
    short moveMax;

    if (guyData->jumpCount > 0) {
        guyData->jumpCount--;
        if (guyData->jumpCount == 0) {
            guyData->jumping = 0;
            shadowSprite(guyData, 0);
        }
    }

    if (!guyData->jumping) {
        moveMax = inSnow ? GUY_MOVE_X_SNOW_MAX : GUY_MOVE_X_MAX;

        joy = joy_read(0);

        // While holding L/R to turn guy, track how many frames the player holds L/R
        // We don't want to turn them more until FRAMES_PER_GUY_TURN have past
        if (JOY_LEFT(joy)) {
            if (pushDir != -1) {
                pushDir = -1;
                pushCount = 0;
            }
            pushCount++;
        } else if (JOY_RIGHT(joy)) {
            if (pushDir != 1) {
                pushDir = 1;
                pushCount = 0;
            }
            pushCount++;
        } else {
            pushDir = 0;
            pushCount = 0;
        }

        // See if enough frames have passed to turn the guy more
        if (pushCount >= FRAMES_PER_GUY_TURN) {
            guyData->guyMoveX += pushDir;
            pushCount = 0;
        }

        // Limit the X move to 2 pixels per frame
        if (guyData->guyMoveX > 3) {
            guyData->guyMoveX = 3;
        } else if (guyData->guyMoveX < -3) {
            guyData->guyMoveX = -3;
        }
    }

    // Move the guy
    guyData->guyX+= guyData->guyMoveX;

    // *** Downhill speed ***

    if (!guyData->jumping) {
        // Deep snow always slows the player to their min speed
        if (inSnow) {
            scrollMax = 1;
        } else {
            // The sharper the player is turning, the slower their speed downhill
            if (guyData->guyMoveX >= 2 || guyData->guyMoveX <= -2) {
                scrollMax = 2;
            } else if (guyData->guyMoveX == 1 || guyData->guyMoveX == -1) {
                scrollMax = 3;
            } else {
                scrollMax = 4;
            }
        }

        // At correct speed
        if (*scrollSpeed == scrollMax) {
            // just reset the counter so it is fresh when acceleration happens
            scrollCount = 0;
        } else if (*scrollSpeed > scrollMax) { 
            // Over max speed, probably hit snow or turned
            *scrollSpeed = scrollMax;
            scrollCount = 0;
        } else {
            // Less than max speed...accelerate
            // Takes some number of frames to hit next speed
            scrollCount++;
            if (scrollCount >= FRAMES_TO_ACCELERATE) {
                scrollCount = 0;
                *scrollSpeed = *scrollSpeed + 1;
            }
        }
    }

    moveSprite(SPRITE1_ADDR, guyData->guyX-scrollX, guyData->guyY);
}
