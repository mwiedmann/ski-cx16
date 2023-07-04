#include <cx16.h>
#include <joystick.h>

#include "config.h"
#include "utils.h"

void init() {
    unsigned char *tileFilename = "tilemap.bin";
    unsigned char *palFilename = "tilemap.pal";

    // Configure the joysticks
    joy_install(cx16_std_joy);

    // Enable both layers
    VERA.display.video = 0b11110001;

    // With 16 pixel tiles, we don't need as many tiles (might need more later for scrolling)
    // Only 640/16 = 40, 480/16=30 (40x30 tile resolution now)
    // Set the Map Height=0 (32), Width=1 (64) // NOW 256x64
    // Set Color Depth to 8 bpp mode
    VERA.layer1.config = 0b11010011;
    VERA.layer0.config = 0b11010011;

    // Get bytes 16-9 of the MapBase addresses and set on both layers
    VERA.layer0.mapbase = L0_MAPBASE_ADDR>>9;
    VERA.layer1.mapbase = L1_MAPBASE_ADDR>>9;

    // Layers can share a tilebase (use the same tiles)
    // Get bytes 16-11 of the new TileBase address
    // ALSO Set Tile W/H (bits 0/1) to 1 for 16 pixel tiles
    VERA.layer0.tilebase = TILEBASE_ADDR>>9 | 0b11;
    VERA.layer1.tilebase = TILEBASE_ADDR>>9 | 0b11;

    loadFileToVRAM(palFilename, PALETTE_ADDR);
    loadFileToVRAM(tileFilename, TILEBASE_ADDR);
}

void setZoom(unsigned char zoomMode) {
    VERA.display.hscale = zoomMode == 0 ? 64 : 128;
    VERA.display.vscale = zoomMode == 0 ? 64 : 128;
}

void clearLayer0() {
    // Note we need a `short` here because there are more than 255 tiles
    unsigned short i;
    
    // Clear layer 0
    VERA.address = L0_MAPBASE_ADDR;
    VERA.address_hi = L0_MAPBASE_ADDR>>16;
    // Always set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Empty tiles
    for (i=0; i<MAPBASE_TILE_COUNT; i++) {
        VERA.data0 = 0;
        VERA.data0 = 0;
    }
}

void clearLayer1() {
    // Note we need a `short` here because there are more than 255 tiles
    unsigned short i;

    // Clear layer 1
    VERA.address = L1_MAPBASE_ADDR;
    VERA.address_hi = L1_MAPBASE_ADDR>>16;
    // Always set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Empty tiles
    for (i=0; i<MAPBASE_TILE_COUNT; i++) {
        VERA.data0 = 0;
        VERA.data0 = 0;
    }
}

void clearLayers() {
    clearLayer0();
    clearLayer1();
}