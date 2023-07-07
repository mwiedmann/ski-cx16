#ifndef SCORES_H
#define SCORES_H

typedef struct Score {
    unsigned short ticks;
    unsigned char name[4];
} Score;

typedef struct ScoreList {
    unsigned char length;
    Score *scores;
} ScoreList;

void displayScores(unsigned char zoomMode, unsigned char gameMode, unsigned short scrollX, unsigned short scrollY, unsigned short newTicks);

#endif