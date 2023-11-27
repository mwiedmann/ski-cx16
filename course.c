#include <cx16.h>
#include <stdlib.h>

#include "course.h"
#include "config.h"
#include "utils.h"

void loadCourses() {
    loadFileToBankedRAM("c0l0.bin", C0_L0_BANK, 0);
    loadFileToBankedRAM("c0l1.bin", C0_L1_BANK, 0);
    loadFileToBankedRAM("c0flags.bin", FLAGS_BANK, 0);
    loadFileToBankedRAM("c0gates.bin", GATES_BANK, 0);

    loadFileToBankedRAM("c1l0.bin", C1_L0_BANK, 0);
    loadFileToBankedRAM("c1l1.bin", C1_L1_BANK, 0);
    loadFileToBankedRAM("c1flags.bin", FLAGS_BANK, FLAG_BANK_SIZE);
    loadFileToBankedRAM("c1gates.bin", GATES_BANK, FLAG_BANK_SIZE);

    loadFileToBankedRAM("c2l0.bin", C2_L0_BANK, 0);
    loadFileToBankedRAM("c2l1.bin", C2_L1_BANK, 0);
    loadFileToBankedRAM("c2flags.bin", FLAGS_BANK, FLAG_BANK_SIZE*2);
    loadFileToBankedRAM("c2gates.bin", GATES_BANK, FLAG_BANK_SIZE*2);

    loadFileToBankedRAM("c15l0.bin", CFINISH_L0_BANK, 0);
    loadFileToBankedRAM("c15l1.bin", CFINISH_L1_BANK, 0);
}

FlagTrackingList * drawCourseFlags(unsigned char course, unsigned char half, unsigned char gameMode) {
    unsigned char len, i;
    unsigned long addr;
    unsigned short bankAddr;
    FlagData *flagData;
    FlagTrackingList *flagTrackingList;

    BANK_NUM = gameMode == GAME_MODE_FLAGS ? FLAGS_BANK : GATES_BANK;

    bankAddr = ((unsigned short)BANK_RAM) + (course * FLAG_BANK_SIZE);
    len = (*(unsigned char*)(bankAddr));

    flagTrackingList = malloc(2 + (sizeof(FlagTrackingData) * len));
    flagTrackingList->length = len;
    flagTrackingList->type = FLAG_TYPE;

    flagData = (FlagData *)(bankAddr+1);

    for (i=0; i<len; i++) {
        // Copy the flag data
        flagTrackingList->trackingData[i].tracked = 0;
        flagTrackingList->trackingData[i].data.row = flagData[i].row;
        flagTrackingList->trackingData[i].data.tile1 = flagData[i].tile1;
        flagTrackingList->trackingData[i].data.col1 = flagData[i].col1;
        flagTrackingList->trackingData[i].data.tile2 = flagData[i].tile2;
        flagTrackingList->trackingData[i].data.col2 = flagData[i].col2;

        // Skip the flags for the other half
        if ((half == 0 && flagData[i].row > 127) || (half == 1 && flagData[i].row <= 127)) {
            continue;
        }

        // TODO: Only supports type=1 for now
        addr = L1_MAPBASE_ADDR + (flagData[i].row * MAPBASE_TILE_WIDTH * 2) + (flagData[i].col1 * 2);
        VERA.address =  addr;
        VERA.address_hi = L1_MAPBASE_ADDR>>16;
        // Set the Increment Mode, turn on bit 4
        VERA.address_hi |= 0b10000;

        VERA.data0 = flagData[i].tile1;
        VERA.data0 = 0;

        // Display both tiles if this is a gate
        if (gameMode == GAME_MODE_GATES) {
            addr = L1_MAPBASE_ADDR + (flagData[i].row * MAPBASE_TILE_WIDTH * 2) + (flagData[i].col2 * 2);
            VERA.address =  addr;
            VERA.address_hi = L1_MAPBASE_ADDR>>16;
            // Set the Increment Mode, turn on bit 4
            VERA.address_hi |= 0b10000;

            VERA.data0 = flagData[i].tile2;
            VERA.data0 = 0;
        }
    }

    return flagTrackingList;
}

FlagTrackingList * drawPartialCourse(unsigned char course, unsigned char half, unsigned char drawFlags, unsigned char gameMode) {
    unsigned char l0bank, l1bank;
    FlagTrackingList *flagTrackingList = 0;

    switch(course) {
        case 0 :
            l0bank = C0_L0_BANK;
            l1bank = C0_L1_BANK;
            break;
        case 1 :
            l0bank = C1_L0_BANK;
            l1bank = C1_L1_BANK;
            break;

        case 2 :
            l0bank = C2_L0_BANK;
            l1bank = C2_L1_BANK;
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

    if (drawFlags && gameMode != GAME_MODE_FREE) {
        flagTrackingList = drawCourseFlags(course, half, gameMode);
    }

    // Don't need flag data for the 2nd half since we already have it
    if (half && flagTrackingList) {
        free(flagTrackingList);
        flagTrackingList = 0;
    }

    return flagTrackingList;
}