#ifndef SCORES_H
#define SCORES_H

typedef struct Score {
    unsigned short ticks;
    char name[4];
} Score;

typedef struct ScoreList {
    unsigned char length;
    Score *scores;
} ScoreList;

void displayScores(unsigned char gameMode, unsigned char courseCount, unsigned char startingCourse,
    unsigned short scrollX, unsigned short scrollY, unsigned short newTicks);

#endif