#include <cx16.h>

#include "course.h"
#include "config.h"
#include "utils.h"

void loadCourses() {
    loadFileToBankedRAM("c0l0.bin", C0_L0_BANK, 0);
    loadFileToBankedRAM("c0l1.bin", C0_L1_BANK, 0);
    loadFileToBankedRAM("c0flags.bin", FLAGS_BANK, 0);

    loadFileToBankedRAM("c1l0.bin", C1_L0_BANK, 0);
    loadFileToBankedRAM("c1l1.bin", C1_L1_BANK, 0);
    loadFileToBankedRAM("c1flags.bin", FLAGS_BANK, FLAG_BANK_SIZE);

    loadFileToBankedRAM("c15l0.bin", CFINISH_L0_BANK, 0);
    loadFileToBankedRAM("c15l1.bin", CFINISH_L1_BANK, 0);
}

void drawCourseFlags(unsigned char course, unsigned char half) {
    unsigned char len, i;
    unsigned long addr;
    unsigned short bankAddr;
    FlagData *flagData;

    BANK_NUM = FLAGS_BANK;

    bankAddr = ((unsigned short)BANK_RAM) + (course * FLAG_BANK_SIZE);
    len = (*(unsigned char*)(bankAddr));
    flagData = (FlagData *)(bankAddr+1);

    for (i=0; i<len; i++) {
        // Skip the flags for the other half
        if ((half == 0 && flagData[i].row > 127) || (half == 1 && flagData[i].row <= 127)) {
            continue;
        }
        addr = L1_MAPBASE_ADDR + (flagData[i].row * MAPBASE_TILE_WIDTH * 2) + (flagData[i].col * 2);
        VERA.address =  addr;
        VERA.address_hi = L1_MAPBASE_ADDR>>16;
        // Set the Increment Mode, turn on bit 4
        VERA.address_hi |= 0b10000;

        VERA.data0 = flagData[i].tile;
        VERA.data0 = 0;
    }
}

void drawPartialCourse(unsigned char course, unsigned char half, unsigned char drawFlags) {
    unsigned char l0bank, l1bank;

    switch(course) {
        case 0 :
            l0bank = C0_L0_BANK;
            l1bank = C0_L1_BANK;
            break;
        case 1 :
            l0bank = C1_L0_BANK;
            l1bank = C1_L1_BANK;
            break;

        case 15 :
            l0bank = CFINISH_L0_BANK;
            l1bank = CFINISH_L1_BANK;
            break;
    }

    l0bank+= half * COURSE_LAYER_HALF_BANK_SIZE;
    l1bank+= half * COURSE_LAYER_HALF_BANK_SIZE;

    copyBankedRAMToVRAM(l0bank, L0_MAPBASE_ADDR + (half*MAPBASE_TILE_COUNT), MAPBASE_TILE_COUNT);
    copyBankedRAMToVRAM(l1bank, L1_MAPBASE_ADDR + (half*MAPBASE_TILE_COUNT), MAPBASE_TILE_COUNT);

    if (drawFlags) {
        drawCourseFlags(course, half);
    }
}