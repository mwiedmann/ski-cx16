#ifndef UTILS_H
#define UTILS_H

// A pointer to Memory location 0
// which holds the number of the current RAM Bank
#define BANK_NUM (*(unsigned char *)0x00)

// Pointers to mem 2/4/6 which are "virtual registers"
// for Kernal Functions, used to pass/get data
#define R0 (*(unsigned short *)0x02)
#define R1 (*(unsigned short *)0x04)
#define R2 (*(unsigned short *)0x06)

void loadFileToVRAM(unsigned char *filename, unsigned long addr);
void loadFileToBankedRAM(unsigned char *filename, unsigned char bank, unsigned short addr);
void copyBankedRAMToVRAM(unsigned char startMemBank, unsigned long vramAddr, unsigned long length);
void message(unsigned char* msg, unsigned char row, unsigned char col, unsigned short scrollX, unsigned short scrollY);
void messageCenter(unsigned char* msg, unsigned char row0, unsigned char row1, unsigned short scrollX, unsigned short scrollY, unsigned short zoomMode);

#endif