#ifndef SPRITES_H
#define SPRITES_H

#define SPRITE1_ADDR 0x1FC08
#define SPRITE_TEXT_ADDR 0x1FC10
#define GUY_MOVE_X_MAX 64
#define GUY_MOVE_X_SNOW_MAX 32

#define SPRITE_TEXT_X 432
#define SPRITE_TEXT_Y 4
#define SPRITE_TEXT_COUNT 12

typedef struct GuyData {
    unsigned short guyX;
    unsigned short guyY;
    short guyMoveX;
    short guyMoveY;
} GuyData;

void spritesConfig(GuyData *guyData);
void spriteText(unsigned char* msg, unsigned char row);
void move(GuyData *guyData, unsigned short *scrollSpeed, unsigned char inSnow);

#endif