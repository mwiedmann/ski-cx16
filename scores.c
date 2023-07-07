#include <cbm.h>
#include <stdio.h>
#include <string.h>
#include <joystick.h>

#include "utils.h"
#include "scores.h"
#include "config.h"

void displayScores(unsigned char zoomMode, unsigned char gameMode, unsigned short scrollX, unsigned short scrollY, unsigned short newTicks) {
    unsigned short mins, secs, milli, ticks, mem;
    signed char b, i;
    unsigned char buf[24];
    unsigned char scoreRow = 99;
    ScoreList scoreList;

    BANK_NUM = SCORE_BANK;

    sprintf(buf, "score%u.bin", gameMode);

    cbm_k_setnam(buf);
    cbm_k_setlfs(0, 8, 0); // has 2 byte header because cbm_k_save will add it

    // Reminder, first param of cbm_k_load of "0" means load into system memory.
    mem = cbm_k_load(0, (unsigned short)BANK_RAM);
    scoreList.length = (mem - ((unsigned short)BANK_RAM)) / sizeof(Score);
    scoreList.scores = (Score*)BANK_RAM;

    messageCenter("HIGH SCORES", 1, 7, scrollX, scrollY, zoomMode);

    for (i=0; i<10; i++) {
        ticks = scoreList.scores[i].ticks;

        if (scoreRow == 99 && newTicks < ticks) {
            scoreRow = i;
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

    if (scoreRow != 99) {
        messageCenter("ENTER YOUR NAME", 14, 20, scrollX, scrollY, zoomMode); 
    }

    waitForButtonPress();
}