#include <cx16.h>
#include <joystick.h>

#include "sprites.h"
#include "config.h"

signed char pushCount = 0, pushDir = 0;

void spriteText(unsigned char* msg, unsigned char row) {
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
        
        // "0123456789:ABCXYZ!#$%+-=."
        // We could reorganize the tile image to have the characters in PETSCII order
        // but this is fine for now
        tile =  msg[i] >= '0' && msg[i]<= ':'
            ? TILE_CHARS_START + (msg[i]-'0') // Numbers and :
            : msg[i] >= 'A' && msg[i]<= 'Z'
                ? TILE_CHARS_START + 12 + (msg[i]-'A') // Letters
                : msg[i] == '!'
                    ? TILE_CHARS_START + 38
                    : msg[i] == '#'
                        ? TILE_CHARS_START + 39
                        : msg[i] == '$'
                            ? TILE_CHARS_START + 40
                            : msg[i] == '%'
                                ? TILE_CHARS_START + 41
                                : msg[i] == '+'
                                    ? TILE_CHARS_START + 42
                                    : msg[i] == '-'
                                        ? TILE_CHARS_START + 43
                                        : msg[i] == '='
                                            ? TILE_CHARS_START + 46
                                            : msg[i] == '.'
                                                ? TILE_CHARS_START + 47
                                                : 0;

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

void spritesConfig(GuyData *guyData, unsigned char zoomMode, unsigned char show) {
    // VRAM address for sprite 1 (this is fixed)
    unsigned long spriteGraphicAddress = TILEBASE_ADDR + (72 * 256);
    guyData->guyX = 480;
    guyData->guyY = 32;
    guyData->guyMoveX = 0;
    guyData->guyMoveY = 0;

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

void move(GuyData *guyData, short scrollX, unsigned short *scrollSpeed, unsigned char inSnow) {
    unsigned char joy;
    unsigned short scrollInc = 0, scrollMax;
    short moveMax;

    moveMax = inSnow ? GUY_MOVE_X_SNOW_MAX : GUY_MOVE_X_MAX;

    joy = joy_read(0);

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

    if (pushCount == 7) {
        guyData->guyMoveX += pushDir;
        pushCount = 0;
    }

    if (guyData->guyMoveX > 3) {
        guyData->guyMoveX = 3;
    } else if (guyData->guyMoveX < -3) {
        guyData->guyMoveX = -3;
    }

    if (guyData->guyMoveX == 3 || guyData->guyMoveX == -3) {
        scrollMax = 1;
    } else if (guyData->guyMoveX == 2 || guyData->guyMoveX == -2) {
        scrollMax = inSnow ? 1 : 2;
    } else if (guyData->guyMoveX == 1 || guyData->guyMoveX == -1) {
        scrollMax = inSnow ? 1 : 3;
    } else {
        scrollMax = inSnow ? 1 : 4;
    }
    
    *scrollSpeed = scrollMax;

    guyData->guyX+= guyData->guyMoveX;

    // Update Sprite 1 X/Y Position
    // Point to Sprite 1 byte 2
    VERA.address = SPRITE1_ADDR+2;
    VERA.address_hi = SPRITE1_ADDR>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;
    
    // Set the X and Y values
    VERA.data0 = guyData->guyX-scrollX;
    VERA.data0 = (guyData->guyX-scrollX)>>8;
    VERA.data0 = guyData->guyY;
    VERA.data0 = guyData->guyY>>8;
}
