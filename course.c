#include <cx16.h>

#include "course.h"
#include "config.h"
#include "utils.h"

void drawCourse() {
    unsigned char len, i;
    unsigned long addr;
    FlagData *flagData;

    loadFileToVRAM("c0l0.bin", L0_MAPBASE_ADDR);
    loadFileToVRAM("c0l1.bin", L1_MAPBASE_ADDR);

    loadFileToBankedRAM("c0flags.bin", FLAGS_BANK);

    len = (*(unsigned char*)BANK_RAM);
    flagData = (FlagData *)(BANK_RAM+1);

    for (i=0; i<len; i++) {
        addr = L1_MAPBASE_ADDR + (flagData[i].row * MAPBASE_TILE_WIDTH * 2) + (flagData[i].col * 2);
        VERA.address =  addr;
        VERA.address_hi = L1_MAPBASE_ADDR>>16;
        // Set the Increment Mode, turn on bit 4
        VERA.address_hi |= 0b10000;

        VERA.data0 = flagData[i].tile;
        VERA.data0 = 0;
    }
}