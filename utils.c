#include <cbm.h>
#include <string.h>

#include "utils.h"
#include "config.h"

void loadFileToVRAM(unsigned char *filename, unsigned long addr) {
    // These 3 functions are basic wrappers for the Kernal Functions

    // You have to first set the name of the file you are working with.
    cbm_k_setnam(filename);

    // Next you setup the LFS (Logical File) for the file
    // First param is the Logical File Number
    //   Use 0 if you are just loading the file
    //   You can use other values to keep multiple files open
    // Second param is the device number
    //   The SD Card on the CX16 is 8
    // The last param is the Secondary Address
    // 0 - File has the 2 byte header, but skip it
    // 1 - File has the 2 byte header, use it
    // 2 - File does NOT have the 2 byte header
    cbm_k_setlfs(0, 8, 2);

    // Finally, load the file somewhere into RAM or VRAM
    // First param of cbm_k_load means:
    //   0, loads into system memory.
    //   1, perform a verify.
    //   2, loads into VRAM, starting from 0x00000 + the specified starting address.
    //   3, loads into VRAM, starting from 0x10000 + the specified starting address.
    // Second param is the 16 bit address 
    cbm_k_load(addr > 0xFFFFL ? 3 : 2, addr);
}

void loadFileToBankedRAM(unsigned char *filename, unsigned char bank, unsigned short addr) {
    // Set the RAM Bank we are loading into
    BANK_NUM = bank;

    // See the chapter on Files for info on these cbm file functions
    cbm_k_setnam(filename);
    cbm_k_setlfs(0, 8, 2);

    // Reminder, first param of cbm_k_load of "0" means load into system memory.
    // WE WANT THIS ONE because Banked RAM is system memory (at address 0xA000)

    // BANK_RAM is a #define provided by cx16.h
    // as a pointer to 0xA000 (which is where Bank RAM starts)
    cbm_k_load(0, (unsigned short)BANK_RAM + addr);
}

void copyBankedRAMToVRAM(unsigned char startMemBank, unsigned long vramAddr, unsigned long length) {
    unsigned long remaining;
    unsigned short i;

    VERA.address = vramAddr;
    VERA.address_hi = vramAddr>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // This crazy routine uses the kernal memory_copy function to bulk copy RAM to VRAM
    // I had to increment the bank and do it in chunks though.
    for (i=0; i<length/8192+1; i++) {
        // Set the RAM Bank we are reading from
        BANK_NUM = startMemBank+i;

         // Virtual Registers for the Kernal Function
        R0 = (unsigned short)BANK_RAM; // Read from the Banked RAM starting Address 0xA000
        R1 = 0x9F23; // Write to the VERA.data0 memory location

        remaining = length - (8192*i);
        if (remaining < 8192) {
            R2 = remaining;
        } else {
            R2 = 8192;
        }

        // Call the memory_copy Kernal Function
        __asm__("jsr $FEE7");
    }
}

void message(unsigned char* msg, unsigned char row, unsigned char col, unsigned short scrollX, unsigned short scrollY) {
    unsigned short i, tile;
    unsigned long addr = L1_MAPBASE_ADDR + (((scrollY>>4) + row) * MAPBASE_TILE_WIDTH * 2) + (((scrollX>>4) + col) * 2);

    // Draw the tileset to layer 0
    VERA.address = addr;
    VERA.address_hi = addr>>16;
    // Always set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    for (i=0; i<MAPBASE_TILE_WIDTH; i++) {
        if (msg[i] == 0) {
            break;
        }

        // "0123456789:ABCXYZ!#$%+-=."
        // We could reorganize the tile image to have the characters in PETSCII order
        // but this is fine for now
        tile =  msg[i] >= '0' && msg[i]<= ':'
            ? TILE_CHARS_START + (msg[i]-'0') // Numbers and :
            : msg[i] >= 'A' && msg[i]<= 'Z'
                ? TILE_CHARS_START + 12 + (msg[i]-'A') // Letters
                : msg[i] == '!'
                    ? TILE_CHARS_START + 38
                    : msg[i] == '#'
                        ? TILE_CHARS_START + 39
                        : msg[i] == '$'
                            ? TILE_CHARS_START + 40
                            : msg[i] == '%'
                                ? TILE_CHARS_START + 41
                                : msg[i] == '+'
                                    ? TILE_CHARS_START + 42
                                    : msg[i] == '-'
                                        ? TILE_CHARS_START + 43
                                        : msg[i] == '='
                                            ? TILE_CHARS_START + 46
                                            : msg[i] == '.'
                                                ? TILE_CHARS_START + 47
                                                : 0;

        VERA.data0 = tile;
        VERA.data0 = 0;
    }
}

void messageCenter(unsigned char* msg, unsigned char row0, unsigned char row1, unsigned short scrollX, unsigned short scrollY, unsigned short zoomMode) {
    unsigned char len, col;
    unsigned char offset = zoomMode == 1 ? 41 : 21;
    len = strlen(msg);

    col = (offset - len)>>1;

    message(msg, zoomMode == 0 ? row0 : row1, col, scrollX, scrollY);
}
