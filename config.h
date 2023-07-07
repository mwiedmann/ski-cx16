#ifndef CONFIG_H
#define CONFIG_H

#define MAPBASE_TILE_WIDTH 64
#define MAPBASE_TILE_HEIGHT 256
#define MAPBASE_TILE_COUNT MAPBASE_TILE_WIDTH * MAPBASE_TILE_HEIGHT
#define MAPBASE_SIZE MAPBASE_TILE_COUNT*2

// Our default Tile and Map Base addresses
#define L0_MAPBASE_ADDR 0x00000L
#define L1_MAPBASE_ADDR L0_MAPBASE_ADDR + MAPBASE_SIZE 
#define TILEBASE_ADDR L1_MAPBASE_ADDR + MAPBASE_SIZE

#define TILE_CHARS_START 97
#define TILESET_SIZE 37120L // (12w * 12h * 16x16pixels) + 1 empty 16x16 tile
#define TILESET_DIGITS_ADDR TILEBASE_ADDR + (TILE_CHARS_START * 256)

#define PALETTE_ADDR 0x1FA00L

#define COURSE_LAYER_BANK_SIZE 4
#define COURSE_LAYER_HALF_BANK_SIZE 2

#define FLAG_BANK_SIZE 1024

#define FLAGS_BANK 1
#define C0_L0_BANK 2
#define C0_L1_BANK C0_L0_BANK + COURSE_LAYER_BANK_SIZE
#define C1_L0_BANK C0_L1_BANK + COURSE_LAYER_BANK_SIZE
#define C1_L1_BANK C1_L0_BANK + COURSE_LAYER_BANK_SIZE

#define CFINISH_L0_BANK 32 + COURSE_LAYER_BANK_SIZE
#define CFINISH_L1_BANK CFINISH_L0_BANK + COURSE_LAYER_BANK_SIZE

#define SCORE_BANK 50

#define GAME_MODE_FREE 0
#define GAME_MODE_FLAGS 1
#define GAME_MODE_GATES 2

void init();
void setZoom(unsigned char zoomMode);
void clearLayers();
void clearLayer0();
void clearLayer1();
void pickModes(unsigned char *zoomMode, unsigned char *gameMode);
void waitForButtonPress();

#endif