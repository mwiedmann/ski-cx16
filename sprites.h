#ifndef SPRITES_H
#define SPRITES_H

#define SPRITE1_ADDR 0x1FC08
#define GUY_MOVE_X_MAX 64
#define GUY_MOVE_X_SNOW_MAX 32

typedef struct GuyData {
    unsigned short guyX;
    unsigned short guyY;
    short guyMoveX;
    short guyMoveY;
} GuyData;

void spritesConfig(GuyData *guyData);
void move(GuyData *guyData, unsigned short *scrollSpeed, unsigned char inSnow);

#endif