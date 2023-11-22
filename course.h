#ifndef COURSE_H
#define COURSE_H

#define COURSE_COUNT 2

#define FLAG_TYPE 1

// Solid tiles (ends the run)
#define RED_NET 11
#define BLUE_NET 12
#define GREEN_TREE_BASE 19
#define DEAD_TREE_BASE 20
#define GREEN_MINI_TREE 31
#define GREEN_MINI_DEAD_TREE 32
#define STUMP 48
#define POLE 67
#define ROCK 82

// Heavy snow tiles
#define SNOW 1
#define SNOW_WITH_DOTS 6
#define SNOW_ANGLED_1 14
#define SNOW_ANGLED_2 17
#define SNOW_ANGLED_3 50
#define SNOW_ANGLED_4 53
// The "arrow" signs won't crash the player but do slow them like heavy snow
// so also count as "inSnow"
#define RED_ARROW_BIG_1 23
#define RED_ARROW_BIG_2 24
#define BLUE_ARROW_BIG_1 35
#define BLUE_ARROW_BIG_2 36

#define JUMP_1 63
#define JUMP_2 64

typedef struct FlagData {
    unsigned char row;
    unsigned char tile1;
    unsigned char col1;
    unsigned char tile2;
    unsigned char col2;
} FlagData;

typedef struct FlagTrackingData {
    unsigned char tracked;
    FlagData data;
} FlagTrackingData;

typedef struct FlagTrackingList {
    unsigned char length;
    unsigned char type;
    FlagTrackingData trackingData[];
} FlagTrackingList;

void loadCourses();
FlagTrackingList* drawPartialCourse(unsigned char course, unsigned char half, unsigned char drawFlags, unsigned char gameMode);

#endif