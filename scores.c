#include <cbm.h>
#include <stdio.h>
#include <string.h>
#include <joystick.h>

#include "utils.h"
#include "scores.h"
#include "config.h"
#include "wait.h"

#define KEY_DELAY 10

void displayScores(unsigned char zoomMode, unsigned char gameMode, unsigned char courseCount, unsigned short scrollX, unsigned short scrollY, unsigned short newTicks) {
    unsigned short mins, secs, milli, ticks, mem;
    signed char b, i;
    unsigned char buf[24];
    unsigned char scoreRow = 99;
    unsigned char nameDone = 0, joy, nameCount = 0, nameChar = 'A';
    ScoreList scoreList;

    BANK_NUM = SCORE_BANK;

    sprintf(buf, "score%u%u.bin", gameMode, courseCount);

    cbm_k_setnam(buf);
    cbm_k_setlfs(0, 8, 0); // has 2 byte header because cbm_k_save will add it

    // Reminder, first param of cbm_k_load of "0" means load into system memory.
    mem = cbm_k_load(0, (unsigned short)BANK_RAM);
    scoreList.length = (mem - ((unsigned short)BANK_RAM)) / sizeof(Score);
    scoreList.scores = (Score*)BANK_RAM;

    sprintf(buf, "%s-%s",
        courseCount == 1
            ? "SHORT"
            : courseCount == 2
                ? "MEDIUM"
                : courseCount == 3
                    ? "LONG" : "EPIC",
        gameMode == GAME_MODE_FREE ? "FREE" : "FLAGS");

    messageCenter(buf, 0, 6, scrollX, scrollY, zoomMode);
    messageCenter("HIGH SCORES", 1, 7, scrollX, scrollY, zoomMode);

    while (!nameDone) {
        for (i=0; i<10; i++) {
            ticks = scoreList.scores[i].ticks;

            if (scoreRow == 99 && newTicks < ticks) {
                scoreRow = i;
                nameCount = 0;
                // Bump everything down
                for (b=8; b>=i; b--) {
                    strcpy(scoreList.scores[b+1].name, scoreList.scores[b].name);
                    scoreList.scores[b+1].ticks = scoreList.scores[b].ticks;
                }
                strcpy(scoreList.scores[i].name, "---");
                scoreList.scores[i].ticks = newTicks;
                ticks = scoreList.scores[i].ticks;
            }
            
            mins = ticks / 3600;
            ticks-= mins * 3600;

            secs = ticks / 60;
            ticks -= secs * 60;

            milli = (ticks*100)/60;

            sprintf(buf, "%u:%02u.%02u %s", mins, secs, milli, scoreList.scores[i].name);

            messageCenter(buf, i+3, i+9, scrollX, scrollY, zoomMode);    
        }

        if (scoreRow != 99 && nameCount < 3) {
            messageCenter("ENTER YOUR NAME", 13, 20, scrollX, scrollY, zoomMode);
            messageCenter("WITH JOYSTICK", 14, 21, scrollX, scrollY, zoomMode);

            joy = joy_read(0);

            if (JOY_DOWN(joy)) {
                nameChar++;
                if (nameChar > 'Z') {
                    nameChar = 'A';
                }
                waitCount(KEY_DELAY);
            } else if (JOY_UP(joy)) {
                nameChar--;
                if (nameChar < 'A') {
                    nameChar = 'Z';
                }
                waitCount(KEY_DELAY);
            } else if (JOY_LEFT(joy) && nameCount > 0) {
                while(JOY_LEFT(joy)) {
                    wait();
                    joy = joy_read(0);
                }
                scoreList.scores[scoreRow].name[nameCount] = '-';
                nameCount--;
                nameChar = scoreList.scores[scoreRow].name[nameCount];
            } else if (JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
                while(JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
                    wait();
                    joy = joy_read(0);
                }
                nameCount++;
            }

            if (nameCount < 3) {
                scoreList.scores[scoreRow].name[nameCount] = nameChar;
            }
            wait();
        } else {
            nameDone = 1;
        }
    }

    // A high score was entered
    // Save the new list
    if (scoreRow != 99) {
        // NOTE: the "@:" prefix allows us to overwrite a file
        sprintf(buf, "@:score%u%u.bin", gameMode, courseCount);
        cbm_k_setnam(buf);
        // SAVE adds the 2 byte header and we can't stop it
        cbm_k_setlfs(0, 8, 0);

        scoreList.length = (mem - ((unsigned short)BANK_RAM)) / sizeof(Score);
        scoreList.scores = (Score*)BANK_RAM;

        cbm_k_save((unsigned short)scoreList.scores, ((unsigned short)scoreList.scores) + (sizeof(Score) * scoreList.length));
    }

    messageCenter(" PRESS A BUTTON ", 13, 20, scrollX, scrollY, zoomMode);
    messageCenter(" TO CONTINUE ", 14, 21, scrollX, scrollY, zoomMode);

    waitForButtonPress();
}