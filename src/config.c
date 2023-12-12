#include <cx16.h>
#include <joystick.h>
#include <stdio.h>

#include "config.h"
#include "utils.h"
#include "wait.h"
#include "sound.h"
#include "joy.h"

extern unsigned char musicOn;
extern unsigned char sfxOn;

void init() {
    char *tileFilename = "tilemap.bin";
    char *palFilename = "tilemap.pal";

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

    loadFileToBankedRAM("c16l0.bin", TITLE_SCREEN_BANK_0, 0);
    loadFileToBankedRAM("c16l1.bin", TITLE_SCREEN_BANK_1, 0);

    loadFileToBankedRAM("c17l0.bin", INST_SCREEN_BANK_0, 0);
    loadFileToBankedRAM("c17l1.bin", INST_SCREEN_BANK_1, 0);
    
    soundInit();
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

void showTitleScreen() {
    loadFileToVRAM("c18l0.bin", L0_MAPBASE_ADDR);
    loadFileToVRAM("c18l1.bin", L1_MAPBASE_ADDR);
}

void showTitleBackground() {
    copyBankedRAMToVRAM(TITLE_SCREEN_BANK_0, L0_MAPBASE_ADDR, 4096);
    copyBankedRAMToVRAM(TITLE_SCREEN_BANK_1, L1_MAPBASE_ADDR, 4096);
}

void showInstructionsBackground() {
    copyBankedRAMToVRAM(INST_SCREEN_BANK_0, L0_MAPBASE_ADDR, 4096);
    copyBankedRAMToVRAM(INST_SCREEN_BANK_1, L1_MAPBASE_ADDR, 4096);
}

void clearLayers() {
    clearLayer0();
    clearLayer1();
}

void showCourseRow(unsigned char courseCount, unsigned char thisCourse, unsigned char selectedCourse, unsigned char row) {
    char buf[40];

    sprintf(buf, "%s %s %s %s",
        selectedCourse == 0 ? "-A-" : " A ",
        selectedCourse == 1 ? "-B-" : " B ",
        selectedCourse == 2 ? "-C-" : " C ",
        selectedCourse == 3 ? "-D-" : " D ");

    messageCenter(courseCount != thisCourse ? " A   B   C   D " : buf, row, row, 0, 0, 1);
}

void pickModes(unsigned char *zoomMode, unsigned char *gameMode, unsigned char *courseCount, unsigned char *course) {
    unsigned char joy, selection = 0;

    showTitleBackground();
    
    // Requires 640 mode
    setZoom(1);

    // Pick graphics mode
    messageCenter("CHOOSE SETTINGS", 8, 8, 0, 0, 1);
    messageCenter("JOYSTICK TO CHANGE", 9, 9, 0, 0, 1);
    messageCenter("PRESS BUTTON TO CONTINUE", 10, 10, 0, 0, 1);

    while (1) {
        joy = joy_read(0);

        if (JOY_UP(joy) || JOY_DOWN(joy)) {
            selection++;
            if (selection == 3) {
                selection = 0;
            }

            waitForRelease();
        }

        if (selection == 0 && (JOY_LEFT(joy) || JOY_RIGHT(joy))) {
            *zoomMode+=1;
            if (*zoomMode == 2) {
                *zoomMode = 0;
            }

            waitForRelease();
        }

        if (selection == 1 && (JOY_LEFT(joy) || JOY_RIGHT(joy))) {
            musicOn+=1;
            if (musicOn == 2) {
                musicOn = 0;
                soundStopChannel(SOUND_PRIORITY_MUSIC);
            } else {
                soundPlayMusic(SOUND_MUSIC_TITLE);
            }

            waitForRelease();
        }

        if (selection == 2 && (JOY_LEFT(joy) || JOY_RIGHT(joy))) {
            sfxOn+=1;
            if (sfxOn == 2) {
                sfxOn = 0;   
            }

            waitForRelease();
        }

        if (JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
            waitForRelease();
            break;
        }

        messageCenter(selection == 0 ? "::GRAPHICS MODE::" : "  GRAPHICS MODE  ", 13, 13, 0, 0, 1);
        messageCenter((*zoomMode) == 0
            ? "-320X240-  640X480 "
            : " 320X240  -640X480-"
            , 14, 14, 0, 0, 1);

        messageCenter(selection == 1 ? "::MUSIC TOGGLE::" : "  MUSIC TOGGLE  ", 17, 17, 0, 0, 1);
        messageCenter(musicOn == 1
            ? "-ON-  OFF "
            : " ON  -OFF-"
            , 18, 18, 0, 0, 1);

        messageCenter(selection == 2 ? "::SFX TOGGLE::" : "  SFX TOGGLE  ", 21, 21, 0, 0, 1);
        messageCenter(sfxOn == 1
            ? "-ON-  OFF "
            : " ON  -OFF-"
            , 22, 22, 0, 0, 1);

        wait();
    }

    // Pick game mode
    showTitleBackground();
    messageCenter("CHOOSE GAME MODE", 6, 6, 0, 0, 1);
    messageCenter("USE JOYSTICK TO SELECT", 7, 7, 0, 0, 1);
    messageCenter("PRESS BUTTON TO CONTINUE", 8, 8, 0, 0, 1);

    while (1) {
        joy = joy_read(0);

        if (JOY_UP(joy)) {
            if (*gameMode == 0) {
                *gameMode = 2;
            } else {
                *gameMode-=1;
            }

            waitForRelease();
        }

        if (JOY_DOWN(joy)) {
            *gameMode+=1;
            if (*gameMode == 3) {
                *gameMode = 0;
            }

            waitForRelease();
        }

        if (JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
            waitForRelease();
            break;
        }

        messageCenter((*gameMode) == GAME_MODE_FREE ? "::OPEN::" : "  OPEN  ", 11, 11, 0, 0, 1);
        messageCenter("TIME ONLY", 12, 12, 0, 0, 1);

        messageCenter((*gameMode) == GAME_MODE_FLAGS ? "::FLAGS::" : "  FLAGS  ", 14, 14, 0, 0, 1);
        messageCenter("SKI AROUND FLAGS", 15, 15, 0, 0, 1);
        
        messageCenter((*gameMode) == GAME_MODE_GATES ? "::GATES::" : "  GATES  ", 17, 17, 0, 0, 1);
        messageCenter("SKI BETWEEN GATES", 18, 18, 0, 0, 1);

        messageCenter("THERE IS A TIME", 20, 20, 0, 0, 1);
        messageCenter("PENALTY FOR CRASHES", 21, 21, 0, 0, 1);
        messageCenter("AND MISSING", 22, 22, 0, 0, 1);
        messageCenter("FLAGS AND GATES", 23, 23, 0, 0, 1);

        wait();
    }

    // Pick game mode
    showTitleBackground();
    messageCenter("COURSE SETTINGS", 7, 7, 0, 0, 1);
    messageCenter("UP-DOWN FOR LENGTH", 8, 8, 0, 0, 1);
    messageCenter("LEFT-RIGHT FOR COURSE", 9, 9, 0, 0, 1);

    while (1) {
        joy = joy_read(0);

        if (JOY_DOWN(joy)) {
            *courseCount+=1;
            if (*courseCount == 5) {
                *courseCount = 1;
            }

            waitForRelease();
        }

        if (JOY_UP(joy)) {
            *courseCount-=1;
            if (*courseCount == 0) {
                *courseCount = 4;
            }

            waitForRelease();
        }

        if (JOY_LEFT(joy)) {
            if (*course == 0) {
                *course = 3;
            } else {
                *course-=1;
            }

            waitForRelease();
        }

        if (JOY_RIGHT(joy)) {
            if (*course == 3) {
                *course = 0;
            } else {
                *course+=1;
            }

            waitForRelease();
        }

        if (JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
            waitForRelease();
            break;
        }

        messageCenter((*courseCount) == 1 ? "::SHORT::" : "  SHORT  ", 12, 12, 0, 0, 1);
        showCourseRow(*courseCount, 1, *course, 13);

        messageCenter((*courseCount) == 2 ? "::MEDIUM::" : "  MEDIUM  ", 15, 15, 0, 0, 1);
        showCourseRow(*courseCount, 2, *course, 16);
        
        messageCenter((*courseCount) == 3 ? "::LONG::" : "  LONG  ", 18, 18, 0, 0, 1);
        showCourseRow(*courseCount, 3, *course, 19);

        messageCenter((*courseCount) == 4 ? "::EPIC::" : "  EPIC  ", 21, 21, 0, 0, 1);
        showCourseRow(*courseCount, 4, *course, 22);

        wait();
    }

    showInstructionsBackground();
    waitForButtonPress();

    showTitleBackground();
}
