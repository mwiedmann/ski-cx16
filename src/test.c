#include <cx16.h>
#include <stdio.h>

unsigned char keyDown[128]; //0 if not pressed, 1 if pressed

unsigned char kscancodes[12] = {0x12, 0x1f, 0x20, 0x21, 0x25, 0x18, 0x27, 0x26, 0x11, 0x13, 0x23, 0x24}; //wasd ijkl qe gh
unsigned char alt_kscancodes[12] = {0x53, 0x4f, 0x54, 0x59, 0x25, 0x18, 0x27, 0x26, 0x17, 0x19, 0x23, 0x24}; //cursors ijkl uo gh

void keyhandler() {
	asm volatile ("tax");
	asm volatile ("and #$7f");
	asm volatile ("tay");
	asm volatile ("txa");
	asm volatile ("and #$80");
	asm volatile ("sta %v, y", keyDown);
	asm volatile ("lda #$00");
	return;
}

void inputInit() {
	unsigned char i=0;
	for (i=0;i<128;i++) {
		keyDown[i] = 1;
	}

	*((void(**)(void)) 0x32e) = keyhandler;
}

unsigned char inputIsKeyDown(unsigned char keycode) {
	return !keyDown[keycode];
}

void main() {
    inputInit();

    while (1) {
        if (input_isKeyDown(0x12)) {
            printf("W");
        }
    }
}