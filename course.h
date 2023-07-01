#ifndef COURSE_H
#define COURSE_H

void loadCourses();
void drawPartialCourse(unsigned char course, unsigned char half);

#define COURSE_COUNT 2

typedef struct FlagData {
    unsigned char tile;
    unsigned char col;
    unsigned char row;
} FlagData;

#endif