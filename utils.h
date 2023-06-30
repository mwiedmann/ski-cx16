#ifndef UTILS_H
#define UTILS_H

// A pointer to Memory location 0
// which holds the number of the current RAM Bank
#define BANK_NUM (*(unsigned char *)0x00)

void loadFileToVRAM(unsigned char *filename, unsigned long addr);
void loadFileToBankedRAM(unsigned char *filename, unsigned char bank);
void message(unsigned char* msg, unsigned char row, unsigned char col);
void messageCenter(unsigned char* msg, unsigned char row);

#endif