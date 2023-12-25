#include <cx16.h>
#include <cbm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <joystick.h>

#include "config.h"
#include "course.h"
#include "utils.h"
#include "wait.h"
#include "sprites.h"
#include "scores.h"
#include "sound.h"
#include "joy.h"

#define MISSED_FLAG_PENALTY_TICKS 300
#define CRASH_PENALTY_TICKS 600
#define COLLISION_MOVE_BACK_PIXELS 64
#define JUMP_COUNT 60
#define RED_FLAG 21
#define BLUE_FLAG 22

GuyData guyData;

short scrollY = 0, previousScroll = 0, scrollX = 0;
unsigned char lastTileX = 0, lastTileY = 0, zoomMode = 1, gameMode = 0;
unsigned short halfScrollLimit;

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

void getSafeSpot() {
    unsigned char l0Tile = 0, l1Tile = 0;
    signed short dir = 16;
    unsigned short orgX;

    orgX = guyData.guyX;

    // Move player a bit back after a collision
    scrollY-= COLLISION_MOVE_BACK_PIXELS;

    // Go from left to right and find a safe tile for the player
    do {
        getCollisionTiles(&l0Tile, &l1Tile);
        
        if ((l1Tile != RED_NET && l1Tile != BLUE_NET && l1Tile != GREEN_TREE_BASE && l1Tile != DEAD_TREE_BASE && l1Tile != GREEN_MINI_TREE &&
        l1Tile != GREEN_MINI_DEAD_TREE && l1Tile != STUMP && l1Tile != POLE && l1Tile != ROCK)
        &&
        // Fast snow tiles
        (l0Tile == 3 || l0Tile == 4 || l0Tile ==  27 || l0Tile == 28 || l0Tile == 39 || l0Tile == 40)) {
            break;
        }

        // Place the player to this potentially safe spot
        guyData.guyX+= dir;

        // If reached the edge, move back to original X and check the other direction for safe spots
        if (guyData.guyX>639) {
            guyData.guyX = orgX;
            if (dir < 0) {
                // No safe spot found, just move up
                break;   
            }
            dir *= -1;
        }
    } while(1);
    
    guyData.guyMoveX = 0;

    // TODO: These are efficiency helpers on getCollisionTiles
    // They need to be reset but this feels bad.
    lastTileX = 0;
    lastTileY = 0;

    // Update the left/right scroll if needed
    scrollX = zoomMode == 0 ? guyData.guyX - halfScrollLimit : 0;
}

void showTimer(unsigned char mins, unsigned char secs, unsigned char milli, unsigned char missed) {
    char msg[24], buf[24];

    sprintf(buf, "%u:%02u.%02u", mins, secs, milli);
    sprintf(msg, "%12s", buf);
    spriteText(msg, 0);

    sprintf(buf, "MISS:%02u", missed);
    sprintf(msg, "%12s", buf);
    spriteText(msg, 1);
}

void showTitle() {
    clearLayers();
    
    // Requires 640 mode
    setZoom(1);

    showTitleScreen();
}

void setScroll() {
    VERA.layer0.vscroll = scrollY;
    VERA.layer1.vscroll = scrollY;
    VERA.layer0.hscroll = scrollX;
    VERA.layer1.hscroll = scrollX;
}

void startingMessage(unsigned char zoomMode) {
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
}

void finialTimerUpdate(unsigned char ticks, unsigned char *milli) {
    *milli = (ticks*100)/60;
}

void refreshTimerFromTicks(unsigned short totalTicks, unsigned char *mins, unsigned char *secs, unsigned char *ticks, unsigned char *milli) {
    *mins = totalTicks / 3600;
    totalTicks-= (*mins) * 3600;

    *secs = totalTicks / 60;
    totalTicks -= (*secs) * 60;

    *ticks = totalTicks;

    *milli = (totalTicks*100)/60;
}

unsigned char checkQuit(unsigned char joy) {
    char save1[MAPBASE_TILE_WIDTH*2], save2[MAPBASE_TILE_WIDTH*2];
    unsigned char joy2, result;

    if (JOY_SELECT(joy)) {
        messageCenterSave("SELECT-SHFT TO QUIT", 7, 14, scrollX, scrollY, zoomMode, save1);
        messageCenterSave("START-RET TO RESUME", 8, 15, scrollX, scrollY, zoomMode, save2);

        waitForRelease();

        while(1) {
            joy2 = joy_read(0) | joy_read(1);
            if (JOY_SELECT(joy2)) {
                waitForRelease();
                result = 1;
                break;
            } else if (JOY_START(joy2)) {
                waitForRelease();
                result = 0;
                break;
            }

            wait();
        }

        restoreRow(save1, 7, 14, scrollY, zoomMode);
        restoreRow(save2, 7, 15, scrollY, zoomMode);

        return result;
    }
}

void main() {
    unsigned char l0Tile, l1Tile, inSnow, joy;
    unsigned short scrollSpeed, scrollLimit, totalTicks;
    unsigned char mins, secs, ticks, milli, missed, madeIt;
    unsigned char course, selectedCourse = 0, courseIndex;
    unsigned char courseCount = 1;
    unsigned char flagNum;
    char save[MAPBASE_TILE_WIDTH*2];

    FlagTrackingList *flagsCurrent = 0, *flagsNext = 0;

    init();
    showTitle();
    messageCenter("LOADING...", 20, 20, 0, 0, 1);
    message("VERSION 1.2", 28, 28, 0, 0);

    loadCourses();
    soundLoadMusic(SOUND_MUSIC_TITLE);
    waitCount(90); // Need this pause or the music stutters for some reason
    soundPlayMusic(SOUND_MUSIC_TITLE);

    messageCenter("PRESS Z OR BUTTON", 20, 20, 0, 0, 1);
    messageCenter("TO START", 21, 21, 0, 0, 1);
    waitForButtonPress();

    while(1) {
        // Reset scrolling
        scrollY = 0;
        scrollX = 0;
        setScroll();

        flagNum = 0;

        spritesConfig(&guyData, 0, 0); // hide sprites

        // Pick the game and graphics mode and set the zoom level accordingly
        courseIndex = 0;
        course = selectedCourse;
        pickModes(&zoomMode, &gameMode, &courseCount, &course);
        selectedCourse = course;
        
        // Show the high scores for this mode
        displayScores(gameMode, courseCount, selectedCourse, scrollX, scrollY, 65535U);

        soundPlayMusic(SOUND_MUSIC_CHILL);

        // Set the zoom level
        setZoom(zoomMode);

        inSnow = 0;

        // Set scroll limits for the lo-res (320x240) graphics mode
        scrollLimit = zoomMode == 0 ? 320 : 0;
        halfScrollLimit = zoomMode == 0 ? 160 : 0;

        spritesConfig(&guyData, zoomMode, 1); // Show the sprites

        // Set the inital scroll position based on the starting position of the guy
        scrollX = zoomMode == 0 ? guyData.guyX - halfScrollLimit : 0;
        scrollSpeed = 0;
        setScroll();

        // Reset the timer
        totalTicks = 0;
        ticks = 0;
        mins = 0;
        secs = 0;
        milli = 0;
        missed = 0;

        madeIt = 0;

        showTimer(mins, secs, milli, missed);

        // Load the top half of the starting course
        flagsCurrent = drawPartialCourse(course, 0, 1, gameMode);

        // Move the player into positon
        move(&guyData, scrollX, &scrollSpeed, inSnow, 0);
        
        // Show the GET READY...GO...
        startingMessage(zoomMode);

        // Main game loop
        while(1) {
            joy = joy_read(0) | joy_read(1);

            // Player can pause and quit the game
            if (checkQuit(joy)) {
                break;
            }

            // Get the tiles on each layer the guy is currently touching
            getCollisionTiles(&l0Tile, &l1Tile);

            if (!guyData.jumping) {
                if (l0Tile == JUMP_1 || l0Tile == JUMP_2) {
                    guyData.jumping = 1;
                    guyData.jumpCount = JUMP_COUNT;
                    shadowSprite(&guyData, 1);
                    soundPlaySFX(SOUND_SFX_WOO, 1);
                }

                // Check for collision with tree bases/stumps/bushes/poles
                if (l1Tile) {
                    // YES - It would be better if these tiles were together in the tileset so I could just do (l1Tile >= X & l1Tile <= Y)
                    // but its too much work to refactor the tileset now. If this becomes a bottleneck I'll do it!
                    if (l1Tile == RED_NET || l1Tile == BLUE_NET || l1Tile == GREEN_TREE_BASE || l1Tile == DEAD_TREE_BASE || l1Tile == GREEN_MINI_TREE ||
                        l1Tile == GREEN_MINI_DEAD_TREE || l1Tile == STUMP || l1Tile == POLE || l1Tile == ROCK) {
                    
                        // We will move the player to a safe space to continue
                        // Show ouch msg and restore tiles
                        soundPlaySFX(SOUND_SFX_CRASH, 1);
                        messageCenterSave("OUCH!!!", 7, 15, scrollX, scrollY, zoomMode, save);
                        waitCount(120);
                        restoreRow(save, 7, 15, scrollY, zoomMode);

                        // Back up and find a safe spot for the player to continue
                        getSafeSpot();
                        inSnow = 0;
                        scrollSpeed = 0;

                        // Move the player into positon
                        move(&guyData, scrollX, &scrollSpeed, inSnow, 0);
                        setScroll();
                        totalTicks+= CRASH_PENALTY_TICKS;
                        
                        // Update all the timer segments from the new totalTicks
                        refreshTimerFromTicks(totalTicks, &mins, &secs, &ticks, &milli);
                        showTimer(mins, secs, milli, missed);
                        waitCount(120);
                    }
                }

                // Check if on course or in heavy snow...changes speed
                // YES - It would also be better if these tiles were together....sigh
                if (l0Tile == SNOW || l0Tile == SNOW_WITH_DOTS || l0Tile == SNOW_ANGLED_1 || l0Tile == SNOW_ANGLED_2 || l0Tile == SNOW_ANGLED_3 ||
                    l0Tile == SNOW_ANGLED_4 || l0Tile == RED_ARROW_BIG_1 || l0Tile == RED_ARROW_BIG_2 || l0Tile == BLUE_ARROW_BIG_1 || l0Tile == BLUE_ARROW_BIG_2) {
                    inSnow = 1;
                } else {
                    inSnow = 0;
                }
            } else {
                moveSprite(SPRITE_SHADOW_ADDR, guyData.guyX-scrollX, guyData.guyY+guyData.jumpCount);
            }

            // Check flags
            if (gameMode == GAME_MODE_FLAGS) {
                if (flagNum < flagsCurrent->length && !flagsCurrent->trackingData[flagNum].tracked) {
                    if (flagsCurrent->trackingData[flagNum].data.tile1 != RED_FLAG && flagsCurrent->trackingData[flagNum].data.tile1 != BLUE_FLAG) {
                        // Extra flags that we don't care about in this mode
                        flagsCurrent->trackingData[flagNum].tracked = 1;
                        flagNum++;
                    } else if (lastTileY > (flagsCurrent->trackingData[flagNum].data.row)) {
                        soundPlaySFX(SOUND_SFX_NO, 1);
                        message("MISSED", 3, flagsCurrent->trackingData[flagNum].data.col1, 0, scrollY);
                        flagsCurrent->trackingData[flagNum].tracked = 1;
                        flagNum++;
                        missed++;

                        // Time penalty
                        totalTicks+= MISSED_FLAG_PENALTY_TICKS;
                        // Update all the timer segments from the new totalTicks
                        refreshTimerFromTicks(totalTicks, &mins, &secs, &ticks, &milli);
                    } else if (
                        (flagsCurrent->trackingData[flagNum].data.tile1 == RED_FLAG && lastTileY == (flagsCurrent->trackingData[flagNum].data.row) && lastTileX <= (flagsCurrent->trackingData[flagNum].data.col1)) ||
                        (flagsCurrent->trackingData[flagNum].data.tile1 == BLUE_FLAG && lastTileY == (flagsCurrent->trackingData[flagNum].data.row) && lastTileX >= (flagsCurrent->trackingData[flagNum].data.col1))
                        ) {
                        soundPlaySFX(SOUND_SFX_YES, 1);
                        flagsCurrent->trackingData[flagNum].tracked = 1;
                        flagNum++;
                    }
                }
            } else if (gameMode == GAME_MODE_GATES) {
                if (flagNum < flagsCurrent->length && !flagsCurrent->trackingData[flagNum].tracked) {
                    if (flagsCurrent->trackingData[flagNum].data.tile1 != RED_GATE && flagsCurrent->trackingData[flagNum].data.tile1 != BLUE_GATE) {
                        // Extra flags that we don't care about in this mode
                        flagsCurrent->trackingData[flagNum].tracked = 1;
                        flagNum++;
                    } else if (lastTileY > (flagsCurrent->trackingData[flagNum].data.row)) {
                        soundPlaySFX(SOUND_SFX_NO, 1);
                        message("MISSED", 3, flagsCurrent->trackingData[flagNum].data.col2, 0, scrollY);
                        flagsCurrent->trackingData[flagNum].tracked = 1;
                        flagNum++;
                        missed++;

                        // Time penalty
                        totalTicks+= MISSED_FLAG_PENALTY_TICKS;
                        // Update all the timer segments from the new totalTicks
                        refreshTimerFromTicks(totalTicks, &mins, &secs, &ticks, &milli);
                    } else if (
                        (lastTileY == flagsCurrent->trackingData[flagNum].data.row &&
                         lastTileX >= flagsCurrent->trackingData[flagNum].data.col1 &&
                         lastTileX <= flagsCurrent->trackingData[flagNum].data.col2)
                        ) {
                        soundPlaySFX(SOUND_SFX_YES, 1);
                        flagsCurrent->trackingData[flagNum].tracked = 1;
                        flagNum++;
                    }
                }
            }
            
            move(&guyData, scrollX, &scrollSpeed, inSnow, joy);

            // Crash if off screen
            if (guyData.guyX > 640) {
                soundPlaySFX(SOUND_SFX_CRASH, 1);
                messageCenterSave("STAY ON COURSE!!!", 7, 15, scrollX, scrollY, zoomMode, save);
                waitCount(120);
                restoreRow(save, 7, 15, scrollY, zoomMode);

                // Move to middle, back up, and find safe spot
                guyData.guyX = 320;
                getSafeSpot();
                inSnow = 0;
                scrollSpeed = 0;

                // Move the player into positon
                move(&guyData, scrollX, &scrollSpeed, inSnow, 0);
                setScroll();
                totalTicks+= CRASH_PENALTY_TICKS;
                
                // Update all the timer segments from the new totalTicks
                refreshTimerFromTicks(totalTicks, &mins, &secs, &ticks, &milli);
                showTimer(mins, secs, milli, missed);
                waitCount(120);
            }

            // Only scroll on odd frames at slowest speed
            if (scrollSpeed == 0 && ((ticks & 1) == 0)) {
                scrollY+= 1;
            } else {
                scrollY+= scrollSpeed;
            }
            // Check max scroll value
            if (scrollY > 4095) {
                scrollY = scrollY - 4095;
            }

            // See if finished
            if (course == 15 && scrollY >= 150 && scrollY < 200) {
                finialTimerUpdate(ticks, &milli);
                showTimer(mins, secs, milli, missed);
                if (guyData.guyX<240 || guyData.guyX > 400) {
                    soundPlaySFX(SOUND_SFX_CRASH, 1);
                    messageCenter("OH NO, MISSED!!!", 7, 15, scrollX, scrollY, zoomMode);
                } else {
                    soundPlayMusic(SOUND_MUSIC_FINISH);
                    messageCenter("FINISHED!!!", 7, 15, scrollX, scrollY, zoomMode);
                    madeIt = 1;
                }
                waitCount(300);
                break;
            }

            // See if we hit any breakpoints where we need to load more course data
            if (scrollY >= 1024 && previousScroll < 1024) {
                drawPartialCourse(course, 1, 1, gameMode);
            } else if (scrollY >= 3072 && previousScroll < 3072) {
                course = nextCourse(courseCount, selectedCourse, &courseIndex);
                if (course == 15) {
                    drawPartialCourse(course, 0, 0, gameMode);
                } else {
                    flagsNext = drawPartialCourse(course, 0, 1, gameMode);
                }
                
            } else if (scrollY < 8 && previousScroll > 4064) {
                if (flagsCurrent) {
                    free(flagsCurrent);
                }
                flagsCurrent = flagsNext;
                flagsNext = 0;
                flagNum = 0;
            }

            previousScroll = scrollY;

            scrollX = guyData.guyX - halfScrollLimit;

            if (scrollX < 0) {
                scrollX = 0;
            } else if (scrollX > scrollLimit) {
                scrollX = scrollLimit;
            }
            
            setScroll();

            // Timer
            ticks++;
            totalTicks++;

            if (ticks == 60) {
                ticks = 0;
                secs++;
                if (secs == 60) {
                    secs = 0;
                    mins++;
                }
            }

            // Only update the timer text every 1/4 second
            if (ticks == 15) {
                milli = 25;
                showTimer(mins, secs, milli, missed);
            } else if (ticks == 30) {
                milli = 50;
                showTimer(mins, secs, milli, missed);
            } else if (ticks == 45) {
                milli = 75;
                showTimer(mins, secs, milli, missed);
            } else if (ticks == 0) {
                milli = 0;
                showTimer(mins, secs, milli, missed);
            }

            
            wait();
        }

        if (flagsCurrent) {
            free(flagsCurrent);
            flagsCurrent = 0;
        }

        if (flagsNext) {
            free(flagsNext);
            flagsNext = 0;
        }

        // Show high scores
        // TODO: Add name to list if score is better
        // Reset scrolling
        scrollY = 0;
        scrollX = 0;
        setScroll();
        clearLayers();
        spritesConfig(&guyData, 0, 0); // hide sprites
        // Set the zoom level
        setZoom(1);
        soundPlayMusic(SOUND_MUSIC_TITLE);
        displayScores(gameMode, courseCount, selectedCourse, scrollX, scrollY, madeIt ? totalTicks : 65535U);
    }
}