#ifndef SPRITES_H
#define SPRITES_H

#define SPRITE1_ADDR 0x1FC08
#define SPRITE_SHADOW_ADDR 0x1FC10
#define SPRITE_TEXT_ADDR 0x1FC18
#define GUY_MOVE_X_MAX 48
#define GUY_MOVE_X_SNOW_MAX 32

#define SPRITE_TEXT_X_320 128
#define SPRITE_TEXT_X_640 448
#define SPRITE_TEXT_Y 0
#define SPRITE_TEXT_COUNT 12

typedef struct GuyData {
    unsigned short guyX;
    unsigned short guyY;
    short guyMoveX;
    short guyMoveY;
    unsigned char jumping;
    unsigned short jumpCount;
} GuyData;

void moveSprite(unsigned long spriteAddr, unsigned short x, unsigned short y);
void shadowSprite(GuyData *guyData, unsigned char show);
void spritesConfig(GuyData *guyData, unsigned char zoomMode, unsigned char show);
void spriteText(unsigned char* msg, unsigned char row);
void move(GuyData *guyData, short scrollX, unsigned short *scrollSpeed, unsigned char inSnow);

#endif