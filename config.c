#include <cx16.h>
#include <joystick.h>

#include "config.h"
#include "utils.h"
#include "wait.h"

void init() {
    unsigned char *tileFilename = "tilemap.bin";
    unsigned char *palFilename = "tilemap.pal";

    // Configure the joysticks
    joy_install(cx16_std_joy);

    // Enable both layers
    VERA.display.video = 0b11110001;

    // With 16 pixel tiles, we don't need as many tiles (might need more later for scrolling)
    // Only 640/16 = 40, 480/16=30 (40x30 tile resolution now)
    // Set the Map Height=0 (32), Width=1 (64) // NOW 256x64
    // Set Color Depth to 8 bpp mode
    VERA.layer1.config = 0b11010011;
    VERA.layer0.config = 0b11010011;

    // Get bytes 16-9 of the MapBase addresses and set on both layers
    VERA.layer0.mapbase = L0_MAPBASE_ADDR>>9;
    VERA.layer1.mapbase = L1_MAPBASE_ADDR>>9;

    // Layers can share a tilebase (use the same tiles)
    // Get bytes 16-11 of the new TileBase address
    // ALSO Set Tile W/H (bits 0/1) to 1 for 16 pixel tiles
    VERA.layer0.tilebase = TILEBASE_ADDR>>9 | 0b11;
    VERA.layer1.tilebase = TILEBASE_ADDR>>9 | 0b11;

    loadFileToVRAM(palFilename, PALETTE_ADDR);
    loadFileToVRAM(tileFilename, TILEBASE_ADDR);
}

void setZoom(unsigned char zoomMode) {
    VERA.display.hscale = zoomMode == 0 ? 64 : 128;
    VERA.display.vscale = zoomMode == 0 ? 64 : 128;
}

void clearLayer0() {
    // Note we need a `short` here because there are more than 255 tiles
    unsigned short i;
    
    // Clear layer 0
    VERA.address = L0_MAPBASE_ADDR;
    VERA.address_hi = L0_MAPBASE_ADDR>>16;
    // Always set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Empty tiles
    for (i=0; i<MAPBASE_TILE_COUNT; i++) {
        VERA.data0 = 0;
        VERA.data0 = 0;
    }
}

void clearLayer1() {
    // Note we need a `short` here because there are more than 255 tiles
    unsigned short i;

    // Clear layer 1
    VERA.address = L1_MAPBASE_ADDR;
    VERA.address_hi = L1_MAPBASE_ADDR>>16;
    // Always set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Empty tiles
    for (i=0; i<MAPBASE_TILE_COUNT; i++) {
        VERA.data0 = 0;
        VERA.data0 = 0;
    }
}

void clearLayers() {
    clearLayer0();
    clearLayer1();
}

void pickModes(unsigned char *zoomMode, unsigned char *gameMode, unsigned char *courseCount) {
    unsigned char joy;

    clearLayers();
    
    // Requires 640 mode
    setZoom(1);

    // Pick graphics mode
    messageCenter("CHOOSE GRAPHICS MODE", 5, 11, 0, 0, 1);
    messageCenter("USE JOYSTICK TO SELECT", 6, 12, 0, 0, 1);

    while (1) {
        joy = joy_read(0);

        if (JOY_UP(joy) || JOY_DOWN(joy)) {
            *zoomMode+=1;
            if (*zoomMode == 2) {
                *zoomMode = 0;
            }

            while(JOY_UP(joy) || JOY_DOWN(joy)) {
                wait();
                joy = joy_read(0);
            }
        }

        if (JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
            while(JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
                wait();
                joy = joy_read(0);
            }
            break;
        }

        messageCenter((*zoomMode) == 0 ? "::320X240::" : "  320X240  ", 7, 14, 0, 0, 1);
        messageCenter("ZOOMED IN ACTION FEEL", 8, 15, 0, 0, 1);
        
        messageCenter((*zoomMode) == 1 ? "::640X480::" : "  640X480  ", 10, 17, 0, 0, 1);
        messageCenter("BIGGER FIELD OF VIEW", 11, 18, 0, 0, 1);

        wait();
    }

    // Pick game mode
    clearLayers();
    messageCenter("CHOOSE GAME MODE", 5, 7, 0, 0, 1);
    messageCenter("USE JOYSTICK TO SELECT", 6, 8, 0, 0, 1);

    while (1) {
        joy = joy_read(0);

        if (JOY_UP(joy) || JOY_DOWN(joy)) {
            *gameMode+=1;
            if (*gameMode == 2) {
                *gameMode = 0;
            }

            while(JOY_UP(joy) || JOY_DOWN(joy)) {
                wait();
                joy = joy_read(0);
            }
        }

        if (JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
            while(JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
                wait();
                joy = joy_read(0);
            }
            break;
        }

        messageCenter((*gameMode) == GAME_MODE_FREE ? "::OPEN::" : "  OPEN  ", 7, 10, 0, 0, 1);
        messageCenter("TIME ONLY", 8, 11, 0, 0, 1);

        messageCenter((*gameMode) == GAME_MODE_FLAGS ? "::FLAGS::" : "  FLAGS  ", 10, 13, 0, 0, 1);
        messageCenter("SKI AROUND FLAGS", 11, 14, 0, 0, 1);
        
        messageCenter((*gameMode) == GAME_MODE_GATES ? "::GATES::" : "  GATES  ", 13, 16, 0, 0, 1);
        messageCenter("COMING SOON!", 14, 17, 0, 0, 1);

        messageCenter("THERE IS A 5 SEC TIME", 16, 19, 0, 0, 1);
        messageCenter("PENALTY FOR MISSING", 17, 20, 0, 0, 1);
        messageCenter("FLAGS AND GATES", 18, 21, 0, 0, 1);

        wait();
    }

    // Pick game mode
    clearLayers();
    messageCenter("CHOOSE RUN LENGTH", 5, 5, 0, 0, 1);
    messageCenter("USE JOYSTICK TO SELECT", 6, 6, 0, 0, 1);

    while (1) {
        joy = joy_read(0);

        if (JOY_DOWN(joy)) {
            *courseCount+=1;
            if (*courseCount == 5) {
                *courseCount = 1;
            }

            while(JOY_DOWN(joy)) {
                wait();
                joy = joy_read(0);
            }
        }

        if (JOY_UP(joy)) {
            *courseCount-=1;
            if (*courseCount == 0) {
                *courseCount = 4;
            }

            while(JOY_UP(joy)) {
                wait();
                joy = joy_read(0);
            }
        }

        if (JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
            while(JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
                wait();
                joy = joy_read(0);
            }
            break;
        }

        messageCenter((*courseCount) == 1 ? "::SHORT::" : "  SHORT  ", 7, 8, 0, 0, 1);
        messageCenter("1 COURSE", 8, 9, 0, 0, 1);

        messageCenter((*courseCount) == 2 ? "::MEDIUM::" : "  MEDIUM  ", 10, 11, 0, 0, 1);
        messageCenter("2 COURSES", 11, 12, 0, 0, 1);
        
        messageCenter((*courseCount) == 3 ? "::LONG::" : "  LONG  ", 13, 14, 0, 0, 1);
        messageCenter("3 COURSES", 14, 15, 0, 0, 1);

        messageCenter((*courseCount) == 4 ? "::EPIC::" : "  EPIC  ", 16, 17, 0, 0, 1);
        messageCenter("4 COURSES", 17, 18, 0, 0, 1);

        messageCenter("LONGER RUNS MAY INCLUDE", 16, 20, 0, 0, 1);
        messageCenter("REPEATED COURSES UNTIL", 16, 21, 0, 0, 1);
        messageCenter("MORE COURSES ARE AVAILABLE", 16, 22, 0, 0, 1);

        wait();
    }
}

void waitForButtonPress() {
    unsigned char joy;

    while(1) {
        joy = joy_read(0);

        if (JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
            while(JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
                wait();
                joy = joy_read(0);
            }
            break;
        }
    }
}