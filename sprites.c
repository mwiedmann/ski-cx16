#include <cx16.h>
#include <joystick.h>

#include "sprites.h"
#include "config.h"

void spritesConfig(GuyData *guyData) {
    // VRAM address for sprite 1 (this is fixed)
    unsigned long spriteGraphicAddress = TILEBASE_ADDR + (72 * 256);
    guyData->guyX = 480;
    guyData->guyY = 40;
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
    VERA.data0 = 0b00001000; // Z-Depth=2
    VERA.data0 = 0b01010000; // 64x64 pixel image
}

void move(GuyData *guyData, unsigned short *scrollSpeed, unsigned char inSnow) {
    unsigned char joy;
    unsigned short scrollInc = 0, scrollMax;
    short moveMax;

    moveMax = inSnow ? GUY_MOVE_X_SNOW_MAX : GUY_MOVE_X_MAX;

    joy = joy_read(0);

    // Steeper angle moves faster
    // if (guyMoveX > 48 || guyMoveX < -48) {
    //     scrollInc = inSnow ? 1 : 2;
    //     scrollMax = inSnow ? 16 : 32;
    // } else if (guyMoveX > 32 || guyMoveX < -32) {
    //     scrollInc = inSnow ? 1 : 3;
    //     scrollMax = inSnow ? 16 : 32;
    // } else if (guyMoveX > 16 || guyMoveX < -16) {
    //     scrollInc = inSnow ? 2 : 3;
    //     scrollMax = inSnow ? 16 : 48;
    // } else {
    //     scrollInc = inSnow ? 2 : 3;
    //     scrollMax = inSnow ? 16 : 64;
    // }

    if (guyData->guyMoveX > 42 || guyData->guyMoveX < -42) {
        scrollInc = inSnow ? 1 : 2;
        scrollMax = inSnow ? 16 : 16;
    } else if (guyData->guyMoveX > 21 || guyData->guyMoveX < -21) {
        scrollInc = inSnow ? 1 : 3;
        scrollMax = inSnow ? 16 : 32;
    } else {
        scrollInc = inSnow ? 2 : 3;
        scrollMax = inSnow ? 16 : 48;
    }

    // TESTING - Uncomment to always scroll slow
    // scrollMax = 16;

    *scrollSpeed += scrollInc;
    if (*scrollSpeed > scrollMax) {
        *scrollSpeed = scrollMax;
    }

    if (JOY_LEFT(joy)) {
        guyData->guyMoveX-= inSnow ? 1 : 2;
        if (guyData->guyMoveX < -moveMax) {
            guyData->guyMoveX = -moveMax;
        }
    } else if (JOY_RIGHT(joy)) {
        guyData->guyMoveX+= inSnow ? 1 : 2;
        if (guyData->guyMoveX > moveMax) {
            guyData->guyMoveX = moveMax;
        }
    }
    
    guyData->guyX+= guyData->guyMoveX>>4;

    // Update Sprite 1 X/Y Position
    // Point to Sprite 1 byte 2
    VERA.address = SPRITE1_ADDR+2;
    VERA.address_hi = SPRITE1_ADDR>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;
    
    // Set the X and Y values
    VERA.data0 = guyData->guyX;
    VERA.data0 = guyData->guyX>>8;
    VERA.data0 = guyData->guyY;
    VERA.data0 = guyData->guyY>>8;
}
