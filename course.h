#ifndef COURSE_H
#define COURSE_H

#define COURSE_COUNT 2

#define FLAG_TYPE 1

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